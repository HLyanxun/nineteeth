/*
 * better_mpu6050.h
 *
 *  Created on: 2024Äê3ÔÂ19ÈÕ
 *      Author: pc
 */

#ifndef BETTER_MPU6050_H_
#define BETTER_MPU6050_H_
#include "zf_common_headfile.h"
extern float Angle_pitch,Angle_roll,Angle_yaw;

void mpu6050_data_dispose(void);
void mpu6050_parameter_tft180_show(void);


#endif /* BETTER_MPU6050_H_ */
