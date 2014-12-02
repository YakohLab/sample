#include "smartphone.h"

class MySmartphone : public Smartphone {
public:
	MySmartphone(int p);
	void recvBinary(float *array, int n);
	void onConnect(const char *from, int w, int h);
	void onClose(void);
};
