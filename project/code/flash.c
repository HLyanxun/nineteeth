/*
 * flash.c
 *
 *  Created on: 2024年4月18日
 *      Author: pc
 */
#include "zf_common_headfile.h"

void Parament_init(void)
{
    flash_buffer_clear();
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);      // 将数据从 flash 读取到缓冲区
    system_delay_ms(50);


}
