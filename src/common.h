/*
 * common.h
 *
 *  Created on: 2013/12/09
 *      Author: yakoh
 */

#ifndef COMMON_H_
#define COMMON_H_
#include <gtkmm.h>
#include <time.h>

const int period = 30; // in millisecond

extern Gtk::Statusbar *statusBar;
extern int statusId, statusEraseId;

gboolean eraseStatusbar(void *p);
gboolean tickServer(void *p);

#endif /* COMMON_H_ */
