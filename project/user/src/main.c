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
//#include "better_fix_mt9v034.h"
#include "math.h"

int main (void)
{
    clock_init(SYSTEM_CLOCK_144M);      // ��ʼ��оƬʱ�� ����Ƶ��Ϊ 120MHz
    debug_init();                       // ��ر��������������ڳ�ʼ��MPU ʱ�� ���Դ���
//    steering_engine_init(TIM1_PWM_MAP0_CH1_A8,50,120);
//    steering_engine_set_angle(TIM1_PWM_MAP0_CH1_A8,90);
//    mt9v03x_init();
//    ImagePerspective_Init();
//    tft180_init();
    timer_init(TIM_3,TIMER_US);
    timer_clear(TIM_3);
//    Image_CompressInit();
    Init_overall();
    All_Init();
     // �˴���д�û����� ���������ʼ�������
     while(1)
     {
//         Camera_tracking();
//         tft180_show_gray_image(0, 0, ex_mt9v03x_binarizeImage[0], image_w, image_h, image_w, image_h, 0);
//         tft180_show_int(0, 8*line_unit, My_Threshold, 4);
         if(mt9v03x_finish_flag==1)
         {
         timer_clear(TIM_3);
         timer_start(TIM_3);
         Image_Process();
         timer_conter=timer_get(TIM_3);
         timer_stop(TIM_3);
         }
         menu_page();


     }
 }

 void pit_hander(void)
 {



 }


