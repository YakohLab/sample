/*
 * manager.h
 *
 *  Created on: 2013/12/17
 *      Author: sdjikken
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include "model.h"
#include "view.h"
#include "mysmartphone.h"
#include "mynetwork.h"

class Manager {
public:
	enum State {
		Run, Stop
	};
	enum Mode {
		Server, Client, Standalone
	};

public:
	static Manager& getInstance();

	void init_status();
	void init_objects();

	const State get_state() const;
	void set_state(State s);

	const Mode get_mode() const;
	void set_mode(Mode s);

	void attend_single_player();
	void attend_player(int id);
	void absent_player(int id);

	static gboolean tick(void *p);
	static gboolean tickServer(void *p);

	Scene& get_scene() {
		return scene;
	}

private:
	Manager(void);
	Manager(Manager&);
	Manager &operator =(Manager&);

	State state;
	Mode mode;

	Model model;
	Scene scene;

//	MySmartphone *smapho;
};

#endif /* MANAGER_H_ */
