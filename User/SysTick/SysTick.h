//2021.8.13
#ifndef _SYSTICK_H
#define _SYSTICK_H

#include "stm32f10x.h"
#include "core_cm3.h"

#define ComplexMode//����ģʽ,����ʹ�ÿ�ע�͵�,�򽫼��ݾɰ汾

#define SystimeType	uint32_t
#define SysLoadTime	72000//72000�ж�����1ms,ע���Ƿ���72M��Ƶ
#define MainFreq	72//��Ƶ,��λMHz
//����ģʽ����:
void SysTick_Init(void);

void Delay_ms_long(int ms);//72M��1ms�ж����4294967290ms

//��ģʽ���ú���,������ͬ��ʵ�ֲ�ͬ:
void Delay_us(int us);//72M���Լ59652000us

void Delay_ms(int ms);//72M���59652ms

//void SysTick_Handler(void);//�˺���������stm32f10x_it.c�д���,����ͻ��ѡ����һ��

#endif

