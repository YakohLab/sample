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

	for(int i=0; i<max_dots; ++i){
		scene.p[fd].dots[i].x+=(mgr.members[fd].input.right-mgr.members[fd].input.left)*5;
		scene.p[fd].dots[i].y+=(mgr.members[fd].input.down-mgr.members[fd].input.up)*5;
	}
	if(mgr.members[fd].input.x!=(-1)){
		scene.p[fd].dots[scene.p[fd].curDots].x=mgr.members[fd].input.x;
		scene.p[fd].dots[scene.p[fd].curDots].y=mgr.members[fd].input.y;
		scene.p[fd].dots[scene.p[fd].curDots].visible=1;
		scene.p[fd].curDots=(scene.p[fd].curDots+1)%max_dots;
	}
	scene.p[fd].ax=mgr.members[fd].input.ax;
	scene.p[fd].ay=mgr.members[fd].input.ay;
	scene.p[fd].az=mgr.members[fd].input.az;
	if(mgr.members[fd].input.key!=0){
		scene.c[0]=mgr.members[fd].input.key;
	}
}
