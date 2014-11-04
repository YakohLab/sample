/*
 * model.h
 *
 *  Created on: 2013/12/09
 *      Author: yakoh
 */

#ifndef MODEL_H_
#define MODEL_H_
#include <time.h>

#include "common.h"

class Model {
public:
	Model();
	virtual ~Model();

	void initModelWithScene(Scene *);
	void preAction();
	void stepPlayer(int, input_t *);
	void postAction();
private:
	Scene *scene;
};

#endif /* MODEL_H_ */
