/*
 * mysmartphone.h
 * Smartphoneを継承して、ゲーム用のスマホサーバ機能を実現
 */
#ifndef MYSMARTPHONE_H_
#define MYSMARTPHONE_H_
#include "Smartphone.h"

// #define SMAPHO_VERBOSE

class MySmartphone : public Smartphone {
public:
	static MySmartphone& getInstance() {
		static MySmartphone instance;
		return instance;
	}
	int startw, starth, startw2, starth2;
	bool drag, pinch;
	void onRecvBinary(float *array, unsigned long int n);
	void onConnect(const char *from, int w, int h);
	void onDisconnect(void);
	void sendImage(const char *filename);
	int get_width(void);
	int get_height(void);
private:
	MySmartphone(void);
};

#endif
