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
	void startServer(int port, const char *name);
	void connectClient(const char *host, int port, const char *name);
	void sendScene(int, Scene &);
	void runClient(void);
	void stopClient(void);
	void runServer(void);
	void stopServer(void);
private:
	void onDisconnect(int id);
	void onRecvFromServer(char *msg);
	void onRecvFromClient(int id, char *msg);

	void disconnectClient(void);
	void sendInput(Input &);
	bool showStatus(void);
	void sendStop(void);

	MyNetwork():Network(){}
	MyNetwork(MyNetwork&);
	void operator =(MyNetwork&);
};

#endif /* MYNETWORK_H_ */
