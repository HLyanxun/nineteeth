/*********************************************************************************************************************
 * 本模块借助USART3实现
 * 简介：接收以~}开头以}~结尾的数据包，数据包前TAG_LENGTH位用于标签匹配，其余的作为数据接收
 * 实现：
*    中断中按照固定格式接收数据包，存入 UART3_get_data[PACKET_MAX_SIZE] 中，
 *   通过调用 PacketTag_Analysis 对 PacketTag_TpDef_struct 结构体数组中的标签匹配，
 *   对挂载变量赋值。
 * 用法：
 * 1.载荷大小通过 #define PACKET_MAX_SIZE 修改
 * 2.PacketTag_TpDef_struct 创建结构体数组，存储标签、挂载函数以及挂载变量
 * 3.数据在中断中存储到 UART3_get_data[] 中，使用时直接用
 * 4.UART3_data_packet_ready 这是接收数据完毕标志位，完成置1
 * 5.提供了多种数据解析格式
 * 注意：
 * 1.使用时需要对USART3初始化、配置中断
 * 2.中断函数的使用关闭了zf_common_debug.h 文件中的 #define DEBUG_UART_USE_INTERRUPT    (0)，如要使用debug uart，请手动打开
 * 例子：main.c
 #include "zf_common_headfile.h"

int test_value_2 = 0;
float test_value_1 = 0.000;

PacketTag_TpDef_struct Test_packet[] = {
    {"TEST", UnpackData_Handle_Float_FireWater, &test_value_1},
    {"TES1", UnpackData_Handle_Int_FireWater, &test_value_2},
    // 添加更多的映射关系
};

int main (void)
{
    clock_init(SYSTEM_CLOCK_120M);      // 初始化芯片时钟 工作频率为 120MHz
    debug_init();                       // 务必保留，本函数用于初始化MPU 时钟 调试串口
    uart_init(DEBUG_UART_INDEX, DEBUG_UART_BAUDRATE, DEBUG_UART_TX_PIN, DEBUG_UART_RX_PIN); // 串口初始化
    uart_rx_interrupt(DEBUG_UART_INDEX, ENABLE);                                            // 开启接收中断
    interrupt_set_priority(USART3_IRQn, (0 << 5) || 1);                                     // 设置usart3的中断优先级

    while(1)
    {
        // 此处编写需要循环执行的代码
        if (UART3_data_packet_ready)
            {
                UART3_data_packet_ready = false;
                printf_USART3("\r\nReturn:%s\r\n", UART3_get_data);
                PacketTag_Analysis(Test_packet,2);
                printf_USART3("\r\ntestv1:%f\r\n", test_value_1);
                printf_USART3("\r\ntestv2:%d\r\n", test_value_2);
            }
        // 此处编写需要循环执行的代码
    }
}
 * 修改记录
 *
 * 日期                                      作者                             备注
 * 2024-03-24                               Sxxx
 ********************************************************************************************************************/
#ifndef UART_DATA_UNPACKER_H
#define UART_DATA_UNPACKER_H

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include <stdbool.h>
#include "zf_driver_uart.h"

#define PACKET_MAX_SIZE 16 // 载荷大小,自定义修改
#define TAG_LENGTH 4       // 标签长度，自定义修改

// 公共变量声明，不可修改
extern uint8_t UART3_get_data[PACKET_MAX_SIZE]; // 串口3数据包，中断中存储，公共访问
extern volatile bool UART3_data_packet_ready;   // 串口3接收完数据包的标志位，公共访问，数据包接收完毕置1

// 以下为私有元素（内部调用，无需关心）
typedef void (*Function_Unpack_Handler)(const char *, void *); // 定义一个函数指针类型，间接调用解包函数
// 按照文本格式提取出float数据,包长度为 标签长度+3+小数长度+整数长度
void UnpackData_Handle_Float_FireWater(const char *packet, void *value);
// 按照文本格式提取出int数据，建议包长度为 标签长度+1+整数长度
void UnpackData_Handle_Int_FireWater(const char *packet, void *value);
// 按ieee754格式提取出int16数据，紧跟着标签的是高八位，下一位是低八位。数据限幅-32100~32100，建议包长度为 标签长度+3
void UnpackData_Handle_Int_IEEE754(const uint8 *packet, void *int_value);

// 私有结构体，封装标签和挂接函数以及存储数值
typedef struct
{
    const char tag[TAG_LENGTH + 1];
    Function_Unpack_Handler function_handler;
    void *value_ptr;
} PacketTag_TpDef_struct;
// 私有元素END

// 公共接口函数
void PacketTag_Analysis(PacketTag_TpDef_struct Tag_packet[], uint8_t tag_count);
void printf_USART3(char *format_str, ...);

#endif // UART_DATA_UNPACKER_H
