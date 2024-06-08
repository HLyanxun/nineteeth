/*
 * Camera_Process.h
 *
 *  Created on: 2024��4��29��
 *      Author: pc
 */

#ifndef CAMERA_PROCESS_H_
#define CAMERA_PROCESS_H_
#include "zf_common_headfile.h"

#define image_w  (70)
#define image_h  (92)

//��͸��
#define RESULT_ROW  (90)           //Y
#define RESULT_COL  (70)           //X
#define         USED_ROW                (248)  //����͸��ͼ������
#define         USED_COL                (180)
#define ImageUsed   *PerImg_ip//*PerImg_ip����ʹ�õ�ͼ��ImageUsedΪ����Ѳ�ߺ�ʶ���ͼ��
#define PER_IMG     mt9v03x_image//SimBinImage:����͸�ӱ任��ͼ��

/*ֱ���ã���Ҫ��*/
void Camera_tracking(void);         //ͼ�����ܺ���
void ImagePerspective_Init(void);
void Inflexion_Point(void);  //�ĸ��յ�  ��ʾ     (������״̬)
void xianshi_a(void);        //������ʾ              (������״̬)





//tft180_show_gray_image(0, 0, ex_mt9v03x_binarizeImage[0], image_w, image_h, image_w, image_h, 0)//������ʾ��ֵ�����ͼ��

//typedef struct {
//    uint8 leftline;////����� �ݶ�ɨ��
//    uint8 rightline;//�ұ���
//    uint8 midline;
//    uint8 wide;                 //���
//    uint8 IsRightFind;          //�ұ��б߱�־
//    uint8 IsLeftFind;         //����б߱�־
//    int LeftBoundary;          //��߽� �������һ������
//    int RightBoundary;     //�ұ߽� �������һ������
//    int De_m;                                 //����ƫ��,��Ϊ�ң���Ϊ��
    //�����ַ���ɨ������
//    int LeftBoundary_First;  //��߽� �����һ������
//    int RightBoundary_First;  //�ұ߽� �����һ������

//} Sideline_status;// �������

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

//typedef struct {
//  int point;
//  uint8 type;
//} JumpPointtypedef;

typedef struct {
     int16 CrossRoad_Flag;
     int16 Bend_Road;                            /*0 :��               1 :�����     2 :�����*/
     int16 image_curvature[2][6];                  /*���ʱ�־λ��һ������0Ϊ��1Ϊ�ң��ڶ����������治ͬ��Բ��ʶ������*/
     int16 image_element_rings[2];                   /*[0]�󻷱�־[1]�һ���־*/
     int16 ring_big_small;                       /*0:��                     1 :��Բ��       2 :СԲ��*/
     int16 image_element_rings_flag;             /*Բ������*/
     int16 straight_long;                        /*��ֱ����־λ*/
     int16 straight[2];                          /*ֱ���жϱ�־���� 1Ϊ ����ֱ 2�ж�ֱ 3Զ��ֱ��0����ֱ�ߡ�1����ֱ��*/
     int16 Garage_Location;                      /*0 :�޳���          1 :�󳵿�       2 :�ҳ���*/
     int16 Zebra_Flag;                           /*0 :�ް�����       1 �󳵿�       2 :�ҳ���*/
     int16 Ramp;                                  /*0 :���µ�          1���µ�*/
     int16 RoadBlock_Flag;                        /*0 :��·��            1 :·��*/
     int16 Out_Road;                               /*0 :�޶�·      1 :��·*/
     int16 stop_flag;                              /*ֹͣ��־λ*/
     int16 run_flag;
} ImageFlagtypedef;

//extern Sideline_status Sideline_status_array[image_h];
extern Image_Status imagestatus;
extern ImageFlagtypedef imageflag;
extern int16 threshold_fix;                                          //��ֵ����ֵ����
extern uint8 My_Threshold;                                           //���դζ�ֵ����ֵ
extern uint8 ex_mt9v03x_binarizeImage[image_h][image_w];

void Binaryzation(void);
//uint8 my_adapt_threshold(uint8 *ex_mt9v03x_binarizeImage, uint16 col, uint16 row);   //ע�������ֵ��һ��Ҫ��ԭͼ��
double Angel_compute(int x1, int y1, int x2, int y2, int x3, int y3);

void  Stayguy_ADS(int x_start, int y_start, int x_end, int y_end);
void search(uint16 break_flag,uint8(*image_u)[image_w],uint8* hightest);
void curvature_judge(void);


#endif /* CAMERA_PROCESS_H_ */
/*
 * Image==Ex_Mt9v03x_Binarizeimage
 * Run_Flag==Imageflag.Run_Flag
 * Out_Flag==Imageflag.Out_Road
 * Curvature_L==imageflag.image_curvature[0][];
 * Curvature_r==imageflag.image_curvature[1][];
 * zebra_crossing_flag==imageflag.Zebra_Flag;
 * Straight_R==Imageflag.Straight[1]
 * Straight_L==Imageflag.Straight[0]
 * Cross_flag==imageflag.crossroad_flag;
 * L_island_flag==imageflag.image_element_rings[0];
 * R_island_flag==imageflag.image_element_rings[1];
 * ips200_draw_point==tft180_draw_point;
 */
