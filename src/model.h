/*
 * model.h
 *
 *  Created on: 2013/12/09
 *      Author: yakoh
 */

#ifndef MODEL_H_
#define MODEL_H_

#include "common.h"

class Model {
public:
	Model();
	virtual ~Model();

	void initModel(void);
	scene_t *doModel(input_t *);
private:
	scene_t scene;
};

#endif /* MODEL_H_ */
