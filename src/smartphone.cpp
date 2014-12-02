/*
 * smartphone.cpp
 * smartphone.h および smartphone.cpp は、変更してはならない！
 * 使い方はsmartphone.hのコメントを参照のこと。
*/
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <gtkmm.h>
#include <regex.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "smartphone.h"
#include "view.h"

Smartphone::Smartphone(int p){
	Glib::RefPtr<Gio::SocketAddress> src_address;

	s.reset();
	w=Gio::Socket::create(Gio::SOCKET_FAMILY_IPV4, Gio::SOCKET_TYPE_STREAM, Gio::SOCKET_PROTOCOL_DEFAULT);
	w->set_blocking(true);
#ifdef USE_SET_OPTION
	w->set_option(IPPROTO_TCP, TCP_NODELAY, 1);
#else
	{
		int on=1;
		setsockopt(w->get_fd(), IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
	}
#endif
	src_address=Gio::InetSocketAddress::create (Gio::InetAddress::create_any (Gio::SOCKET_FAMILY_IPV4), p);
	w->bind(src_address, true);
	w->listen();
#ifdef USE_SOCKETSOURCE
	ws=Gio::SocketSource::create(w, Glib::IO_IN);
#else
	ws=Glib::IOSource::create(w->get_fd(), Glib::IO_IN);
#endif
	ws->connect(sigc::mem_fun(*this, &Smartphone::onAccept));
	ws->attach(Glib::MainContext::get_default());
}

void Smartphone::sendMessage(char *msg){
	int n;
	char header[2];
	if(!s || !s->is_connected())return;
	n=strlen(msg)+1;
	header[0]=0x81;
	header[1]=n;
	s->send(header, 2);
	s->send(msg, n);
}

void Smartphone::sendImage(const char *filename){
	Glib::RefPtr<Gdk::Pixbuf> pixbuf;
	char header[10];
	gsize length;
	if(!s || !s->is_connected())return;
	try{
		pixbuf=Gdk::Pixbuf::create_from_file(filename, width, height, false);
	}catch(Glib::FileError &e){
		return;
	}catch(Gdk::PixbufError &e){
		return;
	}
	char *cp;
	std::vector<Glib::ustring> option_keys, option_values;
	option_keys.push_back(Glib::ustring("quality"));
	option_values.push_back(Glib::ustring("80"));
	pixbuf->save_to_buffer(cp, length, Glib::ustring("jpeg"), option_keys, option_values);

	if(length<126){
		header[0]=0x82;
		header[1]=length;
		s->send(header, 4);
	}else if(length<0xffff){
		header[0]=0x82;
		header[1]=0x7e;
		for(int i=0; i<2; ++i){
			header[3-i]=(length>>(i*8))&0xff;
		}
		s->send(header, 4);
	}else if(length<0x7fffffffffffffff){
		header[0]=0x82;
		header[1]=0x7f;
		for(int i=0; i<8; ++i){
			header[9-i]=(length>>(i*8))&0xff;
		}
		s->send(header, 10);
	}else{
		g_free(cp);
		return;
	}
	s->send(cp, length);
	g_free(cp);
}

bool Smartphone::isConnect(){
	if(s && s->is_connected()){
		return true;
	}else{
		return false;
	}
}

bool Smartphone::onAccept(Glib::IOCondition condition){
	regex_t preg;
	regmatch_t pmatch[5];
	gchar buff[4096], *key;
	size_t nmatch=5;

	s = w->accept();
	s->set_blocking (true);
#ifdef USE_SET_OPTION
	s->set_option(IPPROTO_TCP, TCP_NODELAY, 1);
#else
	{
		int on=1;
		setsockopt(s->get_fd(), IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
	}
#endif

	s->receive(buff, sizeof buff);
	regcomp(&preg, "^Sec-WebSocket-Key: .*==", REG_NEWLINE);
	regexec(&preg, buff, nmatch, pmatch, 0);
	regfree(&preg);
	buff[pmatch[0].rm_eo]=0;
	key=keyReply(&buff[pmatch[0].rm_so+19]);

	Glib::ustring str=Glib::ustring::compose(""
			"HTTP/1.1 101 Switching Protocols\r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Sec-WebSocket-Accept: %1\r\n\r\n", key);
	s->send(str.data(), (unsigned long int)str.length());
	g_free(key);

#ifdef USE_SOCKETSOURCE
	ss=Gio::SocketSource::create(s, Glib::IO_IN);
#else
	ss=Glib::IOSource::create(s->get_fd(), Glib::IO_IN);
#endif
	ss->connect(sigc::mem_fun(*this, &Smartphone::onReceive));
	ss->attach(Glib::MainContext::get_default());

	Glib::RefPtr<Gio::SocketAddress> address;
	address=s->get_remote_address();
	Glib::RefPtr<Gio::InetSocketAddress> isockaddr=
			Glib::RefPtr<Gio::InetSocketAddress>::cast_dynamic(address);
	ipaddr=isockaddr->get_address()->to_string();

	return true;
}

bool Smartphone::onReceive(Glib::IOCondition condition){
	Glib::RefPtr<Gio::SocketAddress> address;
	gchar buff[4096];
	int length, type;
	char opcode;
	int size;
	int masked;
	int offset;

	length=s->receive(buff, sizeof buff);
	if(length<1){
		ss->destroy();
		ss.reset();
		s->close();
		s.reset();
		onClose();
		return false;
	}
	opcode=buff[0]&0xf;
	size=buff[1]&0x7f;
	masked=buff[1]&0x80;
	if(masked){
		offset=6;
		for(int i=0; i<size; ++i){
			buff[i+offset]=buff[i+offset] ^ buff[i%4+2];
		}
	}else{
		offset=2;
	}
	switch(opcode){
	case 0x8: // close
		ss->destroy();
		ss.reset();
		s->close();
		s.reset();
		return 0;
	case 0x1: // text
		recvMessage(buff+offset, size);
		break;
	case 0x2: // binary
		type=(int)(*(float *)&buff[offset]);
		if(type==0){
			width=(int)(*(float *)&buff[offset+sizeof(float)]);
			height=(int)(*(float *)&buff[offset+sizeof(float)*2]);
			onConnect(ipaddr.c_str(), width, height);
		}else{
			recvBinary((float *)&buff[offset], size/sizeof(float));
		}
		break;
		case 0x9: // ping
			buff[0]=(buff[0]&0xf0) + 0x9;
			s->send(buff, length);
			break;
		case 0xa: // pong
			break;
		default:
			break;
	}
	return true;
}

Smartphone::~Smartphone(void){
	if(!s->is_closed()){
		ss->destroy();
		ss.reset();
		s->close();
		s.reset();
		onClose();
	}
	if(!w->is_closed()){
		ws->destroy();
		w->close();
	}
}

char *Smartphone::keyReply(const char *key){
	static char hex[]="0123456789abcdef";
	int l;
	char *cat, *base64, *sha1;

	cat=g_strconcat(key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", NULL);
	sha1=g_compute_checksum_for_string(G_CHECKSUM_SHA1, (const char *)cat, (long int)strlen(cat));
	g_free(cat);

	l=strlen(sha1)/2;
	for(int i=0; i<l; ++i){
		sha1[i]=((strchr(hex, sha1[2*i])-hex)<<4)+
				(strchr(hex, sha1[2*i+1])-hex);
	}
	base64=g_base64_encode((guchar *)sha1, l);
	g_free(sha1);

	return base64;
}
