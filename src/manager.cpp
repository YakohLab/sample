/*
 * manager.cpp
 *
 *  Created on: 2013/12/17
 *      Author: sdjikken
 */

#include "manager.h"

Manager::Manager(void){
//	smapho=new MySmartphone(8888);
	init_status();
}

Manager& Manager::getInstance() {
	static Manager instance;

	return instance;
}

void Manager::init_status() {
	state = Stop;
	mode = Standalone;
}

void Manager::init_objects() {
	model.initModelWithScene(&scene);
	ViewManager::getInstance().init_view_with_scene(&scene);
}

const Manager::State Manager::get_state() const {
	return state;
}
void Manager::set_state(State s) {
	state = s;
}

const Manager::Mode Manager::get_mode() const {
	return mode;
}
void Manager::set_mode(Manager::Mode s) {
	mode = s;
}

void Manager::attend_single_player() {
	scene.p[0].attend = 1;
}

void Manager::attend_player(int id) {
	scene.p[id].attend = 1;
}

void Manager::absent_player(int id) {
	scene.p[id].attend = 0;
}

gboolean Manager::tick(void *p) {
	Manager &mgr = Manager::getInstance();
	ViewManager &view = ViewManager::getInstance();

	view.get_input(&input[0]);
	mgr.model.preAction();
	mgr.model.stepPlayer(0, &input[0]);
	mgr.model.postAction();
	view.update();

	if (mgr.get_state() == Manager::Run) { // trueを返すとタイマーを再設定し、falseならタイマーを停止する
		return true;
	} else {
		view.init_view_with_scene(NULL);
		return false;
	}
}

gboolean Manager::tickServer(void *p) {
	Manager &mgr = Manager::getInstance();
	ViewManager &view = ViewManager::getInstance();
	MyNetwork &net=MyNetwork::getInstance();

	view.get_input(&input[0]); // 他のプレーヤーの入力は、既に通信で非同期に届いている
	mgr.model.preAction();
	for (int i = 0; i < max_players; ++i) {
		if (mgr.scene.p[i].attend) {
			mgr.model.stepPlayer(i, &input[i]);
		}
	}
	mgr.model.postAction();
	for (int i = 1; i < max_players; ++i) { // 自分には送る必要ないので1から
		if (mgr.scene.p[i].attend) {
			net.sendScene(i, mgr.scene);
		}
	}
	view.update();
	if (mgr.get_state() == Manager::Run) { // trueを返すとタイマーを再設定し、falseならタイマーを停止する
		return true;
	} else {
		view.init_view_with_scene(NULL);
		return false;
	}
	return true;
}
