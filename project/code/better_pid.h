/*
 * better_pid.h
 *
 *  Created on: 2024��3��24��
 *      Author: pc
 */

#ifndef BETTER_PID_H_
#define BETTER_PID_H_
#include "zf_common_headfile.h"
float pid(float input, float Kp, float Ki, float Kd, float dt,float target);

#endif /* BETTER_PID_H_ */
