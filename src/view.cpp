#include <iostream>
#include "view.h"
#include "manager.h"
#include "network.h"

MyDrawingArea::MyDrawingArea(BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& g):
	Gtk::DrawingArea(o){
	scene=NULL;
	clearInput();
	input.up=0;
	input.down=0;
	input.left=0;
	input.right=0;
}

MyDrawingArea::~MyDrawingArea(void){
}

void MyDrawingArea::on_realize(void){
	Gtk::DrawingArea::on_realize();
}

bool MyDrawingArea::on_expose_event( GdkEventExpose* e ){
	std::cout << "Exposed" << std::endl;

	if(scene==NULL)return true;
	Cairo::RefPtr<Cairo::Context> cc = this->get_window()->create_cairo_context();

	int ls=fmin(this->get_width()*0.5, this->get_height()*0.5);
	int lm=fmin(this->get_width()*0.4, this->get_height()*0.4);
	int lh=fmin(this->get_width()*0.3, this->get_height()*0.3);

	cc->set_line_width(1.0);
	cc->set_source_rgb(0, 0, 0);
	for(int i=0; i<12; ++i){
		cc->move_to(ls+lm*sin(2.0*M_PI*i/12), ls-lm*cos(2.0*M_PI*i/12));
		cc->line_to(ls+ls*sin(2.0*M_PI*i/12), ls-ls*cos(2.0*M_PI*i/12));
		cc->stroke();
	}

	cc->set_line_width(1.0);
	cc->set_source_rgb(0.2, 0.2, 1.0);
	cc->move_to(ls, ls);
	cc->line_to(ls+ls*sin(2.0*M_PI*scene->tm.tm_sec/60), ls-ls*cos(2.0*M_PI*scene->tm.tm_sec/60));
	cc->stroke();

	cc->set_font_size(30);
	cc->move_to(ls+ls*sin(2.0*M_PI*scene->tm.tm_sec/60), ls-ls*cos(2.0*M_PI*scene->tm.tm_sec/60));
	cc->show_text(std::string(scene->c));

	cc->set_line_width(3.0);
	cc->set_source_rgb(0.2, 0.2, 1.0);
	cc->move_to(ls, ls);
	cc->line_to(ls+lm*sin(2.0*M_PI*(scene->tm.tm_min/60.0+scene->tm.tm_sec/3600.0)),
			ls-lm*cos(2.0*M_PI*(scene->tm.tm_min/60.0+scene->tm.tm_sec/3600.0)));
	cc->stroke();

	cc->set_line_width(5.0);
	cc->set_source_rgb(0.2, 0.2, 1.0);
	cc->move_to(ls, ls);
	cc->line_to(ls+lh*sin(2.0*M_PI*(scene->tm.tm_hour/12.0+scene->tm.tm_min/720.0)),
			ls-lh*cos(2.0*M_PI*(scene->tm.tm_hour/12.0+scene->tm.tm_min/720.0)));
	cc->stroke();

	cc->set_line_width(5.0);
	cc->set_source_rgb(1.0, 0.0, 0.0);
	for(int i=0; i<max_players; ++i){
		cc->set_source_rgb(((i+1)&1)>0, ((i+1)&2)>0, ((i+1)&4)>0);
		if(scene->p[i].attend){
			for(int j=0; j<max_dots; ++j){
/*			if(scene->p[i].dots[j].visible==1){
					cc->arc(scene->p[i].dots[j].x, scene->p[i].dots[j].y, 5, 0, 2.0 * M_PI);
					cc->stroke();
				}
*/			}
		}
	}

	return true;
}

void MyDrawingArea::setScene(Scene *s){
	scene=s;
}

void MyDrawingArea::update(){
	this->queue_draw();
}

void MyDrawingArea::clearInput(void){
	input.x=-1;
	input.y=-1;
	input.key=0;
}

void MyDrawingArea::getInput(input_t *i){
	*i=input;
	clearInput();
}

// PressイベントとReleaseイベントの両方を見ることで
// 押し続けている状態を把握できるようにしている
bool MyDrawingArea::on_key_press_event(GdkEventKey* k){
//	std::cout << "Pressed " << k->keyval << std::endl;
	switch(k->keyval){
	case GDK_KEY_Up:
		input.up=1;
		break;
	case GDK_KEY_Down:
		input.down=1;
		break;
	case GDK_KEY_Left:
		input.left=1;
		break;
	case GDK_KEY_Right:
		input.right=1;
		break;
	default:
		if(GDK_KEY_A<=k->keyval && k->keyval<=GDK_KEY_z){
			input.key=k->keyval;
		}
	}
	return true;
}

bool MyDrawingArea::on_key_release_event(GdkEventKey* k){
//	std::cout << "Released " << k->keyval << std::endl;
	switch(k->keyval){
	case GDK_KEY_Up:
		input.up=0;
		break;
	case GDK_KEY_Down:
		input.down=0;
		break;
	case GDK_KEY_Left:
		input.left=0;
		break;
	case GDK_KEY_Right:
		input.right=0;
		break;
	default:
		if(GDK_KEY_A<=k->keyval && k->keyval<=GDK_KEY_z){
			input.key=0;
		}
	}
	return true;
}

bool MyDrawingArea::on_button_press_event (GdkEventButton* event){
//	std::cout << "Pressed " << event->x << "," << event->y << std::endl;
	input.x=event->x;
	input.y=event->y;
	return true;
}

MyImageMenuItem::MyImageMenuItem(BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& g):
	Gtk::ImageMenuItem(o){
	g->get_widget("window2", subWindow);
}

MyImageMenuItem::~MyImageMenuItem(void){
}

void MyImageMenuItem::on_activate(void){
	Gtk::ImageMenuItem::on_activate();

	Manager &mgr = Manager::get_instance();
	switch(id){
	case 0:
		if(mgr.get_state() != Manager::Run){
			mgr.set_state(Manager::Run);
			statusBar->push(Glib::ustring("Run"), statusId++);
			g_timeout_add(5000, eraseStatusbar, 0);
			switch(mgr.get_mode()){
			case Manager::Standalone:
				mgr.init_objects();
				mgr.attend_single_player();
				g_timeout_add(period, Manager::tick, NULL);
				break;
			case Manager::Server:
				mgr.init_objects();
				process_cmd(0, SCMD_START, 0, NULL);
				break;
			case Manager::Client:
				client_start();
				break;
			}
		}
		break;
	case 1:
		if(mgr.get_state() != Manager::Stop){
			mgr.set_state(Manager::Stop);
			statusBar->push(Glib::ustring("Stop"), statusId++);
			g_timeout_add(5000, eraseStatusbar, 0);
			switch(mgr.get_mode()){
			case Manager::Server:
				server_stop();
				break;
			case Manager::Client:
				client_stop();
				break;
			default:
				break;
			}
		}
		break;
	case 2:
		if(Manager::get_instance().get_state()==Manager::Stop){
			subWindow->show();
		}
		break;
	case 3:
		exit(0);
	}
}
