/*
 * find_line_test.h
 *
 *  Created on: 2024年4月27日
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

#define MAX_BLACK_PIXELS 10 // 区域内黑点数量的最大允许值

#define LimitL(L) (L = ((L < 1) ? 1 : L))    //限幅限幅
#define LimitH(H) (H = ((H > 92) ? 92            : H))  //限幅限幅

typedef struct {
    int row;
    int col;
} Obstacle;
// 检测障碍物位置并存储的函数
//typedef struct {
//    int x;
//    int y;
//} CornerPoint;
////  记录拐点坐标的结构体
typedef struct {
    uint8 leftline;////左边线 梯度扫线
    uint8 rightline;//右边线
    uint8 midline;
    uint8 wide;                 //宽度
    uint8 IsRightFind;          //右边有边标志
    uint8 IsLeftFind;         //左边有边标志
    int LeftBoundary;          //左边界 保存最后一次数据
    int RightBoundary;     //右边界 保存最后一次数据
    int De_m;                                 //中线偏离,正为右，负为左
    //左右手法则扫线数据
    int LeftBoundary_First;  //左边界 保存第一次数据
    int RightBoundary_First;  //右边界 保存第一次数据

} Sideline_status;// 边线情况
typedef struct {
    int point;
    uint8 type;
} Jumppoint;// 黑白跳变点记录
typedef struct {
    int y;
    uint8 num;
} ChangePoint;// 障碍识别辅助
typedef struct {
    //图像信息
    //int TowPoint;                                //控制行，也就是我说的前瞻

    int16 OFFLine;                              //图像顶边
    int16 WhiteLine;                            //双边丢边数
    int16 OFFLineBoundary;                      //八领域截止行
    int16 Miss_Left_lines;                      //左线丢失
    int16 Miss_Right_lines;                     //右线丢失

    //左右手法则扫线数据
    int16 WhiteLine_L;                          //左边丢线数
    int16 WhiteLine_R;                          //右边丢线数
    char status[20];
} Image_Status;
// 图像情况结构体
typedef struct {

    int16 CrossRoad_Flag;
    int16 Bend_Road;                            /*0 :无               1 :右弯道     2 :左弯道*/
    int16 image_element_rings;                  /*0 :无圆环          1 :左圆环       2 :右圆环*/
    int16 ring_big_small;                       /*0:无                     1 :大圆环       2 :小圆环*/
    int16 image_element_rings_flag;             /*圆环进程*/
    int16 straight_long;                        /*长直道标志位*/
    int16 straight_xie;
    int16 Garage_Location;                      /*0 :无车库          1 :左车库       2 :右车库*/
    int16 Zebra_Flag;                           /*0 :无斑马线       1 左车库       2 :右车库*/
    int16 Ramp;                                  /*0 :无坡道          1：坡道*/
    int16 RoadBlock_Flag;                        /*0 :无路障            1 :路障*/
    int16 Out_Road;                               /*0 :无断路      1 :断路*/
    int16 stop_flag;                              /*停止标志位*/
} Image_Flag;// 黑白跳变点记录


extern uint8 ex_mt9v03x_binarizeImage[MT9V03X_H/2][MT9V03X_W/2];
extern uint8 ex_threshold;                                            //大津法阈值
extern int16 exposure_time;                                           //曝光时间
extern int16 threshold_fix;                                          //二值化阈值补正
extern int THRESHOLD;
extern uint16 wide_sum;//////////////////

extern Sideline_status Sideline_status_array[MT9V03X_H/2];
extern Image_Status imagestatus;
extern Image_Flag imageflag;


void Image_Process(void);                   //看不懂用这个就行


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
void Element_Judgment_Zebra();//斑马线判断
void Element_Judgment_Bend();
void Straight_long_judge(void);     //返回结果小于1即为直线
void Straight_long_handle(void);     //返回结果小于1即为直线

void auto_extension_line(void);

#endif /* FIND_LINE_TEST_H_ */
