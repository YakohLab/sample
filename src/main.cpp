#include <iostream>
#include <gtkmm.h>
#include "view.h"
#include "model.h"

using namespace std;
#define UI_FILE "glade.ui"

enum state_t { Run, Stop, BeServer, BeClient } state;

class MyImageMenuItem : public Gtk::ImageMenuItem {
public:
	MyImageMenuItem(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
	virtual ~MyImageMenuItem();
	int id;
protected:
	virtual void on_activate();
};

Gtk::Window *mainWindow;
MyDrawingArea *drawingArea;
Gtk::Statusbar *statusBar;
MyImageMenuItem *menu[5];
Model model;

gboolean tick(void *p){
	input_t input;
	scene_t *scene;
//    cout << "Tick" << endl;
	drawingArea->getInput(&input);
	scene=model.doModel(&input);
    drawingArea->setScene(scene);
    if(state==Run){ // trueを返すとタイマーを再設定し、falseならタイマーを停止する
    	return true;
    }else{
    	return false;
    }
}

MyImageMenuItem::MyImageMenuItem(BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& g):
	Gtk::ImageMenuItem(o){
}
MyImageMenuItem::~MyImageMenuItem(void){
}
void MyImageMenuItem::on_activate(void){
	Gtk::ImageMenuItem::on_activate();
	switch(id){
	case 0:
		state=Run;
		g_timeout_add(30, tick, NULL);
		break;
	case 1:
		state=Stop;
		break;
	case 2:
		state=BeServer;
		break;
	case 3:
		state=BeClient;
		break;
	case 4:
		exit(0);
	}
}

int main(int argc, char *argv[]){
	Gtk::Main kit(argc, argv);
	Glib::RefPtr<Gtk::Builder> builder;
	try {
		builder=Gtk::Builder::create_from_file(UI_FILE);
	}catch(const Glib::FileError &ex){
		cerr << ex.what() << endl;
		return 1;
	}
	builder->get_widget("window1", mainWindow);
	builder->get_widget_derived("drawingarea1", drawingArea);
	builder->get_widget("statusbar1", statusBar);
	builder->get_widget_derived("Start", menu[0]);
	builder->get_widget_derived("Stop", menu[1]);
	builder->get_widget_derived("BeServer", menu[2]);
	builder->get_widget_derived("BeClient", menu[3]);
	builder->get_widget_derived("Quit", menu[4]);
	for(int i=0; i<5; ++i){
		menu[i]->id=i;
	}

	kit.run(*(mainWindow));
	return 0;
}
