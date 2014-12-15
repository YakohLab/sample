#ifndef INPUT_H_
#define INPUT_H_

#include <vector>
#include <map>
#include <gtkmm.h>
#include "common.h"

class InputData {
public:
	int up, down, left, right;
	int x, y;
	int key;
	double ax, ay, az;
};

class Input {
public:
	static Input& getInstance() {
		static Input instance;
		return instance;
	}
	void receiveInput(char *, InputData &data);
	char *packInput(int &);
	void checkInput(void);
	void clearInput(void);
	void set_key(GdkEventKey* k);
	void reset_key(GdkEventKey* k);
	void set_input(int argx, int argy);
	void set_angle(double, double, double);
	InputData input;
private:
	Input(){
		clearInput();
	}
	Input(Input&);
	void operator =(Input&);
	char buffer[1000];
};

class Member {
public:
	int ready;
	std::string name;
	InputData input;
};

struct Dot {
	int x, y, visible;
};

const int max_dots = 5;
struct Player {
	std::string name;
	int id;
	int x, y;
	double ax, ay, az;
	Dot dots[max_dots];
	int curDots;
};

class Scene {
public:
	char c[2];
	struct tm tm;
	std::map<int, Player> p;
	void receiveScene(char *);
	char *packScene(int &len);
	bool valid;
	void init(){};
	Scene(){init();};
private:
	char buffer[1000];
};

#endif
