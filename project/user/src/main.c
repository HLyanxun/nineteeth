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



int main (void)
{
    clock_init(SYSTEM_CLOCK_144M);      // ��ʼ��оƬʱ�� ����Ƶ��Ϊ 120MHz
    debug_init();                       // ��ر��������������ڳ�ʼ��MPU ʱ�� ���Դ���

    tft180_init();
    mt9v03x_init();
    ImagePerspective_Init();

     // �˴���д�û����� ���������ʼ�������


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


