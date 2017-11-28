/*
 * view.cpp
 * 画面表示を司る関数群
 */
#include <iostream>
#include <cmath>
#include <sstream>
#include "view.h"
#include "manager.h"
#include "mynetwork.h"

#if !GTKMM3
const int ss_divisor = 3; // frames
#endif

#ifdef USE_OPENGL
GLuint texname;
Glib::RefPtr<Gdk::Pixbuf> img;
#endif

MyDrawingArea::MyDrawingArea(BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& g):
Gtk::DrawingArea(o){
	MySmartphone &smapho = MySmartphone::getInstance();
	smapho.open(8888);

#ifdef USE_OPENGL
	gl_config = gdk_gl_config_new_by_mode((GdkGLConfigMode)
			(GDK_GL_MODE_RGBA|GDK_GL_MODE_DEPTH|GDK_GL_MODE_DOUBLE));
	gtk_widget_set_gl_capability(&(o->widget), gl_config, NULL, TRUE,
			GDK_GL_RGBA_TYPE);
#endif
}

void MyDrawingArea::on_realize(void){
//		std::cout << "Realized" << std::endl;
	Gtk::DrawingArea::on_realize();
	Gtk::DrawingArea::set_size_request(800, 600);
#ifdef USE_OPENGL
	img = Gdk::Pixbuf::create_from_file("sample.jpg");
#endif
}

#ifdef USE_OPENGL
void showPlayer(int x, int y){
	GLfloat color[4];
	GLUquadricObj *q;
	q = gluNewQuadric();
	color[0]=0.0f; color[1]=0.0f; color[2]=1.0f; color[3]=0.8f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glTranslated(x, y, -100);
	gluCylinder(q, 0, 20, 100, 100, 20);
	gluSphere(q, 10, 100, 100);
	glPopMatrix();
}
#endif

#if GTKMM3
bool MyDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cc ){
	Gtk::DrawingArea::on_draw(cc);
	Manager &mgr = Manager::getInstance();
	Scene &scene=mgr.scene;
#else
bool MyDrawingArea::on_expose_event( GdkEventExpose* e ){
	Cairo::RefPtr<Cairo::Context> cc = this->get_window()->create_cairo_context();
	Gtk::DrawingArea::on_expose_event(e);
	Manager &mgr = Manager::getInstance();
	Scene &scene=mgr.scene;

#endif
	if(!scene.valid){
#ifdef USE_OPENGL
		GdkGLContext *gl_context = gtk_widget_get_gl_context((GtkWidget *)this->gobj());
		GdkGLDrawable *gl_drawable = gtk_widget_get_gl_drawable((GtkWidget *)this->gobj());
		gdk_gl_drawable_gl_begin(gl_drawable, gl_context);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (gdk_gl_drawable_is_double_buffered(gl_drawable)){
			gdk_gl_drawable_swap_buffers(gl_drawable);
		}else{
			glFlush();
		}
		gdk_gl_drawable_gl_end(gl_drawable);
#else
		cc->set_source_rgb(0.8, 0.8, 0.8);
		cc->paint();
#endif
		return true;
	}
	//	std::cout << "Exposed" << std::endl;
	int ls=(int)fmin(this->get_width()*0.5, this->get_height()*0.5);
	int lm=(int)fmin(this->get_width()*0.4, this->get_height()*0.4);
	int lh=(int)fmin(this->get_width()*0.25, this->get_height()*0.25);

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
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
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
//	color[0]=0.7; color[1]=0.7; color[2]=0.7; color[3]=1.0;
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
	color[0]=1.0; color[1]=1.0; color[2]=1.0; color[3]=1.0;
	glLightfv(GL_LIGHT0, GL_SPECULAR, color);
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

	color[0]=0.8f; color[1]=0.8f; color[2]=0.8f; color[3]=1.0f;
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

	color[0]=0.0f; color[1]=0.0f; color[2]=1.0f; color[3]=0.8f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, z, 0);
	glRotated(-6.0*scene.tm.tm_sec+180, 0.0, 1.0, 0.0);
	gluCylinder(q, z/2, z/3, ls, 10, 10);
	glPopMatrix();

	{
		int x, y;
		x=200*sin(2.0*M_PI*scene.tm.tm_sec/60.0);
		y=-200*cos(2.0*M_PI*scene.tm.tm_sec/60.0);
		showPlayer(x, y);
	}

	color[0]=0.0f; color[1]=1.0f; color[2]=0.0f; color[3]=0.8f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, 2*z, 0);
	glRotated(-6.0*(scene.tm.tm_min+scene.tm.tm_sec/60.0)+180, 0.0, 1.0, 0.0);
	gluCylinder(q, z/2, z/3, lm, 10, 10);
	glPopMatrix();

	color[0]=1.0f; color[1]=0.0f; color[2]=0.0f; color[3]=0.8f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, 3*z, 0);
	glRotated(-30.0*(scene.tm.tm_hour+scene.tm.tm_min/60.0)+180, 0.0, 1.0, 0.0);
	gluCylinder(q, z/2, z/3, lh, 10, 10);
	glPopMatrix();


	///画像の読み込み
	color[0]=0.8f; color[1]=1.0f; color[2]=1.0f; color[3]=0.8f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glGenTextures(1,&texname);
	glBindTexture(GL_TEXTURE_2D,texname);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
