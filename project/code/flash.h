/*
 * flash.h
 *
 *  Created on: 2024年4月18日
 *      Author: pc
 */

#ifndef FLASH_H_
#define FLASH_H_
#include "zf_common_headfile.h"

#define FLASH_SECTION_INDEX       (63)                                          // 存储数据用的扇区 倒数第一个扇区
#define FLASH_PAGE_INDEX          (3)                                           // 存储数据用的页码 倒数第一个页码
extern uint8 pwm_test;
void Parameter_init(void);
void Parameter_save(void);
void Parameter_first(void);
void All_Init(void);
void test(void);

#endif /* FLASH_H_ */
