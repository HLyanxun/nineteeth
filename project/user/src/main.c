/*********************************************************************************************************************
* CH32V307VCT6 Opensourec Library 即（CH32V307VCT6 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是CH32V307VCT6 开源库的一部分
*
* CH32V307VCT6 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          main
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          MounRiver Studio V1.8.1
* 适用平台          CH32V307VCT6
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期                                      作者                             备注
* 2022-09-15        大W            first version
********************************************************************************************************************/
//RX(B10) TX(B11)
#include "zf_common_headfile.h"



int main (void)
{
    clock_init(SYSTEM_CLOCK_144M);      // 初始化芯片时钟 工作频率为 120MHz
    debug_init();                       // 务必保留，本函数用于初始化MPU 时钟 调试串口

    tft180_init();
    mt9v03x_init();
    ImagePerspective_Init();

     // 此处编写用户代码 例如外设初始化代码等


     while(1)
     {
//
//         if(mt9v03x_finish_flag==1)
//         {
//             tft180_show_gray_image(0, 0, mt9v03x_image[0], MT9V03X_W, MT9V03X_H, MT9V03X_W/2, MT9V03X_H/2, 0);
             sendimg(mt9v03x_image[0],MT9V03X_W,MT9V03X_H);
//             Camera_tracking();
//         Binaryzation();
//         image_draw();
//             for(uint8 i=0;i<30;i++)
//             {
//                 image[i][45]=255;
//             }


//             tft180_show_gray_image(0, 0, image[0], image_w, image_h, image_w, image_h, 0);
//             test_show_midline();
//             tft180_draw_line(Test_L, 0, Test_L, 90, RGB565_RED);
//             tft180_draw_line(Test_R, 0, Test_R, 90, RGB565_RED);


//             tft180_show_uint(0,92,out_flag,2);
//             int a = (int)midline_and_anglereturn(0);
//             float b = midline_and_anglereturn(1);
//             tft180_show_int(0, 91, a, 3);
//                      tft180_show_float(0, 107, b, 3, 3);
//                      tft180_draw_line(30, 0, 30, 90, RGB565_RED);
//                      tft180_draw_line(70, 0, 70, 90, RGB565_RED);

                     //             mt9v03x_finish_flag=0;
//         }
//         float c,d;
//         c=pi/180;
//         d=45*c;
//         float a=angle_compute(400,0,500,500);
//         float a = 1/10;
//         float b=atan2(45,45)/c;
//         tft180_show_float(0, 0, a, 3,3);
//         tft180_show_float(0, 16, b, 6,3);

//         timer_clear(TIM_3);
//         timer_start(TIM_3);
//         Camera_tracking();
//         timer_conter=timer_get(TIM_3);
//         timer_stop(TIM_3);
     }
 }

 void pit_hander(void)
 {
//     key_scanner();
 }


