/*
 * network.cpp
 *
 *  Created on: 2013/12/10
 *      Author: yakoh
 */

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "network.h"

Network::Network() {
	// TODO Auto-generated constructor stub
	isServerStart=false;
}

Network::~Network() {
	// TODO Auto-generated destructor stub
}

bool Network::startServer(const char *port, const char *name){
	int i, on, w;
	struct sockaddr_in serv;

	if (isServerStart)
		return true;

	if ((w = socket(PF_INET, SOCK_STREAM, 0)) == (-1)) {
		perror("Can't create a socket.\n");
		return false;
	}
	on = 1;
	setsockopt(w, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	setsockopt(w, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = INADDR_ANY;
	serv.sin_port = htons(atoi((const char *) port));

	if (bind(w, (struct sockaddr *) &serv, sizeof(serv)) != 0) {
		perror("Can't bind the socket.\n");
		return false;
	}

	if (listen(w, 0) == -1) {
		perror("Can't listen the socket.\n");
		return false;
	}

	for (i = 0; i < max_players; ++i) {
		members[i].attend = 0;
		members[i].ready = 0;
		members[i].gioc = NULL;
		members[i].sid = 0;
	}

	members[0].gioc = g_io_channel_unix_new(w);
	g_io_channel_set_encoding(members[0].gioc, NULL, NULL);
	g_io_channel_set_buffered(members[0].gioc, FALSE);
	members[0].sid = g_io_add_watch(members[0].gioc, G_IO_IN, this->server_accept,
			NULL);
	members[0].attend = 1;
	num_attend = 1;
	strcpy(members[0].name, name);

	isServerStart = true;
	return true;
}
bool Network::connectServer(const char *, const char *, const char *){
	return true;
}
void Network::sendScene(int, scene_t *){

}
