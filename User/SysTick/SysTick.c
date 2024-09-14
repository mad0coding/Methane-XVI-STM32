#include "SysTick.h"

#ifdef	ComplexMode//若为复杂模式

volatile uint32_t Systime = 0;
//系统定时器中断计次变量,无符号32位且1ms中断时可计1193小时,合49.71天

void SysTick_Init(void)
{
	SysTick_Config(SysLoadTime);
	NVIC_SetPriority(SysTick_IRQn, (1UL <<3) - 1UL);
}

void Delay_us(int us)//72M最多约59638000us
{
	int oldNum = SysTick->VAL;
	int newNum = 0;
	uint32_t CycleNum = us * MainFreq;
	while(CycleNum <= 4294000000u)
	{
		newNum = SysTick->VAL;
		if(newNum > oldNum) CycleNum -= (oldNum + SysLoadTime - newNum);
		else CycleNum -= (oldNum - newNum);
		oldNum = newNum;
	}
}

void Delay_ms(int ms)//72M最多59638ms
{
	int oldNum = SysTick->VAL;
	int newNum = 0;
	uint32_t CycleNum = ms * MainFreq * 1000;
	while(CycleNum <= 4294000000u)
	{
		newNum = SysTick->VAL;
		if(newNum > oldNum) CycleNum -= (oldNum + SysLoadTime - newNum);
		else CycleNum -= (oldNum - newNum);
		oldNum = newNum;
	}
}

void Delay_ms_long(int ms)//72M最多ms
{
	uint32_t LongCycleNum = ms * MainFreq * 1000 / SysLoadTime - 1;
	SysTick->CTRL;
	while(LongCycleNum <= 4294967290u)
	{
		if((SysTick->CTRL)&(1<<16)) LongCycleNum--;
	}
}

//void SysTick_Handler(void)//系统定时器中断
//{
//	Systime++;
//}

#else//若不为复杂模式

void Delay_us(int us)
{
	int i = 0;
	if(us<=233000)
	{
		SysTick_Config(72*us);
		while(!((SysTick->CTRL)&(1<<16)));//一直等到SysTick->CTRL第16位COUNTFLAG为1才退出
	}
	else
	{
		SysTick_Config(72);
		for(i=0;i<us;i++)
		{
			while(!((SysTick->CTRL)&(1<<16)));
		}
	}
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;//系统定时器取消使能
}

void Delay_ms(int ms)
{
	int i = 0;
	SysTick_Config(72000);
	for(i = 0;i<ms;i++)
	{
		while(!((SysTick->CTRL)&(1<<16)));//一直等到SysTick->CTRL第16位COUNTFLAG为1才退出
	}
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;//系统定时器取消使能
}

void SysTick_Handler(void)//系统定时器中断
{
}

#endif

void DelayS_ms(int ms)
{
	int T = 1000 * ms * MainFreq / 9;
	for(;T>0;T--){}
}
