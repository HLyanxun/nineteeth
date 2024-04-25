/*
 * button.c
 *
 *  Created on: 2024年4月13日
 *      Author: pc
 */
#include "zf_common_headfile.h"
int page=0,line=1;
uint8 Button_mode=0;
uint8 mt9v034_mode=0,line_show_mode=0;
const uint8 bao_guang_shi_jian[8][16]=
{
        {0x03,0xF8,0x02,0x08,0xF3,0xF8,0x92,0x08,0x93,0xF8,0x91,0x10,0x97,0xFC,0xF1,0x10},
        {0x9F,0xFE,0x91,0x10,0x92,0x48,0x95,0x54,0xF0,0xE0,0x91,0x50,0x02,0x48,0x00,0xC0},/*"曝",0*/
        {0x01,0x00,0x21,0x08,0x11,0x08,0x09,0x10,0x09,0x20,0x01,0x00,0xFF,0xFE,0x04,0x40},
        {0x04,0x40,0x04,0x40,0x04,0x40,0x08,0x42,0x08,0x42,0x10,0x42,0x20,0x3E,0xC0,0x00},/*"光",1*/
        {0x00,0x08,0x00,0x08,0x7C,0x08,0x44,0x08,0x45,0xFE,0x44,0x08,0x44,0x08,0x7C,0x08},
        {0x44,0x88,0x44,0x48,0x44,0x48,0x44,0x08,0x7C,0x08,0x44,0x08,0x00,0x28,0x00,0x10},/*"时",2*/
        {0x20,0x00,0x13,0xFC,0x10,0x04,0x40,0x04,0x47,0xC4,0x44,0x44,0x44,0x44,0x44,0x44},
        {0x47,0xC4,0x44,0x44,0x44,0x44,0x44,0x44,0x47,0xC4,0x40,0x04,0x40,0x14,0x40,0x08},/*"间",3*/
};
const uint8 she_xiang_tou[8][16]=
{
        {0x10,0x00,0x13,0xFC,0x11,0x08,0x11,0xF8,0xFD,0x08,0x11,0xF8,0x11,0x0E,0x17,0xF8},
        {0x18,0x08,0x37,0xBC,0xD0,0xA4,0x12,0xA4,0x11,0x28,0x12,0x90,0x54,0xA8,0x28,0x46},/*"摄",0*/
        {0x09,0x00,0x09,0xF0,0x0A,0x10,0x17,0xFC,0x1A,0x24,0x32,0x44,0x33,0xFC,0x50,0x80},
        {0x91,0x44,0x16,0xA8,0x11,0x30,0x16,0x68,0x10,0xA8,0x11,0x24,0x16,0xA2,0x10,0x40},/*"像",1*/
        {0x00,0x80,0x00,0x80,0x08,0x80,0x04,0x80,0x24,0x80,0x10,0x80,0x10,0x80,0x00,0x80},
        {0xFF,0xFE,0x01,0x00,0x01,0x40,0x02,0x20,0x04,0x10,0x08,0x08,0x30,0x04,0xC0,0x04},/*"头",2*/

};
const uint8 can_shu_bao_cun[8][16]=
{
        {0x02,0x00,0x04,0x40,0x08,0x20,0x1F,0xF0,0x02,0x00,0x7F,0xFC,0x08,0x20,0x10,0x90},
        {0x23,0x08,0xCC,0x46,0x01,0x80,0x06,0x10,0x18,0x20,0x00,0xC0,0x07,0x00,0x78,0x00},/*"参",0*/
        {0x08,0x20,0x49,0x20,0x2A,0x20,0x08,0x3E,0xFF,0x44,0x2A,0x44,0x49,0x44,0x88,0xA4},
        {0x10,0x28,0xFE,0x28,0x22,0x10,0x42,0x10,0x64,0x28,0x18,0x28,0x34,0x44,0xC2,0x82},/*"数",1*/
        {0x08,0x00,0x0B,0xF8,0x0A,0x08,0x12,0x08,0x12,0x08,0x33,0xF8,0x30,0x40,0x50,0x40},
        {0x97,0xFC,0x10,0xE0,0x11,0x50,0x12,0x48,0x14,0x44,0x18,0x42,0x10,0x40,0x10,0x40},/*"保",2*/
        {0x04,0x00,0x04,0x00,0xFF,0xFE,0x08,0x00,0x08,0x00,0x13,0xF8,0x10,0x10,0x30,0x20},
        {0x50,0x40,0x97,0xFE,0x10,0x40,0x10,0x40,0x10,0x40,0x10,0x40,0x11,0x40,0x10,0x80},/*"存",3*/

};
const uint8 bian_xian[8][16]=
{
        {0x00,0x40,0x20,0x40,0x10,0x40,0x13,0xFC,0x00,0x44,0x00,0x44,0xF0,0x44,0x10,0x84},
        {0x10,0x84,0x11,0x04,0x11,0x04,0x12,0x28,0x14,0x10,0x28,0x00,0x47,0xFE,0x00,0x00},/*"边",0*/
        {0x10,0x50,0x10,0x48,0x20,0x40,0x24,0x5C,0x45,0xE0,0xF8,0x40,0x10,0x5E,0x23,0xE0},
        {0x40,0x44,0xFC,0x48,0x40,0x30,0x00,0x22,0x1C,0x52,0xE0,0x8A,0x43,0x06,0x00,0x02},/*"线",1*/
};
const uint8 xian_shi[8][16]=
{
        {0x00,0x00,0x1F,0xF0,0x10,0x10,0x10,0x10,0x1F,0xF0,0x10,0x10,0x10,0x10,0x1F,0xF0},
        {0x04,0x40,0x44,0x44,0x24,0x44,0x14,0x48,0x14,0x50,0x04,0x40,0xFF,0xFE,0x00,0x00},/*"显",0*/
        {0x00,0x00,0x3F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFE,0x01,0x00},
        {0x01,0x00,0x11,0x10,0x11,0x08,0x21,0x04,0x41,0x02,0x81,0x02,0x05,0x00,0x02,0x00},/*"示",1*/
};
const uint8 yin_cang[8][16]=
{
        {0x00,0x80,0x78,0x80,0x49,0xF8,0x4A,0x08,0x54,0x10,0x53,0xFC,0x60,0x04,0x51,0xFC},
        {0x48,0x04,0x4B,0xFC,0x48,0x04,0x68,0x40,0x51,0x24,0x45,0x2A,0x45,0x0A,0x48,0xF8},/*"隐",0*/
        {0x04,0x40,0xFF,0xFE,0x04,0x48,0x00,0x24,0x1F,0xFE,0x50,0x20,0x57,0xA0,0x75,0x24},
        {0x17,0xA4,0xF4,0xA8,0x57,0xA8,0x55,0x10,0x95,0x12,0x27,0xAA,0x20,0x46,0x40,0x82},/*"藏",1*/
};
const uint8 tu_xiang[8][16]=
{
        {0x00,0x00,0x7F,0xFC,0x42,0x04,0x42,0x04,0x47,0xE4,0x4C,0x44,0x52,0x84,0x41,0x04},
        {0x46,0xC4,0x78,0x3C,0x43,0x04,0x40,0x84,0x46,0x04,0x41,0x84,0x7F,0xFC,0x40,0x04},/*"图",0*/
        {0x09,0x00,0x09,0xF0,0x0A,0x10,0x17,0xFC,0x1A,0x24,0x32,0x44,0x33,0xFC,0x50,0x80},
        {0x91,0x44,0x16,0xA8,0x11,0x30,0x16,0x68,0x10,0xA8,0x11,0x24,0x16,0xA2,0x10,0x40},/*"像",1*/
};
const uint8 hui_du[8][16]=
{
        {0x04,0x00,0x04,0x00,0x04,0x00,0xFF,0xFE,0x08,0x00,0x08,0x80,0x08,0x84,0x12,0x84},
        {0x12,0x88,0x24,0x90,0x29,0x40,0x41,0x40,0x82,0x20,0x04,0x10,0x18,0x08,0x60,0x06},/*"灰",0*/
        {0x01,0x00,0x00,0x80,0x3F,0xFE,0x22,0x20,0x22,0x20,0x3F,0xFC,0x22,0x20,0x22,0x20},
        {0x23,0xE0,0x20,0x00,0x2F,0xF0,0x24,0x10,0x42,0x20,0x41,0xC0,0x86,0x30,0x38,0x0E},/*"度",1*/
};
//const uint8 test[8][16]=
//{
//        {0x00,0x04,0x27,0xC4,0x14,0x44,0x14,0x54,0x85,0x54,0x45,0x54,0x45,0x54,0x15,0x54},
//        {0x15,0x54,0x25,0x54,0xE5,0x54,0x21,0x04,0x22,0x84,0x22,0x44,0x24,0x14,0x08,0x08},/*"测",0*/
//        {0x00,0x28,0x20,0x24,0x10,0x24,0x10,0x20,0x07,0xFE,0x00,0x20,0xF0,0x20,0x17,0xE0},
//        {0x11,0x20,0x11,0x10,0x11,0x10,0x15,0x10,0x19,0xCA,0x17,0x0A,0x02,0x06,0x00,0x02},/*"试",1*/
//};
const uint8 menu[8][16]=
{
{0x08,0x20,0x08,0x20,0xFF,0xFE,0x08,0x20,0x00,0x10,0x00,0xF8,0x3F,0x00,0x11,0x10},
{0x08,0x20,0x01,0x00,0x7F,0xFC,0x05,0x40,0x09,0x20,0x31,0x18,0xC1,0x06,0x01,0x00},/*"菜",0*/
{0x10,0x10,0x08,0x20,0x04,0x40,0x3F,0xF8,0x21,0x08,0x21,0x08,0x3F,0xF8,0x21,0x08},
{0x21,0x08,0x3F,0xF8,0x01,0x00,0x01,0x00,0xFF,0xFE,0x01,0x00,0x01,0x00,0x01,0x00},/*"单",1*/
};
const uint8 mpu6050_parament[8][16]=
{
        {0x00,0x40,0x7C,0x20,0x44,0x20,0x49,0xFE,0x49,0x02,0x52,0x04,0x49,0x00,0x49,0x08},
        {0x45,0x10,0x45,0x20,0x45,0xC0,0x69,0x04,0x51,0x04,0x41,0x04,0x40,0xFC,0x40,0x00},/*"陀",0*/
        {0x10,0x00,0x11,0xFC,0x11,0x24,0x7D,0xFC,0x55,0x24,0x55,0xFC,0x54,0x40,0x54,0x88},
        {0x7D,0xF0,0x50,0x20,0x10,0x44,0x17,0xFE,0x1C,0x22,0xE5,0x24,0x42,0x22,0x00,0x60},/*"螺",1*/
        {0x08,0x80,0x08,0x48,0x0A,0x48,0x12,0x08,0x12,0x08,0x31,0x10,0x31,0x10,0x51,0x10},
        {0x90,0xA0,0x10,0xA0,0x10,0x40,0x10,0x40,0x10,0xA0,0x11,0x10,0x12,0x08,0x14,0x06},/*"仪",2*/
};

