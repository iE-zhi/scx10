#include "stm32f10x.h"
#include "delay.h"
#include "timer.h"
#include "controller.h"
#include "adc.h"

int main(void)
{
	u16 adc_L = 0;
	u16 adc_R = 0;
	float L_Voltage = 0.0;
	float R_Voltage = 0.0;
	float Current_Value = 0.0;
	
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	
	LED_Init();
	uart2_init(115200);
	//24C02总线初始化
	AT24CXX_Init();			//IIC初始化
	Adc_Init();		  		//ADC初始化
  
	//有刷电机PWM初始化
	TIM3_PWM_Init(7199, 0);      //定时器3用于产生PWM控制有刷电机，PWM频率: 72M/7200 = 10KHz
	TIM_SetCompare1(TIM3,0);
	TIM_SetCompare2(TIM3,0);
	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
	
	//使能有刷电机
	Enable_motor();
	
	//转向舵机PWM初始化
	TIM4_PWM_Init(19999, 71);      //定时器4产生50Hz的PWM控制舵机，比较值范围[999,1999]
	Servo_Value = AT24CXX_ReadLenByte(0x10, sizeof(Servo_Value));      //读取保存在24C02中的值
	Servo_Value = (Servo_Value >= 999 && Servo_Value <= 1999) ? Servo_Value : 1499;
	TIM_SetCompare3(TIM4, Servo_Value);      //TIM4_CH3占空比7.5%,使舵机处于中间位置
	TIM_Cmd(TIM4, ENABLE);  //使能TIMx外设
	
	//转向灯定时器初始化
	TIM2_Int_Init(4999,7199);      //定时器2产生2Hz的频率控制转向灯闪烁
	//关闭转向灯
	veer_LED.L_LED_EN = 0;      //失能转向灯控制
	veer_LED.R_LED_EN = 0;
	L_LED = 0;      //关闭灯
	R_LED = 0;
	TIM_Cmd(TIM2, ENABLE);  //使能TIM2外设
	
	//读取过流保护值
	OC_Value = AT24CXX_ReadLenByte(0x1a, sizeof(OC_Value));
	OC_Value = (OC_Value >= 2 && OC_Value <=30) ? OC_Value : 4; 
	
	while (1)
  {
		adc_L = Get_Adc_Average(ADC_Channel_5,10);
		L_Voltage = (float)adc_L*(3.3/4096);
		adc_R = Get_Adc_Average(ADC_Channel_5,10);
		R_Voltage = (float)adc_R*(3.3/4096);
		Current_Value = (L_Voltage + R_Voltage) / 2 * 19.5;
		if (Current_Value > OC_Value)
		{
			// 关闭电机
			TIM_SetCompare1(TIM3,0);
			TIM_SetCompare2(TIM3,0);
			// 关闭串口
			USART_Cmd(USART2, DISABLE);
		}
		delay_ms(20);
  }
}


/******************* 微意电子 *****END OF FILE****/
