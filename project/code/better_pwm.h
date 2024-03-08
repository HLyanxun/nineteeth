/*
 * better_pwm.h
 *
 *  Created on: 2024Äê1ÔÂ17ÈÕ
 *      Author: pc
 */
#ifndef _BETTER_PWM_H_
#define _BETTER_PWM_H_

void steering_engine_init(pwm_channel_enum pin,uint32 freq, int32 angle);
void steering_engine_set_angle(pwm_channel_enum pin,int angle);
int MK021851_interrupt(gpio_pin_enum pin_a,gpio_pin_enum pin_b,int value);


#endif /* BETTER_PWM_C_ */


