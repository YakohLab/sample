/*
 * scene.cpp
 * Sceneをパケットに詰め込むpack関数と、パケットからメンバに取り込むreceive関数
 */
#include "scene.h"
#include <iostream>

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
	if(len>max_scenelen){
		std::cout << "Scene size exceeds max_msglen." << std::endl;
	}
	return buffer;
}

void Scene::init(void){
	c[0]=c[1]=0;
	id=0;
	p.clear();
	valid=false;
}
