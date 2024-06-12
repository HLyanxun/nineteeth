/*
 * Camera_Processing.h
 *
 *  Created on: 2024��3��17��
 *      Author: 21335
 */

#ifndef CODE_CAMERA_PROCESSING_H_
#define CODE_CAMERA_PROCESSING_H_
#include "zf_common_headfile.h"


//��͸��
//#define RESULT_ROW  90           //Y
//#define RESULT_COL  68           //X
//#define         USED_ROW                248  //����͸��ͼ������
//#define         USED_COL                180
//#define PER_IMG     mt9v03x_image//SimBinImage:����͸�ӱ任��ͼ��
//#define ImageUsed   *PerImg_ip//*PerImg_ip����ʹ�õ�ͼ��ImageUsedΪ����Ѳ�ߺ�ʶ���ͼ��
#define RESULT_ROW 90//���ͼ����
#define RESULT_COL 90//��͸��ͼ������
#define         USED_COL                188
#define         USED_ROW                120  //��
#define PER_IMG     mt9v03x_image//SimBinImage:����͸�ӱ任��ͼ��
#define ImageUsed   *PerImg_ip//*PerImg_ip����ʹ�õ�ͼ��ImageUsedΪ����Ѳ�ߺ�ʶ���ͼ��
//��ͼ��
#define image_w  (92)
#define image_h  (92)
extern float track_width;//�������
#define image_xl  ((image_w-track_width)/2)  //��ɫ�������߽��
#define image_xr  ((image_w+track_width)/2)  //��ɫ�������߽��
#define centre_image    (34.0)   //ͼ������



extern uint8 image[image_h][image_w];
extern uint8  My_Threshold, My_Threshold_1; //��ֵ��֮�����ֵ
extern float  My_Threshold_cha ;//��ֵ���� ֮���ӵ�ui������Խ��е���

extern uint8 Find_Boundary_l[300][2];//��߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
extern uint8 Find_Boundary_r[300][2];//�ұ߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
extern uint8 Find_Boundary_z[300][2];//�б߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
extern uint8 lost_line[300][2];//0�� 1��   �Ƿ��� ��ֵ����0���� ����1����
extern uint8 lostline_flag_l,lostline_flag_r;

extern uint16 l_data_statics;//ͳ������ܹ��ĳ���
extern uint16 r_data_statics;//ͳ���ұ��ܹ��ĳ���
extern uint8 hight_image;
extern uint16 centre,centre_last,centre_s;//���ʹ�õ�����ֵ

extern uint8 l_shade_255,r_shade_255; //����Ҳ���Ӱ  Ҳ���� ��ɫ����

extern int zebra_crossing_flag;  //�����߱�־λ

extern int cross_flag;
extern uint8 l_Down[4],l_on[4],r_Down[4],r_on[4]; //ʮ�ֵ��ĸ��յ� 0y1x
extern uint8 curvature_l[6],curvature_r[6];  //���ұ������� ����012  �ж�3  Զ��4
extern uint8 straight_l,straight_r;  //ֱ���ж� 1Ϊ ����ֱ 2�ж�ֱ 3Զ��ֱ
extern uint8 Corners_l,Corners_r;  //����жϱ�־λ 1Ϊ ���
extern uint8 ramp_flag,ramp_widch[4];

extern double curvature_l_l;
extern uint8 is_L_on[4],is_R_on[4];  //���� ���� ���������յ�

extern uint8 is_L_down[3],is_R_down[3];  //���� ���� ���� �����յ�0y1x
extern int L_island_flag,R_island_flag;  //
extern uint32 island_length;
extern int island_big;


extern int speed_flag,speed_in_flag;
extern float speed_increase;
extern int32 meanCurvatureApprox;



extern int run_flag ;//����
extern int out_flag ;//����
extern uint16 lr_en_speed;//����ƽ���������ۼ�ֵ
extern int check_flag;//�������
extern int16 l_en_speed,r_en_speed; //�� �� ��������ֵ
extern int16 l_en_speed_last,r_en_speed_last; //�� �� ��������һ�ε�ֵ
extern float speed_l,speed_r; //�ٶ�
extern float Servos_out; //������ֵ

typedef struct _attitude_t   // ��Ҫ��ÿ����ֵ¼����Ӧ�Ƕ�
{
    float roll;
    float pitch;
    float yaw;
} attitude_t;
typedef struct PID
{
    float Kp;   //ֱ�� p
    float Ki;
    float Kd;
    float Err;
    float Err_last;
    float Out;
} PID;
extern PID Motor_pid_l;
extern PID Motor_pid_r;
extern attitude_t g_attitude;
extern attitude_t angle;  //����ʹ��

uint8 get_Threshold(void);
void ImagePerspective_Init(void);
void empty_flag(void);
uint8 my_adapt_threshold(uint8 *image, uint16 col, uint16 row);
uint8 my_adapt_threshold_2(uint8 *image, uint16 col, uint16 row);
uint8 otsuThreshold(uint8 *image, uint16 width,  uint16 height);
void Binaryzation(void);
void image_draw(void);
void shade_compute(void);
int Find_Boundary(void);
void search(uint16 break_flag,uint8(*image_u)[image_w],uint8* hight);
void lost_line_judge(void);
void curvature_judge(void);
void Stayguy_ADS(int x_start, int y_start, int x_end, int y_end);
double Angel_compute(int x1,int y1,int x2,int y2,int x3,int y3);
void find_cross_down_l(void);
void find_cross_down_r(void);
void find_cross_on_l(void);
void find_cross_on_r(void);
void find_incline_cross_on_l(void);
void find_incline_cross_on_r(void);
void cross_checkout(void);
void cross_judgment_positive(void);
void cross_judgment_slanting(void);
void cross_dispose(void);
void Inflexion_Point(void);
void xianshi_a(void);
void Camera_tracking(void);
//void analyzeCurvatureWithInterval(uint8(*points)[300], uint16 n, int interval);

#endif /* CODE_CAMERA_PROCESSING_H_ */
