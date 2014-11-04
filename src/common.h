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
#include <vector>

const int max_dots = 5;
const int max_players = 4;
const int period = 30;
const int max_msglen = 1400;

struct input_t {
	int up, down, left, right;
	int x, y;
	int key;
};

struct dot {
	int x, y, visible;
};

//*********************************************
struct unit{	//ユニットの情報	☆
	int act;	//操作中かどうか
	int x, y;		//座標
	int hp;	//体力

	//以下は非表示情報

	int pw;	//攻撃力
	int move; //移動力
	int weapon; //攻撃方法

};
//**********************************************
struct castle{	 //城(拠点)の情報
		int hp;		//体力
		int x, y;	//座標

};
//**********************************************
struct player_t {		//プレイヤーの情報
	int attend;		//参加、非参加の情報
	char name[20];	//名前
	unit  Unit[10];	//自分のチームのユニット
	castle Castle;	//城

};
//**********************************************
struct field{		//フィールド情報


};

//**********************************************
struct item{
	int kind;	//種類。木箱とか、木とか、動物とか。
	int hp;		//体力・０になると被破壊。


};
//**********************************************
struct stage{
	field Field;
	item Item;


};
//**********************************************

struct Scene {		//サーバーがクライアントに送っている情報
	char c[2];
	struct tm tm;	//時間らしい
	player_t p[max_players];	//参加人数
	stage nowstage;

};
//**********************************************

extern Gtk::Statusbar *statusBar;
extern int statusId, statusEraseId;
//extern Scene *scene;
extern input_t input[max_players];

gboolean eraseStatusbar(void *p);
gboolean tickServer(void *p);
void process_a_step(Scene *s, input_t *in);

#endif /* COMMON_H_ */
