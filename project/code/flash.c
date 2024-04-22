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
void Parament_init(void)
{
    flash_buffer_clear();
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);      // �����ݴ� flash ��ȡ��������
    system_delay_ms(50);

    kp   =          flash_union_buffer[0].float_type;
    ki   =          flash_union_buffer[1].float_type;
    kd   =          flash_union_buffer[2].float_type;
    dt   =          flash_union_buffer[3].float_type;
    target   =      flash_union_buffer[4].float_type;
    threshold_fix = flash_union_buffer[5].int16_type;

}
void Parament_save(void)
{
    flash_buffer_clear();
   system_delay_ms(50);
   flash_union_buffer[0].float_type = kp;
   flash_union_buffer[1].float_type = ki;
   flash_union_buffer[2].float_type = kd;
   flash_union_buffer[3].float_type = dt;
   flash_union_buffer[4].float_type = target;
   flash_union_buffer[5].int16_type = threshold_fix;
   flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);        // ��ָ�� Flash ������ҳ��д�뻺��������
   flash_buffer_clear();
}
void Parament_first(void)
{
    flash_erase_sector(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    flash_buffer_clear();
    flash_union_buffer[0].float_type = 0.0;
    flash_union_buffer[1].float_type = 0.0;
    flash_union_buffer[2].float_type = 0.0;
    flash_union_buffer[3].float_type = 0.0;
    flash_union_buffer[4].float_type = 47.0;
    flash_union_buffer[5].int16_type = 10;
    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);     // ��ָ�� Flash ������ҳ��д�뻺��������
    flash_buffer_clear();                                                  // ��ջ�����
}

