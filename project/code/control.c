/*
 * control.c
 *
 *  Created on: 2024Äê4ÔÂ11ÈÕ
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"
float kp,ki,kd,dt,target;
uint8 offset_point_strat=MT9V03X_H/4,offset_point_end=MT9V03X_H/3;              //
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
            for(uint8 i=offset_point_end;i>offset_point_strat;i--){input+=Sideline_status_array[i].midline;}
//
            input=input/(offset_point_end-offset_point_strat);
            int output=pid(input, kp, ki, kd, dt, target);
            if(output>160)output=160;
            if(output<20)output=70;

            steering_engine_set_angle(TIM1_PWM_MAP0_CH1_A8,output);
        }
    }



}

