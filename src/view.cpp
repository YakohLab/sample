#include <iostream>
#include <time.h>
#include "view.h"

MyDrawingArea::MyDrawingArea(BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& g):
	Gtk::DrawingArea(o){
	for(int i=0; i<max_dots; ++i){
		dots[i].visible=0;
	}
	curDots=0;
	c[0]=' ';
	c[1]=0;
}

MyDrawingArea::~MyDrawingArea(void){
}

void MyDrawingArea::on_realize(void){
	Gtk::DrawingArea::on_realize();
}

bool MyDrawingArea::on_key_press_event(GdkEventKey* k){
	std::cout << "Typed " << k->keyval << std::endl;
	c[0]=k->keyval;
	return true;
}

bool MyDrawingArea::on_expose_event( GdkEventExpose* e ){
	std::cout << "Exposed" << std::endl;

	time_t t;
	tm *tm;
	t=time(NULL);
	tm=localtime(&t);
	Gdk::Rectangle rect;
	rect.set_x(0);
	rect.set_y(0);
	rect.set_width(this->get_width());
	rect.set_height(this->get_height());
	Cairo::RefPtr<Cairo::Context> cc = this->get_window()->create_cairo_context();

	this->get_window()->begin_paint_rect(rect);

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
	cc->line_to(ls+ls*sin(2.0*M_PI*tm->tm_sec/60), ls-ls*cos(2.0*M_PI*tm->tm_sec/60));
	cc->stroke();

	cc->set_font_size(30);
	cc->move_to(ls+ls*sin(2.0*M_PI*tm->tm_sec/60), ls-ls*cos(2.0*M_PI*tm->tm_sec/60));
	cc->show_text(std::string(c));

	cc->set_line_width(3.0);
	cc->set_source_rgb(0.2, 0.2, 1.0);
	cc->move_to(ls, ls);
	cc->line_to(ls+lm*sin(2.0*M_PI*(tm->tm_min/60.0+tm->tm_sec/3600.0)),
			ls-lm*cos(2.0*M_PI*(tm->tm_min/60.0+tm->tm_sec/3600.0)));
	cc->stroke();

	cc->set_line_width(5.0);
	cc->set_source_rgb(0.2, 0.2, 1.0);
	cc->move_to(ls, ls);
	cc->line_to(ls+lh*sin(2.0*M_PI*(tm->tm_hour/12.0+tm->tm_min/720.0)),
			ls-lh*cos(2.0*M_PI*(tm->tm_hour/12.0+tm->tm_min/720.0)));
	cc->stroke();

	cc->set_line_width(5.0);
	cc->set_source_rgb(1.0, 0.0, 0.0);
	for(int i=0; i<max_dots; ++i){
		if(dots[i].visible==1){
			cc->arc(dots[i].x, dots[i].y, 5, 0, 2.0 * M_PI);
			cc->stroke();
		}
	}

	this->get_window()->end_paint();

	return true;
}

void MyDrawingArea::update(void){
	this->queue_draw();
}

bool MyDrawingArea::on_button_press_event (GdkEventButton* event){
	std::cout << "Pressed " << event->x << "," << event->y << std::endl;
	dots[curDots].x=event->x;
	dots[curDots].y=event->y;
	dots[curDots].visible=1;
	curDots=(curDots+1)%max_dots;
	return true;
}
