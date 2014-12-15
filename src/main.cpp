#include <iostream>
#include <cmath>
#include <string>
#include <gtkmm.h>

#include "input.h"
#include "view.h"
#include "model.h"
#include "mynetwork.h"
#include "manager.h"
using namespace std;

#define UI_FILE "glade.ui"

static Gtk::Window *mainWindow, *subWindow;
static Gtk::FileChooserDialog *chooser;
static Gtk::Entry *sip, *sport, *cip, *cport, *name;
static Gtk::RadioButton *standalone, *server, *client;
static Gtk::Button *ok;
static MyImageMenuItem *menu[5];
static Model *model;

Gtk::Statusbar *statusBar;
int statusId, statusEraseId;

gboolean eraseStatusbar(void *p) {
	statusBar->pop(statusEraseId++);
	return false;
}

void subCancel(void){
	chooser->hide();
}

void subSend(void){
	MySmartphone &smapho = MySmartphone::getInstance();
	if(smapho.isConnected()){
		smapho.sendImage((const char *)(chooser->get_filename().c_str()));
	}
	chooser->hide();
}

void subHide(void) {
	Manager &mgr = Manager::getInstance();
	MyNetwork &net=MyNetwork::getInstance();
	MySmartphone &smapho = MySmartphone::getInstance();

	if (server->get_active() && mgr.get_mode()!=Manager::Server){
		net.disconnect();
		if(net.startServer(std::atoi(sport->get_text().c_str()), name->get_text().c_str())){
			mgr.set_mode(Manager::Server);
		}else{
			mgr.set_mode(Manager::Standalone);
		}
	} else if (client->get_active() && mgr.get_mode()!=Manager::Client){
		net.closeServer();

		if(net.connectClient(cip->get_text().c_str(), std::atoi(cport->get_text().c_str()),
				name->get_text().c_str())){
			mgr.set_mode(Manager::Client);
		}else{
			mgr.set_mode(Manager::Standalone);
		}
	} else {
		net.closeServer();
		net.disconnect();
		mgr.set_mode(Manager::Standalone);
	}
	switch(mgr.get_mode()){
	case Manager::Standalone:
		standalone->set_active();
		break;
	case Manager::Server:
		server->set_active();
		break;
	case Manager::Client:
		client->set_active();
		break;
	}
	subWindow->hide();
	smapho.open(8888);
}


int main(int argc, char *argv[]) {
	Gtk::Main kit(argc, argv);
#ifdef USE_OPENGL
	gdk_gl_init(&argc, &argv);
#endif
	Manager &mgr = Manager::getInstance();
	mgr.init_status();
	model=new Model;
	Glib::RefPtr<Gtk::Builder> builder;
	try {
		builder = Gtk::Builder::create_from_file(UI_FILE);
	} catch (const Glib::FileError &ex) {
		cerr << ex.what() << endl;
		return 1;
	}
	builder->get_widget("window1", mainWindow);
	builder->get_widget("window2", subWindow);
	builder->get_widget("window3", chooser);
	builder->get_widget("button1", ok);
	ok->signal_clicked().connect(sigc::pointer_functor0<void>(subHide));
	builder->get_widget("button2", ok);
	ok->signal_clicked().connect(sigc::pointer_functor0<void>(subCancel));
	builder->get_widget("button3", ok);
	ok->signal_clicked().connect(sigc::pointer_functor0<void>(subSend));
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
	builder->get_widget_derived("SendImage", menu[3]);
	builder->get_widget_derived("Quit", menu[4]);
	for (int i = 0; i < 5; ++i) {
		menu[i]->menuId = i;
	}

	ViewManager::getInstance().init_view(drawingArea);

	kit.run(*(mainWindow));
	return 0;
}
