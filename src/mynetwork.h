/*
 * mynetwork.h
 *
 *  Created on: 2014/12/09
 *      Author: sdjikken
 */

#ifndef MYNETWORK_H_
#define MYNETWORK_H_

// From client to server
#define SCMD_CONNECT	0	// username is stored in data area
#define SCMD_START		1	// no data
#define SCMD_STOP		2	// no data
#define SCMD_DISCONNECT	3	// no data
#define SCMD_INPUT		4	// input structure is stored
// From server to client
#define SCMD_STATUS		10	// # of connected users and ready to start users are stored
#define SCMD_DRAW		11	// scene data structure is stored
struct message_t {
	int command;
	int length;
};

unsigned int get_myip(void);

void sendScene(int i, Scene *);

bool server_setup(const char *, const char *, input_t *);
void server_start(void);
gboolean server_receive(GIOChannel*, GIOCondition, void*);
void server_stop(void);
bool server_terminate(void);

bool client_setup(const char *, const char *, const char *);
void client_start(void);
gboolean client_receive(GIOChannel*, GIOCondition, void*);
void client_stop(void);
bool client_terminate(void);
void process_cmd(int id, int command, int length, GIOChannel* gioc);

#endif /* MYNETWORK_H_ */
