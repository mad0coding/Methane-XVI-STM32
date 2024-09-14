#ifndef __TIMFORKB_H
#define __TIMFORKB_H

#include "stm32f10x.h"

#define ifTimEncode		0//0ʹ���ж�,1ʹ��TIM

extern uint16_t IrqTimValue;

#define ENCODER_TIM_PERIOD (u16)(65535)   //���ɴ���65535 ��ΪF103�Ķ�ʱ����16λ��

/**********************�߼���ʱ��TIM1��������**********************/
#define			BUZZ_TIM					TIM1
#define			BUZZ_TIM_APBxClock_FUN		RCC_APB2PeriphClockCmd
#define			BUZZ_TIM_CLK				RCC_APB2Periph_TIM1
#define			BUZZ_TIM_Period				61069
#define			BUZZ_TIM_Prescaler			8//61069��8Ϊ131Hz(C3��)
/**********************ͨ�ö�ʱ��TIM2��������**********************/
#define			LED_TIM						TIM2
#define			LED_TIM_APBxClock_FUN		RCC_APB1PeriphClockCmd
#define			LED_TIM_CLK					RCC_APB1Periph_TIM2
#define			LED_TIM_Period				7999
#define			LED_TIM_Prescaler			2//7999��2Ϊ3kHz


//*********��ʱ����ʼ��*********//
void TIM1_PWM_Init(void);
void TIM2_PWM_Init(void);
void TIM3_Encoder_Init(void);
//*****************************//

//********����������ж�********//
void TIM3_IRQHandler(void);
//*****************************//
#endif
