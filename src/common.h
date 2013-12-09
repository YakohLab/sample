/*
 * common.h
 *
 *  Created on: 2013/12/09
 *      Author: yakoh
 */

#ifndef COMMON_H_
#define COMMON_H_

const int max_dots=5;

struct input_t {
	int up, down, left, right;
	int x, y;
	int key;
};

struct dot{
	int x, y, visible;
};

struct scene_t {
    dot dots[max_dots];
    int curDots;
    char c[2];
};

#endif /* COMMON_H_ */
