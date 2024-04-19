/*
 * better_pid.c
 *
 *  Created on: 2024年3月24日
 *      Author: pc
 */
#include "zf_common_headfile.h"


#include <math.h>

float pid(float input, float Kp, float Ki, float Kd, float dt,float target)
{
    // 定义静态变量，用于保存历史误差和积分值
    static float previous_error = 0;
    static float integral = 0;
    float error=0;
    error=(input-target);
    // 计算比例项、积分项和微分项
    float proportional = Kp * error;
    integral += error * dt;
    float derivative = (error - previous_error) / dt;

    // 计算PID控制器的输出值
    float output = proportional + Ki * integral + Kd * derivative;

    // 保存当前误差值，用于下一次计算微分项
    previous_error = error;

    // 返回PID控制器的输出值
    return output;
}

