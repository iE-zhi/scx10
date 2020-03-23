#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "stm32f10x.h"
#include "usart.h"
#include "sys.h"
#include "24cxx.h"
#include "crc16.h"

#define F_LED PBout(4)
#define B_LED PBout(7)
#define L_LED PBout(5)
#define R_LED PBout(6)

struct LED
{
	unsigned char L_LED_EN : 1;
	unsigned char R_LED_EN : 1;
};

extern struct LED veer_LED;
extern int Servo_Value;
extern u8 OC_Value;
//extern int Acc_Value;

void para_usart_data(void);
void LED_Init(void);
void Enable_motor(void);

#endif
