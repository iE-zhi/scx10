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
	
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	
	LED_Init();
	uart2_init(115200);
	//24C02���߳�ʼ��
	AT24CXX_Init();			//IIC��ʼ��
	Adc_Init();		  		//ADC��ʼ��
  
	//��ˢ���PWM��ʼ��
	TIM3_PWM_Init(7199, 0);      //��ʱ��3���ڲ���PWM������ˢ�����PWMƵ��: 72M/7200 = 10KHz
	TIM_SetCompare1(TIM3,0);
	TIM_SetCompare2(TIM3,0);
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
	
	//ʹ����ˢ���
	Enable_motor();
	
	//ת����PWM��ʼ��
	TIM4_PWM_Init(19999, 71);      //��ʱ��4����50Hz��PWM���ƶ�����Ƚ�ֵ��Χ[999,1999]
	Servo_Value = AT24CXX_ReadLenByte(0x10, sizeof(Servo_Value));      //��ȡ������24C02�е�ֵ
	Servo_Value = (Servo_Value >= 999 && Servo_Value <= 1999) ? Servo_Value : 1499;
	TIM_SetCompare3(TIM4, Servo_Value);      //TIM4_CH3ռ�ձ�7.5%,ʹ��������м�λ��
	TIM_Cmd(TIM4, ENABLE);  //ʹ��TIMx����
	
	//ת��ƶ�ʱ����ʼ��
	TIM2_Int_Init(4999,7199);      //��ʱ��2����2Hz��Ƶ�ʿ���ת�����˸
	//�ر�ת���
	veer_LED.L_LED_EN = 0;      //ʧ��ת��ƿ���
	veer_LED.R_LED_EN = 0;
	L_LED = 0;      //�رյ�
	R_LED = 0;
	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIM2����
	
	//��ȡ��������ֵ
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
			// �رյ��
			TIM_SetCompare1(TIM3,0);
			TIM_SetCompare2(TIM3,0);
			// �رմ���
			USART_Cmd(USART2, DISABLE);
		}
		delay_ms(20);
  }
}


/******************* ΢����� *****END OF FILE****/
