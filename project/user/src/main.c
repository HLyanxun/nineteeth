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
#include "zf_common_headfile.h"
#include "math.h"
uint8 mode=0;

int main (void)
{
    clock_init(SYSTEM_CLOCK_120M);      // ��ʼ��оƬʱ�� ����Ƶ��Ϊ 120MHz
    debug_init();                       // ��ر��������������ڳ�ʼ��MPU ʱ�� ���Դ���

    tft180_init();
    pit_ms_init(TIM2_PIT,20);
    mt9v03x_init();

    gpio_init(C5, GPI, 1, GPI_PULL_UP);
    gpio_init(E2, GPI, 1, GPI_PULL_UP);
    gpio_init(E3, GPI, 1, GPI_PULL_UP);
    gpio_init(E4, GPI, 1, GPI_PULL_UP);
     // �˴���д�û����� ���������ʼ�������

     while(1)
     {
         tft180_show_uint(0,70, mode,1);
         mt9v034_binaryzation_conduct();
         mt9v034_tft180_show();

         // �˴���д��Ҫѭ��ִ�еĴ���
     }
 }

 void pit_hander(void)
 {
     static uint8 a,b;
     ex_binaryzation_conduct_adjustment=MK021851_interrupt(E2,E4,ex_binaryzation_conduct_adjustment);
     b=a;
     a=gpio_get_level(C5);
     if(a==1 && b==0)
     {
         mode++;
         if(mode>=3)mode=0;
     }

 }


