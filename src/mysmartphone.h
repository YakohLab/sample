#include "Smartphone.h"

class MySmartphone : public Smartphone {
public:
	MySmartphone(int p);
	void onRecvBinary(float *array, int n);
	void onConnect(const char *from, int w, int h);
	void onDisconnect(void);
};