const uint8 erzhihua[8][16]=
{
        {0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00},/*"二",0*/
        {0x08,0x40,0x08,0x40,0x0F,0xFC,0x10,0x40,0x10,0x40,0x33,0xF8,0x32,0x08,0x53,0xF8},
        {0x92,0x08,0x13,0xF8,0x12,0x08,0x13,0xF8,0x12,0x08,0x12,0x08,0x1F,0xFE,0x10,0x00},/*"值",1*/
        {0x08,0x80,0x08,0x80,0x08,0x84,0x10,0x88,0x10,0x90,0x30,0xA0,0x30,0xC0,0x50,0x80},
        {0x91,0x80,0x12,0x80,0x14,0x80,0x10,0x82,0x10,0x82,0x10,0x82,0x10,0x7E,0x10,0x00},/*"化",2*/
};
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

            LimitL(left);                                                             //这里就是对传给Get_Border_And_SideType()函数的扫描区间进行一个限幅操作。
            LimitH(left);
            LimitL(mid);                                                             //这里就是对传给Get_Border_And_SideType()函数的扫描区间进行一个限幅操作。
            LimitH(mid);
            LimitL(right);                                                             //这里就是对传给Get_Border_And_SideType()函数的扫描区间进行一个限幅操作。
            LimitH(right);


            tft180_draw_point(left, i, RGB565_RED);  // 绘制中线
            tft180_draw_point(mid,i,RGB565_BLUE);
            tft180_draw_point(right,i,RGB565_PURPLE);
        }
}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     状态显示
//--------------------------------------------------------------------------------------------------------------------------
void status_show(void)
{
//    if(!zxy)tft180_show_string(70, 81, "loading");
//    else
        if(imageflag.image_element_rings_flag)tft180_show_string(0, 8*line_unit, "element");
    else if(imageflag.Zebra_Flag)tft180_show_string(0,8*line_unit,"_Zebra_");
    else if(imageflag.Bend_Road)tft180_show_string(0,8*line_unit,"_Bend__");
    else if(imagestatus.WhiteLine >= 8)tft180_show_string(0,8*line_unit,"cross__");
    else tft180_show_string(0,8*line_unit,"normal_");
}

