/*
 * flash.c
 *
 *  Created on: 2024��4��18��
 *      Author: pc
 */
#include "zf_common_headfile.h"

void Parament_init(void)
{
    flash_buffer_clear();
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);      // �����ݴ� flash ��ȡ��������
    system_delay_ms(50);


}
