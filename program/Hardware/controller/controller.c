/*
/ 2020.3.14 ָ���������ʵ�ִ������ݽ��������Ƴ��Ķ���
* CRCУ������ǰ�������ֽ�У��õ�
*
- ��ˢ����Ͷ������ָ�� -
| ��ͷ   ����   �������   ����ٶ�   �������   ����Ƕ�   CRC16���ֽ�   CRC16���ֽ�  ������
| 0x23   0xa5   0x56(��)   0x00~0xff  0x58(��)   0x00~0xff     0x00          0x00       \r\n
|               0x65(��)              0x85(��)
|               0x88(ͣ)              0x88(����)
*
- �ƿ������� -
| ��ͷ   ����   ǰ��   ���   ��ת���   ��ת���   CRC16���ֽ�   CRC16���ֽ�   ������
| 0x23   0xa6   0x01   0x01     0x01       0x01        0x00          0x00        \r\n   (���е�0x01Ϊ��,0x00Ϊ��)
*
- �������΢������ - 
| ��ͷ   ����   �������   CRC16���ֽ�   CRC16���ֽ�   ������
| 0x23   0xa8   0x85(��)      0x00          0x00        \r\n
|               0x58(��)
|               0x88(�ָ�Ĭ�ϵ�ֵ Servo_Value=1499)
*
- �������ֵ�������� -
| ��ͷ   ����   CRC16���ֽ�   CRC16���ֽ�   ������
| 0x23   0xb0      0x00          0x00        \r\n
*
/ 2020.3.15 ��ӹ�������ָ�� 
- ���ù�������ֵ���� -
| ��ͷ   ����     ��������ֵ   CRC16���ֽ�   CRC16���ֽ�   ������
| 0x23   0xb2   0x00~1E(0-30A)    0x00          0x00        \r\n
*
* �������ݽ��մ�ű���USART_RX_BUF
*/
#include "controller.h"

struct LED veer_LED;

int Servo_Value = 1499;    // ת��������ֵ
//int Acc_Value = 1499;   // ���Ŷ������ֵ
u8 OC_Value = 4;      // Ĭ�Ϲ�������ֵ

// �������
static void Motor_control(void)
{
	float Speed_Value = 0.0;
	
	Speed_Value = (float)USART_RX_BUF[3] / 255.0 * 3599 + 0.5;
	if ((unsigned int)Speed_Value == 0)
		USART_RX_BUF[2] = 0x88;
	switch(USART_RX_BUF[2])
	{
		case 0x56: // �����ת
			(3600+(int)Speed_Value<=7199) ? TIM_SetCompare1(TIM3,3600 + (int)Speed_Value) : TIM_SetCompare1(TIM3, 7199);
			(3599-(int)Speed_Value>=0) ? TIM_SetCompare2(TIM3,3599 - (int)Speed_Value) : TIM_SetCompare1(TIM3, 0);
			break;
		case 0x65: // �����ת
			(3599-(int)Speed_Value>=0) ? TIM_SetCompare1(TIM3,3599 - (int)Speed_Value) : TIM_SetCompare1(TIM3, 0);
			(3600+(int)Speed_Value<=7199) ? TIM_SetCompare2(TIM3,3600 + (int)Speed_Value) : TIM_SetCompare1(TIM3, 7199);
			break;
		case 0x88: // ���ͣת
			TIM_SetCompare1(TIM3,0);
			TIM_SetCompare2(TIM3,0);
			break;
		default:
			break;
	}
}	

// ʹ�ܵ������
void Enable_motor(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_0);
}

// �������
static void Servo_control(void)
{
	float Angle_Value = 0.0;
	
	Angle_Value = (float)USART_RX_BUF[5] / 255.0 * 500 + 0.5;
	if ((unsigned int)Angle_Value == 0)
		USART_RX_BUF[4] = 0x88;
	switch(USART_RX_BUF[4])
	{
		case 0x85: // ��ת
			(Servo_Value-(int)Angle_Value >= 999) ? TIM_SetCompare3(TIM4, Servo_Value - (int)Angle_Value) : TIM_SetCompare3(TIM4, 999);
			break;
		case 0x58: // ��ת
			(Servo_Value+(int)Angle_Value <= 1999) ? TIM_SetCompare3(TIM4, Servo_Value + (int)Angle_Value) : TIM_SetCompare3(TIM4, 1999);
			break;
		case 0x88: // ����
			TIM_SetCompare3(TIM4, Servo_Value);
		default:
			break;
	}
}

//����IO�ڳ�ʼ��
void LED_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ���˿�
}

// ���ƿ���
static void LED_control(void)
{
	if (USART_RX_BUF[2])
		F_LED = 1;
	else
		F_LED = 0;
	if (USART_RX_BUF[3])
		B_LED = 1;
	else
		B_LED = 0;
	if (USART_RX_BUF[4])
		veer_LED.L_LED_EN = 1;
	else
	{
		veer_LED.L_LED_EN = 0;
		L_LED = 0;
	}
	if (USART_RX_BUF[5])
		veer_LED.R_LED_EN = 1;
	else
	{
		veer_LED.R_LED_EN = 0;
		R_LED = 0;
	}
}

//ת����΢��
static void Servo_FineTuning(void)
{
	switch(USART_RX_BUF[2])
	{
		case 0x58: //����΢��
			if (Servo_Value > 999)
			{
				Servo_Value--;
				TIM_SetCompare3(TIM4, Servo_Value);
			}
			break;
		case 0x85: //����΢��
			if (Servo_Value < 1999)
			{
				Servo_Value++;
				TIM_SetCompare3(TIM4, Servo_Value);
			}
			break;
		case 0x88: //�ָ�Ĭ�ϵ�ֵ
			Servo_Value = 1499;
			TIM_SetCompare3(TIM4, Servo_Value);
			break;
		default:
			break;
	}
}

//����������ֵ
static void Save_ServoValue(void)
{
	AT24CXX_WriteLenByte(0x10, Servo_Value, sizeof(Servo_Value));
}

//�����������ֵ
static void Set_OCValue(void)
{
	OC_Value = USART_RX_BUF[2];
	AT24CXX_WriteLenByte(0x1a, OC_Value, sizeof(OC_Value));
}

void para_usart_data(void)
{
	unsigned short cal_crc = 0;
	unsigned short rec_crc = 0;

	rec_crc = (USART_RX_BUF[(USART_RX_STA&0X3FFF)-1] << 8) | USART_RX_BUF[(USART_RX_STA&0X3FFF)-2];
	cal_crc = CRC16((char*)USART_RX_BUF);
	if (USART_RX_BUF[0] == '#' && cal_crc == rec_crc)
	{
		switch(USART_RX_BUF[1])
		{
			case 0xa5: // ������������
				Motor_control();
				Servo_control();
				break;
			case 0xa6: // ���ƿ���
				LED_control();
				break;
			case 0xa8: // ���΢��
				Servo_FineTuning();
				break;
			case 0xb0: // �������ֵ����
				Save_ServoValue();
				break;
			case 0xb2: // ���ù�������ֵ
				Set_OCValue();
				break;
			default:
				break;
		}
	}
}
