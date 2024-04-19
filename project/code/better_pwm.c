/*
 * better_pwm.c
 *
 *  Created on: 2024年1月17日
 *      Author: pc
 */
//#include "zf_driver_pwm.h"
#include "zf_common_headfile.h"

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     初始化舵机
// 参数说明     pin             PWM通道号及引脚
// 参数说明     freq            PWM频率
// 参数说明     angle           舵机角度
// 使用示例     steering_engine_init(TIM1_PWM_MAP0_CH1_A8,50,90);
// 备注信息     角度为参考值与实际角度具有一定误差
//-------------------------------------------------------------------------------------------------------------------

void steering_engine_init(pwm_channel_enum pin,uint32 freq, int angle)
{
//        angle=(angle>180)?180:angle;
//        angle=(angle<0)?0:angle;
//        angle=250+angle*(1000/180);
    if(angle>180)
        angle=180;
   if(angle<0)
       angle=0;
   angle=250+angle*(900/180);


        pwm_init(pin,freq,angle);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     舵机角度设定
// 参数说明     pin             PWM通道号及引脚
// 参数说明     angle           舵机角度
// 使用示例     steering_engine_init(TIM1_PWM_MAP0_CH1_A8,90);
// 备注信息     角度为参考值与实际角度具有一定误差
//-------------------------------------------------------------------------------------------------------------------
void steering_engine_set_angle(pwm_channel_enum pin,float angle)
{

    if(angle>180)
           angle=180;
      if(angle<0)
          angle=0;
      angle=250+angle*(900/180);
    pwm_set_duty(pin,(int)angle);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     旋转编码器的输出检测
// 返回值         旋转编码器旋转量，正转增大，逆转减小
// 参数说明     pin_a   旋转编码器a端
// 参数说明     pin_b   旋转编码器b端
// 参数说明     value   通过旋转编码器直接进行调节的值
// 使用实例     MK021851_interrupt(E2,C3,a);
//-------------------------------------------------------------------------------------------------------------------

int MK021851_interrupt(gpio_pin_enum pin_a,gpio_pin_enum pin_b,int value)
{

    static uint8 a,b,La,Lb,flag,turn;
    La=a;
    Lb=b;
    a=gpio_get_level(pin_a);
    b=gpio_get_level(pin_b);
    if(La!=1 || Lb!=1 || a!=1 || b!=1)                                              //判断是正反转，1为正转，2为反转；
    {
        if(flag==0)
        {if(La==1 && Lb==1){if(a==0 && b==1){turn=1;}if(a==1 && b==0){turn=2;}}}
        flag=1;                                                                     //判断正反转之后，让标志位置1，减少后续判断；
    }
    else
    {
        flag=0;
        turn=0;
    }
    if(turn==1)
    {if(La!=a){value++;}}
    if(turn==2)
    {if(La!=a){value--;}}
    return value;
}
//--------------------------------------------------------------------------------------------------------------------
//  函数简介         pwm初始化再度封装，直接输入占空比
//  使用示例        better_pwm_init(A8,50,5);//占空比为5%的pwm（定时器2用于主函数定时，建议引脚E9 E11 E13 E14 A8 A9 A10 A11）;
//--------------------------------------------------------------------------------------------------------------------
void better_pwm_init(pwm_channel_enum pin, uint32 freq, float duty)
{
    duty=(duty/100)*PWM_DUTY_MAX;
    pwm_init(pin, freq, duty);
}
//--------------------------------------------------------------------------------------------------------------------
// 函数简介     pwm设置再度封装，直接输入占空比
// 使用实例
//--------------------------------------------------------------------------------------------------------------------
void better_pwm_set_duty(pwm_channel_enum pin,float duty)
{
    duty=(duty/100)*PWM_DUTY_MAX;
    pwm_set_duty(pin, (uint32)duty);
}
