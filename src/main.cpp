#include <iostream>
#include <gtkmm.h>
#include "view.h"
#include "model.h"
#include "network.h"
#include "manager.h"

using namespace std;
#define UI_FILE "glade.ui"

static Gtk::Window *mainWindow, *subWindow;
static MyDrawingArea *drawingArea;
static Gtk::Entry *sip, *sport, *cip, *cport, *name;
static Gtk::RadioButton *standalone, *server, *client;
static Gtk::Button *ok;
static MyImageMenuItem *menu[4];
static Model *model;
input_t input[max_players];

Gtk::Statusbar *statusBar;
int statusId, statusEraseId;
Scene *scene;

void process_a_step(Scene *s, input_t *in) {
	ViewManager &view =ViewManager::get_instance();
	view.init_view_with_scene(s);
	view.update();
	view.get_input(in);
}

gboolean eraseStatusbar(void *p) {
	statusBar->pop(statusEraseId++);
	return false;
}

void subHide(void) {
//	cout << "name=" << name->get_text() << endl;
	Manager &mgr = Manager::get_instance();

	if (server->get_active()) {
		client_terminate();

		mgr.set_mode(Manager::Server);
		if (!server_setup(sport->get_text().c_str(), name->get_text().c_str(),
				input)) {
			mgr.set_mode(Manager::Standalone);
		}
		scene = new Scene;
		model->initModelWithScene(scene);
		scene->p[0].attend = 1;
	} else if (client->get_active()) {
		server_terminate();
		mgr.set_mode(Manager::Client);
		if (!client_setup(cip->get_text().c_str(), cport->get_text().c_str(),
				name->get_text().c_str())) {
			mgr.set_mode(Manager::Standalone);
		}
	} else {
		server_terminate();
		client_terminate();
		mgr.set_mode(Manager::Standalone);
	}
	subWindow->hide();
}

int main(int argc, char *argv[]) {
	Manager &mgr = Manager::get_instance();
	mgr.init_status();

	model = new Model;
	statusId = statusEraseId = 0;
	Gtk::Main kit(argc, argv);
	Glib::RefPtr<Gtk::Builder> builder;
	try {
		builder = Gtk::Builder::create_from_file(UI_FILE);
	} catch (const Glib::FileError &ex) {
		cerr << ex.what() << endl;
		return 1;
	}
	builder->get_widget("window1", mainWindow);
	builder->get_widget("window2", subWindow);
	builder->get_widget("button1", ok);
	ok->signal_clicked().connect(sigc::pointer_functor0<void>(subHide));
	builder->get_widget("sip", sip);
	builder->get_widget("sport", sport);
	builder->get_widget("cip", cip);
	builder->get_widget("cport", cport);
	builder->get_widget("name", name);
	builder->get_widget("standalone", standalone);
	builder->get_widget("server", server);
	builder->get_widget("client", client);
	builder->get_widget_derived("drawingarea1", drawingArea);
	builder->get_widget("statusbar1", statusBar);
	builder->get_widget_derived("Start", menu[0]);
	builder->get_widget_derived("Stop", menu[1]);
	builder->get_widget_derived("SetMode", menu[2]);
	builder->get_widget_derived("Quit", menu[3]);
	for (int i = 0; i < 4; ++i) {
		menu[i]->id = i;
	}

	ViewManager::get_instance().init_view(drawingArea);

	kit.run(*(mainWindow));
	return 0;
}
