/*
 * scene.h
 * 表示するデータを定義するSceneクラス
 */
#ifndef SCENE_H_
#define SCENE_H_
#include <gtkmm.h>


const int max_scenelen = 1000;

struct Dot {
	int x, y, visible;
};

const int max_dots = 5;
struct Player { // 要素数を可変にしたい場合は、packSceneとreceiveSceneを注意深く書き直す必要がある
	char name[20];
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
	int id;
	void receiveScene(char *);
	char *packScene(int &len);
	bool valid;
	void init();
	Scene(){init();};
private:
	char buffer[max_scenelen];
};

#endif
