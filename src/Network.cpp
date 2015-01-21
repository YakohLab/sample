/*
 * Network.cpp
 * TCP/IPソケット通信による基本的なサーバ・クライアント通信機能を実現
 * サーバ・クライアント間の通信方法、基本的なパケット構造を定義
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

#include "Network.h"
#include "manager.h"

Network::Network(void){
	s.socket.reset();
	w.socket.reset();
}

Network::~Network(void){
	for(Clients::iterator i=c.begin(); i!=c.end(); ++i){
		i->source.reset();
		i->socket->close();
		i->socket.reset();
	}
	if(s.socket && !s.socket->is_closed()){
		s.source.reset();
		s.socket->close();
		s.socket.reset();
	}
	if(w.socket && !w.socket->is_closed()){
		w.source.reset();
		w.socket->close();
		w.socket.reset();
	}
}

bool Network::openServer(int p){
	int pp=p;
	Glib::RefPtr<Gio::SocketAddress> src_address;

	s.socket.reset();
	do{
		try{
			w.socket=Gio::Socket::create(Gio::SOCKET_FAMILY_IPV4, Gio::SOCKET_TYPE_STREAM, Gio::SOCKET_PROTOCOL_DEFAULT);
			w.socket->set_blocking(true);
#ifdef USE_SET_OPTION
			w->set_option(IPPROTO_TCP, TCP_NODELAY, 1);
#else
			{
				int on=1;
				setsockopt(w.socket->get_fd(), IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
			}
#endif
			src_address=Gio::InetSocketAddress::create (Gio::InetAddress::create_any (Gio::SOCKET_FAMILY_IPV4), pp);
			w.socket->bind(src_address, true);
		}catch(const Glib::Error &ex){
			w.socket.reset();
			++pp;
		}
	}while(!w.socket);
	if(p!=pp){
		std::cout << "希望のポート番号が使用中のため、ポート番号 " << pp << " でゲームサーバになりました。" << std::endl;
	}
	w.socket->listen();
#ifdef USE_SOCKETSOURCE
	w.source=Gio::SocketSource::create(w, Glib::IO_IN);
#else
	w.source=Glib::IOSource::create(w.socket->get_fd(), Glib::IO_IN);
#endif
	w.source->connect(sigc::mem_fun(*this, &Network::onAccept));
	w.source->attach(Glib::MainContext::get_default());
	return true;
}

void Network::closeServer(void){
	if(w.socket && !w.socket->is_closed()){
		w.source.reset();
		w.socket->close();
		w.socket.reset();
	}
}

bool Network::connect(const char *ip, int port){
	Glib::RefPtr<Gio::SocketAddress> srv_address;

	s.socket=Gio::Socket::create(Gio::SOCKET_FAMILY_IPV4, Gio::SOCKET_TYPE_STREAM, Gio::SOCKET_PROTOCOL_DEFAULT);
	s.socket->set_blocking(true);
#ifdef USE_SET_OPTION
	w->set_option(IPPROTO_TCP, TCP_NODELAY, 1);
#else
	{
		int on=1;
		setsockopt(s.socket->get_fd(), IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
	}
#endif
	Glib::ListHandle<Glib::RefPtr<Gio::InetAddress> > addrs=Gio::Resolver::get_default()->lookup_by_name(Glib::ustring(ip));
	for(Glib::ListHandle<Glib::RefPtr<Gio::InetAddress> >::iterator i=addrs.begin(); i!=addrs.end(); ++i){
		srv_address=Gio::InetSocketAddress::create(*i, port);
		if(srv_address->get_family()!=Gio::SOCKET_FAMILY_IPV4){
			continue;
		}
		try{
			s.socket->connect(srv_address);
		}catch(Glib::Error &ex){
			s.socket->close();
			s.socket.reset();
			std::cerr << "Connect to server: " << ex.what() << std::endl;
			return false;
		}
		if(s.socket->is_connected())break;
	}
#ifdef USE_SOCKETSOURCE
	s.source=Gio::SocketSource::create(s, Glib::IO_IN);
#else
	s.source=Glib::IOSource::create(s.socket->get_fd(), Glib::IO_IN);
#endif
	s.source->connect(sigc::mem_fun(*this, &Network::onReceive));
	s.source->attach(Glib::MainContext::get_default());
	return true;
}

void Network::disconnect(void){
	if(s.socket && !s.socket->is_closed()){
		s.source.reset();
		s.socket->close();
		s.socket.reset();
	}
}

void Network::sendToClient(int fd, void *msg, int len){
	for(Clients::iterator i=c.begin(); i!=c.end(); ++i){
		if(i->socket && i->socket->get_fd()==fd){
			i->socket->send((char *)msg, len);
			return;
		}
	}
}

void Network::sendToServer(void *msg, int len){
	if(s.socket){
		s.socket->send((char *)msg, len);
	}
}

bool Network::onAccept(Glib::IOCondition condition){
	SS tmp;

	tmp.socket=w.socket->accept();
	tmp.socket->set_blocking (true);
#ifdef USE_SET_OPTION
	tmp.socket->set_option(IPPROTO_TCP, TCP_NODELAY, 1);
#else
	{
		int on=1;
		setsockopt(tmp.socket->get_fd(), IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
	}
#endif

#ifdef USE_SOCKETSOURCE
	tmp.source=Gio::SocketSource::create(tmp.socket, Glib::IO_IN);
#else
	tmp.source=Glib::IOSource::create(tmp.socket->get_fd(), Glib::IO_IN);
#endif
	tmp.source->connect(sigc::mem_fun(*this, &Network::onReceive));
	tmp.source->attach(Glib::MainContext::get_default());

	c.push_back(tmp);
	onConnect(tmp.socket->get_fd());
	return true;
}

bool Network::onReceive(Glib::IOCondition condition){
	gchar buff[4096], *p;
	int length, remain;
	Header *hp=(Header *)buff;
	Manager &mgr=Manager::getInstance();

	if(s.socket && s.socket->condition_check(condition)){
		try{
			length=s.socket->receive(buff, sizeof(Header));
		}catch(const Glib::Error &ex){
			std::cout << ex.what() << std::endl;
			onDisconnect(s.socket->get_fd());
			s.source.reset();
			s.socket->close();
			s.socket.reset();
			mgr.init_status();
			return false;
		}
		if(length==0){
			onDisconnect(s.socket->get_fd());
			s.source.reset();
			s.socket->close();
			s.socket.reset();
			mgr.init_status();
			return false;
		}
		if(hp->length>0){
			p=buff+sizeof(Header);
			remain=hp->length;
			do{
				length=s.socket->receive(p, remain);
				remain-=length;
				p+=length;
			}while(remain>0);
		}
		onRecvFromServer(buff);
	}else{
		for(Clients::iterator i=c.begin(); i!=c.end(); ++i){
			if(i->socket->condition_check(condition)){
				try{
					length=i->socket->receive(buff, sizeof(Header));
				}catch(const Glib::Error &ex){
					std::cout << ex.what() << std::endl;
					onDisconnect(i->socket->get_fd());
					i->source.reset();
					i->socket->close();
					i->socket.reset();
					i=c.erase(i);
					return false;
				}
				if(length==0){
					onDisconnect(i->socket->get_fd());
					i->source.reset();
					i->socket->close();
					i->socket.reset();
					i=c.erase(i);
					return false;
				}
				if(hp->length>0){
					p=buff+sizeof(Header);
					remain=hp->length;
					do{
						length=i->socket->receive(p, remain);
						remain-=length;
						p+=length;
					}while(remain>0);
				}
				onRecvFromClient(i->socket->get_fd(), buff);
			}
		}
	}
	return true;
}
