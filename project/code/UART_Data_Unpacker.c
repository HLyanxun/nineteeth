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
#include "UART_Data_Unpacker.h"

// ������������
uint8_t UART3_get_data[PACKET_MAX_SIZE] = {0}; // ����3���յ�����
volatile bool UART3_data_packet_ready = false; // ����3���������ݰ��ı�־λ����ɽ�����1

/**
 * @brief �Ի��������������ǩƥ�������ʹ�á�
 * @param Tag_packet[]�� �ṹ�����飬���ڴ������ұ��ű�ǩ�����Ӻ��������
 * @param tag_count�� ���ұ�����
 * @note  ע������ı�ǩ����ȷ
 *
 */
void PacketTag_Analysis(PacketTag_TpDef_struct Tag_packet[], uint8_t tag_count)
{
    uint8_t i = 0;
    for (i = 0; i < tag_count; i++)
    {
        // ����ǩ�Ƿ�ƥ��
        if (strncmp(UART3_get_data, Tag_packet[i].tag, TAG_LENGTH) == 0)
        {
            // ���ù����Ĵ�����
            Tag_packet[i].function_handler(UART3_get_data, Tag_packet[i].value_ptr);
            return; // �ҵ�ƥ����󷵻�
        }
    }
    uint8_t error_tag[TAG_LENGTH + 1] = {0};
    strncpy(error_tag, UART3_get_data, TAG_LENGTH);
    error_tag[TAG_LENGTH] = '\0';                        // ��ʽ�����ÿ���ֹ��
    printf_USART3("The \"%s\" is not find.", error_tag); // ������
    // ���û���ҵ��κ�ƥ�䣬���������ﴦ�����
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
    uint8_t highByte = *(packet + TAG_LENGTH);    // ��ȡ���ֽ�
    uint8_t lowByte = *(packet + TAG_LENGTH + 1); // ��ȡ���ֽ�
    *(int16_t *)int_value = (int16_t)((highByte << 8) | lowByte);
}

/**
 *  @brief ʹ��USART3�����printf
 *  @param ��ʽ���ַ���
 *  @return void
 *  @warning
 *  @note ���ӣ� printf_USART3("text:%d", 1212); ���ڽ��յ� text:1212
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
 *  @brief USART3�жϴ����������չ̶����ݰ�����
 *  @note ��isr.h����������isr.c��USART3_IRQ_Function(void)����
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
