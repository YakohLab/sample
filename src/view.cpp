#include <iostream>
#include <cmath>
#include "view.h"
#include "manager.h"
#include "mynetwork.h"
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>

MyDrawingArea *drawingArea;

MyDrawingArea::MyDrawingArea(BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& g):
Gtk::DrawingArea(o){
	Manager &mgr = Manager::getInstance();
	mgr.scene.init();;

#ifdef USE_OPENGL
	gl_config = gdk_gl_config_new_by_mode((GdkGLConfigMode)
			(GDK_GL_MODE_RGBA|GDK_GL_MODE_DEPTH|GDK_GL_MODE_DOUBLE));
	gtk_widget_set_gl_capability(&(o->widget), gl_config, NULL, TRUE,
			GDK_GL_RGBA_TYPE);
#endif
}

MyDrawingArea::~MyDrawingArea(void){
}

void MyDrawingArea::on_realize(void){
	//	std::cout << "Realized" << std::endl;
	Gtk::DrawingArea::on_realize();
}

#if GTKMM3
bool MyDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cc ){
	Gtk::DrawingArea::on_draw(cc);
#else
bool MyDrawingArea::on_expose_event( GdkEventExpose* e ){
	Cairo::RefPtr<Cairo::Context> cc = this->get_window()->create_cairo_context();
	Gtk::DrawingArea::on_expose_event(e);
	Manager &mgr = Manager::getInstance();
	Scene &scene=mgr.scene;

#endif
	if(!scene.valid)return true;
	//	std::cout << "Exposed" << std::endl;

	int ls=fmin(this->get_width()*0.5f, this->get_height()*0.5f);
	int lm=fmin(this->get_width()*0.4f, this->get_height()*0.4f);
	int lh=fmin(this->get_width()*0.25f, this->get_height()*0.25f);

#ifdef USE_OPENGL
	int z=ls/30;
	GdkGLContext *gl_context = gtk_widget_get_gl_context((GtkWidget *)this->gobj());
	GdkGLDrawable *gl_drawable = gtk_widget_get_gl_drawable((GtkWidget *)this->gobj());

	gdk_gl_drawable_gl_begin(gl_drawable, gl_context);
	glViewport(0, 0, this->get_width(), this->get_height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, 1, ls/10, 5.0*ls);
	gluLookAt(0, 3.0*ls, 2.0*ls, 0.0, 0.0, 0.0, 0.0, 9.0, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	GLfloat position[4]={-(GLfloat)ls, (GLfloat)ls, (GLfloat)ls, (GLfloat)1};
	GLfloat color[4];
	GLUquadricObj *q;

	glLightfv(GL_LIGHT0, GL_POSITION, position);
//	color[0]=0.0; color[1]=0.0; color[2]=0.0; color[3]=1.0;
//	glLightfv(GL_LIGHT0, GL_AMBIENT, color);
	color[0]=0.7; color[1]=0.7; color[2]=0.7; color[3]=1.0;
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
//	color[0]=1.0; color[1]=1.0; color[2]=1.0; color[3]=1.0;
//	glLightfv(GL_LIGHT0, GL_SPECULAR, color);
	q = gluNewQuadric();

	//	color[0]=0.2; color[1]=0.2; color[2]=0.2; color[3]=1.0;
	//	glMaterialfv(GL_FRONT, GL_AMBIENT, color);
	//	color[0]=0.8; color[1]=0.8; color[2]=0.8; color[3]=1.0;
	//	glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
	color[0]=1.0; color[1]=1.0; color[2]=1.0; color[3]=0.0;
	glMaterialfv(GL_FRONT, GL_SPECULAR, color);
	color[0]=128.0;
	glMaterialfv(GL_FRONT, GL_SHININESS, color);
	color[0]=0; color[1]=0; color[2]=0; color[3]=1.0;
	glMaterialfv(GL_FRONT, GL_EMISSION, color);

	color[0]=0.8; color[1]=0.8; color[2]=0.8; color[3]=1.0;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, -z/2, 0);
	glRotated(270, 1, 0, 0);
	gluDisk(q, 0, ls, 120, 10);
	glPopMatrix();

	for(int i=0; i<12; ++i){
		glPushMatrix();
		glRotated(-30*i+180, 0.0, 1.0, 0.0);
		glTranslated(0, -z/2, lm);
		gluCylinder(q, z, z, ls-lm, 10, 10);
		glPopMatrix();
	}

	color[0]=0.0; color[1]=0.0; color[2]=1.0; color[3]=0.8;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, z, 0);
	glRotated(-6.0*scene.tm.tm_sec+180, 0.0, 1.0, 0.0);
	gluCylinder(q, z/2, z/3, ls, 10, 10);
	glPopMatrix();

	color[0]=0.0; color[1]=1.0; color[2]=0.0; color[3]=0.8;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, 2*z, 0);
	glRotated(-6.0*(scene.tm.tm_min+scene.tm.tm_sec/60.0)+180, 0.0, 1.0, 0.0);
	gluCylinder(q, z/2, z/3, lm, 10, 10);
	glPopMatrix();

	color[0]=1.0; color[1]=0.0; color[2]=0.0; color[3]=0.8;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, 3*z, 0);
	glRotated(-30.0*(scene.tm.tm_hour+scene.tm.tm_min/60.0)+180, 0.0, 1.0, 0.0);
	gluCylinder(q, z/2, z/3, lh, 10, 10);
	glPopMatrix();

	for(std::map<int, Player>::iterator p=scene.p.begin(); p!=scene.p.end(); ++p){
		color[0]=((p->first+1)&1)>0; color[1]=((p->first+1)&2)>0; color[2]=((p->first+1)&4)>0; color[3]=1.0;
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		for(int j=0; j<max_dots; ++j){
			if(p->second.dots[j].visible==1){
				glPushMatrix();
				glTranslated((p->second.dots[j].x-this->get_width()/2)/2, 0, (p->second.dots[j].y-this->get_height()/2)/2);
				glRotated(270, 1, 0, 0);
				gluDisk(q, z, 2*z, 120, 10);
				glPopMatrix();
			}
		}
	}

	color[0]=0.8; color[1]=0.8; color[2]=0.8; color[3]=0.8;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glCullFace(GL_FRONT);
	glPushMatrix();
	double roll=atan2(scene.p[0].ax, scene.p[0].az)*180/M_PI;
	double pitch=atan2(scene.p[0].ay, scene.p[0].az)*180/M_PI;
	glRotated(pitch, 1.0, 0.0, 0.0);
	glRotated(roll, 0.0, 0.0, 1.0);
	gdk_gl_draw_teapot(true, lh/4);
	glPopMatrix();

	if (gdk_gl_drawable_is_double_buffered(gl_drawable)){
		gdk_gl_drawable_swap_buffers(gl_drawable);
	}else{
		glFlush();
	}
	gdk_gl_drawable_gl_end(gl_drawable);
