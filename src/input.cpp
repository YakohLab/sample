/*
 * input.cpp
 * InputDataやSceneをパケットに詰め込むpack関数と、パケットからメンバに取り込むreceive関数
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
	if(len>max_msglen){
		std::cout << "Input size exceeds max_msglen." << std::endl;
	}
	return buffer;
}

void Scene::receiveScene(char *tmp){
	int size, j;
	if(valid)return;
	c[0]=*(char *)tmp; tmp+=sizeof(char);
	c[1]=*(char *)tmp; tmp+=sizeof(char);
	id=*(char *)tmp; tmp+=sizeof(char);
	tm=*(struct tm *)tmp; tmp+=sizeof(struct tm);
	p.clear();								// まずコンテナを空にして、
	size=*(char *)tmp; tmp+=sizeof(char);	// 可変長のデータの個数を受け取り
	for(int i=0; i<size; ++i){				// その数だけ繰り返し、コンテナに格納する
		j=*(char *)tmp; tmp+=sizeof(char);
		p[j]=*(Player *)tmp; tmp+=sizeof(Player);
	}
	valid=true;
}

char *Scene::packScene(int &len){
	char *tmp=buffer;
	*(char *)tmp=c[0]; tmp+=sizeof(char);
	*(char *)tmp=c[1]; tmp+=sizeof(char);
	*(char *)tmp=id; tmp+=sizeof(char);
	*(struct tm *)tmp=tm; tmp+=sizeof(struct tm);
	*(char *)tmp=p.size(); tmp+=sizeof(char); // 可変長のデータは、まず個数を送る
	for(std::map<int, Player>::iterator i=p.begin(); i!=p.end(); ++i){
		*(char *)tmp=i->first; tmp+=sizeof(char);
		*(Player *)tmp=i->second; tmp+=sizeof(Player);
	}
	len=tmp-buffer;
	if(len>max_msglen){
		std::cout << "Scene size exceeds max_msglen." << std::endl;
	}
	return buffer;
}

void Input::clearInput(void){
	input.up=input.down=input.left=input.right=0;
	input.x=input.y=-1;
	input.key=0;
	input.ax=input.ay=input.az=0;
}

void Input::set_input(int argx, int argy){
	input.x=argx;
	input.y=argy;
}

void Input::set_SmaphoInput(int argx, int argy){
	ViewManager &vmr=ViewManager::getInstance();
	MySmartphone &smapho=MySmartphone::getInstance();
	input.x=vmr.get_width()*argx/smapho.get_width();
	input.y=vmr.get_height()*argy/smapho.get_height();
}

void Input::set_angle(double argax, double argay, double argaz){
	input.ax=argax;
	input.ay=argay;
	input.az=argaz;
}

void Input::set_key(GdkEventKey* k){
	//	std::cout << "Released " << k->keyval << std::endl;
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
	//	std::cout << "Released " << k->keyval << std::endl;
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

void Scene::init(void){
	c[0]=c[1]=0;
	id=0;
	p.clear();
	valid=false;
}
