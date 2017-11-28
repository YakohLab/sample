/*
 * model.h
 */
#ifndef MODEL_H_
#define MODEL_H_
#include <time.h>

#include "input.h"

class Model {
public:
	void initModel(void);
	void preAction(void);
	void stepPlayer(int fd);
	void postAction(void);
};

#endif /* MODEL_H_ */
