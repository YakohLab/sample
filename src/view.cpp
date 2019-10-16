/*
 * view.cpp
 * 画面表示を司る関数群
 */
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include "view.h"
#include "manager.h"
#include "mynetwork.h"

#if !GTKMM3
const int ss_divisor = 3; // frames
#endif

void MyDrawingArea::on_realize(void) {
//		std::cout << "Realized" << std::endl;
	Gtk::DrawingArea::on_realize();
	Gtk::DrawingArea::set_size_request(800, 600);
}

#if GTKMM3
bool MyDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cc ) {
	Gtk::DrawingArea::on_draw(cc);
	Manager &mgr = Manager::getInstance();
	Scene &scene=mgr.scene;
#else
bool MyDrawingArea::on_expose_event(GdkEventExpose* e) {
	Cairo::RefPtr<Cairo::Context> cc =
			this->get_window()->create_cairo_context();
	Gtk::DrawingArea::on_expose_event(e);
	Manager &mgr = Manager::getInstance();
	Scene &scene = mgr.scene;
#endif

	if (!scene.valid) {
		cc->set_source_rgb(0.8, 0.8, 0.8);
		cc->paint();
		return true;
	}

	//	std::cout << "Exposed" << std::endl;
	int ls = (int) fmin(this->get_width() * 0.5, this->get_height() * 0.5);
	int lm = (int) fmin(this->get_width() * 0.4, this->get_height() * 0.4);
	int lh = (int) fmin(this->get_width() * 0.25, this->get_height() * 0.25);

	cc->set_source_rgb(0.8, 0.8, 0.8);
	cc->paint();
	cc->set_line_width(1.0);
	cc->set_source_rgb(0, 0, 0);
	for(int i=0; i<12; ++i) {
		cc->move_to((double)(ls+lm*sin(2.0*M_PI*i/12)), (double)(ls-lm*cos(2.0*M_PI*i/12)));
		cc->line_to((double)(ls+ls*sin(2.0*M_PI*i/12)), (double)(ls-ls*cos(2.0*M_PI*i/12)));
		cc->stroke();
	}

	cc->set_font_size(16);
	cc->move_to((double)(ls+ls*sin(2.0*M_PI*scene.tm.tm_sec/60)),
			(double)(ls-ls*cos(2.0*M_PI*scene.tm.tm_sec/60)));
	cc->show_text(std::string(scene.c).c_str());
	cc->move_to(4, 20);
	if(scene.id==-1) {
		cc->show_text(std::string("Standalone"));
	} else if(scene.id==0) {
		cc->show_text(std::string("Server"));
	} else {
		std::stringstream ss;
		ss << scene.id;
		cc->show_text(std::string("Client #")+ss.str());
	}

	cc->set_line_width(3.0); // second hand
	cc->set_source_rgb(0.0, 0.0, 1.0);
	cc->move_to(ls, ls);
	cc->line_to((double)(ls+ls*sin(2.0*M_PI*scene.tm.tm_sec/60)),
			(double)(ls-ls*cos(2.0*M_PI*scene.tm.tm_sec/60)));
	cc->stroke();

	cc->set_line_width(8.0);// minute hand
	cc->set_source_rgb(0.0, 1.0, 0.0);
	cc->move_to(ls, ls);
	cc->line_to((double)(ls+lm*sin(2.0*M_PI*(scene.tm.tm_min/60.0+scene.tm.tm_sec/3600.0))),
			(double)(ls-lm*cos(2.0*M_PI*(scene.tm.tm_min/60.0+scene.tm.tm_sec/3600.0))));
	cc->stroke();

	cc->set_line_width(10.0);// hour hand
	cc->set_source_rgb(1.0, 0.0, 0.0);
	cc->move_to(ls, ls);
	cc->line_to((double)(ls+lh*sin(2.0*M_PI*(scene.tm.tm_hour/12.0+scene.tm.tm_min/720.0))),
			(double)(ls-lh*cos(2.0*M_PI*(scene.tm.tm_hour/12.0+scene.tm.tm_min/720.0))));
	cc->stroke();

	cc->set_line_width(5.0);
	cc->set_source_rgb(1.0, 0.0, 0.0);
	for(Players::iterator it=scene.p.begin(); it!=scene.p.end(); ++it) {
		Player &p=it->second;
		cc->set_source_rgb(p.r, p.g, p.b);
		for(int j=0; j<max_dots; ++j) {
			if(p.dots[j].visible==1) {
				cc->arc((double)p.dots[j].x, (double)p.dots[j].y,
						5.0, 0.0, (double)(2.0*M_PI));
				cc->stroke();
			}
		}
	}
	scene.valid = false;

	return true;
}

void MyDrawingArea::update(void) {
	this->queue_draw();
#ifdef USE_OPENGL
	ViewManager &vmr = ViewManager::getInstance();
	vmr.glArea->queue_render();
	vmr.glArea->queue_draw();
#endif
}

// PressイベントとReleaseイベントの両方を見ることで
// 押し続けている状態を把握できるようにできる
bool MyDrawingArea::on_key_press_event(GdkEventKey* k) {
	Input &input = Input::getInstance();
	input.set_key(k);
	return false;
}

bool MyDrawingArea::on_key_release_event(GdkEventKey* k) {
	Input &input = Input::getInstance();
	input.reset_key(k);
	return true;
}

bool MyDrawingArea::on_button_press_event(GdkEventButton* event) {
	Input &input = Input::getInstance();
	input.set_input((int) event->x, (int) event->y);
	return true;
}

MyImageMenuItem::MyImageMenuItem(BaseObjectType* o,
		const Glib::RefPtr<Gtk::Builder>& g) :
		Gtk::ImageMenuItem(o) {
	menuId = -1;
}

void MyImageMenuItem::on_activate(void) {
	Gtk::ImageMenuItem::on_activate();

	Manager &mgr = Manager::getInstance();
	MyNetwork &net = MyNetwork::getInstance();
	ViewManager &vmr = ViewManager::getInstance();

	switch (menuId) {
	case 0:
		vmr.menu[0]->set_sensitive(false);
		vmr.menu[1]->set_sensitive(true);
		vmr.menu[2]->set_sensitive(false);
		vmr.menu[4]->set_sensitive(false);
		mgr.set_state(Manager::Run);
		vmr.push(std::string("Run"));
		switch (mgr.get_mode()) {
		case Manager::Standalone:
			mgr.init_objects();
			mgr.startStandaloneTick(vmr.name->get_text());
			break;
		case Manager::Server:
			mgr.init_objects();
			net.runServer();
			break;
		case Manager::Client:
			net.runClient();
			break;
		}
		break;
	case 1:
		vmr.menu[0]->set_sensitive(true);
		vmr.menu[1]->set_sensitive(false);
		vmr.menu[2]->set_sensitive(true);
		vmr.menu[4]->set_sensitive(true);
		mgr.set_state(Manager::Stop);
		vmr.push(std::string("Stop"));
		switch (mgr.get_mode()) {
		case Manager::Server:
			net.stopServer();
			break;
		case Manager::Client:
			net.stopClient();
			break;
		default:
			break;
		}
		break;
	case 2:
		vmr.subWindow->show();
		break;
	case 3:
		vmr.chooser->show();
		break;
	case 4:
		net.disconnect();
		net.closeServer();
		exit(0);
	}
}

MyStatusbar::MyStatusbar(BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& g) :
		Gtk::Statusbar(o) {
	statusId = 0;
}

void MyStatusbar::pushTemp(std::string s) {
	push(s, statusId);
	sigc::slot<bool> slot = sigc::bind(
			sigc::mem_fun(*this, &MyStatusbar::erase), statusId);
	Glib::signal_timeout().connect(slot, 5000);
	statusId++;
}

bool MyStatusbar::erase(int i) {
	pop(i);
	return false;
}

void ViewManager::subCancel(void) {
	chooser->hide();
}

void ViewManager::subSend(void) {
	MySmartphone &smapho = MySmartphone::getInstance();
	if (smapho.isConnected()) {
		smapho.sendImage((const char *) (chooser->get_filename().c_str()));
	}
	chooser->hide();
}

void ViewManager::subHide(void) {
	Manager &mgr = Manager::getInstance();
	MyNetwork &net = MyNetwork::getInstance();

	if (server->get_active() && mgr.get_mode() != Manager::Server) {
		net.disconnect();
		if (net.startServer(
				(short unsigned int) std::atoi(sport->get_text().c_str()),
				name->get_text().c_str())) {
			mgr.set_mode(Manager::Server);
		} else {
			mgr.set_mode(Manager::Standalone);
		}
	} else if (client->get_active() && mgr.get_mode() != Manager::Client) {
		net.closeServer();

		if (net.connectClient(cip->get_text().c_str(),
				(short unsigned int) std::atoi(cport->get_text().c_str()),
				name->get_text().c_str())) {
			mgr.set_mode(Manager::Client);
		} else {
			mgr.set_mode(Manager::Standalone);
		}
	} else {
		net.closeServer();
		net.disconnect();
		mgr.set_mode(Manager::Standalone);
	}
	switch (mgr.get_mode()) {
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
}

Gtk::Window *ViewManager::init(Glib::RefPtr<Gtk::Builder> builder) {
	builder->get_widget("window1", mainWindow);
	builder->get_widget("window2", subWindow);
	builder->get_widget("window3", chooser);
	builder->get_widget("button1", ok);
	ok->signal_clicked().connect(sigc::mem_fun0(*this, &ViewManager::subHide));
	builder->get_widget("button2", ok);
	ok->signal_clicked().connect(
			sigc::mem_fun0(*this, &ViewManager::subCancel));
	builder->get_widget("button3", ok);
	ok->signal_clicked().connect(sigc::mem_fun0(*this, &ViewManager::subSend));
	builder->get_widget("sip", sip);
	builder->get_widget("sport", sport);
	builder->get_widget("cip", cip);
	builder->get_widget("cport", cport);
	builder->get_widget("name", name);
	builder->get_widget("standalone", standalone);
	builder->get_widget("server", server);
	builder->get_widget("client", client);
	builder->get_widget_derived("drawingarea1", drawingArea);
	builder->get_widget_derived("drawingarea2", glArea);
	builder->get_widget_derived("statusbar1", statusbar);
	builder->get_widget_derived("Start", menu[0]);
	builder->get_widget_derived("Stop", menu[1]);
	menu[1]->set_sensitive(false);
	builder->get_widget_derived("SetMode", menu[2]);
	builder->get_widget_derived("SendImage", menu[3]);
	menu[3]->set_sensitive(false);
	builder->get_widget_derived("Quit", menu[4]);
	for (int i = 0; i < 5; ++i) {
		menu[i]->menuId = i;
	}
	return mainWindow;
}
