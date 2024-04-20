/*
 * button.c
 *
 *  Created on: 2024年4月13日
 *      Author: pc
 */
#include "zf_common_headfile.h"
int page=0,line=1;
uint8 Button_mode=0;
Button_Stash Button[KEY_Number];
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
void choose_(uint8 x,uint8 y)
{
//    uint8 j=x;
//    for(uint8 i=y+4;i<y+8;i++)
//    {
//        tft180_draw_line(j, i, x+2, i, RGB565_BLUE);
//        j-=2;
//    }
//    for(uint8 i=y+8;i<y+12;i++)
//        {
//        j+=2;
//        tft180_draw_line(j, i, x+2, i, RGB565_BLUE);
//        }
    if(y)tft180_draw_line(0, y, x, y, RGB565_BLACK);
    tft180_draw_line(0, y+line_unit, x, y+line_unit, RGB565_BLACK);
    tft180_draw_line(x, y, x, y+line_unit, RGB565_BLACK);
}

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
    static uint8 mt9v034_mode=0,line_show_mode=0;
    uint8 yu_zhi_show=0;
    static int fps_count=0;
    switch(mode)
    {
    case 1:
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
       if(!mt9v034_mode){tft180_show_gray_image(1, 1, ex_mt9v03x_binarizeImage[0], MT9V03X_W/2, MT9V03X_H/2, (MT9V03X_W/2), (MT9V03X_H/2), 0);
       tft180_show_chinese(0, 6*line_unit, 16, tu_xiang[0], 2, RGB565_BLACK);
       tft180_show_string(2*column_unit,6*line_unit,":");
       tft180_show_chinese(column_unit*2.5, 6*line_unit, 16, erzhihua[0], 3, RGB565_BLACK);}
       else {tft180_displayimage03x(mt9v03x_image[0],MT9V03X_W/2,MT9V03X_H/2);
       tft180_show_chinese(0, 6*line_unit, 16, tu_xiang[0], 2, RGB565_BLACK);
       tft180_show_string(2*column_unit,6*line_unit,":");
       tft180_show_chinese(column_unit*2.5, 6*line_unit, 16, hui_du[0], 2, RGB565_BLACK);}

       //边线显示控制
       if(!line_show_mode){line_visualization();
       tft180_show_chinese(0, 7*line_unit, 16, bian_xian[0], 2, RGB565_BLACK);
       tft180_show_string(2*line_unit,7*line_unit,":");
       tft180_show_chinese(2*line_unit+8, 7*line_unit, 16, xian_shi[0], 2, RGB565_BLACK);}
       else{tft180_show_chinese(0, 7*line_unit, 16, bian_xian[0], 2, RGB565_BLACK);
       tft180_show_string(2*line_unit,7*line_unit,":");
       tft180_show_chinese(2*line_unit+8, 7*line_unit, 16, yin_cang[0], 3, RGB565_BLACK);}

//        line_visualization();
        fps_count++;

        if(fps_count>=30)
        {
            fps_show=timer_get(TIM_3);
            fps_show=(fps_count/fps_show)*1000;
            if(fps_show>=MT9V03X_FPS_DEF)fps_show=MT9V03X_FPS_DEF;
            fps_count=0;

            yu_zhi_show=ex_threshold;

//            tft180_show_uint(7*line_unit, 0, mode, 1);
            //fps显示
            tft180_show_float(column_unit*2,4*line_unit,fps_show,6,2);
        }
            tft180_show_string(0, 4*line_unit, "fps:");

            //二值化阈值显示
            tft180_show_chinese(0, 5*line_unit, 16, yu_zhi[0], 4, RGB565_BLACK);
            tft180_show_string(column_unit*2,5*line_unit,":");
            tft180_show_uint(column_unit*2.5,5*line_unit,yu_zhi_show,3);
            //底层边线坐标显示
            tft180_show_uint(0,8*line_unit,Sideline_status_array[59].leftline,2);
            tft180_show_uint(2*column_unit,8*line_unit,Sideline_status_array[59].midline,2);
            tft180_show_uint(4*column_unit,8*line_unit,Sideline_status_array[59].rightline,3);
            //停止flag
            tft180_show_string(0,9*line_unit,"stop:");
            tft180_show_uint(column_unit*2.5,9*line_unit,imageflag.stop_flag,1);
            //斑马线flag
//            tft180_show_string(4*column_unit,9*line_unit,"flag:");
//            tft180_show_uint(6.5*column_unit,9*line_unit,imageflag.Zebra_Flag,1);

//            static uint8 temp;
//            temp=!temp;
//            tft180_show_uint(80,9*line_unit,temp,5);

            status_show();
            break;

    case 3://陀螺仪数据显示，以及预设角度调整
        tft180_clear();
        tft180_show_uint(142, 0, mode, 1);
        mpu6050_parameter_tft180_show();
        break;
    }
}
//---------------------------------------------------------------------------
// 函数简介     实现菜单功能的按键的初始化
//---------------------------------------------------------------------------
void menu_button_init(void)
{
    gpio_init(E2, GPI, 0, GPI_PULL_UP);
    for(uint8 i=0; i<KEY_Number;i++)
    {
        Button[i].decetion_flag=0;
        Button[i].time=0;
    }
}
//---------------------------------------------------------------------------
// 函数简介     按键扫描
// 备注信息     Button数组共4个，分别对应line number Toggle enter
//---------------------------------------------------------------------------
void menu_button_scan(void)
{
    Button[2].level_last=Button[2].level;
    Button[2].level=gpio_get_level(E2);
}