//---------------------------------------------------------------------------------------------------------------------------
// 函数简介     对各项参数显示
//---------------------------------------------------------------------------------------------------------------------------
void tft180show_better(uint8 mode)
{
    float fps_show=0;

    static uint8 yu_zhi_show;
    static int fps_count=0;
    switch(mode)
    {
    case 1:
        tft180_show_string(0,0,"pid");

        tft180_show_string(0, kp_31, "kp:");
        tft180_show_float(column_unit*1.5,kp_31,kp,2,1);

        tft180_show_string(0,ki_31,"ki:");
        tft180_show_float(column_unit*1.5,ki_31,ki,2,1);

        tft180_show_string(0,kd_31,"kd:");
        tft180_show_float(column_unit*1.5,kd_31,kd,2,1);

        tft180_show_string(0,dt_31,"dt:");
        tft180_show_float(column_unit*1.5,dt_31,dt,2,1);

        tft180_show_string(0,target_31,"target:");
        tft180_show_float(column_unit*3.5,target_31,target,2,1);

        break;
    case 2://二值化图像以及扫线情况显示
        if(fps_count==0)
        {
            timer_clear(TIM_3);
        }
        //图像显示控制
       if(!mt9v034_mode){tft180_show_gray_image(0, 0, ex_mt9v03x_binarizeImage[0], MT9V03X_W/2, MT9V03X_H/2, (MT9V03X_W/2), (MT9V03X_H/2), 0);
       tft180_show_chinese(0, mt9v034_21, 16, tu_xiang[0], 2, RGB565_BLACK);
       tft180_show_string(2*column_unit,mt9v034_21,":");
       tft180_show_chinese(column_unit*2.5, mt9v034_21, 16, erzhihua[0], 3, RGB565_BLACK);}
       else {tft180_displayimage03x(mt9v03x_image[0],MT9V03X_W/2,MT9V03X_H/2);
       tft180_show_chinese(0, mt9v034_21, 16, tu_xiang[0], 2, RGB565_BLACK);
       tft180_show_string(2*column_unit,mt9v034_21,":");
       tft180_show_chinese(column_unit*2.5, mt9v034_21, 16, hui_du[0], 2, RGB565_BLACK);}

       //边线显示控制
       if(!line_show_mode){line_visualization();
       tft180_show_chinese(0, line_show_21, 16, bian_xian[0], 2, RGB565_BLACK);
       tft180_show_string(2*line_unit,line_show_21,":");
       tft180_show_chinese(2*line_unit+8, line_show_21, 16, xian_shi[0], 2, RGB565_BLACK);}
       else{tft180_show_chinese(0, line_show_21, 16, bian_xian[0], 2, RGB565_BLACK);
       tft180_show_string(2*line_unit,line_show_21,":");
       tft180_show_chinese(2*line_unit+8, line_show_21, 16, yin_cang[0], 3, RGB565_BLACK);}

       fps_count++;
       if(fps_count>=30)
       {
           fps_show=timer_get(TIM_3);
           fps_show=(fps_count/fps_show)*1000;
           if(fps_show>=MT9V03X_FPS_DEF)fps_show=MT9V03X_FPS_DEF;
           fps_count=0;

           yu_zhi_show=ex_threshold;

            //fps显示
           tft180_show_float(column_unit*2,4*line_unit,fps_show,6,2);
       }
            tft180_show_string(0, 4*line_unit, "fps:");

            //二值化阈值显示
            tft180_show_chinese(0, erzhi_21, 16, yu_zhi[0], 4, RGB565_BLACK);
            tft180_show_string(column_unit*2,erzhi_21,":   +");
            tft180_show_uint(column_unit*2.5,erzhi_21,yu_zhi_show,3);
            tft180_show_int(column_unit*4.5,erzhi_21,threshold_fix,2);
            //底层边线坐标显示
//            tft180_show_uint(0,8*line_unit,Sideline_status_array[59].leftline,2);
//            tft180_show_uint(2*column_unit,8*line_unit,Sideline_status_array[59].midline,2);
//            tft180_show_uint(4*column_unit,8*line_unit,Sideline_status_array[59].rightline,3);
            //停止flag
//            tft180_show_string(0,9*line_unit,"stop:");
//            tft180_show_uint(column_unit*2.5,9*line_unit,imageflag.stop_flag,1);
            //斑马线flag
//            tft180_show_string(4*column_unit,9*line_unit,"flag:");
//            tft180_show_uint(6.5*column_unit,9*line_unit,imageflag.Zebra_Flag,1);

//            static uint8 temp;
//            temp=!temp;
//            tft180_show_uint(80,9*line_unit,temp,5);
            //道路状态显示
            status_show();
            break;

    case 3://陀螺仪数据显示，以及预设角度调整
        mpu6050_parameter_tft180_show();
        break;
    case 4:
        tft180_show_gray_image(0, 0, ex_mt9v03x_binarizeImage[0], MT9V03X_W/2, MT9V03X_H/2, (MT9V03X_W/2), (MT9V03X_H/2), 0);
        line_visualization();
        tft180_show_string(0,4*line_unit,"WhiteLine:");
        tft180_show_int(5*column_unit,4*line_unit,imagestatus.WhiteLine,3);

        tft180_show_string(0,5*line_unit,"element:");
        tft180_show_int(4*column_unit,5*line_unit,imageflag.image_element_rings,1);

        tft180_show_string(0,6*line_unit,"ele_flag:");
        tft180_show_int(4.5*column_unit,6*line_unit,imageflag.image_element_rings_flag,1);


        status_show();
        break;
    case 5:
        tft180_displayimage03x(mt9v03x_image[0],MT9V03X_W/2,MT9V03X_H/2);
        tft180_show_chinese(0,baoguangtime_41,16,bao_guang_shi_jian[0],4,RGB565_BLACK);
        tft180_show_string(4*column_unit, baoguangtime_41, ":");
        tft180_show_int(4.5*column_unit,baoguangtime_41,exposure_time,4);
        mt9v03x_set_exposure_time(exposure_time);           //设置曝光时间
        break;
    }
}