#else
	cc->set_source_rgb(0.8, 0.8, 0.8);
	cc->paint();
	cc->set_line_width(1.0);
	cc->set_source_rgb(0, 0, 0);
	for(int i=0; i<12; ++i){
		cc->move_to((double)(ls+lm*sin(2.0*M_PI*i/12)), (double)(ls-lm*cos(2.0*M_PI*i/12)));
		cc->line_to((double)(ls+ls*sin(2.0*M_PI*i/12)), (double)(ls-ls*cos(2.0*M_PI*i/12)));
		cc->stroke();
	}

	cc->set_line_width(3.0); // second hand
	cc->set_source_rgb(0.0, 0.0, 1.0);
	cc->move_to(ls, ls);
	cc->line_to((double)(ls+ls*sin(2.0*M_PI*scene.tm.tm_sec/60)),
			(double)(ls-ls*cos(2.0*M_PI*scene.tm.tm_sec/60)));
	cc->stroke();

	cc->set_font_size(30);
	cc->move_to((double)(ls+ls*sin(2.0*M_PI*scene.tm.tm_sec/60)),
			(double)(ls-ls*cos(2.0*M_PI*scene.tm.tm_sec/60)));
	cc->show_text(std::string(scene.c).c_str());

	cc->set_line_width(8.0); // minute hand
	cc->set_source_rgb(0.0, 1.0, 0.0);
	cc->move_to(ls, ls);
	cc->line_to((double)(ls+lm*sin(2.0*M_PI*(scene.tm.tm_min/60.0+scene.tm.tm_sec/3600.0))),
			(double)(ls-lm*cos(2.0*M_PI*(scene.tm.tm_min/60.0+scene.tm.tm_sec/3600.0))));
	cc->stroke();

	cc->set_line_width(10.0); // hour hand
	cc->set_source_rgb(1.0, 0.0, 0.0);
	cc->move_to(ls, ls);
	cc->line_to((double)(ls+lh*sin(2.0*M_PI*(scene.tm.tm_hour/12.0+scene.tm.tm_min/720.0))),
			(double)(ls-lh*cos(2.0*M_PI*(scene.tm.tm_hour/12.0+scene.tm.tm_min/720.0))));
	cc->stroke();

	cc->set_line_width(5.0);
	cc->set_source_rgb(1.0, 0.0, 0.0);
	for(std::map<int, Player>::iterator p=scene.p.begin(); p!=scene.p.end(); ++p){
		cc->set_source_rgb(((p->first+1)&1)>0, ((p->first+1)&2)>0, ((p->first+1)&4)>0);
		for(int j=0; j<max_dots; ++j){
			if(p->second.dots[j].visible==1){
				cc->arc((double)p->second.dots[j].x, (double)p->second.dots[j].y,
						5.0, 0.0, (double)(2.0*M_PI));
				cc->stroke();
			}
		}
	}
