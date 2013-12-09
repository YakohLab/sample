#include <iostream>
#include <gtkmm.h>
#include "view.h"
#include "model.h"
#include "network.h"

using namespace std;
#define UI_FILE "glade.ui"

enum state_t { Run, Stop } state;
enum comm_t { Server, Client, Standalone } comm;

class MyImageMenuItem : public Gtk::ImageMenuItem {
public:
	MyImageMenuItem(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
	virtual ~MyImageMenuItem();
	int id;
protected:
	virtual void on_activate();
};

static Gtk::Window *mainWindow, *subWindow;
static MyDrawingArea *drawingArea;
static Gtk::Entry *sip, *sport, *cip, *cport, *name;
static Gtk::RadioButton *standalone, *server, *client;
static Gtk::Button *ok;
static MyImageMenuItem *menu[4];
static Model *model;
static input_t input[max_players];

Gtk::Statusbar *statusBar;
int statusId, statusEraseId;
scene_t *scene;

gboolean tick(void *p){
//    cout << "Tick" << endl;
	drawingArea->getInput(&input[0]);
	model->preAction();
	model->stepPlayer(0, &input[0]);
	model->postAction();
    drawingArea->setScene(scene);
    if(state==Run){ // trueを返すとタイマーを再設定し、falseならタイマーを停止する
    	return true;
    }else{
	    drawingArea->setScene(NULL);
		delete scene;
		scene=NULL;
    	return false;
    }
}

gboolean tickServer(void *p){
	drawingArea->getInput(&input[0]); // 他のプレーヤーの入力は、通信で非同期に届いている
	model->preAction();
	for(int i=0; i<max_players; ++i){
		if(scene->p[i].attend){
			model->stepPlayer(i, &input[i]);
		}
	}
	model->postAction();
	for(int i=1; i<max_players; ++i){ // 自分には送る必要ないので1から
		if(scene->p[i].attend){
			sendScene(i, scene);
		}
	}
	return true;
}

void process_a_step(scene_t *s, input_t *in) {
	drawingArea->setScene(s);
	drawingArea->getInput(in);
}

gboolean eraseStatusbar(void *p){
    statusBar->pop(statusEraseId++);
    return false;
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
		if(state!=Run){
			state=Run;
			switch(comm){
			case Standalone:
				scene=new scene_t;
				model->initModel(scene);
				scene->p[0].attend=1;
				g_timeout_add(period, tick, NULL);
				statusBar->push(Glib::ustring("Run"), statusId++);
				g_timeout_add(5000, eraseStatusbar, 0);
				break;
			case Server:
				scene=new scene_t;
				model->initModel(scene);
				scene->p[0].attend=1;
				process_cmd(0, SCMD_START, 0, NULL);
				break;
			case Client:
				break;
			}
		}
		break;
	case 1:
		if(state!=Stop){
			state=Stop;
			statusBar->push(Glib::ustring("Stop"), statusId++);
			g_timeout_add(5000, eraseStatusbar, 0);
		}
		break;
	case 2:
		if(state==Stop){
			subWindow->show();
		}
		break;
	case 3:
		exit(0);
	}
}

void subHide(void){
//	cout << "name=" << name->get_text() << endl;
	if(server->get_active()){
		comm=Server;
//		cout << sip->get_text() << ":" << sport->get_text() << endl;
		if(server_setup(sport->get_text().c_str(), name->get_text().c_str())){
			comm=Standalone;
		}
	}else if(client->get_active()){
		comm=Client;
//		cout << cip->get_text() << ":" << cport->get_text() << endl;
		if(client_setup(cip->get_text().c_str(), cport->get_text().c_str(),
				name->get_text().c_str())==false){
			comm=Standalone;
		}
	}else{
		comm=Standalone;
	}
	subWindow->hide();
}

int main(int argc, char *argv[]){
	state=Stop;
	comm=Standalone;
	model=new Model;
	statusId=statusEraseId=0;
	Gtk::Main kit(argc, argv);
	Glib::RefPtr<Gtk::Builder> builder;
	try {
		builder=Gtk::Builder::create_from_file(UI_FILE);
	}catch(const Glib::FileError &ex){
		cerr << ex.what() << endl;
		return 1;
	}
	builder->get_widget("window1", mainWindow);
	builder->get_widget("window2", subWindow);
	builder->get_widget("button1", ok);
    ok->signal_clicked().connect(
                    sigc::pointer_functor0<void>(subHide));
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
	for(int i=0; i<4; ++i){
		menu[i]->id=i;
	}

	kit.run(*(mainWindow));
	return 0;
}
