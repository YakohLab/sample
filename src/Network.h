#ifndef NETWORK_H
#define NETWORK_H
#include <iostream>
#include <vector>
#include <gtkmm.h>
#include <glibmm.h>

struct Header {
	enum {CONNECT, START, STOP, INPUT, STATUS, DRAW} command;
	int length;
};

class Network {
public:
	Network(void);
	virtual ~Network(void);
	void closeServer(void);
	void disconnect(void);
	bool openServer(int port);
	bool connect(const char *, int port);
	void sendToServer(void *, int len);
	void sendToClient(int fd, void *, int len);
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
	SS w, s; // wait, server
	std::vector<SS> c; // clients
	bool onAccept(Glib::IOCondition condition);
	bool onReceive(Glib::IOCondition condition);
};

#endif
