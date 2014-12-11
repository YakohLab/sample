/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * network.cc
 * Copyright (C) Takahiro Yakoh 2011 <yakoh@sd.keio.ac.jp>
 * $Revision: 1.20 $
 */

#include "mynetwork.h"
#include "view.h"
#include "model.h"
#include "manager.h"

class Member {
public:
	Member(void){
		ready=0;
		fd=0;
	};
	int ready;
	std::string name;
	int fd;
};

std::vector<Member> members;
static int server_flag = 0;

void MyNetwork::onDisconnect(int id){
	Manager &mgr = Manager::getInstance();
	if(mgr.get_mode()==Manager::Server){
		for(std::vector<Member>::iterator i=members.begin(); i!=members.end(); ++i){
			if(i->fd==id){
				members.erase(i);
				break;
			}
		}
		showStatus();
	}else{ // Client
		mgr.set_mode(Manager::Standalone);
		mgr.set_state(Manager::Stop);
	}
};

void MyNetwork::onRecvFromServer(char *msg){
	Header *h;
	h=(Header *)msg;
	Member tmp;
	Manager &mgr = Manager::getInstance();
	switch(h->command){
	case Header::STATUS:
		statusBar->push(Glib::ustring(msg+sizeof(Header)), statusId++);
		g_timeout_add(5000, eraseStatusbar, 0);
		break;
	case Header::DRAW:
		bcopy(msg+sizeof(Header), (gchar *) &(mgr.get_scene()), sizeof(Scene));
		process_a_step(&(mgr.get_scene()), &input[0]);
		sendInput(input[0]);
		break;
	case Header::STOP:
		mgr.set_state(Manager::Stop);
		break;
	case Header::CONNECT: // Followings are not sent from server
	case Header::START:
	case Header::INPUT:
	default:
		break;
	}
}

void MyNetwork::onRecvFromClient(int fd, char *msg){
	int j=0;
	Manager &mgr = Manager::getInstance();
	Header *h;
	h=(Header *)msg;
	Member tmp;
	switch(h->command){
	case Header::CONNECT:
		tmp.fd=fd;
		tmp.name=std::string(msg+sizeof(Header));
		members.push_back(tmp);
		showStatus();
		break;
	case Header::START:
		for(std::vector<Member>::iterator i=members.begin(); i!=members.end(); ++i){
			if(i->fd==fd){
				i->ready=1;
				break;
			}
		}
		if(showStatus()){
			server_flag = 1;
			for (unsigned int i = 0; i < max_players; ++i) {
				if (i<members.size()) {
					mgr.attend_player(i);
					strcpy(mgr.get_scene().p[i].name, members[i].name.c_str());
				} else {
					mgr.absent_player(i);
				}
			}
			mgr.set_state(Manager::Run);
			g_timeout_add(period, Manager::tickServer, (gpointer) NULL);
		}
		break;
	case Header::STOP:
		for(std::vector<Member>::iterator i=members.begin(); i!=members.end(); ++i){
//			if(i->fd==fd){
				i->ready=0;
//				break;
//			}
		}
		mgr.set_state(Manager::Stop);
		showStatus();
		sendStop();
		break;
	case Header::INPUT:
		for(std::vector<Member>::iterator i=members.begin(); i!=members.end(); ++i, ++j){
			if(i->fd==fd){
				bcopy(msg+sizeof(Header), (gchar *) &input[j], sizeof(Input));
				break;
			}
		}
		break;
	case Header::STATUS: // Followings are not sent from client
	case Header::DRAW:
	default:
		break;
	}
}

void MyNetwork::connectClient(const char *host, int port, const char *name){
	Header h;
	connect(host, port);
	h.command=Header::CONNECT;
	h.length=strlen(name)+1;
	sendToServer(&h, sizeof(Header));
	sendToServer((void *)name, h.length);
}

void MyNetwork::runClient(void){
	Header h;
	h.command=Header::START;
	h.length=0;
	sendToServer(&h, sizeof(Header));
}

void MyNetwork::stopClient(void){
	Header h;
	h.command=Header::STOP;
	h.length=0;
	sendToServer(&h, sizeof(Header));
}

void MyNetwork::disconnectClient(void){
}

void MyNetwork::sendInput(Input &a){
	Header h;
	h.command=Header::INPUT;
	h.length=sizeof(Input);
	sendToServer(&h, sizeof(Header));
	sendToServer((char *)&a, sizeof(Input));
}

void MyNetwork::startServer(int port, const char *name){
	openServer(port);
	members.clear();
	Member tmp;
	tmp.name=std::string(name);
	members.push_back(tmp);
}

void MyNetwork::runServer(void){
	Manager &mgr = Manager::getInstance();
	for(std::vector<Member>::iterator i=members.begin(); i!=members.end(); ++i){
		if(i->fd==0){
			i->ready=1;
			break;
		}
	}
	if(showStatus()){
		server_flag = 1;
		for (unsigned int i = 0; i < max_players; ++i) {
			if (i<members.size()) {
				mgr.attend_player(i);
				strcpy(mgr.get_scene().p[i].name, members[i].name.c_str());
			} else {
				mgr.absent_player(i);
			}
		}
		mgr.set_state(Manager::Run);
		g_timeout_add(period, Manager::tickServer, (gpointer) NULL);
	}
}

void MyNetwork::stopServer(void){
	Manager &mgr = Manager::getInstance();
	for(std::vector<Member>::iterator i=members.begin(); i!=members.end(); ++i){
//		if(i->fd==0){
			i->ready=0;
//			break;
//		}
	}
	mgr.set_state(Manager::Stop);
	showStatus();
	sendStop();
}

void MyNetwork::sendScene(int id, Scene &s){
	int j=0;
	Header h;
	h.command=Header::DRAW;
	h.length=sizeof(Scene);
	for(std::vector<Member>::iterator i=members.begin(); i!=members.end(); ++i, ++j){
		if(j==id){
			sendToClient(i->fd, &h, sizeof(Header));
			sendToClient(i->fd, &s, sizeof(Scene));
		}
	}
}

void MyNetwork::sendStop(void){
	Header h;
	h.command=Header::STOP;
	h.length=0;
	for(std::vector<Member>::iterator i=members.begin(); i!=members.end(); ++i){
		if(i->fd!=0){
			sendToClient(i->fd, &h, sizeof(Header));
		}
	}
}

bool MyNetwork::showStatus(void){
		char buffer[50];
		unsigned int r=0;
		Header h;

		for(std::vector<Member>::iterator i=members.begin(); i!=members.end(); ++i){
			if(i->ready){
				r++;
			}
		}
		sprintf(buffer, "%d / %lu", r, members.size());

		h.command=Header::STATUS;
		h.length=strlen(buffer)+1;
		for(std::vector<Member>::iterator i=members.begin(); i!=members.end(); ++i){
			if(i->fd!=0){
				sendToClient(i->fd, &h, sizeof(Header));
				sendToClient(i->fd, buffer, h.length);
			}
		}
		statusBar->push(Glib::ustring(buffer), statusId++);
		g_timeout_add(5000, eraseStatusbar, 0);

		if (r==members.size()){
			return true;
		}else{
			return false;
		}
}
