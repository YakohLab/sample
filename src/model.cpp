/*
 * model.cpp
 *
 *  Created on: 2013/12/09
 *      Author: yakoh
 */

#include <iostream>
#include "model.h"

Model::Model() {
	// TODO Auto-generated constructor stub
}

Model::~Model() {
	// TODO Auto-generated destructor stub
}

void Model::initModelWithScene(Scene *s){
//	std::cout << "Init" << std::endl;
	scene=s;
	time_t t;
	t=time(NULL);
	localtime_r(&t, &scene->tm);
	scene->c[0]=0;
	scene->c[1]=0;
	for(int i=0; i<max_players; ++i){
		scene->p[i].attend=0;
		scene->p[i].curDots=0;
		for(int j=0; j<max_dots; ++j){
			scene->p[i].dots[j].visible=0;
		}
	}
}

void Model::preAction(void){
	time_t t;
	t=time(NULL);
	localtime_r(&t, &scene->tm);
}

void Model::postAction(void){

}

void Model::stepPlayer(int id, Input *input){
	for(int i=0; i<max_dots; ++i){
		scene->p[id].dots[i].x+=(input->right-input->left)*5;
		scene->p[id].dots[i].y+=(input->down-input->up)*5;
	}
	if(input->x!=(-1)){
		scene->p[id].dots[scene->p[id].curDots].x=input->x;
		scene->p[id].dots[scene->p[id].curDots].y=input->y;
		scene->p[id].dots[scene->p[id].curDots].visible=1;
		scene->p[id].curDots=(scene->p[id].curDots+1)%max_dots;
	}
	scene->p[id].ax=input->ax;
	scene->p[id].ay=input->ay;
	scene->p[id].az=input->az;
	if(input->key!=0){
		scene->c[0]=input->key;
	}
}
