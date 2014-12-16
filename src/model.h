/*
 * model.h
 *
 *  Created on: 2013/12/09
 *      Author: yakoh
 */

#ifndef MODEL_H_
#define MODEL_H_
#include <time.h>

#include "input.h"

class Model {
public:
	void initModel(void);
	void preAction();
	void stepPlayer(int fd);
	void postAction();
};

#endif /* MODEL_H_ */
