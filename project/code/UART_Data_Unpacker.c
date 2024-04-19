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
#include "UART_Data_Unpacker.h"

// 公共变量定义
uint8_t UART3_get_data[PACKET_MAX_SIZE] = {0}; // 串口3接收的数据
volatile bool UART3_data_packet_ready = false; // 串口3接收完数据包的标志位，完成接收置1

/**
 * @brief 对缓存包处理，包括标签匹配和数据使用。
 * @param Tag_packet[]： 结构体数组，用于创建查找表存放标签、链接函数与变量
 * @param tag_count： 查找表数量
 * @note  注意填入的标签数正确
 *
 */
void PacketTag_Analysis(PacketTag_TpDef_struct Tag_packet[], uint8_t tag_count)
{
    uint8_t i = 0;
    for (i = 0; i < tag_count; i++)
    {
        // 检查标签是否匹配
        if (strncmp(UART3_get_data, Tag_packet[i].tag, TAG_LENGTH) == 0)
        {
            // 调用关联的处理函数
            Tag_packet[i].function_handler(UART3_get_data, Tag_packet[i].value_ptr);
            return; // 找到匹配项后返回
        }
    }
    uint8_t error_tag[TAG_LENGTH + 1] = {0};
    strncpy(error_tag, UART3_get_data, TAG_LENGTH);
    error_tag[TAG_LENGTH] = '\0';                        // 显式地设置空终止符
    printf_USART3("The \"%s\" is not find.", error_tag); // 错误处理
    // 如果没有找到任何匹配，可以在这里处理错误
    // U1_printf("The \"%s\" is not find.", Tag_packet[i].tag);
}

void UnpackData_Handle_Float_FireWater(const char *packet, void *value)
{
    // char numString[PACKET_MAX_SIZE];
    // strncpy(numString, packet + TAG_LENGTH, PACKET_MAX_SIZE - 1);
    *((float *)value) = atof(packet + TAG_LENGTH);
}
void UnpackData_Handle_Int_FireWater(const char *packet, void *value)
{
    // char numString[PACKET_MAX_SIZE];
    // strncpy(numString, packet + TAG_LENGTH, PACKET_MAX_SIZE - 1);
    *((int *)value) = atoi(packet + TAG_LENGTH);
}
void UnpackData_Handle_Int16_IEEE754(const uint8 *packet, void *int_value)
{
    uint8_t highByte = *(packet + TAG_LENGTH);    // 获取高字节
    uint8_t lowByte = *(packet + TAG_LENGTH + 1); // 获取低字节
    *(int16_t *)int_value = (int16_t)((highByte << 8) | lowByte);
}

/**
 *  @brief 使用USART3输出的printf
 *  @param 格式化字符串
 *  @return void
 *  @warning
 *  @note 例子： printf_USART3("text:%d", 1212); 串口接收到 text:1212
 */
void printf_USART3(char *format_str, ...)
{
    uint8_t buffer[64];
    va_list arg;
    va_start(arg, format_str);
    vsprintf(buffer, format_str, arg);
    va_end(arg);
    uart_write_string(DEBUG_UART_INDEX, buffer);
}

/**
 *  @brief USART3中断处理函数，按照固定数据包接收
 *  @note 在isr.h中声明，在isr.c中USART3_IRQ_Function(void)引用
 *  @warning
 */
void USART3_IRQ_Function(void)
{
    static uint8_t UART3_get_data_index = 0;
    static uint8_t prev_byte = 0;
    static bool start_load_packet_flag = false;
    uint8_t byte = 0;
    if (uart_query_byte(DEBUG_UART_INDEX, &byte))
    {
        if (!start_load_packet_flag)
        {
            if (prev_byte == '~' && byte == '}')
            {
                start_load_packet_flag = true;
                UART3_get_data_index = 0;
            }
        }
        else
        {
            if (prev_byte == '}' && byte == '~' && start_load_packet_flag)
            {
                UART3_get_data[UART3_get_data_index - 1] = '\0';
                UART3_data_packet_ready = true;
                start_load_packet_flag = false;
            }
            else if (UART3_get_data_index < PACKET_MAX_SIZE)
            {
                UART3_get_data[UART3_get_data_index++] = byte;
            }
            else
            {
                memset(UART3_get_data, 0, PACKET_MAX_SIZE);
                printf_USART3("Error: Data packet overflow.\r\n");
                start_load_packet_flag = false;
            }
        }
        prev_byte = byte;
    }
}
