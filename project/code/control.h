/*
 * control.h
 *
 *  Created on: 2024��4��11��
 *      Author: pc
 */

#ifndef CONTROL_H_
#define CONTROL_H_
#include "zf_common_headfile.h"
extern float kp,ki,kd,dt,target;

#define pi  (3.14)

void Forward_control(void);


#endif /* CONTROL_H_ */
