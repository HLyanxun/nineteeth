/*
 * Camera_Process.h
 *
 *  Created on: 2024年4月29日
 *      Author: pc
 */

#ifndef CAMERA_PROCESS_H_
#define CAMERA_PROCESS_H_
#include "zf_common_headfile.h"

#define image_w  (70)
#define image_h  (92)

//逆透视
#define RESULT_ROW  (90)           //Y
#define RESULT_COL  (70)           //X
#define         USED_ROW                (248)  //用于透视图的行列
#define         USED_COL                (180)
#define ImageUsed   *PerImg_ip//*PerImg_ip定义使用的图像，ImageUsed为用于巡线和识别的图像
#define PER_IMG     mt9v03x_image//SimBinImage:用于透视变换的图像

/*直接用，都要用*/
void Camera_tracking(void);         //图像处理总函数
void ImagePerspective_Init(void);
void Inflexion_Point(void);  //四个拐点  显示     (不可用状态)
void xianshi_a(void);        //边线显示              (不可用状态)





//tft180_show_gray_image(0, 0, ex_mt9v03x_binarizeImage[0], image_w, image_h, image_w, image_h, 0)//用来显示二值化后的图像

//typedef struct {
//    uint8 leftline;////左边线 梯度扫线
//    uint8 rightline;//右边线
//    uint8 midline;
//    uint8 wide;                 //宽度
//    uint8 IsRightFind;          //右边有边标志
//    uint8 IsLeftFind;         //左边有边标志
//    int LeftBoundary;          //左边界 保存最后一次数据
//    int RightBoundary;     //右边界 保存最后一次数据
//    int De_m;                                 //中线偏离,正为右，负为左
    //左右手法则扫线数据
//    int LeftBoundary_First;  //左边界 保存第一次数据
//    int RightBoundary_First;  //右边界 保存第一次数据

//} Sideline_status;// 边线情况

typedef struct {
    //图像信息
    //int TowPoint;                                //控制行，也就是我说的前瞻

    int16 OFFLine;                              //图像顶边
    int16 WhiteLine;                            //双边丢边数
    int16 OFFLineBoundary;                      //八领域截止行
    int16 Miss_Left_lines;                      //左线丢失
    int16 Miss_Right_lines;                     //右线丢失
    int Det_True;                               //中线误差
    //左右手法则扫线数据
    int16 WhiteLine_L;                          //左边丢线数
    int16 WhiteLine_R;                          //右边丢线数
    char status[20];
} Image_Status;

//typedef struct {
//  int point;
//  uint8 type;
//} JumpPointtypedef;

typedef struct {
     int16 CrossRoad_Flag;
     int16 Bend_Road;                            /*0 :无               1 :右弯道     2 :左弯道*/
     int16 image_curvature[2][6];                  /*曲率标志位第一个参数0为左，1为右，第二个参数储存不同段圆环识别数据*/
     int16 image_element_rings[2];                   /*[0]左环标志[1]右环标志*/
     int16 ring_big_small;                       /*0:无                     1 :大圆环       2 :小圆环*/
     int16 image_element_rings_flag;             /*圆环进程*/
     int16 straight_long;                        /*长直道标志位*/
     int16 straight[2];                          /*直线判断标志数组 1为 近端直 2中段直 3远段直【0】左直线【1】右直线*/
     int16 Garage_Location;                      /*0 :无车库          1 :左车库       2 :右车库*/
     int16 Zebra_Flag;                           /*0 :无斑马线       1 左车库       2 :右车库*/
     int16 Ramp;                                  /*0 :无坡道          1：坡道*/
     int16 RoadBlock_Flag;                        /*0 :无路障            1 :路障*/
     int16 Out_Road;                               /*0 :无断路      1 :断路*/
     int16 stop_flag;                              /*停止标志位*/
     int16 run_flag;
} ImageFlagtypedef;

//extern Sideline_status Sideline_status_array[image_h];
extern Image_Status imagestatus;
extern ImageFlagtypedef imageflag;
extern int16 threshold_fix;                                          //二值化阈值补正
extern uint8 My_Threshold;                                           //最终の二值化阈值
extern uint8 ex_mt9v03x_binarizeImage[image_h][image_w];

void Binaryzation(void);
//uint8 my_adapt_threshold(uint8 *ex_mt9v03x_binarizeImage, uint16 col, uint16 row);   //注意计算阈值的一定要是原图像
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
