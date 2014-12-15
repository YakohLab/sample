#include "mysmartphone.h"
#include <iostream>

#include "input.h"
#include "view.h"

void MySmartphone::onRecvBinary(float *array, int n){
	int w, h;
	Input &input=Input::getInstance();

	switch((int)array[0]){
	case 1: // touch start
	case 2: // touch move
	case 3: // touch end
		std::cout << (int)array[0] << " ";
		for(int i=1; i<n; i+=2){
			w=(int)array[i];
			h=(int)array[i+1];
			std::cout << "(" << w << "," << h << ") ";
		}
		std::cout << std::endl;
		std::flush(std::cout);
		if(drawingArea && n>1){
			input.set_input(w, h);
		}
		break;
	case 4: // accelerometer
		if(drawingArea){
			input.set_angle(array[1], array[2], array[3]);
		}
		break;
	}
}

void MySmartphone::onConnect(const char *from, int w, int h){
	std::cout << "Connected from " << from << ", screen size="<< w << "x" << h << std::endl;
}

void MySmartphone::onDisconnect(void){
	std::cout << "Closed" << std::endl;
}

void MySmartphone::sendImage(const char *filename){
	Glib::RefPtr<Gdk::Pixbuf> pixbuf;
	try{
		pixbuf=Gdk::Pixbuf::create_from_file(filename, width, height, false);
	}catch(Glib::FileError &e){
		return;
	}catch(Gdk::PixbufError &e){
		return;
	}
	sendPixbuf(pixbuf);
}
