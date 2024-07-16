/*
 * remake_camera.h
 *
 *  Created on: 2024年7月4日
 *      Author: pc
 */
//已经全都木大木大木大木大木大木大木大木大木大木大木大木大木大了
#ifndef REMAKE_CAMERA_H_
#define REMAKE_CAMERA_H_
#include "zf_common_headfile.h"

//检测选项开关
#define track_width_debug   (0)          //赛道宽度标定工具显示
#define default_side_choose (0)         //0默认选择左边线巡线，1默认选择右边线巡线
#define track_show          (1)          //是否显示巡线结果
#define image_type          (0)         //0二值化图像，1灰度图像
#define track_show_8        (0)         //是否显示八邻域巡线结果

//算法定义
#define LimitL(L) (L = ((L < 1) ? 1 : L))    //限幅限幅
#define LimitH(H) (H = ((H > 89) ? 89 : H))  //限幅限幅

//int LimitL(uint8 x);

//二值化
#define My_Threshold_cha     (7)        //二值化阈值补偿值

//逆透视
#define RESULT_ROW 90//结果图行列
#define RESULT_COL 90//于透视图的行列
#define         USED_COL                188
#define         USED_ROW                120
#define PER_IMG     mt9v03x_image//SimBinImage:用于透视变换的图像
#define ImageUsed   *PerImg_ip//*PerImg_ip定义使用的图像，ImageUsed为用于巡线和识别的图像

//处理用图像
#define image_w  (92)
#define image_h  (92)
#define track_width  (40)        //赛道宽度
#define ImageScanInterval   (5)                        //扫边的范围
extern uint8 image[image_h][image_w];  //使用的图像

//总图像参数
#define line_midpoint (44)
#define image_bottom_value (89)
#define image_side_width  (89)
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
    int16 CrossRoad_Flag;
     int16 Bend_Road;                            /*0 :无               1 :右弯道     2 :左弯道*/
     int16 image_element_rings;                  /*0 :无圆环          1 :左圆环       2 :右圆环*/
     int16 ring_big_small;                       /*0:无                     1 :大圆环       2 :小圆环*/
     int16 image_element_rings_flag;             /*圆环进程*/
     int16 straight_long[3];                        /*长直道标志位,1远端，2中段，3近端，标志置1代表存在*/
//     int16 Garage_Location;                      /*0 :无车库          1 :左车库       2 :右车库*/
     int16 Zebra_Flag;                           /*0 :无斑马线       1 左车库       2 :右车库*/
     int16 Zebra_Flag_count;
//     int16 Ramp;                                  /*0 :无坡道          1：坡道*/
     int16 RoadBlock_Flag;                        /*0 :无路障            1 :路障*/
     int16 Out_Road;                               /*0 :无断路      其他 :出界*/
     int16 stop_flag;                              /*停止标志位*/
     int16 run_flag;
     int16 mid_choose;                              //这个标志是为了让巡线在遇到障碍的时候能够换边
} ImageFlagtypedef;

extern Image_Status imagestatus;
extern Sideline_status Sideline_status_array[90];
extern ImageFlagtypedef imageflag;

//每行赛道的类型（暂存）
typedef struct {
  int point;
  uint8 type;
} JumpPointtypedef;//T正常，W跳变，H大跳变


/*直接用这俩函数就行*//*直接用这俩函数就行*//*直接用这俩函数就行*/
/*直接用这俩函数就行*//*直接用这俩函数就行*//*直接用这俩函数就行*/
/*直接用这俩函数就行*//*直接用这俩函数就行*//*直接用这俩函数就行*/
/*直接用这俩函数就行*//*直接用这俩函数就行*//*直接用这俩函数就行*/
void ALL_init(void);                                                //初始化摄像头，逆透视，tft屏幕
void Camera_tracking(void);                                         //图像处理总函数
float midline_and_anglereturn(uint8 mode);
/*直接用这俩函数就行*//*直接用这俩函数就行*//*直接用这俩函数就行*/
/*直接用这俩函数就行*//*直接用这俩函数就行*//*直接用这俩函数就行*/
/*直接用这俩函数就行*//*直接用这俩函数就行*//*直接用这俩函数就行*/
/*直接用这俩函数就行*//*直接用这俩函数就行*//*直接用这俩函数就行*/



void ImagePerspective_Init(void);   //逆透视初始化
void Binaryzation(void);            //获取二值化阈值
void image_draw(void);             //二值化
void Get_BaseLine(void);          //基础图像巡线处理
void Get_AllLine(void);           //全图像巡线
void camera_tft180show(void);     //显示巡线结果
float angle_compute(uint8 x1,uint8 y1,uint8 x2,uint8 y2);
void connect_line_subsidiary(uint8 y_up,uint8 y_down,uint8 x_up,uint8 x_down,uint8 mode);






#endif /* REMAKE_CAMERA_H_ */
