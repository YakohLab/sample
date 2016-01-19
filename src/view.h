/*
 * view.h
 * 画面に関連するウィジェットの定義と、それらをまとめるためのViewManagerクラスの定義
 */
#ifndef VIEW_H_
#define VIEW_H_

// #define USE_OPENGL
// #define USE_OPENGLUT
#include <gtkmm.h>
#define GTKMM3	GTK_VERSION_GE(3,0)
#ifdef USE_OPENGL
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#ifdef USE_OPENGLUT
#include <GL/glut.h>
#endif
#endif

class ViewManager;

class MyDrawingArea: public Gtk::DrawingArea {
public:
	MyDrawingArea(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
	void update();
protected:
	virtual void on_realize();
	virtual bool on_key_press_event(GdkEventKey*);
	virtual bool on_key_release_event(GdkEventKey*);
	virtual bool on_button_press_event(GdkEventButton*);
#if GTKMM3
	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>&);
#else
	virtual bool on_expose_event(GdkEventExpose*);
#endif
private:
#ifdef USE_OPENGL
	GdkGLConfig *gl_config;
#endif
};

class MyImageMenuItem: public Gtk::ImageMenuItem {
	friend class ViewManager;
public:
	MyImageMenuItem(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
protected:
	virtual void on_activate();
private:
	int menuId;
};

class MyStatusbar: public Gtk::Statusbar{
public:
	MyStatusbar(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
	void pushTemp(std::string);
private:
	bool erase(int);
	int statusId;
};

class ViewManager {
	friend class MyImageMenuItem;
public:
	static ViewManager& getInstance() {
		static ViewManager instance;
		return instance;
	}
	void update(void){
		drawingArea->update();
	}
	void push(std::string s){
		statusbar->pushTemp(s);
	}
	int get_width(void){
		return drawingArea->get_width();
	}
	int get_height(void){
		return drawingArea->get_height();
	}
	Gtk::Window *init(Glib::RefPtr<Gtk::Builder>);
	MyImageMenuItem *menu[5];
private:
	ViewManager(){};
	ViewManager(ViewManager&);
	void operator =(ViewManager&);
	void subCancel(void);
	void subSend(void);
	void subHide(void);
	Gtk::Window *mainWindow, *subWindow;
	Gtk::FileChooserDialog *chooser;
	Gtk::Entry *sip, *sport, *cip, *cport, *name;
	Gtk::RadioButton *standalone, *server, *client;
	Gtk::Button *ok;
	MyDrawingArea *drawingArea;
	MyStatusbar *statusbar;
};

#endif /* VIEW_H_ */
