/*
 * mysmartphone.cpp
 * Smartphoneを継承して、ゲーム用のスマホサーバ機能を実現
 */
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
#ifdef SMAPHO_VERBOSE
		std::cout << (int)array[0] << " ";
#endif
		for(int i=1; i<n; i+=2){
			w=(int)array[i];
			h=(int)array[i+1];
#ifdef SMAPHO_VERBOSE
			std::cout << "(" << w << "," << h << ") ";
#endif
		}
#ifdef SMAPHO_VERBOSE
		std::cout << std::endl;
		std::flush(std::cout);
#endif
		if(n>1){
			input.set_SmaphoInput(w, h);
		}
		break;
	case 4: // accelerometer
#ifdef SMAPHO_VERBOSE
		std::cout << array[0] << ", " << array[1] << ", " << array[2] << ", " << array[3] << std::endl;
#endif
		input.set_angle(array[1], array[2], array[3]);
		break;
	}
}

void MySmartphone::onConnect(const char *from, int w, int h){
	ViewManager &vmr=ViewManager::getInstance();
#ifdef SMAPHO_VERBOSE
	std::cout << "Connected from " << from << ", screen size="<< w << "x" << h << std::endl;
#endif
	vmr.menu[3]->set_sensitive(true);
}

void MySmartphone::onDisconnect(void){
	ViewManager &vmr=ViewManager::getInstance();
#ifdef SMAPHO_VERBOSE
	std::cout << "Closed" << std::endl;
#endif
	vmr.menu[3]->set_sensitive(false);
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

int MySmartphone::get_width(void){
	return width;
}

int MySmartphone::get_height(void){
	return height;
}
