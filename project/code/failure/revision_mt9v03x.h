/*
 * revision_mt9v03x.h
 *
 *  Created on: 2024年4月28日
 *      Author: pc
 */

#ifndef REVISION_MT9V03X_H_
#define REVISION_MT9V03X_H_
#include "zf_common_headfile.h"

#define LCDH 60                              //用于图像处理的图像高度（行）
#define LCDW 94                              //用于图像处理的图像宽度（列）
#define ImageSensorMid 39                    //图像的屏幕中点
#define LimitL(L) (L = ((L < 1) ? 1 : L))    //限幅限幅
#define LimitH(H) (H = ((H > 78) ? 78 : H))  //限幅限幅
#define Garage_Number 2                      //车库次数
extern int Mid_line[60];                   //最开始写的Get_SideLine搜边线函数里面用来存放中线的数组
extern int Left_line[60];                  //最开始写的Get_SideLine搜边线函数里面用来存放左边线的数组
extern int Right_line[60];                 //最开始写的Get_SideLine搜边线函数里面用来存放右边线的数组
extern int Out_Garage_Help_Flag;   // 1 :左边出车库   2 :右边出车库
extern int Out_Garage_Help_enable;   // 1 :出车库   2 :已出车库
extern int RoadBlock_length;
extern uint8 ex_mt9v03x_binarizeImage[LCDH][LCDW];
extern uint8* Image_Use[LCDH][LCDW];
extern uint8 Threshold;
extern uint16 threshold1,threshold2,threshold3,block_yuzhi;
extern uint16 yuzhi1,yuzhi2,yuzhi3;
extern uint16 Ramp_cancel,circle_stop,block_num,duan_num;
extern int Black;
extern uint8 Auto_Through_RoadBlock ; // 自动选择通过路障方式   1 : 左通过路障   2 : 右通过路障
extern uint8 RoadBlock_Flag ; //路障位置  1 : 第一段打角  2 : 编码器计数行进路程  3 : 第二段打角回到赛道  4 : 为了让某个函数常进特地写的数字
extern uint8 RoadBlock_Thruough_Flag ; //路障通过方式  1 : 左通过  2 : 右通过
extern uint8 RoadBlock_Thruough_Flag_Record,Bend_Thruough_RoadBlock_Flag,ICM20602_Clear_Flag ;
extern uint8 Regular_RoadBlock;
//每一行的属性
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

typedef struct {
  int point;
  uint8 type;
} JumpPointtypedef;

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
