#include <gtkmm.h>
#include "common.h"

#ifndef VIEW_H_
#define VIEW_H_

class MyDrawingArea : public Gtk::DrawingArea {
public:
	MyDrawingArea(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
	virtual ~MyDrawingArea();
	void setScene(scene_t *);
	void getInput(input_t *);
protected:
    virtual void on_realize();
    virtual bool on_key_press_event(GdkEventKey*);
    virtual bool on_expose_event(GdkEventExpose*);
    virtual bool on_button_press_event (GdkEventButton*);
private:
    input_t input;
    scene_t scene;
    void clearInput(void);
};

#endif /* VIEW_H_ */
