#include "DS18B20.h"
#include "delay.h"

/**************************************************************************************
 * ��  �� : ����DS18B20�õ���I/O��
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
static void DS18B20_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOE_CLK_ENABLE();  // ����GPIOEʱ��

    GPIO_InitStruct.Pin = GPIO_PIN_2;    // PE2
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // �������
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // ��������
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  // ����
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);  // ��ʼ��GPIOE Pin2

    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);  // ����Ϊ�ߵ�ƽ
}

/**************************************************************************************
 * ��  �� : ����ʹDS18B20-DATA���ű�Ϊ����ģʽ
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
static void DS18B20_Mode_IPU(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  // ����ģʽ
    GPIO_InitStruct.Pull = GPIO_PULLUP;      // ��������
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);  // ��ʼ��GPIOE Pin2Ϊ����
}

/**************************************************************************************
 * ��  �� : ����ʹDS18B20-DATA���ű�Ϊ���ģʽ
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
static void DS18B20_Mode_Out_PP(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // �������
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // ��������
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  // ����
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);  // ��ʼ��GPIOE Pin2Ϊ���
}

/**************************************************************************************
 * ��  �� : �������ӻ����͸�λ����
 * ��  �� : ��
 * ����ֵ : ��
 **************************************************************************************/
static void DS18B20_Rst(void)
{
    DS18B20_Mode_Out_PP();     // ��������Ϊ�������

    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);  // �����͵�ƽ��λ�ź�
    delay_us(750);  // 750us�ĵ͵�ƽ
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);    // ��������
    delay_us(15);    // �ȴ��ӻ����ʹ�������
}

/**************************************************************************************
 * ��  �� : ���ӻ����������صĴ�������
 * ��  �� : ��
 * ����ֵ : 0���ɹ�   1��ʧ��
 **************************************************************************************/
static uint8_t DS18B20_Presence(void)
{
    uint8_t pulse_time = 0;

    DS18B20_Mode_IPU();    // ����Ϊ��������

    // �ȴ���������ĵ�������������Ϊһ��60~240us�ĵ͵�ƽ�ź�
    while (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) && pulse_time < 100)
    {
        pulse_time++;
        delay_us(1);
    }

    if (pulse_time >= 100)
        return 1;  // ��ʱ����ȡʧ��

    pulse_time = 0;

    while (!HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) && pulse_time < 240)
    {
        pulse_time++;
        delay_us(1);
    }

    if (pulse_time >= 240)
        return 1;  // ��ʱ����ȡʧ��

    return 0;  // �ɹ�
}

/**************************************************************************************
 * ��  �� : ��DS18B20��ȡһ��bit
 * ��  �� : ��
 * ����ֵ : uint8_t 
 **************************************************************************************/
static uint8_t DS18B20_Read_Bit(void)
{
    uint8_t dat;

    DS18B20_Mode_Out_PP();

    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);  // �����͵�ƽ�ź�
    delay_us(10);  // 10us�͵�ƽʱ��

    DS18B20_Mode_IPU();  // ����Ϊ����

    if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) == GPIO_PIN_SET)
        dat = 1;
    else
        dat = 0;

    delay_us(45);  // �ȴ��ȶ�

    return dat;
}

/**************************************************************************************
 * ��  �� : ��DS18B20��һ���ֽڣ���λ����
 * ��  �� : ��
 * ����ֵ : uint8_t  
 **************************************************************************************/
uint8_t DS18B20_Read_Byte(void)
{
    uint8_t i, j, dat = 0;

    for (i = 0; i < 8; i++)
    {
        j = DS18B20_Read_Bit();  // ��DS18B20��ȡһ��bit
        dat = (dat) | (j << i);
    }

    return dat;
}

/**************************************************************************************
 * ��  �� : дһ���ֽڵ�DS18B20����λ����
 * ��  �� : uint8_t
 * ����ֵ : ��  
 **************************************************************************************/
void DS18B20_Write_Byte(uint8_t dat)
{
    uint8_t i, testb;
    DS18B20_Mode_Out_PP();

    for (i = 0; i < 8; i++)
    {
        testb = dat & 0x01;
        dat = dat >> 1;

        if (testb)
        {
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);  // д0
            delay_us(8);   // 1us < ��ʱ < 15us

            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);  // ��������
            delay_us(58);    // 58us + 8us > 60us
        }
        else
        {
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);  // д0
            delay_us(70);  // 60us < Tx 0 < 120us

            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);  // ��������
            delay_us(2);   // 1us < Trec < ����
        }
    }
}

/**************************************************************************************
 * ��  �� : ��ʼDS18B20
 * ��  �� : ��
 * ����ֵ : ��  
 **************************************************************************************/
void DS18B20_Start(void)
{
    DS18B20_Rst();           // �������ӻ����͸�λ����
    DS18B20_Presence();      // ���ӻ����������صĴ�������
    DS18B20_Write_Byte(0xCC);  // ����ROM
    DS18B20_Write_Byte(0x44);  // ��ʼת��
}

/**************************************************************************************
 * ��  �� : DS18B20��ʼ������
 * ��  �� : ��
 * ����ֵ : uint8_t
 **************************************************************************************/
uint8_t DS18B20_Init(void)
{
    DS18B20_GPIO_Config();
    DS18B20_Rst();

    return DS18B20_Presence();
}

/**************************************************************************************
 * ��  �� : ��DS18B20��ȡ�¶�ֵ
 * ��  �� : ��  
 * ����ֵ : float 
 **************************************************************************************/
float DS18B20_Get_Temp(void)
{
    uint8_t tpmsb, tplsb;
    short s_tem;
    float f_tem;

    DS18B20_Rst();
    DS18B20_Presence();
    DS18B20_Write_Byte(0xCC);  // ����ROM
    DS18B20_Write_Byte(0x44);  // ��ʼת��

    DS18B20_Rst();
    DS18B20_Presence();
    DS18B20_Write_Byte(0xCC);  // ����ROM
    DS18B20_Write_Byte(0xBE);  // ��ȡ�¶�ֵ

    tplsb = DS18B20_Read_Byte();
    tpmsb = DS18B20_Read_Byte();

    s_tem = tpmsb << 8;
    s_tem = s_tem | tplsb;

    if (s_tem < 0)  // ���¶�
        f_tem = (~s_tem + 1) * 0.0625;
    else
        f_tem = (s_tem * 0.0625);

    return f_tem;
}