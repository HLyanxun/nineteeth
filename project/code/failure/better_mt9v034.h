/*
 * better_mt9v034.h
 *
 *  Created on: 2024年1月20日
 *      Author: pc
 */

#ifndef BETTER_MT9V034_H_
#define BETTER_MT9V034_H_
#include "zf_common_headfile.h"

#define pi                                      (3.14)               //圆周率
#define RANK_   3                                                   //多项式次数
#define Row                                     MT9V03X_H
#define Col                                     MT9V03X_W
#define find_start                              (15)
#define find_end                                (170)
#define annular_peremeter                       (5)
#define smartcar_state_parament                 (5)
#define not_lose_line_parameter                 (50)

extern uint8 ex_zebra_pass_count;                                 //斑马线通过次数计数
extern uint8 ex_mt9v03x_image_binaryzation_flag;                  //每帧图像二值化完成标志位
extern uint8 ex_zebra_crossing_flag;                              //斑马线出现标志位
extern int ex_binaryzation_conduct_adjustment;                      //二值化阈值调整值，用于做二值化阈值人为修正
extern uint8 ex_mt9v03x_image_binaryzation[MT9V03X_H][MT9V03X_W];   //二值化后图像储存数组（请谨慎修改，有概率导致程序损坏）
extern uint8 ex_roundabout_type;                                    //环岛类型
extern uint8 ex_roundabout_state;                                   //环岛状态
extern uint16 ex_midline[MT9V03X_H];                                //中线数据储存数组
extern uint16 ex_leftline[MT9V03X_H];                               //左边线数据储存数组
extern uint16 ex_rightline[MT9V03X_H];                              //右边线数据储存数组
extern uint8 ex_crossroad_flag;                                     //十字路口标志位
extern uint8 ex_crossroad_state;                                    //十字路口状态

int8 smartcar_state_detection(void);
float arctan(float x,float y);
uint8 straight_line_judgment(uint16 arr[Row]);
int8 lost_line_left(void);
int8 lost_line_right(void);
void sobe_binaryzation_conduct(void);
void sobelAutoThreshold (u8 source[MT9V03X_H/2][MT9V03X_W],u8 target[MT9V03X_H/2][MT9V03X_W]);
void mt9v034_binaryzation_conduct(void);
void mt9v034_tft180_show(void);
uint8 binaryzation_value(void);
void zebra_crossing(void);
void connect_line(uint8 x1,uint8 y1,uint8 x2,uint8 y2);
void roundabout_dispose(void);
void roundabout_detection(void);
int8 roundabout_annular_detection(void);
int8 crossroad_detection(void);
int16 change_detection(uint8 select,uint8 location);
void horizontal_line(void);
void crossroad_conduct(void);
void mt9v034_tft180_dajin_show(void);


#endif /* BETTER_MT9V034_H_ */
