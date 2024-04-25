/*
 * flash.c
 *
 *  Created on: 2024年4月18日
 *      Author: pc
 */
#include "zf_common_headfile.h"
// ***************** 如果需要保存数据，务必取消勾选Erase All(擦除所有选项) *************************** //
// ***************** 如果需要保存数据，务必取消勾选Erase All(擦除所有选项) *************************** //
// ***************** 如果需要保存数据，务必取消勾选Erase All(擦除所有选项) *************************** //

// ***************** 擦写FLASH的时候，设置系统频率必须小于或等于120M *************************** //
// ***************** 擦写FLASH的时候，设置系统频率必须小于或等于120M *************************** //
// ***************** 擦写FLASH的时候，设置系统频率必须小于或等于120M *************************** //
//----------------------------------------------------------------------------------------------------------
// 函数简介     初始化flash，将存在flash中的数据读取出来
// 使用实例     Parament_init();
//----------------------------------------------------------------------------------------------------------
void Parament_init(void)
{
    flash_buffer_clear();
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);      // 将数据从 flash 读取到缓冲区
    system_delay_ms(50);

    kp   =          flash_union_buffer[0].float_type;
    ki   =          flash_union_buffer[1].float_type;
    kd   =          flash_union_buffer[2].float_type;
    dt   =          flash_union_buffer[3].float_type;
    target   =      flash_union_buffer[4].float_type;
    threshold_fix = flash_union_buffer[5].int16_type;
    exposure_time = flash_union_buffer[6].int16_type;
    mt9v03x_set_exposure_time(exposure_time);
}
//----------------------------------------------------------------------------------------------------------
// 函数简介     本次ui调参数据烧录到flash中
// 使用实例     Parament_save();
// 备注信息     断电前别忘保存
//----------------------------------------------------------------------------------------------------------
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
   flash_union_buffer[6].int16_type = exposure_time;
   flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);        // 向指定 Flash 扇区的页码写入缓冲区数据
   flash_buffer_clear();
}
//-----------------------------------------------------------------------------------------------------------
// 函数简介     将新数据添加进flash中时，将所有数据恢复默认值（第一次添加才需要，用完记得删）、
// 使用实例     Parament_first();
// 备注信息     别忘用，别忘删！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
//-----------------------------------------------------------------------------------------------------------
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
    flash_union_buffer[6].int16_type = 512;
    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);     // 向指定 Flash 扇区的页码写入缓冲区数据
    flash_buffer_clear();                                                  // 清空缓冲区
}

