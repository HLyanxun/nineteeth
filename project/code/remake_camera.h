/*
 * remake_camera.h
 *
 *  Created on: 2024��7��4��
 *      Author: pc
 */
//�Ѿ�ȫ��ľ��ľ��ľ��ľ��ľ��ľ��ľ��ľ��ľ��ľ��ľ��ľ��ľ����
#ifndef REMAKE_CAMERA_H_
#define REMAKE_CAMERA_H_
#include "zf_common_headfile.h"

//���ѡ���
#define track_width_debug   (0)          //������ȱ궨������ʾ
#define default_side_choose (0)         //0Ĭ��ѡ�������Ѳ�ߣ�1Ĭ��ѡ���ұ���Ѳ��
#define track_show          (1)          //�Ƿ���ʾѲ�߽��
#define image_type          (0)         //0��ֵ��ͼ��1�Ҷ�ͼ��
#define track_show_8        (0)         //�Ƿ���ʾ������Ѳ�߽��

//�㷨����
#define LimitL(L) (L = ((L < 1) ? 1 : L))    //�޷��޷�
#define LimitH(H) (H = ((H > 89) ? 89 : H))  //�޷��޷�

//int LimitL(uint8 x);

//��ֵ��
#define My_Threshold_cha     (7)        //��ֵ����ֵ����ֵ

//��͸��
#define RESULT_ROW 90//���ͼ����
#define RESULT_COL 90//��͸��ͼ������
#define         USED_COL                188
#define         USED_ROW                120
#define PER_IMG     mt9v03x_image//SimBinImage:����͸�ӱ任��ͼ��
#define ImageUsed   *PerImg_ip//*PerImg_ip����ʹ�õ�ͼ��ImageUsedΪ����Ѳ�ߺ�ʶ���ͼ��

//������ͼ��
#define image_w  (92)
#define image_h  (92)
#define track_width  (40)        //�������
#define ImageScanInterval   (5)                        //ɨ�ߵķ�Χ
extern uint8 image[image_h][image_w];  //ʹ�õ�ͼ��

//��ͼ�����
#define line_midpoint (44)
#define image_bottom_value (89)
#define image_side_width  (89)
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
    int16 CrossRoad_Flag;
     int16 Bend_Road;                            /*0 :��               1 :�����     2 :�����*/
     int16 image_element_rings;                  /*0 :��Բ��          1 :��Բ��       2 :��Բ��*/
     int16 ring_big_small;                       /*0:��                     1 :��Բ��       2 :СԲ��*/
     int16 image_element_rings_flag;             /*Բ������*/
     int16 straight_long[3];                        /*��ֱ����־λ,1Զ�ˣ�2�жΣ�3���ˣ���־��1�������*/
//     int16 Garage_Location;                      /*0 :�޳���          1 :�󳵿�       2 :�ҳ���*/
     int16 Zebra_Flag;                           /*0 :�ް�����       1 �󳵿�       2 :�ҳ���*/
     int16 Zebra_Flag_count;
//     int16 Ramp;                                  /*0 :���µ�          1���µ�*/
     int16 RoadBlock_Flag;                        /*0 :��·��            1 :·��*/
     int16 Out_Road;                               /*0 :�޶�·      ���� :����*/
     int16 stop_flag;                              /*ֹͣ��־λ*/
     int16 run_flag;
     int16 mid_choose;                              //�����־��Ϊ����Ѳ���������ϰ���ʱ���ܹ�����
} ImageFlagtypedef;

extern Image_Status imagestatus;
extern Sideline_status Sideline_status_array[90];
extern ImageFlagtypedef imageflag;

//ÿ�����������ͣ��ݴ棩
typedef struct {
  int point;
  uint8 type;
} JumpPointtypedef;//T������W���䣬H������


/*ֱ����������������*//*ֱ����������������*//*ֱ����������������*/
/*ֱ����������������*//*ֱ����������������*//*ֱ����������������*/
/*ֱ����������������*//*ֱ����������������*//*ֱ����������������*/
/*ֱ����������������*//*ֱ����������������*//*ֱ����������������*/
void ALL_init(void);                                                //��ʼ������ͷ����͸�ӣ�tft��Ļ
void Camera_tracking(void);                                         //ͼ�����ܺ���
float midline_and_anglereturn(uint8 mode);
/*ֱ����������������*//*ֱ����������������*//*ֱ����������������*/
/*ֱ����������������*//*ֱ����������������*//*ֱ����������������*/
/*ֱ����������������*//*ֱ����������������*//*ֱ����������������*/
/*ֱ����������������*//*ֱ����������������*//*ֱ����������������*/



void ImagePerspective_Init(void);   //��͸�ӳ�ʼ��
void Binaryzation(void);            //��ȡ��ֵ����ֵ
void image_draw(void);             //��ֵ��
void Get_BaseLine(void);          //����ͼ��Ѳ�ߴ���
void Get_AllLine(void);           //ȫͼ��Ѳ��
void camera_tft180show(void);     //��ʾѲ�߽��
float angle_compute(uint8 x1,uint8 y1,uint8 x2,uint8 y2);
void connect_line_subsidiary(uint8 y_up,uint8 y_down,uint8 x_up,uint8 x_down,uint8 mode);






#endif /* REMAKE_CAMERA_H_ */
