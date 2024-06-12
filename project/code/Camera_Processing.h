/*
 * Camera_Processing.h
 *
 *  Created on: 2024年3月17日
 *      Author: 21335
 */

#ifndef CODE_CAMERA_PROCESSING_H_
#define CODE_CAMERA_PROCESSING_H_
#include "zf_common_headfile.h"


//逆透视
//#define RESULT_ROW  90           //Y
//#define RESULT_COL  68           //X
//#define         USED_ROW                248  //用于透视图的行列
//#define         USED_COL                180
//#define PER_IMG     mt9v03x_image//SimBinImage:用于透视变换的图像
//#define ImageUsed   *PerImg_ip//*PerImg_ip定义使用的图像，ImageUsed为用于巡线和识别的图像
#define RESULT_ROW 90//结果图行列
#define RESULT_COL 90//于透视图的行列
#define         USED_COL                188
#define         USED_ROW                120  //用
#define PER_IMG     mt9v03x_image//SimBinImage:用于透视变换的图像
#define ImageUsed   *PerImg_ip//*PerImg_ip定义使用的图像，ImageUsed为用于巡线和识别的图像
//总图像
#define image_w  (92)
#define image_h  (92)
extern float track_width;//赛道宽度
#define image_xl  ((image_w-track_width)/2)  //白色赛道左侧边界点
#define image_xr  ((image_w+track_width)/2)  //白色赛道左侧边界点
#define centre_image    (34.0)   //图像中心



extern uint8 image[image_h][image_w];
extern uint8  My_Threshold, My_Threshold_1; //二值化之后的阈值
extern float  My_Threshold_cha ;//阈值补偿 之后扔到ui里面可以进行调节

extern uint8 Find_Boundary_l[300][2];//左边界线 0为y  1为x 存储内容为八邻域扫描出来的边界
extern uint8 Find_Boundary_r[300][2];//右边界线 0为y  1为x 存储内容为八邻域扫描出来的边界
extern uint8 Find_Boundary_z[300][2];//中边界线 0为y  1为x 存储内容为八邻域扫描出来的边界
extern uint8 lost_line[300][2];//0左 1右   是否丢线 数值等于0不丢 等于1丢线
extern uint8 lostline_flag_l,lostline_flag_r;

extern uint16 l_data_statics;//统计左边总共的长度
extern uint16 r_data_statics;//统计右边总共的长度
extern uint8 hight_image;
extern uint16 centre,centre_last,centre_s;//舵机使用的中线值

extern uint8 l_shade_255,r_shade_255; //左侧右侧阴影  也就是 白色方块

extern int zebra_crossing_flag;  //斑马线标志位

extern int cross_flag;
extern uint8 l_Down[4],l_on[4],r_Down[4],r_on[4]; //十字的四个拐点 0y1x
extern uint8 curvature_l[6],curvature_r[6];  //左右边线曲率 近段012  中段3  远段4
extern uint8 straight_l,straight_r;  //直线判断 1为 近端直 2中段直 3远段直
extern uint8 Corners_l,Corners_r;  //弯道判断标志位 1为 弯道
extern uint8 ramp_flag,ramp_widch[4];

extern double curvature_l_l;
extern uint8 is_L_on[4],is_R_on[4];  //环岛 左上 右上两个拐点

extern uint8 is_L_down[3],is_R_down[3];  //环岛 左下 右下 两个拐点0y1x
extern int L_island_flag,R_island_flag;  //
extern uint32 island_length;
extern int island_big;


extern int speed_flag,speed_in_flag;
extern float speed_increase;
extern int32 meanCurvatureApprox;



extern int run_flag ;//发车
extern int out_flag ;//出界
extern uint16 lr_en_speed;//左右平均编码器累计值
extern int check_flag;//发车检查
extern int16 l_en_speed,r_en_speed; //左 右 编码器的值
extern int16 l_en_speed_last,r_en_speed_last; //左 右 编码器上一次的值
extern float speed_l,speed_r; //速度
extern float Servos_out; //舵机输出值

typedef struct _attitude_t   // 需要对每个数值录入相应角度
{
    float roll;
    float pitch;
    float yaw;
} attitude_t;
typedef struct PID
{
    float Kp;   //直线 p
    float Ki;
    float Kd;
    float Err;
    float Err_last;
    float Out;
} PID;
extern PID Motor_pid_l;
extern PID Motor_pid_r;
extern attitude_t g_attitude;
extern attitude_t angle;  //环岛使用

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
