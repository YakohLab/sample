/*
 * manager.cpp
 * ゲームの進行を司る。
 * タイマーを利用し、サンプリング周期毎に入力・処理・出力を繰り返すという流れを実現する。
 */
#include "manager.h"
#include "input.h"

const int period = 33; // in millisecond

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

void Manager::tickClient(void){
	ViewManager &view =ViewManager::getInstance();
	view.update();
}

bool Manager::tickServer(void) {
	Manager &mgr = Manager::getInstance();
	ViewManager &view = ViewManager::getInstance();
	MyNetwork &net=MyNetwork::getInstance();
	Input &input=Input::getInstance();

	mgr.members[0].input=input.input; // 自分は必ず0番に入っている
	input.clearInput();
	mgr.model.preAction();
	for(Members::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i){
		mgr.model.stepPlayer(i->first);
	}
	mgr.model.postAction();
	net.sendScene(mgr.scene);
	mgr.scene.id=mgr.members[0].id; // 自分は必ず0番に入っている
	mgr.scene.valid=true;
	view.update();
	if (mgr.get_state() == Run) { // trueを返すとタイマーを再設定し、falseならタイマーを停止する
		return true;
	} else {
		return false;
	}
}

void Manager::startStandaloneTick(std::string n){
	Input &input=Input::getInstance();
	Manager &mgr = Manager::getInstance();
	input.clearInput();
	mgr.scene.init();

	mgr.members.clear();
	Member tmp;
	tmp.name=n;
	tmp.ready=1;
	tmp.id=-1; // standaloneの時はidを-1にすることで、servermodeと区別できるようにしている
	mgr.members.insert(Members::value_type(0, tmp));

	model.initModel();
	sigc::slot<bool> slot = sigc::mem_fun(*this, &Manager::tickServer);
	Glib::signal_timeout().connect(slot, period);
}

void Manager::startServerTick(void){
	Input &input=Input::getInstance();
	Player p;
	int j=0;
	input.clearInput();
	scene.init();
	for(Members::iterator i=members.begin(); i!=members.end(); ++i, ++j){
		// 参加者のnameとidを確定する
		p.setName(i->second.name.c_str());
		// 初期化したinputをコピーすることで、各々のinputを初期化する
		members[i->first].input=input.input;
		members[i->first].id=j;
		scene.p.insert(Players::value_type(j, p));
	}
	model.initModel();
	sigc::slot<bool> slot = sigc::mem_fun(*this, &Manager::tickServer);
	Glib::signal_timeout().connect(slot, period);
}