//---------------------------------------------------------------------------
// 函数简介     按键状态获取
// 参数说明 interrupt_time      定时器扫描周期单位ms
// 备注信息     D按下，U未按下，L长按;     中断10ms扫描一次
//---------------------------------------------------------------------------
void button_status_get(uint8 interrupt_time)
{
    menu_button_scan();
    for(uint8 i=0;i<KEY_Number;i++)
    {
        if(Button[i].level)
        {
             if(Button[i].level!=Button[i].level_last)
             {
                Button[i].status='D';
             }else {
                Button[i].time+=1;
                if(Button[i].time>(500/interrupt_time))//0.5s没有松开按键
                {
                    if(Button[i].time>(600/interrupt_time))
                        {
                            Button[i].time=(500/interrupt_time);
                            if(Button[i].status=='L')Button[i].status='S';//每隔10ms让’L‘信号变为’S‘信号，让数据+1后在变回’L‘
                        }

                }
            }
        }else
        {
            if(Button[i].level!=Button[i].level_last)
            {
                Button[i].status='U';
            }
            if(Button[i].time!=0)
            {
                Button[i].time=0;
            }
        }

    }
}
//----------------------------------------------------------------------------------------
// 函数简介     按钮increase reduce的功能实现
//----------------------------------------------------------------------------------------
void increase_reduce(void)
{
    switch (page)
    {
        case 11:
            if(Button[1].status=='D'&& Button[1].decetion_flag==0)
            {

                Button[1].decetion_flag=1;
            }

            break;
        case 21:
            if(Button[1].status=='D'&& Button[1].decetion_flag==0)
            {
                switch(line)
                {

                case 5:
                    break;//对阈值的修改
                default:
                    break;
                }
                Button[1].decetion_flag=1;
            }
            break;
        case 31:
            if(!Button_mode)
            {
                if(Button[1].status=='D'&& Button[1].decetion_flag==0)
                {
                    switch(line)
                    {
                    case (kp_31/16):kp+=0.1;break;
                    case (ki_31/16):ki+=0.1;break;
                    case (kd_31/16):kd+=0.1;break;
                    case (dt_31/16):dt+=0.1;break;
                    case (target_31/16):target+=0.1;break;
                    }
                Button[1].decetion_flag=1;
                }
                if(Button[1].status=='S')
                {
                    switch(line)
                    {
                    case (kp_31/16):kp+=0.1;break;
                    case (ki_31/16):ki+=0.1;break;
                    case (kd_31/16):kd+=0.1;break;
                    case (dt_31/16):dt+=0.1;break;
                    case (target_31/16):target+=0.1;break;
                    }
                    Button[1].status='L';
                }
            }
            else
            {
                if(Button[1].status=='D'&& Button[1].decetion_flag==0)
                {
                    switch(line)
                    {
                        case (kp_31/16):kp+=0.1;break;
                        case (ki_31/16):ki+=0.1;break;
                        case (kd_31/16):kd+=0.1;break;
                        case (dt_31/16):dt+=0.1;break;
                        case (target_31/16):target+=0.1;break;
                    }
                    Button[1].decetion_flag=1;
                }
                if(Button[1].status=='S')
                {
                    switch(line)
                    {
                    case (kp_31/16):kp+=0.1;break;
                    case (ki_31/16):ki+=0.1;break;
                    case (kd_31/16):kd+=0.1;break;
                    case (dt_31/16):dt+=0.1;break;
                    case (target_31/16):target+=0.1;break;
                    }
                    Button[1].status='L';
                }
            }

            break;

        default:
            break;
    }
}
//----------------------------------------------------------------------------------------
// 函数简介      菜单界面显示与操作
// 备注信息     tft范围为横向最大128竖向最大160，单个中文字符大小为16*line_unit，而英文字符则为8*line_unit，也就是在竖向共有10行，每行能容纳8个字
//----------------------------------------------------------------------------------------
void menu_page(void)
{
    tft180_show_uint(7*line_unit, 0, Button_mode, 1);
    if(Button[2].status=='D'&& Button[2].decetion_flag==0)
        {
            Button_mode=!Button_mode;
            Button[2].decetion_flag=1;
        }
    if(Button[0].status=='D'&& Button[0].decetion_flag==0)
    {
        if(!Button_mode)line++;
        else line--;
        tft180_clear();
        Button[0].decetion_flag=1;
    }
    if(Button[3].status=='D'&& Button[3].decetion_flag==0 && Button_mode!=0)
    {
        page=0;
        Button[3].decetion_flag=1;
    }
    switch(page)
    {
    case 0:
        tft180_show_chinese(0,0,16,menu[0],2,RGB565_BLACK);

        tft180_show_string(0,mpu6050_0,"1.");
        tft180_show_chinese(column_unit,mpu6050_0,16,mpu6050_parament[0],3,RGB565_BLACK);

        tft180_show_string(0,tuxiang_0,"2.");
        tft180_show_chinese(column_unit,tuxiang_0,16,tu_xiang[0],3,RGB565_BLACK);

        tft180_show_string(0,pid_0,"3.");
        tft180_show_string(column_unit,pid_0,"pid");
        if(line<page_0_min)line=3;
        if(line>page_0_max)line=1;

        if(Button[3].status=='D'&& Button[3].decetion_flag==0)
        {
            page=line*10+1;
            line=0;
            tft180_clear();
            Button[3].decetion_flag=1;
        }

        choose_(5*column_unit,((line*line_unit)-1));
        break;
    case 11:
        tft180show_better(3);
        break;
    case 21:
        tft180show_better(2);
        if(line>page_shexiangtou_max)line=page_shexiangtou_min;
        if(line<page_shexiangtou_min)line=page_shexiangtou_max;
        break;
    case 31:
        tft180show_better(1);
        if(line>page_pid_max)line=page_pid_min;
        if(line<page_pid_min)line=page_pid_max;
        break;
    }
   for(uint8 i=0; i<KEY_Number;i++)
   {
       if(Button[i].status!='D' && Button[i].decetion_flag!=0)
       {
           Button[i].decetion_flag=0;
       }
   }

//    tft180_show_string();
}
