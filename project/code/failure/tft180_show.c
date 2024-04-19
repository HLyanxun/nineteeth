/*
 * tft180_show.c
 *
 *  Created on: 2024年3月11日
 *      Author: pc
 */
#include "zf_common_headfile.h"



const uint8 yu_zhi[8][16]=
{
        {0x20,0x00,0x17,0xFC,0x00,0x84,0x40,0xA4,0x40,0x94,0x5F,0xF4,0x40,0x84,0x4E,0xA4},
        {0x4A,0xA4,0x4E,0xA4,0x40,0xC4,0x46,0x54,0x58,0xB4,0x41,0x14,0x42,0x04,0x40,0x0C},/*"阈",0*/
        {0x08,0x40,0x08,0x40,0x0F,0xFC,0x10,0x40,0x10,0x40,0x33,0xF8,0x32,0x08,0x53,0xF8},
        {0x92,0x08,0x13,0xF8,0x12,0x08,0x13,0xF8,0x12,0x08,0x12,0x08,0x1F,0xFE,0x10,0x00},/*"值",1*/
};

//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     将寻线所得边线可视化显示
//--------------------------------------------------------------------------------------------------------------------------
void line_visualization(void)
{
    uint8 left,mid,right;
    for (int i = 58; i > 0; i--)
        {
            left=Sideline_status_array[i].leftline;
            mid=Sideline_status_array[i].midline;
            right=Sideline_status_array[i].rightline;
//        if(!flag)
//        {

            LimitL(left);                                                             //这里就是对传给Get_Border_And_SideType()函数的扫描区间进行一个限幅操作。
            LimitH(left);
            LimitL(mid);                                                             //这里就是对传给Get_Border_And_SideType()函数的扫描区间进行一个限幅操作。
            LimitH(mid);
            LimitL(right);                                                             //这里就是对传给Get_Border_And_SideType()函数的扫描区间进行一个限幅操作。
            LimitH(right);
//            flag=1;
//        }
//            tft180_draw_point(Sideline_status_array[i].leftline, i, RGB565_RED);  // 绘制中线
//            tft180_draw_point(Sideline_status_array[i].midline,i,RGB565_BLUE);
//            tft180_draw_point(Sideline_status_array[i].rightline,i,RGB565_PURPLE);
            tft180_draw_point(left, i, RGB565_RED);  // 绘制中线
            tft180_draw_point(mid,i,RGB565_BLUE);
            tft180_draw_point(right,i,RGB565_PURPLE);
            //tft180_draw_point(((Sideline_status_array[i].rightline+Sideline_status_array[i].leftline)/2)+1, i+1, RGB565_RED);  // 绘制中线
        }
}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     状态显示
//--------------------------------------------------------------------------------------------------------------------------
void status_show(void)
{
//    if(!zxy)tft180_show_string(70, 81, "loading");
//    else
        if(imageflag.image_element_rings_flag)tft180_show_string(70, 81, "element");
    else if(imageflag.Zebra_Flag)tft180_show_string(70,81,"_Zebra_");
    else if(imageflag.Bend_Road)tft180_show_string(70,81,"_Bend__");
    else if(imagestatus.WhiteLine >= 8)tft180_show_string(70,81,"cross__");
    else tft180_show_string(70,81,"normal_");
}

//---------------------------------------------------------------------------------------------------------------------------
// 函数简介     对各项参数显示
//---------------------------------------------------------------------------------------------------------------------------
void tft180show_better(uint8 mode)
{
    float fps_show=0;
    static uint8 mt9v034_mode=0;
    uint8 yu_zhi_show=0;
    static int fps_count=0;
    switch(mode)
    {

    case 2://二值化图像以及扫线情况显示
        if(fps_count==0)
        {
            timer_clear(TIM_3);
        }
        tft180_show_gray_image(1, 1, ex_mt9v03x_binarizeImage[0], MT9V03X_W/2, MT9V03X_H/2, (MT9V03X_W/2), (MT9V03X_H/2), 0);


//        line_visualization();
        fps_count++;

        if(fps_count>=30)
        {
            fps_show=timer_get(TIM_3);
            fps_show=(fps_count/fps_show)*1000;
            if(fps_show>=MT9V03X_FPS_DEF)fps_show=MT9V03X_FPS_DEF;
            fps_count=0;
            yu_zhi_show=ex_threshold;

//            tft180_show_uint(7*16, 0, mode, 1);
            //fps显示
            tft180_show_string(1, 4*16, "fps:");
            tft180_show_float(32,4*16,fps_show,6,2);
            //二值化阈值显示
            tft180_show_chinese(0, 5*16, 16, yu_zhi[0], 4, RGB565_BLACK);
            tft180_show_string(32,5*16,":");
            tft180_show_uint(16*3-8,81,yu_zhi_show,3);

            tft180_show_uint(0,6*16,Sideline_status_array[59].leftline,2);
            tft180_show_uint(2*16,6*16,Sideline_status_array[59].midline,2);
            tft180_show_uint(4*16,6*16,Sideline_status_array[59].rightline,3);

            tft180_show_string(0,7*16,"stop:");
            tft180_show_uint(44,7*16,imageflag.stop_flag,3);

            tft180_show_string(0,8*16,"flag:");
            tft180_show_uint(3*16,8*16,imageflag.Zebra_Flag,5);

            tft180_show_uint(80,9*16,imageflag.image_element_rings,5);

            status_show();

        }

                break;

    case 3://陀螺仪数据显示，以及预设角度调整
        tft180_clear();
        tft180_show_uint(142, 0, mode, 1);
        mpu6050_parameter_tft180_show();
        break;
    }
}
