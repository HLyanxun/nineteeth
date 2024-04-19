/*********************************************************************************************************************
 * ��ģ�����USART3ʵ��
 * ��飺������~}��ͷ��}~��β�����ݰ������ݰ�ǰTAG_LENGTHλ���ڱ�ǩƥ�䣬�������Ϊ���ݽ���
 * ʵ�֣�
*    �ж��а��չ̶���ʽ�������ݰ������� UART3_get_data[PACKET_MAX_SIZE] �У�
 *   ͨ������ PacketTag_Analysis �� PacketTag_TpDef_struct �ṹ�������еı�ǩƥ�䣬
 *   �Թ��ر�����ֵ��
 * �÷���
 * 1.�غɴ�Сͨ�� #define PACKET_MAX_SIZE �޸�
 * 2.PacketTag_TpDef_struct �����ṹ�����飬�洢��ǩ�����غ����Լ����ر���
 * 3.�������ж��д洢�� UART3_get_data[] �У�ʹ��ʱֱ����
 * 4.UART3_data_packet_ready ���ǽ���������ϱ�־λ�������1
 * 5.�ṩ�˶������ݽ�����ʽ
 * ע�⣺
 * 1.ʹ��ʱ��Ҫ��USART3��ʼ���������ж�
 * 2.�жϺ�����ʹ�ùر���zf_common_debug.h �ļ��е� #define DEBUG_UART_USE_INTERRUPT    (0)����Ҫʹ��debug uart�����ֶ���
 * ���ӣ�main.c
 #include "zf_common_headfile.h"

int test_value_2 = 0;
float test_value_1 = 0.000;

PacketTag_TpDef_struct Test_packet[] = {
    {"TEST", UnpackData_Handle_Float_FireWater, &test_value_1},
    {"TES1", UnpackData_Handle_Int_FireWater, &test_value_2},
    // ��Ӹ����ӳ���ϵ
};

int main (void)
{
    clock_init(SYSTEM_CLOCK_120M);      // ��ʼ��оƬʱ�� ����Ƶ��Ϊ 120MHz
    debug_init();                       // ��ر��������������ڳ�ʼ��MPU ʱ�� ���Դ���
    uart_init(DEBUG_UART_INDEX, DEBUG_UART_BAUDRATE, DEBUG_UART_TX_PIN, DEBUG_UART_RX_PIN); // ���ڳ�ʼ��
    uart_rx_interrupt(DEBUG_UART_INDEX, ENABLE);                                            // ���������ж�
    interrupt_set_priority(USART3_IRQn, (0 << 5) || 1);                                     // ����usart3���ж����ȼ�

    while(1)
    {
        // �˴���д��Ҫѭ��ִ�еĴ���
        if (UART3_data_packet_ready)
            {
                UART3_data_packet_ready = false;
                printf_USART3("\r\nReturn:%s\r\n", UART3_get_data);
                PacketTag_Analysis(Test_packet,2);
                printf_USART3("\r\ntestv1:%f\r\n", test_value_1);
                printf_USART3("\r\ntestv2:%d\r\n", test_value_2);
            }
        // �˴���д��Ҫѭ��ִ�еĴ���
    }
}
 * �޸ļ�¼
 *
 * ����                                      ����                             ��ע
 * 2024-03-24                               Sxxx
 ********************************************************************************************************************/
#ifndef UART_DATA_UNPACKER_H
#define UART_DATA_UNPACKER_H

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include <stdbool.h>
#include "zf_driver_uart.h"

#define PACKET_MAX_SIZE 16 // �غɴ�С,�Զ����޸�
#define TAG_LENGTH 4       // ��ǩ���ȣ��Զ����޸�

// �������������������޸�
extern uint8_t UART3_get_data[PACKET_MAX_SIZE]; // ����3���ݰ����ж��д洢����������
extern volatile bool UART3_data_packet_ready;   // ����3���������ݰ��ı�־λ���������ʣ����ݰ����������1

// ����Ϊ˽��Ԫ�أ��ڲ����ã�������ģ�
typedef void (*Function_Unpack_Handler)(const char *, void *); // ����һ������ָ�����ͣ���ӵ��ý������
// �����ı���ʽ��ȡ��float����,������Ϊ ��ǩ����+3+С������+��������
void UnpackData_Handle_Float_FireWater(const char *packet, void *value);
// �����ı���ʽ��ȡ��int���ݣ����������Ϊ ��ǩ����+1+��������
void UnpackData_Handle_Int_FireWater(const char *packet, void *value);
// ��ieee754��ʽ��ȡ��int16���ݣ������ű�ǩ���Ǹ߰�λ����һλ�ǵͰ�λ�������޷�-32100~32100�����������Ϊ ��ǩ����+3
void UnpackData_Handle_Int_IEEE754(const uint8 *packet, void *int_value);

// ˽�нṹ�壬��װ��ǩ�͹ҽӺ����Լ��洢��ֵ
typedef struct
{
    const char tag[TAG_LENGTH + 1];
    Function_Unpack_Handler function_handler;
    void *value_ptr;
} PacketTag_TpDef_struct;
// ˽��Ԫ��END

// �����ӿں���
void PacketTag_Analysis(PacketTag_TpDef_struct Tag_packet[], uint8_t tag_count);
void printf_USART3(char *format_str, ...);

#endif // UART_DATA_UNPACKER_H
