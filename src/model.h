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

	void initModel(scene_t *);
	void preAction();
	void stepPlayer(int, input_t *);
	void postAction();
private:
	scene_t *scene;
};

#endif /* MODEL_H_ */