//	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	if(img->get_n_channels()==3){
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,img->get_width(),img->get_height(),0,GL_RGB,GL_UNSIGNED_BYTE,img->get_pixels());
	}else {
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img->get_width(),img->get_height(),0,GL_RGBA,GL_UNSIGNED_BYTE,img->get_pixels());
	}

	//テクスチャ貼り付け
	glNormal3d(0,1,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,texname);
	glColor3d(0,0,0);
	glBegin(GL_QUADS);
	glTexCoord2d(0,1); glVertex3d(ls/2,10,ls/2);
	glTexCoord2d(0,0); glVertex3d(ls/2,10,0);
	glTexCoord2d(1,0); glVertex3d(0,10,0);
	glTexCoord2d(1,1); glVertex3d(0,10,ls/2);
	glEnd();
	glDisable(GL_TEXTURE_2D);

#ifdef USE_OPENGLUT
	std::string s("SD Experiments F");
	glColor3f(1, 0, 0);
	glRasterPos3i(-120, 1, 20);
	for(unsigned int i=0; i<s.length(); ++i){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s.c_str()[i]);
	}
#endif

	for(Players::iterator p=scene.p.begin(); p!=scene.p.end(); ++p){
		color[0]=p->second.r;
		color[1]=p->second.g;
		color[2]=p->second.b;
		color[3]=1.0;
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

	color[0]=0.8f; color[1]=0.8f; color[2]=0.8f; color[3]=0.8f;
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
#if GTKMM3
#else
	MySmartphone &smapho=MySmartphone::getInstance();
	static int frame=0;
	if((++frame%ss_divisor)==0 && smapho.isConnected() && mgr.get_mode()==Manager::Server && mgr.members.size()==1){
		Glib::RefPtr<Gdk::Pixmap> pixmap=this->get_snapshot();
		Glib::RefPtr<Gdk::Pixbuf> pixbuf=Gdk::Pixbuf::create((Glib::RefPtr<Gdk::Drawable>)pixmap,
				0, 0, this->get_width(), this->get_height());
		pixbuf=pixbuf->scale_simple(smapho.get_width(), smapho.get_height(),  Gdk::INTERP_NEAREST);
		smapho.sendPixbuf((Glib::RefPtr<Gdk::Pixbuf>)pixbuf);
	}
#endif
	cc->set_source_rgb(0.8, 0.8, 0.8);
	cc->paint();
	cc->set_line_width(1.0);
	cc->set_source_rgb(0, 0, 0);
	for(int i=0; i<12; ++i){
		cc->move_to((double)(ls+lm*sin(2.0*M_PI*i/12)), (double)(ls-lm*cos(2.0*M_PI*i/12)));
		cc->line_to((double)(ls+ls*sin(2.0*M_PI*i/12)), (double)(ls-ls*cos(2.0*M_PI*i/12)));
		cc->stroke();
	}

	cc->set_font_size(16);
	cc->move_to((double)(ls+ls*sin(2.0*M_PI*scene.tm.tm_sec/60)),
			(double)(ls-ls*cos(2.0*M_PI*scene.tm.tm_sec/60)));
	cc->show_text(std::string(scene.c).c_str());
	cc->move_to(4, 20);
	if(scene.id==-1){
		cc->show_text(std::string("Standalone"));
	}else if(scene.id==0){
		cc->show_text(std::string("Server"));
	}else{
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
	for(Players::iterator p=scene.p.begin(); p!=scene.p.end(); ++p){
		cc->set_source_rgb(p->second.r, p->second.g, p->second.b);
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
// 押し続けている状態を把握できるようにできる
bool MyDrawingArea::on_key_press_event(GdkEventKey* k){
	Input &input=Input::getInstance();
	input.set_key(k);
	return false;
}

bool MyDrawingArea::on_key_release_event(GdkEventKey* k){
	Input &input=Input::getInstance();
	input.reset_key(k);
	return true;
}

bool MyDrawingArea::on_button_press_event (GdkEventButton* event){
	Input &input=Input::getInstance();
	input.set_input((int)event->x, (int)event->y);
	return true;
}

MyImageMenuItem::MyImageMenuItem(BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& g):
					Gtk::ImageMenuItem(o){
	menuId=-1;
}

void MyImageMenuItem::on_activate(void){
	Gtk::ImageMenuItem::on_activate();

	Manager &mgr = Manager::getInstance();
	MyNetwork &net=MyNetwork::getInstance();
	ViewManager &vmr=ViewManager::getInstance();

	switch(menuId){
	case 0:
		vmr.menu[0]->set_sensitive(false);
		vmr.menu[1]->set_sensitive(true);
		vmr.menu[2]->set_sensitive(false);
		vmr.menu[4]->set_sensitive(false);
		mgr.set_state(Manager::Run);
		vmr.push(std::string("Run"));
		switch(mgr.get_mode()){
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

MyStatusbar::MyStatusbar(BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& g):
Gtk::Statusbar(o){
	statusId=0;
}

void MyStatusbar::pushTemp(std::string s){
	push(s, statusId);
	sigc::slot<bool> slot = sigc::bind(sigc::mem_fun(*this, &MyStatusbar::erase), statusId);
	Glib::signal_timeout().connect(slot, 5000);
	statusId++;
}

bool MyStatusbar::erase(int i){
	pop(i);
	return false;
}

void ViewManager::subCancel(void){
	chooser->hide();
}

void ViewManager::subSend(void){
	MySmartphone &smapho = MySmartphone::getInstance();
	if(smapho.isConnected()){
		smapho.sendImage((const char *)(chooser->get_filename().c_str()));
	}
	chooser->hide();
}

void ViewManager::subHide(void) {
	Manager &mgr = Manager::getInstance();
	MyNetwork &net=MyNetwork::getInstance();

	if (server->get_active() && mgr.get_mode()!=Manager::Server){
		net.disconnect();
		if(net.startServer((short unsigned int)std::atoi(sport->get_text().c_str()), name->get_text().c_str())){
			mgr.set_mode(Manager::Server);
		}else{
			mgr.set_mode(Manager::Standalone);
		}
	} else if (client->get_active() && mgr.get_mode()!=Manager::Client){
		net.closeServer();

		if(net.connectClient(cip->get_text().c_str(), (short unsigned int)std::atoi(cport->get_text().c_str()),
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
}

Gtk::Window *ViewManager::init(Glib::RefPtr<Gtk::Builder> builder){
	builder->get_widget("window1", mainWindow);
	builder->get_widget("window2", subWindow);
	builder->get_widget("window3", chooser);
	builder->get_widget("button1", ok);
	ok->signal_clicked().connect(sigc::mem_fun0(*this, &ViewManager::subHide));
	builder->get_widget("button2", ok);
	ok->signal_clicked().connect(sigc::mem_fun0(*this, &ViewManager::subCancel));
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
	builder->get_widget_derived("statusbar1", statusbar);
	builder->get_widget_derived("Start", menu[0]);
	builder->get_widget_derived("Stop", menu[1]); menu[1]->set_sensitive(false);
	builder->get_widget_derived("SetMode", menu[2]);
	builder->get_widget_derived("SendImage", menu[3]); menu[3]->set_sensitive(false);
	builder->get_widget_derived("Quit", menu[4]);
	for (int i = 0; i < 5; ++i) {
		menu[i]->menuId = i;
	}
	return mainWindow;
}