//----------------------------------------------------------------------------------------
// 函数简介     按钮increase reduce的功能实现
//----------------------------------------------------------------------------------------
void increase_reduce(void)
{
    switch (page)
    {
        case (((mpu6050_0/column_unit)*10)+1):      //陀螺仪控制

            break;
        case (((tuxiang_0/column_unit)*10)+1):      //图像处理控制
            if(!Button_mode)
            {
                if(key_get_state(KEY_3)==KEY_LONG_PRESS || key_get_state(KEY_3)==KEY_SHORT_PRESS )
                {
                    switch(line)
                    {
                    case (erzhi_21/line_unit):threshold_fix++;break;
                    case (line_show_21/line_unit): line_show_mode=!line_show_mode; break;
                    case (mt9v034_21/line_unit): mt9v034_mode=!mt9v034_mode;break;
                    }
                }
            }
            else
            {
                if(key_get_state(KEY_3)==KEY_LONG_PRESS || key_get_state(KEY_3)==KEY_SHORT_PRESS )
                {
                    switch(line)
                    {
                    case (erzhi_21/line_unit):threshold_fix--;break;
                    case (line_show_21/line_unit): line_show_mode=!line_show_mode; break;
                    case (mt9v034_21/line_unit): mt9v034_mode=!mt9v034_mode;break;
                    }
                }
            }
            break;
        case (((pid_0/column_unit)*10)+1):          //pid控制
            if(!Button_mode)
            {
                if(key_get_state(KEY_3)==KEY_LONG_PRESS || key_get_state(KEY_3)==KEY_SHORT_PRESS )
                {
                    switch(line)
                    {
                    case (kp_31/line_unit):kp+=0.1;break;
                    case (ki_31/line_unit):ki+=0.1;break;
                    case (kd_31/line_unit):kd+=0.1;break;
                    case (dt_31/line_unit):dt+=0.1;break;
                    case (target_31/line_unit):target+=0.1;break;
                    }

                }

            }
            else
            {

                if(key_get_state(KEY_3)==KEY_LONG_PRESS || key_get_state(KEY_3)==KEY_SHORT_PRESS )
                {
                    switch(line)
                    {
                        case (kp_31/line_unit):kp-=0.1;break;
                        case (ki_31/line_unit):ki-=0.1;break;
                        case (kd_31/line_unit):kd-=0.1;break;
                        case (dt_31/line_unit):dt-=0.1;break;
                        case (target_31/line_unit):target-=0.1;break;
                    }
                }

            }

            break;
        case (((shexiangtou_0/column_unit)*10)+1):      //摄像头控制
                if(!Button_mode)
                {
                    if(key_get_state(KEY_3)==KEY_LONG_PRESS || key_get_state(KEY_3)==KEY_SHORT_PRESS )
                    {
                        switch(line)
                        {
                        case (baoguangtime_41/line_unit):exposure_time++;break;
                        }

                    }

                }
                else
                {

                    if(key_get_state(KEY_3)==KEY_LONG_PRESS || key_get_state(KEY_3)==KEY_SHORT_PRESS )
                    {
                        switch(line)
                        {
                        case (baoguangtime_41/line_unit):exposure_time--;break;
                        }
                    }

                }
            break;
    }
}
//---------------------------------------------------------------------------------------------------------
// 函数简介     ui界面选择框
// 参数说明     x   选择框右边界
// 参数说明     y   选择框起始行
//---------------------------------------------------------------------------------------------------------
void choose_(uint8 x,uint8 y)
{
    if(y!=0)tft180_draw_line(0, y, x, y, RGB565_BLACK);
    tft180_draw_line(0, y+line_unit, x, y+line_unit, RGB565_BLACK);
    tft180_draw_line(x, y, x, y+line_unit, RGB565_BLACK);
}

