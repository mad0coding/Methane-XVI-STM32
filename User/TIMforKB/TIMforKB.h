#ifndef __TIMFORKB_H
#define __TIMFORKB_H

#include "stm32f10x.h"

#define ifTimEncode		0//0使用中断,1使用TIM

extern uint16_t IrqTimValue;

#define ENCODER_TIM_PERIOD (u16)(65535)   //不可大于65535 因为F103的定时器是16位的

/**********************高级定时器TIM1参数定义**********************/
#define			BUZZ_TIM					TIM1
#define			BUZZ_TIM_APBxClock_FUN		RCC_APB2PeriphClockCmd
#define			BUZZ_TIM_CLK				RCC_APB2Periph_TIM1
#define			BUZZ_TIM_Period				61069
#define			BUZZ_TIM_Prescaler			8//61069和8为131Hz(C3音)
/**********************通用定时器TIM2参数定义**********************/
#define			LED_TIM						TIM2
#define			LED_TIM_APBxClock_FUN		RCC_APB1PeriphClockCmd
#define			LED_TIM_CLK					RCC_APB1Periph_TIM2
#define			LED_TIM_Period				7999
#define			LED_TIM_Prescaler			2//7999和2为3kHz


//*********定时器初始化*********//
void TIM1_PWM_Init(void);
void TIM2_PWM_Init(void);
void TIM3_Encoder_Init(void);
//*****************************//

//********编码器溢出中断********//
void TIM3_IRQHandler(void);
//*****************************//
#endif
