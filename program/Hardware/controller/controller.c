/*
/ 2020.3.14 指定基础命令，实现串口数据解析，控制车的动作
* CRC校验码由前面所有字节校验得到
*
- 有刷电机和舵机控制指令 -
| 包头   命令   电机方向   电机速度   舵机方向   舵机角度   CRC16低字节   CRC16高字节  结束符
| 0x23   0xa5   0x56(正)   0x00~0xff  0x58(左)   0x00~0xff     0x00          0x00       \r\n
|               0x65(反)              0x85(右)
|               0x88(停)              0x88(中心)
*
- 灯控制命令 -
| 包头   命令   前灯   后灯   左转向灯   右转向灯   CRC16低字节   CRC16高字节   结束符
| 0x23   0xa6   0x01   0x01     0x01       0x01        0x00          0x00        \r\n   (所有灯0x01为开,0x00为关)
*
- 舵机中心微调命令 - 
| 包头   命令   舵机方向   CRC16低字节   CRC16高字节   结束符
| 0x23   0xa8   0x85(左)      0x00          0x00        \r\n
|               0x58(右)
|               0x88(恢复默认的值 Servo_Value=1499)
*
- 舵机中心值保存命令 -
| 包头   命令   CRC16低字节   CRC16高字节   结束符
| 0x23   0xb0      0x00          0x00        \r\n
*
/ 2020.3.15 添加过流保护指令 
- 设置过流保护值命令 -
| 包头   命令     过流保护值   CRC16低字节   CRC16高字节   结束符
| 0x23   0xb2   0x00~1E(0-30A)    0x00          0x00        \r\n
*
* 串口数据接收存放变量USART_RX_BUF
*/
#include "controller.h"

struct LED veer_LED;

int Servo_Value = 1499;    // 转向舵机中心值
//int Acc_Value = 1499;   // 油门舵机中心值
u8 OC_Value = 4;      // 默认过流保护值

// 电机控制
static void Motor_control(void)
{
	float Speed_Value = 0.0;
	
	Speed_Value = (float)USART_RX_BUF[3] / 255.0 * 3599 + 0.5;
	if ((unsigned int)Speed_Value == 0)
		USART_RX_BUF[2] = 0x88;
	switch(USART_RX_BUF[2])
	{
		case 0x56: // 电机正转
			(3600+(int)Speed_Value<=7199) ? TIM_SetCompare1(TIM3,3600 + (int)Speed_Value) : TIM_SetCompare1(TIM3, 7199);
			(3599-(int)Speed_Value>=0) ? TIM_SetCompare2(TIM3,3599 - (int)Speed_Value) : TIM_SetCompare1(TIM3, 0);
			break;
		case 0x65: // 电机反转
			(3599-(int)Speed_Value>=0) ? TIM_SetCompare1(TIM3,3599 - (int)Speed_Value) : TIM_SetCompare1(TIM3, 0);
			(3600+(int)Speed_Value<=7199) ? TIM_SetCompare2(TIM3,3600 + (int)Speed_Value) : TIM_SetCompare1(TIM3, 7199);
			break;
		case 0x88: // 电机停转
			TIM_SetCompare1(TIM3,0);
			TIM_SetCompare2(TIM3,0);
			break;
		default:
			break;
	}
}	

// 使能电机控制
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

// 舵机控制
static void Servo_control(void)
{
	float Angle_Value = 0.0;
	
	Angle_Value = (float)USART_RX_BUF[5] / 255.0 * 500 + 0.5;
	if ((unsigned int)Angle_Value == 0)
		USART_RX_BUF[4] = 0x88;
	switch(USART_RX_BUF[4])
	{
		case 0x85: // 左转
			(Servo_Value-(int)Angle_Value >= 999) ? TIM_SetCompare3(TIM4, Servo_Value - (int)Angle_Value) : TIM_SetCompare3(TIM4, 999);
			break;
		case 0x58: // 右转
			(Servo_Value+(int)Angle_Value <= 1999) ? TIM_SetCompare3(TIM4, Servo_Value + (int)Angle_Value) : TIM_SetCompare3(TIM4, 1999);
			break;
		case 0x88: // 中心
			TIM_SetCompare3(TIM4, Servo_Value);
		default:
			break;
	}
}

//车灯IO口初始化
void LED_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化端口
}

// 车灯控制
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

//转向舵机微调
static void Servo_FineTuning(void)
{
	switch(USART_RX_BUF[2])
	{
		case 0x58: //向左微调
			if (Servo_Value > 999)
			{
				Servo_Value--;
				TIM_SetCompare3(TIM4, Servo_Value);
			}
			break;
		case 0x85: //向右微调
			if (Servo_Value < 1999)
			{
				Servo_Value++;
				TIM_SetCompare3(TIM4, Servo_Value);
			}
			break;
		case 0x88: //恢复默认的值
			Servo_Value = 1499;
			TIM_SetCompare3(TIM4, Servo_Value);
			break;
		default:
			break;
	}
}

//保存舵机中心值
static void Save_ServoValue(void)
{
	AT24CXX_WriteLenByte(0x10, Servo_Value, sizeof(Servo_Value));
}

//保存过流保护值
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
			case 0xa5: // 电机、舵机控制
				Motor_control();
				Servo_control();
				break;
			case 0xa6: // 车灯控制
				LED_control();
				break;
			case 0xa8: // 舵机微调
				Servo_FineTuning();
				break;
			case 0xb0: // 舵机中心值保存
				Save_ServoValue();
				break;
			case 0xb2: // 设置过流保护值
				Set_OCValue();
				break;
			default:
				break;
		}
	}
}
