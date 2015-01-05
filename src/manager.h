/*
 * manager.h
 * ゲームの状態を表すManagerクラスを定義
 */
#ifndef MANAGER_H_
#define MANAGER_H_

#include "model.h"
#include "view.h"
#include "mysmartphone.h"
#include "mynetwork.h"

class Member {
public:
	int ready;
	int id;
	std::string name;
	InputData input;
};

typedef std::map<int, Member> Members;

class Manager {
public:
	enum State {
		Run, Stop
	};
	enum Mode {
		Server, Client, Standalone
	};
	static Manager& getInstance();

	void init_status();
	void init_objects();

	const State get_state() const;
	void set_state(State s);

	const Mode get_mode() const;
	void set_mode(Mode s);

	bool tickServer(void);
	void tickClient(void);
	void startServerTick(void);
	void startStandaloneTick(std::string);
	Members members;

	Scene scene;
private:
	Manager(void);
	Manager(Manager&);
	Manager &operator =(Manager&);

	State state;
	Mode mode;
	Model model;
};

#endif /* MANAGER_H_ */
