/*
 * model.cpp
 *
 *  Created on: 2013/12/09
 *      Author: yakoh
 */

#include "model.h"

Model::Model() {
	// TODO Auto-generated constructor stub
	initModel();
}

Model::~Model() {
	// TODO Auto-generated destructor stub
}

void Model::initModel(void){
	scene.curDots=0;
	for(int i=0; i<max_dots; ++i){
		scene.dots[i].visible=0;
	}
}

scene_t *Model::doModel(input_t *input){
	for(int i=0; i<max_dots; ++i){
		scene.dots[i].x+=(input->right-input->left)*5;
		scene.dots[i].y+=(input->down-input->up)*5;
	}
	if(input->x!=(-1)){
		scene.dots[scene.curDots].x=input->x;
		scene.dots[scene.curDots].y=input->y;
		scene.dots[scene.curDots].visible=1;
		scene.curDots=(scene.curDots+1)%max_dots;
	}
	return &scene;
}
