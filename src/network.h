/*
 * network.h
 *
 *  Created on: 2013/12/10
 *      Author: yakoh
 */

#ifndef NETWORK_H_
#define NETWORK_H_
#include <glib.h>
#include "common.h"

struct member_t {
        int attend, ready;
        char name[20];
        struct input_t input;
        GIOChannel *gioc;
        guint sid;
};

class Network {
public:
	Network();
	virtual ~Network();
	bool startServer(const char *, const char *);
	bool connectServer(const char *, const char *, const char *);
	void sendScene(int, scene_t *);
private:
	bool isServerStart, isConnect;
	member_t members[max_players];
	int num_attend;
	gboolean server_accept(GIOChannel *gioc, GIOCondition cond, void *arg);

};

#endif /* NETWORK_H_ */
