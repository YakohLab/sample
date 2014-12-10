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
	void sendToServer(void *, int len);
	void sendToClient(int fd, void *, int len);
	void openServer(int port);
	void closeServer(void);
	void connect(const char *, int port);
	void disconnect(void);
	virtual void onConnect(int id){};
	virtual void onDisconnect(int id){};
	virtual void onRecvFromServer(char *msg){};
	virtual void onRecvFromClient(int id, char *msg){};
	virtual ~Network(void);
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
