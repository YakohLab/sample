/*
 * common.h
 *
 *  Created on: 2013/12/09
 *      Author: yakoh
 */

#ifndef COMMON_H_
#define COMMON_H_
#include <time.h>

const int max_dots=5;
const int max_players=4;
const int period=30;

struct input_t {
	int up, down, left, right;
	int x, y;
	int key;
};

struct dot{
	int x, y, visible;
};

struct player_t {
	int attend;
	int x, y;
    dot dots[max_dots];
    int curDots;
};

struct scene_t {
    char c[2];
	struct tm tm;
	player_t p[max_players];
};

#endif /* COMMON_H_ */
