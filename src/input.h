/*
 * input.h
 * 受け付ける入力データを定義するInputDataと、関連するInputクラス
 */
#ifndef INPUT_H_
#define INPUT_H_

// #define INPUT_VERBOSE
#include <vector>
#include <map>
#include <gtkmm.h>

const unsigned long int max_inputlen = 100;

struct InputData { // 要素数を可変にしたい場合は、packInputとreceiveInputを注意深く書き直す必要がある
	int up, down, left, right, dragx, dragy;
	int x, y;
	int key;
	double ax, ay, az, scale, angle, scaleHold, angleHold;
};

class Input {
public:
	static Input& getInstance(void) {
		static Input instance;
		return instance;
	}
	void receiveInput(char *, InputData &data);
	char *packInput(unsigned long int &);
	void clearInput(void);
	void set_key(GdkEventKey* k);
	void reset_key(GdkEventKey* k);
	void set_input(int argx, int argy);
	void set_SmaphoInput(int argx, int argy);
	void set_SmaphoAngle(double, double, double);
	void set_SmaphoPinch(double scale, double angle);
	void set_SmaphoDrag(int, int);
	void set_SmaphoClear(void);
	InputData input;
private:
	Input(void){
		clearInput();
		input.scale=1;
		input.scaleHold=1;
		input.angleHold=0;
	}
	Input(Input&);
	char buffer[max_inputlen];
};

#endif
