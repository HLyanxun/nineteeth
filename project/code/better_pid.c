/*
 * better_pid.c
 *
 *  Created on: 2024��3��24��
 *      Author: pc
 */
#include "zf_common_headfile.h"


#include <math.h>

float pid(float input, float Kp, float Ki, float Kd, float dt,float target)
{
    // ���徲̬���������ڱ�����ʷ���ͻ���ֵ
    static float previous_error = 0;
    static float integral = 0;
    float error=0;
    error=(input-target);
    // ���������������΢����
    float proportional = Kp * error;
    integral += error * dt;
    float derivative = (error - previous_error) / dt;

    // ����PID�����������ֵ
    float output = proportional + Ki * integral + Kd * derivative;

    // ���浱ǰ���ֵ��������һ�μ���΢����
    previous_error = error;

    // ����PID�����������ֵ
    return output;
}

