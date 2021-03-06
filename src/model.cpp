/*
 * model.cpp
 * 入力データInputDataの内容を場面Sceneに反映させる
 * 不公平にならないよう注意が必要
 */
#include <iostream>
#include "model.h"
#include "manager.h"
#include "input.h"

void Model::initModel(void){
//	std::cout << "Init" << std::endl;
	Scene &scene=Manager::getInstance().scene;
	time_t t;
	t=time(NULL);
	localtime_r(&t, &scene.tm);
	scene.c[0]=0;
	scene.c[1]=0;
	scene.valid=false;
	for(Players::iterator i=scene.p.begin(); i!=scene.p.end(); ++i){
		Player &pl=i->second;
		switch(i->first){
		case 0: pl.g=pl.b=0; pl.r=1; break;
		case 1: pl.r=pl.g=0; pl.b=1; break;
		case 2: pl.r=pl.b=0; pl.g=1; break;
		case 3: pl.g=pl.b=1; pl.r=0; break;
		case 4: pl.r=pl.g=1; pl.b=0; break;
		case 5: pl.r=pl.b=1; pl.g=0; break;
		}
		pl.curDots=0;
		for(int j=0; j<max_dots; ++j){
			pl.dots[j].visible=0;
		}
		pl.scale=1.0;
		pl.angle=0.0;
	}
}

void Model::preAction(void){ // 衝突判定など、判定のみを行う。公平のため、ここで動かしてはいけない
	time_t t;
	Manager &mgr = Manager::getInstance();
	Scene &scene=Manager::getInstance().scene;
	int ii, jj;
	t=time(NULL);
	localtime_r(&t, &scene.tm);
	ii=0;
	for(Members::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i, ++ii){
		jj=0;
		for(Members::iterator j=mgr.members.begin(); j!=mgr.members.end(); ++j, ++jj){
			if(ii==jj)continue;
			Player &pi=scene.p[ii];
		    Player &pj=scene.p[jj];
		    if(pi.curDots==pj.curDots){
		    	std::cout << "Bingo!" << std::endl;
		    }
		}
	}
}

void Model::postAction(void){ // 全プレイヤーの動作を終えた後に、全体の状況を変えたい場合はここで処理する

}

void Model::stepPlayer(int fd){ // 各プレイヤーの動作を行う。公平のため、ここでは判断を行ってはいけない
	Manager &mgr = Manager::getInstance();
	Scene &scene=mgr.scene;
	InputData &input=mgr.members[fd].input;
	int id=0;
	for(Members::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i, ++id){
		if(i->first==fd)break;
	}
    if(scene.p.size()==0)return;
    Player &player=scene.p[id];

	for(int i=0; i<max_dots; ++i){
		player.dots[i].x+=(input.right-input.left)*5+input.dragx;
		player.dots[i].y+=(input.down-input.up)*5+input.dragy;
	}
	if(input.x!=(-1)){
		player.dots[player.curDots].x=input.x;
		player.dots[player.curDots].y=input.y;
		player.dots[player.curDots].visible=1;
		player.curDots=(player.curDots+1)%max_dots;
	}
	player.setAccel(input.ax, input.ay, input.az);
	if(input.key!=0){
		scene.c[0]=(char)input.key;
	}
	player.scale=input.scale;
	player.angle=input.angle;
}
