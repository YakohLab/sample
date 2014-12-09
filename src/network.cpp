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

#include "network.h"

Network::Network(void){
	s.socket.reset();
	w.socket.reset();
}

Network::~Network(void){
	for(std::vector<SS>::iterator i=c.begin(); i!=c.end(); ++i){
		i->socket.reset();
	}
	s.socket.reset();
	w.socket.reset();
}

void Network::startServer(int p){
	Glib::RefPtr<Gio::SocketAddress> src_address;

	s.socket.reset();
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
	src_address=Gio::InetSocketAddress::create (Gio::InetAddress::create_any (Gio::SOCKET_FAMILY_IPV4), p);
	w.socket->bind(src_address, true);
	w.socket->listen();
#ifdef USE_SOCKETSOURCE
	w.source=Gio::SocketSource::create(w, Glib::IO_IN);
#else
	w.source=Glib::IOSource::create(w.socket->get_fd(), Glib::IO_IN);
#endif
	w.source->connect(sigc::mem_fun(*this, &Network::onAccept));
	w.source->attach(Glib::MainContext::get_default());
}

void Network::stopServer(void){
	if(!w.socket->is_closed()){
		w.source->destroy();
		w.socket->close();
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
	gchar buff[4096];
	int length;

	if(s.socket && s.socket->condition_check(condition)){
		length=s.socket->receive(buff, sizeof buff);
		if(length<1){
			onDisconnect(s.socket->get_fd());
			s.source->destroy();
			s.source.reset();
			s.socket->close();
			s.socket.reset();
			return false;
		}else{
			onRecvFromServer((char *)buff, length);
		}
	}else{
		for(std::vector<SS>::iterator i=c.begin(); i!=c.end(); ++i){
			if(i->socket->condition_check(condition)){
				length=i->socket->receive(buff, sizeof buff);
				if(length<1){
					onDisconnect(i->socket->get_fd());
					i->source->destroy();
					i->source.reset();
					i->socket->close();
					i->socket.reset();
					i=c.erase(i);
					return false;
				}else{
					onRecvFromClient(i->socket->get_fd(), (char *)buff, length);
				}
			}
		}
	}
	return true;
}
