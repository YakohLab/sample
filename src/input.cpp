/*
 * input.cpp
 * InputDataをパケットに詰め込むpack関数と、パケットからメンバに取り込むreceive関数
 * 各種入力デバイスからのイベントを、InputDataへ反映させる方法を定める関数
 */
#include "input.h"
#include "view.h"
#include "mysmartphone.h"
#include <iostream>
void Input::receiveInput(char *tmp, InputData &data){ // サーバとして、クライアントの入力を受け取る
	data=*(InputData *)tmp;
}

char *Input::packInput(int &len){
	char *tmp=buffer;
	*(InputData *)tmp=input; tmp+=sizeof(InputData);
	clearInput();
	len=tmp-buffer;
	if(len>max_inputlen){
		std::cout << "Input size exceeds max_msglen." << std::endl;
	}
	return buffer;
}

void Input::clearInput(void){
	input.x=input.y=-1;
//	input.ax=input.ay=input.az=0;
}

void Input::set_input(int argx, int argy){
	input.x=argx;
	input.y=argy;
}

void Input::set_SmaphoInput(int argx, int argy){
	ViewManager &vmr=ViewManager::getInstance();
	MySmartphone &smapho=MySmartphone::getInstance();
	set_input(vmr.get_width()*argx/smapho.get_width(),
			vmr.get_height()*argy/smapho.get_height());
}

void Input::set_angle(double argax, double argay, double argaz){
	input.ax=argax;
	input.ay=argay;
	input.az=argaz;
}

void Input::set_key(GdkEventKey* k){
#ifdef INPUT_VERBOSE
	std::cout << "Pressed " << k->keyval << std::endl;
#endif
	switch(k->keyval){
	case GDK_KEY_Up:
		input.up=1;
		break;
	case GDK_KEY_Down:
		input.down=1;
		break;
	case GDK_KEY_Left:
		input.left=1;
		break;
	case GDK_KEY_Right:
		input.right=1;
		break;
	default:
		if(GDK_KEY_A<=k->keyval && k->keyval<=GDK_KEY_z){
			input.key=k->keyval;
		}
		break;
	}
	return;
}

void Input::reset_key(GdkEventKey* k){
#ifdef INPUT_VERBOSE
	std::cout << "Released " << k->keyval << std::endl;
#endif
	switch(k->keyval){
	case GDK_KEY_Up:
		input.up=0;
		break;
	case GDK_KEY_Down:
		input.down=0;
		break;
	case GDK_KEY_Left:
		input.left=0;
		break;
	case GDK_KEY_Right:
		input.right=0;
		break;
	default:
		if(GDK_KEY_A<=k->keyval && k->keyval<=GDK_KEY_z){
			input.key=0;
		}
		break;
	}
	return;
}
