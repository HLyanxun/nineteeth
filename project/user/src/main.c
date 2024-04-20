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
#include "better_fix_mt9v034.h"


#include "math.h"

uint8 mode=0;

int test_value_2 = 0;
float test_value_1 = 0.000;

//PacketTag_TpDef_struct Test_packet[] = {
//    {"TEST", UnpackData_Handle_Float_FireWater, &test_value_1},
//    {"TES1", UnpackData_Handle_Int_FireWater, &test_value_2},
//    // 添加更多的映射关系
//};
int main (void)
{
    clock_init(SYSTEM_CLOCK_120M);      // 初始化芯片时钟 工作频率为 120MHz
    debug_init();                       // 务必保留，本函数用于初始化MPU 时钟 调试串口
    steering_engine_init(TIM1_PWM_MAP0_CH1_A8,50,120);
    steering_engine_set_angle(TIM1_PWM_MAP0_CH1_A8,90);
////
//    pwm_init(TIM1_PWM_MAP0_CH1_A8,50,650);
//    pit_ms_init(TIM2_PIT,2);
    tft180_init();
    mt9v03x_init();
//    uart_init(DEBUG_UART_INDEX, DEBUG_UART_BAUDRATE, DEBUG_UART_TX_PIN, DEBUG_UART_RX_PIN); // 串口初始化
//    uart_rx_interrupt(DEBUG_UART_INDEX, ENABLE);                                            // 开启接收中断
//    interrupt_set_priority(USART3_IRQn, (0 << 5) || 1);                                     // 设置usart3的中断优先级
//    while(1)
//    {
//
//        if(mt9v03x_init())
//        {
//            tft180_show_string(60,0,"mt9v03x init error.");// mt9v03x 初始化失败
//        }else {
//            break;
//        }
//    }
////    gpio_init(E2, GPI, 0, GPI_PULL_UP);
////    gpio_init(C3, GPI, 0, GPI_PULL_UP);
////    gpio_init(E5, GPI, 0, GPI_PULL_UP);
    timer_init(TIM_3,TIMER_MS);
    timer_start(TIM_3);
    flag_init();
////    better_pwm_init(TIM1_PWM_MAP0_CH4_A11,50,5.0);


     // 此处编写用户代码 例如外设初始化代码等


     while(1)
     {
         menu_page();
//         if (UART3_data_packet_ready)
//         {
//             UART3_data_packet_ready = false;
//             printf_USART3("\r\nReturn:%s\r\n", UART3_get_data);
//             PacketTag_Analysis(Test_packet,2);
//             printf_USART3("\r\ntestv1:%f\r\n", test_value_1);
//             printf_USART3("\r\ntestv2:%d\r\n", test_value_2);
//         }
         if(mt9v03x_finish_flag)
         {
//         otsuThreshold_get();
             ex_threshold=otsuThreshold(mt9v03x_image);
             binarizeImage();
             baseline_get();
             allline_get();
//         if(!imageflag.Out_Road)
//                             Search_Border_OTSU(ex_mt9v03x_binarizeImage, LCDH, LCDW, LCDH - 2);//58行位底行
//                         else
//                             imagestatus.OFFLineBoundary = 5;
         Scan_Element();
//         Element_Judgment_Left_Rings();
//         Element_Judgment_Right_Rings();
//         Element_Judgment_RoadBlock();
//         Element_Judgment_Zebra();
//         Element_Judgment_Bend();
//         Straight_long_judge();
//         Straight_long_handle();
         //auto_extension_line();
         Element_Handle();
//         tft180_show_uint(80,129,zxy,3);
         Forward_control();
         mt9v03x_finish_flag=0;
         }
//
//         //auto_extension_line();
//
//         /*以上为检测区域*/
//
////         for(float i=2.5;i<12.5;i+=1.0)
////         {
////              better_pwm_set_duty(TIM1_PWM_MAP0_CH4_A11,i);
////              system_delay_ms(100);
////         }
////         for(float i=12.5;i>2.5;i-=1.0)
////                 {
////                      better_pwm_set_duty(TIM1_PWM_MAP0_CH4_A11,i);
////                      system_delay_ms(100);
////                 }
//        menu_page();
//
//         tft180show_better(2);
         //tft180_displayimage03x(mt9v03x_image[0],MT9V03X_W/2,MT9V03X_H/2);
     }
 }

 void pit_hander(void)
 {
//     static uint8 time,time2;
//     static uint8 a,b;
//     if(time<10)
//     {
//         time++;
//     }else {
//         time=0;
//         b=a;
//         a=gpio_get_level(C5);
//         if(a==1 && b==0)
//         {
//             mode++;
//             if(mode>=2)mode=0;
//         }
//         mpu6050_data_dispose();
//
//     }
//     if(time2<10)
//     {
//         time2++;
//     }else {
//         time2=0;
//         zxy=MK021851_interrupt(E2,C3,zxy);
////         period_dispose();
//    }
//

 }


