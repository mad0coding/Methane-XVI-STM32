#ifndef __ADC_DMA_H
#define	__ADC_DMA_H

#include "stm32f10x.h"

// 注意：用作ADC采集的IO必须没有复用，否则采集电压会有影响
/********************ADC1输入通道（引脚）配置**************************/
#define    ADC_APBxClock_FUN             RCC_APB2PeriphClockCmd
#define    ADC_CLK                       RCC_APB2Periph_ADC1

#define    ADC_GPIO_APBxClock_FUN        RCC_APB2PeriphClockCmd
#define    ADC_GPIO_CLK                  RCC_APB2Periph_GPIOA
#define    ADC_PORT                      GPIOA


// 转换通道个数
#define    NOFCHANEL		2

#define    ADC_PIN0			GPIO_Pin_0
#define    ADC_CHANNEL0		ADC_Channel_0

#define    ADC_PIN1			GPIO_Pin_1
#define    ADC_CHANNEL1		ADC_Channel_1

//#define    ADC_PIN2			GPIO_Pin_2
//#define    ADC_CHANNEL2		ADC_Channel_2

//#define    ADC_PIN3			GPIO_Pin_3
//#define    ADC_CHANNEL3		ADC_Channel_3

//#define    ADC_PIN4			GPIO_Pin_4
//#define    ADC_CHANNEL4		ADC_Channel_4

//#define    ADC_PIN5			GPIO_Pin_5
//#define    ADC_CHANNEL5		ADC_Channel_5


// ADC1 对应 DMA1通道1，ADC3对应DMA2通道5，ADC2没有DMA功能
#define    ADC_x                         ADC1
#define    ADC_DMA_CHANNEL               DMA1_Channel1
#define    ADC_DMA_CLK                   RCC_AHBPeriph_DMA1


/**************************函数声明********************************/
void ADCx_Init(void);


#endif /* __ADC_H */

