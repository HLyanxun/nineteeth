/*********************************************************************************************************************
* CH32V307VCT6 Opensourec Library ����CH32V307VCT6 ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
* Copyright (c) 2022 SEEKFREE ��ɿƼ�
*
* ���ļ���CH32V307VCT6 ��Դ���һ����
*
* CH32V307VCT6 ��Դ�� ��������
* �����Ը��������������ᷢ���� GPL��GNU General Public License���� GNUͨ�ù������֤��������
* �� GPL �ĵ�3�棨�� GPL3.0������ѡ��ģ��κκ����İ汾�����·�����/���޸���
*
* ����Դ��ķ�����ϣ�����ܷ������ã�����δ�������κεı�֤
* ����û�������������Ի��ʺ��ض���;�ı�֤
* ����ϸ����μ� GPL
*
* ��Ӧ�����յ�����Դ���ͬʱ�յ�һ�� GPL �ĸ���
* ���û�У������<https://www.gnu.org/licenses/>
*
* ����ע����
* ����Դ��ʹ�� GPL3.0 ��Դ���֤Э�� �����������Ϊ���İ汾
* �������Ӣ�İ��� libraries/doc �ļ����µ� GPL3_permission_statement.txt �ļ���
* ���֤������ libraries �ļ����� �����ļ����µ� LICENSE �ļ�
* ��ӭ��λʹ�ò����������� ���޸�����ʱ���뱣����ɿƼ��İ�Ȩ����������������
*
* �ļ�����          main
* ��˾����          �ɶ���ɿƼ����޹�˾
* �汾��Ϣ          �鿴 libraries/doc �ļ����� version �ļ� �汾˵��
* ��������          MounRiver Studio V1.8.1
* ����ƽ̨          CH32V307VCT6
* ��������          https://seekfree.taobao.com/
*
* �޸ļ�¼
* ����                                      ����                             ��ע
* 2022-09-15        ��W            first version
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
//    // ��Ӹ����ӳ���ϵ
//};
int main (void)
{
    clock_init(SYSTEM_CLOCK_120M);      // ��ʼ��оƬʱ�� ����Ƶ��Ϊ 120MHz
    debug_init();                       // ��ر��������������ڳ�ʼ��MPU ʱ�� ���Դ���
    steering_engine_init(TIM1_PWM_MAP0_CH1_A8,50,120);
    steering_engine_set_angle(TIM1_PWM_MAP0_CH1_A8,90);
////
//    pwm_init(TIM1_PWM_MAP0_CH1_A8,50,650);
//    pit_ms_init(TIM2_PIT,2);
    tft180_init();
    mt9v03x_init();
//    uart_init(DEBUG_UART_INDEX, DEBUG_UART_BAUDRATE, DEBUG_UART_TX_PIN, DEBUG_UART_RX_PIN); // ���ڳ�ʼ��
//    uart_rx_interrupt(DEBUG_UART_INDEX, ENABLE);                                            // ���������ж�
//    interrupt_set_priority(USART3_IRQn, (0 << 5) || 1);                                     // ����usart3���ж����ȼ�
//    while(1)
//    {
//
//        if(mt9v03x_init())
//        {
//            tft180_show_string(60,0,"mt9v03x init error.");// mt9v03x ��ʼ��ʧ��
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


     // �˴���д�û����� ���������ʼ�������


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
//                             Search_Border_OTSU(ex_mt9v03x_binarizeImage, LCDH, LCDW, LCDH - 2);//58��λ����
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
//         /*����Ϊ�������*/
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


