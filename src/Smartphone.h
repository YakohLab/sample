/*
 * Smartphone.h
 * 使い方
 * smartphone.h および smartphone.cpp は、変更してはならない！
 * クラスSmartphoneを継承して、独自の派生クラスを作成すること。
 * コンストラクタは、必ず基底クラスのコンストラクタを呼び出すこと。
 * つまり MySmartphone(int p):Smartphone(p){}; などとすればよい。
 *
 * タッチイベントが届くと、recvBinary()が呼び出される。独自の入力処理を行うため、この関数をオーバーライドすること。
 * 引数はfloatの配列と、その要素数である。配列の最初の要素はタイプで、1, 2, 3, 4のいずれかである。
 * タイプ1, 2, 3はonTouchStart, onTouchMove, onTouchEndに対応し、継続する要素はx, yの対が連続する。
 * タッチを検出した指の本数に依存して要素数が変わるので、要素数を参照すること。
 * タイプ4は加速度センサ値であり、継続する3つの要素はx, y, zの加速度値である。
 *
 * スマホから接続があると、onConnect()が呼び出される。必要に応じてオーバーライドすること。
 *
 * スマホからの接続が切れると、onClose()が呼び出される。必要に応じてオーバーライドすること。
 *
 * スマホの背景は、sendImage()によって変更できる。画像フォーマットは任意、サイズは画面に合わせて自動伸縮される。
 * sendMessage()によって、スマホ画面にメッセージを表示できる。
 *
 * スマホの接続があるかどうかは、isConnect()で確認できる。
 */
#ifndef SMARTPHONE_H
#define SMARTPHONE_H
#include <gtkmm.h>
#include <glibmm.h>

// #define USE_SOCKETSOURCE	// available from glibmm 2.42
// #define USE_SET_OPTION	// available from glibmm 2.36

class Smartphone {
public:
	Smartphone(void);
	bool open(unsigned short port);
	void close(void);
	void sendMessage(char *msg);
	void sendPixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int quality=80);
	bool isConnected(void);
	virtual void onConnect(const char *, int w, int h){};
	virtual void onDisconnect(void){};
	virtual void onRecvMessage(char *msg, unsigned long int n){};
	virtual void onRecvBinary(float *array, unsigned long int n){};
	virtual ~Smartphone(void);
protected:
	int width, height;
private:
	Glib::RefPtr<Gio::Socket> w, s; // wait, server
#ifdef USE_SOCKETSOURCE
	Glib::RefPtr<Gio::SocketSource> ss, ws;
#else
	Glib::RefPtr<Glib::IOSource> ss, ws;
#endif
	unsigned short port;
	bool connected;
	char *keyReply(const char *key);
	std::string ipaddr;
	bool onAccept(Glib::IOCondition condition);
	bool onReceive(Glib::IOCondition condition);
};
#endif
