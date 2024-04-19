/*
 * button.h
 *
 *  Created on: 2024Äê4ÔÂ13ÈÕ
 *      Author: pc
 */

#ifndef BUTTON_H_
#define BUTTON_H_
#include "zf_common_headfile.h"
#define KEY_Number      (4)
typedef struct{
    uint8 status;
    uint8 time;
    uint8 level;
    uint8 level_last;
    uint8 decetion_flag;
}Button_Stash;
#define page_0_max                      (3)
#define page_0_min                      (1)
#define page_shexiangtou_max            (4)
#define page_shexiangtou_min            (9)
#define page_pid_max                    (5)
#define page_pid_min                    (0)

#define mpu6050_0                       (16)
#define tuxiang_0                       (2*16)
#define pid_0                           (3*16)

#define kp_31                           (0)
#define ki_31                           (16)
#define kd_31                           (2*16)
#define dt_31                           (3*16)
#define target_31                       (4*16)

extern Button_Stash Button[KEY_Number];

void menu_page(void);
void button_status_get(uint8 interrupt_time);
void menu_button_init(void);


#endif /* BUTTON_H_ */
