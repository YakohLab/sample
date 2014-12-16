/*
 * common.h
 *
 *  Created on: 2013/12/09
 *      Author: yakoh
 */

#ifndef COMMON_H_
#define COMMON_H_
#include <gtkmm.h>
#include <time.h>

const int max_dots = 5;
const int max_players = 4;
const int period = 30;
const int max_msglen = 1400;

struct input_t {
	int up, down, left, right;
	int x, y;
	int key;
	double ax, ay, az;
};

struct dot {
	int x, y, visible;
};

struct player_t {
	int attend;
	char name[20];
	int x, y;
	double ax, ay, az;
	dot dots[max_dots];
	int curDots;
};

struct Scene {
	char c[2];
	struct tm tm;
	int id;
	player_t p[max_players];
};

extern Gtk::Statusbar *statusBar;
extern int statusId, statusEraseId;
//extern Scene *scene;
extern input_t input[max_players];

gboolean eraseStatusbar(void *p);
gboolean tickServer(void *p);
void process_a_step(Scene *s, input_t *in);

#endif /* COMMON_H_ */
