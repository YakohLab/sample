/*
 * mynetwork.h
 * Networkを継承した、ゲーム用の通信関数をまとめたもの
 */
#include <cstring>
#include "mynetwork.h"
#include "input.h"
#include "view.h"
#include "model.h"
#include "manager.h"

MyNetwork::MyNetwork(void):Network(){
}

void MyNetwork::onDisconnect(int fd){
	Manager &mgr = Manager::getInstance();
	if(mgr.get_mode()==Manager::Server){
		mgr.members.erase(fd);
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
	ViewManager &vmr=ViewManager::getInstance();

	switch(h->command){
	case Header::STATUS:
		vmr.push(std::string(msg+sizeof(Header)));
		break;
	case Header::DRAW:
		mgr.scene.receiveScene(msg+sizeof(Header));
		mgr.tickClient();
		sendInput();
		break;
	case Header::STOP:
		mgr.set_state(Manager::Stop);
		vmr.menu[0]->set_sensitive(true);
		vmr.menu[1]->set_sensitive(false);
		vmr.menu[2]->set_sensitive(true);
		vmr.menu[4]->set_sensitive(true);
		break;
	case Header::CONNECT: // Followings are not sent from server
	case Header::START:
	case Header::INPUT:
	default:
		break;
	}
}

void MyNetwork::onRecvFromClient(int fd, char *msg){
	Manager &mgr = Manager::getInstance();
	Input &input = Input::getInstance();
	ViewManager &vmr=ViewManager::getInstance();
	Header *h;
	h=(Header *)msg;
	Member tmp;
	switch(h->command){
	case Header::CONNECT:
		if(mgr.get_state()==Manager::Run){
			disconnect();
			return;
		}
		tmp.name=std::string(msg+sizeof(Header));
		tmp.ready=0;
		mgr.members.insert(Members::value_type(fd, tmp));
		showStatus();
		break;
	case Header::START:
		mgr.members[fd].ready=1;
		if(showStatus()){
			mgr.startServerTick();
		}
		break;
	case Header::STOP:
		for(Members::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i){
			i->second.ready=0;
		}
		mgr.set_state(Manager::Stop);
		showStatus();
		sendStop();
		vmr.menu[0]->set_sensitive(true);
		vmr.menu[1]->set_sensitive(false);
		vmr.menu[2]->set_sensitive(true);
		vmr.menu[4]->set_sensitive(true);
		break;
	case Header::INPUT:
		input.receiveInput(msg+sizeof(Header), mgr.members[fd].input);
		break;
	case Header::STATUS: // Followings are not sent from client
	case Header::DRAW:
	default:
		break;
	}
}

bool MyNetwork::connectClient(const char *host, short unsigned int port, const char *name){
	Header h;
	if(!connect(host, port)){
		return false;
	}
	h.command=Header::CONNECT;
	h.length=strlen(name)+1;
	sendToServer(&h, sizeof(Header));
	sendToServer((void *)name, h.length);
	return true;
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

void MyNetwork::sendInput(void){
	Header h;
	char *c;
	Input &input=Input::getInstance();
	h.command=Header::INPUT;
	c=input.packInput(h.length);
	sendToServer(&h, sizeof(Header));
	sendToServer(c, h.length);
}

bool MyNetwork::startServer(short unsigned int port, const char *name){
	Manager &mgr = Manager::getInstance();
	if(!openServer(port)){
		return false;
	}
	mgr.members.clear();
	Member tmp;
	tmp.name=std::string(name);
	tmp.ready=0;
	tmp.id=0;
	mgr.members.insert(Members::value_type(0, tmp));
	return true;
}

void MyNetwork::runServer(void){
	Manager &mgr = Manager::getInstance();
	mgr.members[0].ready=1;
	if(showStatus()){
		mgr.startServerTick();
	}
}

void MyNetwork::stopServer(void){
	Manager &mgr = Manager::getInstance();
	for(Members::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i){
		i->second.ready=0;
	}
	mgr.set_state(Manager::Stop);
	showStatus();
	sendStop();
}

void MyNetwork::sendScene(Scene &s){
	Header h;
	Manager &mgr = Manager::getInstance();
	char *p;

	for(Members::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i){
		if(i->first!=0){
			h.command=Header::DRAW;
			s.id=i->second.id;
			p=s.packScene(h.length);
			sendToClient(i->first, &h, sizeof(Header));
			sendToClient(i->first, p, h.length);
		}
	}
}

void MyNetwork::sendStop(void){
	Header h;
	Manager &mgr = Manager::getInstance();
	h.command=Header::STOP;
	h.length=0;
	for(Members::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i){
		if(i->first!=0){
			sendToClient(i->first, &h, sizeof(Header));
		}
	}
}

bool MyNetwork::showStatus(void){
		char buffer[50];
		unsigned int r=0;
		Header h;
		Manager &mgr = Manager::getInstance();
		ViewManager &vmr=ViewManager::getInstance();

		for(Members::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i){
			if(i->second.ready){
				r++;
			}
		}
		sprintf(buffer, "%d / %d", r, (int)mgr.members.size());

		h.command=Header::STATUS;
		h.length=strlen(buffer)+1;
		for(Members::iterator i=mgr.members.begin(); i!=mgr.members.end(); ++i){
			if(i->first!=0){
				sendToClient(i->first, &h, sizeof(Header));
				sendToClient(i->first, buffer, h.length);
			}
		}
		vmr.push(std::string(buffer));

		if (r==mgr.members.size()){
			return true;
		}else{
			return false;
		}
}
