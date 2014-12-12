#include "Smartphone.h"

class MySmartphone : public Smartphone {
public:
	static MySmartphone& getInstance() {
		static MySmartphone instance;
		return instance;
	}
	void onRecvBinary(float *array, int n);
	void onConnect(const char *from, int w, int h);
	void onDisconnect(void);
	void sendImage(const char *filename);
private:
	MySmartphone():Smartphone(){}
	MySmartphone(MySmartphone&);
	void operator =(MySmartphone&);
};