//---------------------------------------------------------------------------------------------------------
// 函数简介      菜单界面显示与操作
// 备注信息     tft范围为横向最大128竖向最大160，单个中文字符大小为16*line_unit，而英文字符则为8*line_unit，也就是在竖向共有10行，每行能容纳8个字
//---------------------------------------------------------------------------------------------------------
void menu_page(void)
{
    key_scanner();
    tft180_show_uint(7*line_unit, 0, Button_mode, 1);

    if(key_get_state(KEY_2)==KEY_SHORT_PRESS)
        {
            Button_mode=!Button_mode;
        }

    if(key_get_state(KEY_4)==KEY_SHORT_PRESS)
    {
        if(!Button_mode)line++;
        else line--;
        tft180_clear();
    }

    if(key_get_state(KEY_1)==KEY_SHORT_PRESS && Button_mode!=0)
    {
        page=0;
        line=10;
        tft180_clear();

    }
    switch(page)
    {
    case 0:
        if(line<page_0_min)line=page_0_max;
        if(line>page_0_max)line=page_0_min;
        tft180_show_chinese(1,0,16,menu[0],2,RGB565_BLACK);

        tft180_show_string(0,line_unit,"1.");
        tft180_show_chinese(column_unit,mpu6050_0,16,mpu6050_parament[0],3,RGB565_BLACK);

        tft180_show_string(0,2*line_unit,"2.");
        tft180_show_chinese(column_unit,tuxiang_0,16,tu_xiang[0],3,RGB565_BLACK);

        tft180_show_string(0,3*line_unit,"3.");
        tft180_show_string(column_unit,pid_0,"pid");

        tft180_show_string(0,4*line_unit,"4.");
        tft180_show_string(1*column_unit,parament_debug_0,"parament_show");

        tft180_show_string(0,5*line_unit,"5.");
        tft180_show_chinese(1*column_unit, parament_save_0, 16, can_shu_bao_cun[0], 4, RGB565_BLACK);

        tft180_show_string(0,6*line_unit,"6.");
        tft180_show_chinese(1*column_unit,shexiangtou_0,16,she_xiang_tou[0],3,RGB565_BLACK);

        choose_(5*column_unit,((line*line_unit)-1));
//        if(Button[3].status=='D'&& Button[3].decetion_flag==0)
        if(key_get_state(KEY_1)==KEY_SHORT_PRESS && Button_mode==0)
        {
            if(line==(parament_save_0/line_unit)){Parament_save();tft180_show_string(0, 8*line_unit, "save success!");}
            else{
                page=line*10+1;
                line=10;                                //一定要注意给每一页的line设置最大最小值，否则会出错
                tft180_clear();
            }

//            Button[3].decetion_flag=1;
        }
        break;
    case (((mpu6050_0/column_unit)*10)+1):
        if(line>page_mpu6050_max)line=page_mpu6050_min;
        if(line<page_mpu6050_min)line=page_mpu6050_max;
        tft180show_better(3);
        break;
    case (((tuxiang_0/column_unit)*10)+1):
        if(line>page_tuxiang_max)line=page_tuxiang_min;
        if(line<page_tuxiang_min)line=page_tuxiang_max;
        tft180show_better(2);
        choose_(6*column_unit,((line*line_unit)-1));
        break;
    case (((parament_debug_0/column_unit)*10)+1):
        if(line!=1)line=1;
        tft180show_better(4);
        break;
    case (((pid_0/column_unit)*10)+1):
        if(line>page_pid_max)line=page_pid_min;
        if(line<page_pid_min)line=page_pid_max;
        choose_(6*column_unit,((line*line_unit)-1));
        tft180show_better(1);
        break;
    case (((shexiangtou_0/column_unit)*10)+1):
        if(line!=page_shexiangtou)line=page_shexiangtou;
        choose_(6*column_unit,((line*line_unit)-1));
        tft180show_better(5);
        break;
    }
    increase_reduce();

}
