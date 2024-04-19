/*
 * control.c
 *
 *  Created on: 2024年4月11日
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"
float kp,ki,kd,dt,target;
////    float input = 0.0;      // 初始值
//CascadePIDController cascadepid;    // 创建串级PID控制器对象
//
//float angle_control(float input)
//{
//    int16 output;
//    CascadePIDController_Init(&cascadepid, inner_kp, inner_ki, inner_kd, inner_dt, outer_kp, outer_ki, outer_kd, outer_dt, min, max, setpoint, input);  // 初始化串级PID控制器
//    for (int i = 0; i < 100; i++) {
//       int16 output = CascadePIDController_Compute(&cascadepid);  // 计算输出
////        printf("Output: %f\n", output);                             // 输出结果
//        input += output * outer_dt;                                  // 更新输入值
//    }
//    return output;
//}
//
////    return 0;
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

//            better_pwm_set_duty(TIM1_PWM_MAP0_CH1_A8, output);
        }
    }
//
//    if(angle==0)angle=90;
//        tft180_show_float(80, 113, angle, 3);



}

