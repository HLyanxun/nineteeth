/*
 * control.c
 *
 *  Created on: 2024Äê4ÔÂ11ÈÕ
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"
float kp,ki,kd,dt,target;

float arctan(float x,float y)
{
    float temp;
    temp=atan(x/y);
    temp=(180/pi)*temp;
    return temp;
}
void Forward_control(void)
{
//    int16 angle=0,temp_1=0,de_l,temp,temp_a,temp_b;
    float input=0;
    if(!imageflag.stop_flag)
    {

        if(!imageflag.straight_long)
        {
//
            for(uint8 i=40;i>30;i--){input+=Sideline_status_array[i].midline;}
//
            input=input/10;
            int output=pid(input, kp, ki, kd, dt, target);
            if(output>160)output=160;
            if(output<20)output=70;

            steering_engine_set_angle(TIM1_PWM_MAP0_CH1_A8,output);
        }
    }



}

