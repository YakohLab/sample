/*
 * mynetwork.h
 *
 *  Created on: 2014/12/09
 *      Author: sdjikken
 */

#ifndef MYNETWORK_H_
#define MYNETWORK_H_
#include "Network.h"
#include "common.h"

class MyNetwork : public Network {
public:
	static MyNetwork& getInstance() {
		static MyNetwork instance;
		return instance;
	}
	void onConnect(int id);
	void onDisconnect(int id);
	void onRecvFromServer(char *msg);
	void onRecvFromClient(int id, char *msg);

	void connectClient(const char *host, int port, const char *name);
	void runClient(void);
	void stopClient(void);
	void disconnectClient(void);
	void sendInput(Input &);

	void startServer(int port, const char *name);
	void runServer(void);
	void stopServer(void);
	void terminateServer(void);

	void sendScene(int, Scene &);

	bool showStatus(void);
private:
	MyNetwork():Network(){}
	MyNetwork(MyNetwork&);
	void operator =(MyNetwork&);
};

#endif /* MYNETWORK_H_ */
