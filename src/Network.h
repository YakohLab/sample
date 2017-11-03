/*
 * Network.h
 * TCP/IPソケット通信による基本的なサーバ・クライアント通信機能を実現
 */
#ifndef NETWORK_H
#define NETWORK_H
#include <iostream>
#include <vector>
#include <gtkmm.h>
#include <glibmm.h>

struct Header {
	enum {CONNECT, START, STOP, INPUT, STATUS, DRAW} command;
	unsigned long int length;
};

class Network {
public:
	Network(void);
	virtual ~Network(void);
	void closeServer(void);
	void disconnect(void);
	bool openServer(short unsigned int port);
	bool connect(const char *, short unsigned int port);
	void sendToServer(void *, unsigned long int len);
	void sendToClient(int fd, void *, unsigned long int len);
protected:
	virtual void onConnect(int fd){};
	virtual void onDisconnect(int fd){};
	virtual void onRecvFromServer(char *msg){};
	virtual void onRecvFromClient(int fd, char *msg){};
private:
	struct SS {
		Glib::RefPtr<Gio::Socket> socket;
	#ifdef USE_SOCKETSOURCE
		Glib::RefPtr<Gio::SocketSource> source;
	#else
		Glib::RefPtr<Glib::IOSource> source;
	#endif
	};
	typedef std::vector<SS> Clients;
	SS w, s; // wait, server
	Clients c; // clients
	bool onAccept(Glib::IOCondition condition);
	bool onReceive(Glib::IOCondition condition);
};

#endif
