/*
 * button.h
 *
 *  Created on: 2024年4月13日
 *      Author: pc
 */

#ifndef BUTTON_H_
#define BUTTON_H_
#include "zf_common_headfile.h"
        /*tft范围为横向最大128竖向最大160，单个中文字符大小为16*line_unit，而英文字符则为8*line_unit，也就是在竖向共有10行，每行能容纳8个字*/
        /*tft范围为横向最大128竖向最大160，单个中文字符大小为16*line_unit，而英文字符则为8*line_unit，也就是在竖向共有10行，每行能容纳8个字*/
        /*tft范围为横向最大128竖向最大160，单个中文字符大小为16*line_unit，而英文字符则为8*line_unit，也就是在竖向共有10行，每行能容纳8个字*/
#define line_unit                       (17)
#define column_unit                     (16)

#define page_mpu6050_min                (0)
#define page_mpu6050_max                (3)
#define page_0_max                      (7)
#define page_0_min                      (1)
#define page_tuxiang_min                (4)
#define page_tuxiang_max                (8)
#define page_pid_max                    (5)
#define page_pid_min                    (1)
#define page_shexiangtou_min            (4)
#define page_shexiangtou_max            (4)

#define mpu6050_0                       (line_unit)
#define tuxiang_0                       (2*line_unit)
#define pid_0                           (3*line_unit)
#define shexiangtou_0                   (4*line_unit)
#define parameter_debug_0                (5*line_unit)
#define parameter_save_0                 (6*line_unit)
#define parameter_init_0                 (7*line_unit)


#define kp_31                           (1*line_unit)
#define ki_31                           (2*line_unit)
#define kd_31                           (3*line_unit)
#define dt_31                           (4*line_unit)
#define target_31                       (5*line_unit)

#define erzhi_21                        (5*line_unit)
#define mt9v034_21                      (6*line_unit)
#define line_show_21                    (7*line_unit)
#define runflag_21                      (8*line_unit)

#define baoguangtime_41                 (4*line_unit)

extern int16 exposure_time;

void menu_page(void);
void button_status_get(uint8 interrupt_time);
void line_visualization(void);



#endif /* BUTTON_H_ */
