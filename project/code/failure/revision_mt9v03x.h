/*
 * revision_mt9v03x.h
 *
 *  Created on: 2024��4��28��
 *      Author: pc
 */

#ifndef REVISION_MT9V03X_H_
#define REVISION_MT9V03X_H_
#include "zf_common_headfile.h"

#define LCDH 60                              //����ͼ�����ͼ��߶ȣ��У�
#define LCDW 94                              //����ͼ�����ͼ���ȣ��У�
#define ImageSensorMid 39                    //ͼ�����Ļ�е�
#define LimitL(L) (L = ((L < 1) ? 1 : L))    //�޷��޷�
#define LimitH(H) (H = ((H > 78) ? 78 : H))  //�޷��޷�
#define Garage_Number 2                      //�������
extern int Mid_line[60];                   //�ʼд��Get_SideLine�ѱ��ߺ�����������������ߵ�����
extern int Left_line[60];                  //�ʼд��Get_SideLine�ѱ��ߺ������������������ߵ�����
extern int Right_line[60];                 //�ʼд��Get_SideLine�ѱ��ߺ���������������ұ��ߵ�����
extern int Out_Garage_Help_Flag;   // 1 :��߳�����   2 :�ұ߳�����
extern int Out_Garage_Help_enable;   // 1 :������   2 :�ѳ�����
extern int RoadBlock_length;
extern uint8 ex_mt9v03x_binarizeImage[LCDH][LCDW];
extern uint8* Image_Use[LCDH][LCDW];
extern uint8 Threshold;
extern uint16 threshold1,threshold2,threshold3,block_yuzhi;
extern uint16 yuzhi1,yuzhi2,yuzhi3;
extern uint16 Ramp_cancel,circle_stop,block_num,duan_num;
extern int Black;
extern uint8 Auto_Through_RoadBlock ; // �Զ�ѡ��ͨ��·�Ϸ�ʽ   1 : ��ͨ��·��   2 : ��ͨ��·��
extern uint8 RoadBlock_Flag ; //·��λ��  1 : ��һ�δ��  2 : �����������н�·��  3 : �ڶ��δ�ǻص�����  4 : Ϊ����ĳ�����������ص�д������
extern uint8 RoadBlock_Thruough_Flag ; //·��ͨ����ʽ  1 : ��ͨ��  2 : ��ͨ��
extern uint8 RoadBlock_Thruough_Flag_Record,Bend_Thruough_RoadBlock_Flag,ICM20602_Clear_Flag ;
extern uint8 Regular_RoadBlock;
//ÿһ�е�����
typedef struct {
    uint8 leftline;////����� �ݶ�ɨ��
    uint8 rightline;//�ұ���
    uint8 midline;
    uint8 wide;                 //���
    uint8 IsRightFind;          //�ұ��б߱�־
    uint8 IsLeftFind;         //����б߱�־
    int LeftBoundary;          //��߽� �������һ������
    int RightBoundary;     //�ұ߽� �������һ������
    int De_m;                                 //����ƫ��,��Ϊ�ң���Ϊ��
    //�����ַ���ɨ������
    int LeftBoundary_First;  //��߽� �����һ������
    int RightBoundary_First;  //�ұ߽� �����һ������

} Sideline_status;// �������

typedef struct {
    //ͼ����Ϣ
    //int TowPoint;                                //�����У�Ҳ������˵��ǰհ

    int16 OFFLine;                              //ͼ�񶥱�
    int16 WhiteLine;                            //˫�߶�����
    int16 OFFLineBoundary;                      //�������ֹ��
    int16 Miss_Left_lines;                      //���߶�ʧ
    int16 Miss_Right_lines;                     //���߶�ʧ
    int Det_True;                               //�������
    //�����ַ���ɨ������
    int16 WhiteLine_L;                          //��߶�����
    int16 WhiteLine_R;                          //�ұ߶�����
    char status[20];
} Image_Status;

typedef struct {
  int point;
  uint8 type;
} JumpPointtypedef;

typedef struct {
    int16 CrossRoad_Flag;
     int16 Bend_Road;                            /*0 :��               1 :�����     2 :�����*/
     int16 image_element_rings;                  /*0 :��Բ��          1 :��Բ��       2 :��Բ��*/
     int16 ring_big_small;                       /*0:��                     1 :��Բ��       2 :СԲ��*/
     int16 image_element_rings_flag;             /*Բ������*/
     int16 straight_long;                        /*��ֱ����־λ*/
     int16 straight_xie;
     int16 Garage_Location;                      /*0 :�޳���          1 :�󳵿�       2 :�ҳ���*/
     int16 Zebra_Flag;                           /*0 :�ް�����       1 �󳵿�       2 :�ҳ���*/
     int16 Ramp;                                  /*0 :���µ�          1���µ�*/
     int16 RoadBlock_Flag;                        /*0 :��·��            1 :·��*/
     int16 Out_Road;                               /*0 :�޶�·      1 :��·*/
     int16 stop_flag;                              /*ֹͣ��־λ*/
     int16 run_flag;
} ImageFlagtypedef;

extern Sideline_status Sideline_status_array[60];
extern Image_Status imagestatus;
extern ImageFlagtypedef imageflag;

extern uint32 break_road(uint8 line_start);
uint8 get_Threshold(uint8* ex_mt9v03x_binarizeImage[][LCDW],uint16 col, uint16 row);
void Image_CompressInit(void);
void Image_Process(void);
void Get_SideLine(void);
void Get_BinaryImage(void);
void Element_Handle(void);
extern uint8 Garage_Location_Flag;
void Get_Border_And_SideType(uint8* p,uint8 type,int L,int H,JumpPointtypedef* Q) ;
void Get_BaseLine(void);
void Get_AllLine(void);
void Get_ExtensionLine(void);
void Flag_init(void);
void Auto_RoadBlock_Through();


#endif /* REVISION_MT9V03X_H_ */
