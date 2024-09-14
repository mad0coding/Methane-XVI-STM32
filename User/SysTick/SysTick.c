#include "SysTick.h"

#ifdef	ComplexMode//��Ϊ����ģʽ

volatile uint32_t Systime = 0;
//ϵͳ��ʱ���жϼƴα���,�޷���32λ��1ms�ж�ʱ�ɼ�1193Сʱ,��49.71��

void SysTick_Init(void)
{
	SysTick_Config(SysLoadTime);
	NVIC_SetPriority(SysTick_IRQn, (1UL <<3) - 1UL);
}

void Delay_us(int us)//72M���Լ59638000us
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

void Delay_ms(int ms)//72M���59638ms
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

void Delay_ms_long(int ms)//72M���ms
{
	uint32_t LongCycleNum = ms * MainFreq * 1000 / SysLoadTime - 1;
	SysTick->CTRL;
	while(LongCycleNum <= 4294967290u)
	{
		if((SysTick->CTRL)&(1<<16)) LongCycleNum--;
	}
}

//void SysTick_Handler(void)//ϵͳ��ʱ���ж�
//{
//	Systime++;
//}

#else//����Ϊ����ģʽ

void Delay_us(int us)
{
	int i = 0;
	if(us<=233000)
	{
		SysTick_Config(72*us);
		while(!((SysTick->CTRL)&(1<<16)));//һֱ�ȵ�SysTick->CTRL��16λCOUNTFLAGΪ1���˳�
	}
	else
	{
		SysTick_Config(72);
		for(i=0;i<us;i++)
		{
			while(!((SysTick->CTRL)&(1<<16)));
		}
	}
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;//ϵͳ��ʱ��ȡ��ʹ��
}

void Delay_ms(int ms)
{
	int i = 0;
	SysTick_Config(72000);
	for(i = 0;i<ms;i++)
	{
		while(!((SysTick->CTRL)&(1<<16)));//һֱ�ȵ�SysTick->CTRL��16λCOUNTFLAGΪ1���˳�
	}
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;//ϵͳ��ʱ��ȡ��ʹ��
}

void SysTick_Handler(void)//ϵͳ��ʱ���ж�
{
}

#endif

void DelayS_ms(int ms)
{
	int T = 1000 * ms * MainFreq / 9;
	for(;T>0;T--){}
}
