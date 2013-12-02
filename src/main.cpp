#include <iostream>
#include <gtkmm.h>
#include "view.h"

using namespace std;
#define UI_FILE "glade.ui"

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
int state;

gboolean tick(void *p){
    cout << "Tick" << endl;
    drawingArea->update();
    if(state==1){
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
		state=1;
		g_timeout_add(1000, tick, NULL);
		break;
	case 1:
		state=0;
		break;
	case 2:
		state=0;
		break;
	case 3:
		state=0;
		break;
	case 4:
		exit(0);
	}
}

void on_menu_test1(void)
{
	cout << "Called" << endl;
    return;
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
	builder->get_widget_derived("Connect", menu[3]);
	builder->get_widget_derived("Quit", menu[4]);
	for(int i=0; i<5; ++i){
		menu[i]->id=i;
	}

	kit.run(*(mainWindow));
	return 0;
}
