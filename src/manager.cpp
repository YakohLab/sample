/*
 * manager.cpp
 *
 *  Created on: 2013/12/17
 *      Author: sdjikken
 */

#include "manager.h"
#include "input.h"

const int period = 30; // in millisecond

Manager::Manager(void){
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
	model.initModel();
	Input &input=Input::getInstance();
	input.clearInput();
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

bool Manager::tick(void) {
	Manager &mgr = Manager::getInstance();
	ViewManager &view = ViewManager::getInstance();
	Input &input = Input::getInstance();

	view.checkInput();
	mgr.members[0].input=input.input;
	input.clearInput();
	mgr.model.preAction();
	mgr.model.stepPlayer(0);
	mgr.model.postAction();
	mgr.scene.valid=true;
	view.update();

	if (mgr.get_state() == Run) { // trueを返すとタイマーを再設定し、falseならタイマーを停止する
		return true;
	} else {
		return false;
	}
}

void Manager::tickClient(void){
	ViewManager &view =ViewManager::getInstance();
	view.update();
	view.checkInput();
}

bool Manager::tickServer(void) {
	Manager &mgr = Manager::getInstance();
	ViewManager &view = ViewManager::getInstance();
	MyNetwork &net=MyNetwork::getInstance();
	Input &input=Input::getInstance();

	mgr.members[0].input=input.input;
	input.clearInput();
	mgr.model.preAction();
	for(std::map<int, Member>::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i){
		mgr.model.stepPlayer(i->first);
	}
	mgr.model.postAction();
	net.sendScene(mgr.scene);
	mgr.scene.id=0; // serverのidはゼロ
	mgr.scene.valid=true;
	view.update();
	view.checkInput(); // 他のプレーヤーの入力は、既に通信で非同期に届いている
	if (mgr.get_state() == Run) { // trueを返すとタイマーを再設定し、falseならタイマーを停止する
		return true;
	} else {
		return false;
	}
}

void Manager::startStandaloneTick(void){
	Input &input=Input::getInstance();
	Manager &mgr = Manager::getInstance();
	input.clearInput();
	mgr.scene.init();
	mgr.scene.id=-1;
	sigc::slot<bool> slot = sigc::mem_fun(*this, &Manager::tick);
	Glib::signal_timeout().connect(slot, period);
}

void Manager::startServerTick(void){
	Input &input=Input::getInstance();
	Player p;
	int j=0;
	input.clearInput();
	scene.init();
	for(std::map<int, Member>::iterator i=members.begin(); i!=members.end(); ++i, ++j){
		// 参加者のnameとidを確定する
		strcpy(p.name, i->second.name.c_str());
		// 初期化したinputをコピーすることで、各々のinputを初期化する
		members[i->first].input=input.input;
		scene.p[j]=p;
	}
	model.initModel();
	set_state(Run);
	sigc::slot<bool> slot = sigc::mem_fun(*this, &Manager::tickServer);
	Glib::signal_timeout().connect(slot, period);
}
