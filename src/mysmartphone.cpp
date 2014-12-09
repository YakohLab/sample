#include "mysmartphone.h"
#include <iostream>
#include "view.h"

using namespace std;

MySmartphone::MySmartphone(int p):Smartphone(p){
}
void MySmartphone::onRecvBinary(float *array, int n){
	int w, h;
	switch((int)array[0]){
	case 1: // touch start
	case 2: // touch move
	case 3: // touch end
		cout << (int)array[0] << " ";
		for(int i=1; i<n; i+=2){
			w=(int)array[i];
			h=(int)array[i+1];
			cout << "(" << w << "," << h << ") ";
		}
		cout << endl;
		flush(cout);
		if(drawingArea && n>1){
			drawingArea->set_input(w, h);
		}
		break;
	case 4: // accelerometer
		if(drawingArea){
			drawingArea->set_angle(array[1], array[2], array[3]);
		}
		break;
	}
}

void MySmartphone::onConnect(const char *from, int w, int h){
	cout << "Connected from " << from << ", screen size="<< w << "x" << h << endl;
}

void MySmartphone::onDisconnect(void){
	cout << "Closed" << endl;
}
