#ifndef NETWORK_H
#define NETWORK_H
#include <gtkmm.h>
#include <glibmm.h>

class Network {
public:
	Network(void);
	void sendToServer(void *, int len);
	void sendToClient(int fd, void *, int len);
	void startServer(int port);
	void stopServer(void);
	void connect(const char *);
	virtual void onConnect(int id){
		std::cout << "Connected from " << id << std::endl;
	};
	virtual void onDisconnect(int id){
		std::cout << "Disconnected by " << id << std::endl;
	};
	virtual void onRecvFromServer(char *msg, int n){
		msg[n]=0;
		std::cout << "Server sends me " << msg << std::endl;
	};
	virtual void onRecvFromClient(int id, char *msg, int n){
		msg[n]=0;
		std::cout << id << " sends me " << msg << std::endl;
	};
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
