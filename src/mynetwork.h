/*
 * mynetwork.h
 * Networkを継承した、ゲーム用の通信関数をまとめたもの
 */
#ifndef MYNETWORK_H_
#define MYNETWORK_H_
#include "Network.h"
#include "input.h"
#include "scene.h"

class MyNetwork : public Network {
public:
	static MyNetwork& getInstance() {
		static MyNetwork instance;
		return instance;
	}
	bool startServer(int port, const char *name);
	bool connectClient(const char *host, int port, const char *name);
	void sendScene(Scene &);
	void runClient(void);
	void stopClient(void);
	void runServer(void);
	void stopServer(void);
private:
	void onDisconnect(int id);
	void onRecvFromServer(char *msg);
	void onRecvFromClient(int id, char *msg);

	void disconnectClient(void);
	void sendInput(void);
	bool showStatus(void);
	void sendStop(void);

	MyNetwork(void);
};

#endif /* MYNETWORK_H_ */
