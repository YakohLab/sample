#include "input.h"
#include <iostream>
void Input::receiveInput(char *tmp, InputData &data){ // サーバとして、クライアントの入力を受け取る
	data.up=*(short int *)tmp; tmp+=sizeof(short int);
	data.down=*(short int *)tmp; tmp+=sizeof(short int);
	data.left=*(short int *)tmp; tmp+=sizeof(short int);
	data.right=*(short int *)tmp; tmp+=sizeof(short int);
	data.x=*(short int *)tmp; tmp+=sizeof(short int);
	data.y=*(short int *)tmp; tmp+=sizeof(short int);
	data.key=*(short int *)tmp; tmp+=sizeof(short int);
	data.ax=*(double *)tmp; tmp+=sizeof(double);
	data.ay=*(double *)tmp; tmp+=sizeof(double);
	data.az=*(double *)tmp; tmp+=sizeof(double);
}

char *Input::packInput(int &len){
	char *tmp=buffer;
	*(short int *)tmp=input.up; tmp+=sizeof(short int);
	*(short int *)tmp=input.down; tmp+=sizeof(short int);
	*(short int *)tmp=input.left; tmp+=sizeof(short int);
	*(short int *)tmp=input.right; tmp+=sizeof(short int);
	*(short int *)tmp=input.x; tmp+=sizeof(short int);
	*(short int *)tmp=input.y; tmp+=sizeof(short int);
	*(short int *)tmp=input.key; tmp+=sizeof(short int);
	*(double *)tmp=input.ax; tmp+=sizeof(double);
	*(double *)tmp=input.ay; tmp+=sizeof(double);
	*(double *)tmp=input.az; tmp+=sizeof(double);
	clearInput();
	len=tmp-buffer;
	return buffer;
}

void Scene::receiveScene(char *tmp){
	int size, id;
	if(valid)return;
	c[0]=*(char *)tmp; tmp+=sizeof(char);
	c[1]=*(char *)tmp; tmp+=sizeof(char);
	tm=*(struct tm *)tmp; tmp+=sizeof(struct tm);
	p.clear();								// まずコンテナを空にして、
	size=*(char *)tmp; tmp+=sizeof(char);	// 可変長のデータの個数を受け取り
	for(int i=0; i<size; ++i){				// その数だけ繰り返し、コンテナに格納する
		id=*(char *)tmp; tmp+=sizeof(char);
		p[id]=*(Player *)tmp; tmp+=sizeof(Player);
	}
	valid=true;
}

char *Scene::packScene(int &len){
	char *tmp=buffer;
	*(char *)tmp=c[0]; tmp+=sizeof(char);
	*(char *)tmp=c[1]; tmp+=sizeof(char);
	*(struct tm *)tmp=tm; tmp+=sizeof(struct tm);
	*(char *)tmp=p.size(); tmp+=sizeof(char); // 可変長のデータは、まず個数を送る
	for(std::map<int, Player>::iterator i=p.begin(); i!=p.end(); ++i){
		*(char *)tmp=i->first; tmp+=sizeof(char);
		*(Player *)tmp=i->second; tmp+=sizeof(Player);
	}
	len=tmp-buffer;
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
	p.clear();
	valid=false;
}
