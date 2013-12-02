#include <iostream>
#include <gtkmm.h>
#include "view.h"

using namespace std;
#define UI_FILE "glade.ui"

Gtk::Window *mainWindow;
MyDrawingArea *drawingArea;
Gtk::Statusbar *statusBar;

gboolean tick(void *p){
    cout << "Tick" << endl;
    drawingArea->update();
    return true;
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

	g_timeout_add(1000, tick, NULL);

	kit.run(*(mainWindow));
	return 0;
}
