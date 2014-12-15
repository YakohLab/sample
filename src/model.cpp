/*
 * model.cpp
 *
 *  Created on: 2013/12/09
 *      Author: yakoh
 */

#include <iostream>
#include "model.h"
#include "manager.h"

#include "input.h"

Model::Model() {
	// TODO Auto-generated constructor stub
}

Model::~Model() {
	// TODO Auto-generated destructor stub
}

void Model::initModel(void){
//	std::cout << "Init" << std::endl;
	Scene &scene=Manager::getInstance().scene;
	time_t t;
	t=time(NULL);
	localtime_r(&t, &scene.tm);
	scene.c[0]=0;
	scene.c[1]=0;
	for(std::map<int, Player>::iterator i=scene.p.begin(); i!=scene.p.end(); ++i){
		i->second.curDots=0;
		for(int j=0; j<max_dots; ++j){
			i->second.dots[j].visible=0;
		}
	}
}

void Model::preAction(void){
	time_t t;
	Scene &scene=Manager::getInstance().scene;
	t=time(NULL);
	localtime_r(&t, &scene.tm);
}

void Model::postAction(void){

}

void Model::stepPlayer(int fd){
	Manager &mgr = Manager::getInstance();
	Scene &scene=mgr.scene;
	InputData &input=mgr.members[fd].input;
	int id=0;
	for(std::map<int, Member>::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i, ++id){
		if(i->first==fd)break;
	}
	Player &player=scene.p[id];

	for(int i=0; i<max_dots; ++i){
		player.dots[i].x+=(input.right-input.left)*5;
		player.dots[i].y+=(input.down-input.up)*5;
	}
	if(input.x!=(-1)){
		player.dots[player.curDots].x=input.x;
		player.dots[player.curDots].y=input.y;
		player.dots[player.curDots].visible=1;
		player.curDots=(player.curDots+1)%max_dots;
	}
	player.ax=input.ax;
	player.ay=input.ay;
	player.az=input.az;
	if(input.key!=0){
		scene.c[0]=input.key;
	}
}
