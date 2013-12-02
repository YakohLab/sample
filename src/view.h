#include <gtkmm.h>

#ifndef VIEW_H_
#define VIEW_H_

struct dot{
	int x, y, visible;
};
const int max_dots=5;

class MyDrawingArea : public Gtk::DrawingArea {
public:
	MyDrawingArea(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
	virtual ~MyDrawingArea();
	void update();
protected:
    virtual void on_realize();
    virtual bool on_key_press_event(GdkEventKey*);
    virtual bool on_expose_event(GdkEventExpose*);
    virtual bool on_button_press_event (GdkEventButton*);
private:
    dot dots[max_dots];
    int curDots;
    char c[2];
};

#endif /* VIEW_H_ */
