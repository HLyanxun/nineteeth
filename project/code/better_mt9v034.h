/*
 * better_mt9v034.h
 *
 *  Created on: 2024��1��20��
 *      Author: pc
 */

#ifndef BETTER_MT9V034_H_
#define BETTER_MT9V034_H_
#include "zf_common_headfile.h"

#define pi                                      (3.14)               //Բ����
#define standard_track_center_line              (94)
#define number_of_detection_lines               (10)
#define RANK_   3                                                   //����ʽ����
#define Row                                     MT9V03X_H
#define Col                                     MT9V03X_W
#define find_start                              (15)
#define find_end                                (170)
#define annular_peremeter                       (5)
#define smartcar_state_parament                 (5)
#define not_lose_line_parameter                 (50)

extern uint8 ex_zebra_pass_count;                                 //������ͨ����������
extern uint8 ex_mt9v03x_image_binaryzation_flag;                  //ÿ֡ͼ���ֵ����ɱ�־λ
extern uint8 ex_zebra_crossing_flag;                              //�����߳��ֱ�־λ
extern int ex_binaryzation_conduct_adjustment;                      //��ֵ����ֵ����ֵ����������ֵ����ֵ��Ϊ����
extern unsigned char ex_track_situation;                            //·���жϱ�־λ�����Ż�ɾ����
extern int8 ex_central_deviation_value[number_of_detection_lines];  //�����������꣨���Ż�ɾ����
extern uint8 ex_mt9v03x_image_binaryzation[MT9V03X_H][MT9V03X_W];   //��ֵ����ͼ�񴢴����飨������޸ģ��и��ʵ��³����𻵣�
extern uint8 ex_roundabout_type;                                    //��������
extern uint8 ex_roundabout_state;                                   //����״̬

void line_visualization();
int8 smartcar_state_detection(void);
float arctan(float x,float y);
int8 roundabout_annular_detection(void);
uint8 straight_line_judgment(int arr[Row]);
int8 lost_line_left(void);
int8 lost_line_right(void);
void sobe_binaryzation_conduct(void);
void sobelAutoThreshold (u8 source[MT9V03X_H/2][MT9V03X_W],u8 target[MT9V03X_H/2][MT9V03X_W]);
int deviate_angle(void);
int deviate_distance(void);
void find_track_center_line(void);
void road_condition_judgment(void);
void mt9v034_binaryzation_conduct(void);
void mt9v034_tft180_show(void);
uint8 binaryzation_value(void);
void track_situation_analyse(void);


#endif /* BETTER_MT9V034_H_ */
