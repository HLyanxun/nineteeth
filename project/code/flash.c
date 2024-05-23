/*
 * flash.c
 *
 *  Created on: 2024��4��18��
 *      Author: pc
 */
#include "zf_common_headfile.h"
// ***************** �����Ҫ�������ݣ����ȡ����ѡErase All(��������ѡ��) *************************** //
// ***************** �����Ҫ�������ݣ����ȡ����ѡErase All(��������ѡ��) *************************** //
// ***************** �����Ҫ�������ݣ����ȡ����ѡErase All(��������ѡ��) *************************** //

// ***************** ��дFLASH��ʱ������ϵͳƵ�ʱ���С�ڻ����120M *************************** //
// ***************** ��дFLASH��ʱ������ϵͳƵ�ʱ���С�ڻ����120M *************************** //
// ***************** ��дFLASH��ʱ������ϵͳƵ�ʱ���С�ڻ����120M *************************** //
//----------------------------------------------------------------------------------------------------------
// �������     ��ʼ��flash��������flash�е����ݶ�ȡ����
// ʹ��ʵ��     Parament_init();
//----------------------------------------------------------------------------------------------------------
void Parameter_init(void)
{
    flash_buffer_clear();
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);      // �����ݴ� flash ��ȡ��������
    system_delay_ms(50);

    kp   =          flash_union_buffer[0].float_type;
    ki   =          flash_union_buffer[1].float_type;
    kd   =          flash_union_buffer[2].float_type;
    dt   =          flash_union_buffer[3].float_type;
    target   =      flash_union_buffer[4].float_type;
    threshold_fix = flash_union_buffer[5].int8_type;
    exposure_time = flash_union_buffer[6].int16_type;
    mt9v03x_set_exposure_time(exposure_time);
}
//----------------------------------------------------------------------------------------------------------
// �������     ����ui����������¼��flash��
// ʹ��ʵ��     Parament_save();
// ��ע��Ϣ     �ϵ�ǰ��������
//----------------------------------------------------------------------------------------------------------
void Parameter_save(void)
{
    flash_buffer_clear();
   system_delay_ms(50);
   flash_union_buffer[0].float_type = kp;
   flash_union_buffer[1].float_type = ki;
   flash_union_buffer[2].float_type = kd;
   flash_union_buffer[3].float_type = dt;
   flash_union_buffer[4].float_type = target;
   flash_union_buffer[5].int16_type = threshold_fix;
   flash_union_buffer[6].int16_type = exposure_time;
   flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);        // ��ָ�� Flash ������ҳ��д�뻺��������
   flash_buffer_clear();
}
//-----------------------------------------------------------------------------------------------------------
// �������     ����������ӽ�flash��ʱ�����������ݻָ�Ĭ��ֵ����һ����Ӳ���Ҫ������ǵ�ɾ����
// ʹ��ʵ��     Parament_first();
// ��ע��Ϣ     �����ã�����ɾ��������������������������������������������������������������������������
//-----------------------------------------------------------------------------------------------------------
void Parameter_first(void)
{
    flash_erase_sector(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    flash_buffer_clear();
    flash_union_buffer[0].float_type = 0.0;
    flash_union_buffer[1].float_type = 0.0;
    flash_union_buffer[2].float_type = 0.0;
    flash_union_buffer[3].float_type = 0.0;
    flash_union_buffer[4].float_type = 47.0;
    flash_union_buffer[5].int16_type = 10;
    flash_union_buffer[6].int16_type = 512;
    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);     // ��ָ�� Flash ������ҳ��д�뻺��������
    flash_buffer_clear();                                                  // ��ջ�����
}
//-----------------------------------------------------------------------------------------------------------
// �������     �������г�ʼ������������©��
//-----------------------------------------------------------------------------------------------------------
void All_Init(void)
{
    //    Parament_first();//����µı�����flashʱ���и��д���
    Parameter_init();
    tft180_init();
    key_init(10);       //���������ж�����
    mt9v03x_init();
    pit_ms_init(TIM2_PIT,10);
    Flag_init();
//    timer_init(TIM_3,TIMER_MS);
//    Flag_init();
}
//---------------------------
 uint8 wocaonima=10;
void test(void)
{


    if(key_get_state(KEY_3)==KEY_SHORT_PRESS)
        {
            wocaonima++;
        }
    if(key_get_state(KEY_4)==KEY_SHORT_PRESS)
    {
        wocaonima--;
    }
//    if(wocaonima<5)wocaonima=5;
//    if(wocaonima>10)wocaonima=10;

    better_pwm_set_duty(A1,wocaonima);
    tft180_show_uint(0,0,wocaonima,3);
}
