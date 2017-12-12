/*
 * mysmartphone.cpp
 * Smartphoneを継承して、ゲーム用のスマホサーバ機能を実現
 */
#include "mysmartphone.h"
#include <iostream>

#include "input.h"
#include "view.h"

MySmartphone::MySmartphone(void):Smartphone(){
}

void MySmartphone::onRecvBinary(float *array, unsigned long int n){
	int w, h, w2, h2;
	Input &input=Input::getInstance();

	switch((int)array[0]){
	case 1: // touch start
	case 2: // touch move
	case 3: // touch end
#ifdef SMAPHO_VERBOSE
		std::cout << (int)array[0] << " ";
#endif
		for(unsigned long int i=1; i<n; i+=2){
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
		if((int)array[0]==1){
			startw=(int)array[1];
			starth=(int)array[2];
			if(n>3){
				startw2=(int)array[3];
				starth2=(int)array[4];
			}
		}
		if((int)array[0]==2){
			if(n>3){
				pinch=true;
				w=(int)array[1];
				h=(int)array[2];
				w2=(int)array[3];
				h2=(int)array[4];
				input.set_SmaphoPinch(sqrt((w2-w)*(w2-w)+(h2-h)*(h2-h))/sqrt((startw2-startw)*(startw2-startw)+(starth2-starth)*(starth2-starth)),
						atan2(w2-w,h2-h)-atan2(startw2-startw,starth2-starth));
				startw=(int)array[1];
				starth=(int)array[2];
				startw2=(int)array[3];
				starth2=(int)array[4];
			}else{
				drag=true;
				w=(int)array[1];
				h=(int)array[2];
				input.set_SmaphoDrag(w-startw, h-starth);
				startw=(int)array[1];
				starth=(int)array[2];
			}
		}
		if((int)array[0]==3){
			if(drag || pinch){
				drag=pinch=false;
			}else{
				input.set_SmaphoInput(startw, starth);
			}
		}
		break;
	case 4: // accelerometer
#ifdef SMAPHO_VERBOSE
		std::cout << array[0] << ", " << array[1] << ", " << array[2] << ", " << array[3] << std::endl;
#endif
		input.set_SmaphoAngle(array[1], array[2], array[3]);
		break;
	}
}

void MySmartphone::onConnect(const char *from, int w, int h){
	ViewManager &vmr=ViewManager::getInstance();
#ifdef SMAPHO_VERBOSE
	std::cout << "Connected from " << from << ", screen size="<< w << "x" << h << std::endl;
#endif
	vmr.menu[3]->set_sensitive(true);
	sendImage("smapho.png");
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
