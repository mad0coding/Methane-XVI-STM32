//2021.8.13
#ifndef _SYSTICK_H
#define _SYSTICK_H

#include "stm32f10x.h"
#include "core_cm3.h"

#define ComplexMode//复杂模式,若不使用可注释掉,则将兼容旧版本

#define SystimeType	uint32_t
#define SysLoadTime	72000//72000中断周期1ms,注意是否是72M主频
#define MainFreq	72//主频,单位MHz
//复杂模式函数:
void SysTick_Init(void);

void Delay_ms_long(int ms);//72M、1ms中断最多4294967290ms

//两模式共用函数,声明相同但实现不同:
void Delay_us(int us);//72M最多约59652000us

void Delay_ms(int ms);//72M最多59652ms

//void SysTick_Handler(void);//此函数可能在stm32f10x_it.c中存在,若冲突则选择保留一个

#endif

