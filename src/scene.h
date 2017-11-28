/*
 * scene.h
 * 表示するデータを定義するSceneクラス
 */
#ifndef SCENE_H_
#define SCENE_H_
#include <gtkmm.h>

const unsigned long int max_scenelen = 1000;

struct Dot {
	int x, y, visible;
};

const int max_dots = 5;
class Player { // 要素数を可変にしたい場合は、packSceneとreceiveSceneを注意深く書き直す必要がある
public:
	void setName(const char *);
	void setAccel(double ax, double ay, double az);
private:
	char name[20];
	int x, y;
public:
	double ax, ay, az;
	Dot dots[max_dots];
	int curDots;
    Player(int);
    Player(void){};
};

typedef std::map<int, Player> Players;

class Scene {
public:
	char c[2];
	struct tm tm;
	Players p;
	int id;
	void receiveScene(char *);
	char *packScene(unsigned long int &len);
	bool valid;
	void init(void);
	Scene(void);
private:
	char buffer[max_scenelen];
};

#endif