#endif
	scene.valid=false;
	return true;
}

void MyDrawingArea::update(){
	this->queue_draw();
}

// PressイベントとReleaseイベントの両方を見ることで
// 押し続けている状態を把握できるようにできるが、
// これらは即時性のない入力なので使わない。
// 替わりに毎回checkInputを呼び出して、類似のイベントを発生するようにした。
bool MyDrawingArea::on_key_press_event(GdkEventKey* k){
	//	std::cout << "Pressed " << k->keyval << std::endl;
//	Input &input=Input::getInstance();
//	input.set_key(k);
	return true;
}

bool MyDrawingArea::on_key_release_event(GdkEventKey* k){
	//	std::cout << "Released " << k->keyval << std::endl;
//	Input &input=Input::getInstance();
//	input.reset_key(k);
	return true;
}

bool MyDrawingArea::on_button_press_event (GdkEventButton* event){
	//	std::cout << "Pressed " << event->x << "," << event->y << std::endl;
	Input &input=Input::getInstance();
	input.set_input(event->x, event->y);
	return true;
}

MyImageMenuItem::MyImageMenuItem(BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& g):
					Gtk::ImageMenuItem(o){
	g->get_widget("window2", subWindow);
	g->get_widget("window3", fileWindow);
	menuId=-1;
}

MyImageMenuItem::~MyImageMenuItem(void){
}

void MyImageMenuItem::on_activate(void){
	Gtk::ImageMenuItem::on_activate();

	Manager &mgr = Manager::getInstance();
	MyNetwork &net=MyNetwork::getInstance();

	switch(menuId){
	case 0:
		if(mgr.get_state() != Manager::Run){
			mgr.set_state(Manager::Run);
			statusBar->push(Glib::ustring("Run"), statusId++);
			g_timeout_add(5000, eraseStatusbar, 0);
			switch(mgr.get_mode()){
			case Manager::Standalone:
				mgr.init_objects();
				mgr.startStandaloneTick();
				break;
			case Manager::Server:
				mgr.init_objects();
				net.runServer();
				break;
			case Manager::Client:
				net.runClient();
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
				net.stopServer();
				break;
			case Manager::Client:
				net.stopClient();
				break;
			default:
				break;
			}
		}
		break;
	case 2:
		if(Manager::getInstance().get_state()==Manager::Stop){
			subWindow->show();
		}
		break;
	case 3:
		fileWindow->show();
		break;
	case 4:
		net.disconnect();
		net.closeServer();
		exit(0);
	}
}

void ViewManager::checkInput(void){ // 自分の入力を与える
	Input &input = Input::getInstance();
	guint *keyval;
	gint n;
	char keys[32];
	int i, j, k;
	KeyCode keycode;
	GdkEventKey e;
	Display* d=GDK_WINDOW_XDISPLAY(Glib::unwrap(drawingArea->get_window()));
	XQueryKeymap(d, keys);
	for(i=0; i<32; ++i){
		if(keys[i]!=0){
			for(j=0; j<8; ++j){
				if(keys[i] & 1<<j){
					keycode=i*8+j;
					if(gdk_keymap_get_entries_for_keycode(NULL, keycode, NULL, &keyval, &n)){
						// std::cout << n << ": ";
						for(k=0; k<n; ++k){
							e.keyval=keyval[k];
							input.set_key(&e);
							// std::cout << std::hex << e.keyval << ", ";
						}
						g_free(keyval);
						// std::cout << std::endl;
					}
				}
			}
		}
	}
}
