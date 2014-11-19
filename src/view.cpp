#include <iostream>
#include <cmath>
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
	input.ax=0;
	input.ay=0;
	input.az=9.8;
#ifdef USE_OPENGL
	gl_config = gdk_gl_config_new_by_mode((GdkGLConfigMode)
			(GDK_GL_MODE_RGBA|GDK_GL_MODE_DEPTH|GDK_GL_MODE_DOUBLE));
	gtk_widget_set_gl_capability(&(o->widget), gl_config, NULL, TRUE,
			GDK_GL_RGBA_TYPE);
#endif
}

MyDrawingArea::~MyDrawingArea(void){
}

void MyDrawingArea::set_input(int x, int y){
	input.x=x;
	input.y=y;
}

void MyDrawingArea::set_angle(double ax, double ay, double az){
	input.ax=ax;
	input.ay=ay;
	input.az=az;
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
#endif
	if(scene==NULL)return true;
	//	std::cout << "Exposed" << std::endl;

	int ls=fmin(this->get_width()*0.5, this->get_height()*0.5);
	int lm=fmin(this->get_width()*0.4, this->get_height()*0.4);
	int lh=fmin(this->get_width()*0.3, this->get_height()*0.3);

#ifdef USE_OPENGL
	int z=ls/30;
	GdkGLContext *gl_context = gtk_widget_get_gl_context((GtkWidget *)this->gobj());
	GdkGLDrawable *gl_drawable = gtk_widget_get_gl_drawable((GtkWidget *)this->gobj());

	gdk_gl_drawable_gl_begin(gl_drawable, gl_context);
	glViewport(0, 0, this->get_width(), this->get_height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, 1, ls/10, 5.0*ls);
	//	gluLookAt(0, 2.0*ls, 2.0*ls, 0.0, 0.0, 0.0, scene->p[0].ax, scene->p[0].ay, scene->p[0].az);
	gluLookAt(0, 3.0*ls, 2.0*ls, 0.0, 0.0, 0.0, 0.0, 9.0, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0.0, 0.0, 0.0, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_CULL_FACE);
//	glCullFace(GL_FRONT);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glColor3d(0.2, 0.2, 1.0);
	glLineWidth(1);
	GLfloat position[4]={0, ls, 0, 1};
	GLfloat color[3];
	GLUquadricObj *q;

	glLightfv(GL_LIGHT0, GL_POSITION, position);
	q = gluNewQuadric();

	color[0]=0.8; color[1]=0.8; color[2]=0.8;
	glLightfv(GL_LIGHT0, GL_AMBIENT, color);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
	glLightfv(GL_LIGHT0, GL_SPECULAR, color);

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

	color[0]=0.0; color[1]=0.0; color[2]=1.0;
	glLightfv(GL_LIGHT0, GL_AMBIENT, color);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
	glLightfv(GL_LIGHT0, GL_SPECULAR, color);
	glPushMatrix();
	glTranslated(0, z, 0);
	glRotated(-6.0*scene->tm.tm_sec+180, 0.0, 1.0, 0.0);
	gluCylinder(q, ls/50.0, 0, ls, 10, 10);
	glPopMatrix();

	color[0]=0.0; color[1]=1.0; color[2]=0.0;
	glLightfv(GL_LIGHT0, GL_AMBIENT, color);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
	glLightfv(GL_LIGHT0, GL_SPECULAR, color);
	glPushMatrix();
	glTranslated(0, 2*z, 0);
	glRotated(-6.0*(scene->tm.tm_min+scene->tm.tm_sec/60.0)+180, 0.0, 1.0, 0.0);
	gluCylinder(q, ls/50.0, 0, lm, 10, 10);
	glPopMatrix();

	color[0]=1.0; color[1]=0.0; color[2]=0.0;
	glLightfv(GL_LIGHT0, GL_AMBIENT, color);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
	glLightfv(GL_LIGHT0, GL_SPECULAR, color);
	glPushMatrix();
	glTranslated(0, 3*z, 0);
	glRotated(-30.0*(scene->tm.tm_hour+scene->tm.tm_min/60.0)+180, 0.0, 1.0, 0.0);
	gluCylinder(q, ls/50.0, 0, lh, 10, 10);
	glPopMatrix();

	color[0]=1; color[1]=1; color[2]=1;
	glLightfv(GL_LIGHT0, GL_AMBIENT, color);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
	glLightfv(GL_LIGHT0, GL_SPECULAR, color);

	glPushMatrix();
	double roll=atan2(scene->p[0].ax, scene->p[0].az)*180/M_PI;
	double pitch=atan2(scene->p[0].ay, scene->p[0].az)*180/M_PI;
	glRotated(pitch, 1.0, 0.0, 0.0);
	glRotated(roll, 0.0, 0.0, 1.0);
	gdk_gl_draw_teapot(true, lh/4);
	glPopMatrix();

	for(int i=0; i<max_players; ++i){
		color[0]=((i+1)&1)>0; color[1]=((i+1)&2)>0; color[2]=((i+1)&4)>0;
		glLightfv(GL_LIGHT0, GL_AMBIENT, color);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
		glLightfv(GL_LIGHT0, GL_SPECULAR, color);
		if(scene->p[i].attend){
			for(int j=0; j<max_dots; ++j){
				if(scene->p[i].dots[j].visible==1){
					glPushMatrix();
					glTranslated((scene->p[i].dots[j].x-this->get_width()/2)/2, 0, (scene->p[i].dots[j].y-this->get_height()/2)/2);
					glRotated(270, 1, 0, 0);
					gluDisk(q, z, 2*z, 120, 10);
					glPopMatrix();
				}
			}
		}
	}

	if (gdk_gl_drawable_is_double_buffered(gl_drawable)){
		gdk_gl_drawable_swap_buffers(gl_drawable);
	}else{
		glFlush();
	}
	gdk_gl_drawable_gl_end(gl_drawable);
#else
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
				if(scene->p[i].dots[j].visible==1){
					cc->arc(scene->p[i].dots[j].x, scene->p[i].dots[j].y, 5, 0, 2.0 * M_PI);
					cc->stroke();
				}
			}
		}
	}
#endif
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
	g->get_widget("window3", fileWindow);
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
		fileWindow->show();
		break;
	case 4:
		exit(0);
	}
}
