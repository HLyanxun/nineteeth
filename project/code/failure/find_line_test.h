/*
 * find_line_test.h
 *
 *  Created on: 2024��4��27��
 *      Author: pc
 */

#ifndef FIND_LINE_TEST_H_
#define FIND_LINE_TEST_H_
#include "zf_common_headfile.h"
#define LCDW    MT9V03X_W/2
#define LCDH    MT9V03X_H/2

#define IMAGE_WIDTH  MT9V03X_W/2
#define IMAGE_HEIGHT MT9V03X_H/2
#define Row  MT9V03X_W
#define Col MT9V03X_H

#define MAX_BLACK_PIXELS 10 // �����ںڵ��������������ֵ

#define LimitL(L) (L = ((L < 1) ? 1 : L))    //�޷��޷�
#define LimitH(H) (H = ((H > 92) ? 92            : H))  //�޷��޷�

typedef struct {
    int row;
    int col;
} Obstacle;
// ����ϰ���λ�ò��洢�ĺ���
//typedef struct {
//    int x;
//    int y;
//} CornerPoint;
////  ��¼�յ�����Ľṹ��
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
    int point;
    uint8 type;
} Jumppoint;// �ڰ�������¼
typedef struct {
    int y;
    uint8 num;
} ChangePoint;// �ϰ�ʶ����
typedef struct {
    //ͼ����Ϣ
    //int TowPoint;                                //�����У�Ҳ������˵��ǰհ

    int16 OFFLine;                              //ͼ�񶥱�
    int16 WhiteLine;                            //˫�߶�����
    int16 OFFLineBoundary;                      //�������ֹ��
    int16 Miss_Left_lines;                      //���߶�ʧ
    int16 Miss_Right_lines;                     //���߶�ʧ

    //�����ַ���ɨ������
    int16 WhiteLine_L;                          //��߶�����
    int16 WhiteLine_R;                          //�ұ߶�����
    char status[20];
} Image_Status;
// ͼ������ṹ��
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
} Image_Flag;// �ڰ�������¼


extern uint8 ex_mt9v03x_binarizeImage[MT9V03X_H/2][MT9V03X_W/2];
extern uint8 ex_threshold;                                            //�����ֵ
extern int16 exposure_time;                                           //�ع�ʱ��
extern int16 threshold_fix;                                          //��ֵ����ֵ����
extern int THRESHOLD;
extern uint16 wide_sum;//////////////////

extern Sideline_status Sideline_status_array[MT9V03X_H/2];
extern Image_Status imagestatus;
extern Image_Flag imageflag;


void Image_Process(void);                   //���������������


void Get_Border_And_SideType(uint8* p,uint8 type,int L,int H,Jumppoint* Q);
int16 otsuThreshold( uint8 inputImage[MT9V03X_H][MT9V03X_W]);
//void binarizeImage(uint8 x_s,uint8 y_s,uint8 x_e,uint8 y_e);
void flag_init(void);
void baseline_get(void);
void allline_get(void);
void Search_Bottom_Line_OTSU(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline);
void Search_Left_and_Right_Lines(uint8 imageInput[LCDH][LCDW],uint8 row,uint8 col,uint8 Bottonline);
void Search_Border_OTSU(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline);

void Element_Handle(void);
void Scan_Element(void);
void Element_Judgment_RoadBlock(void);
void Element_Judgment_Left_Rings(void);
void Element_Judgment_Right_Rings(void);
void Element_Judgment_Zebra();//�������ж�
void Element_Judgment_Bend();
void Straight_long_judge(void);     //���ؽ��С��1��Ϊֱ��
void Straight_long_handle(void);     //���ؽ��С��1��Ϊֱ��

void auto_extension_line(void);

#endif /* FIND_LINE_TEST_H_ */
