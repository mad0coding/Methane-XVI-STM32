#include "TIMforKB.h"
#include "GPIOpinInit.h"
//2023.12.28

extern uint32_t Systime;

uint8_t TimFilterValue = 1;//编码器滤波参数

void TIM1_PWM_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	//uint16_t pulse = period * vol / 1000;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);//使能TIM1外设时钟使能
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);//使能GPIO外设时钟使能
	                                                                     	

   //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	TIM_TimeBaseStructure.TIM_Period = BUZZ_TIM_Period; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80K
	TIM_TimeBaseStructure.TIM_Prescaler = BUZZ_TIM_Prescaler; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0; //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx
	
	TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE 主输出使能	

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1预装载使能	 
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //使能TIMx在ARR上的预装载寄存器
	
	TIM_Cmd(TIM1, ENABLE);  //使能TIM1
}

void TIM2_PWM_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//时基结构体
	TIM_OCInitTypeDef  TIM_OCInitStructure;//输出比较结构体
	
	// 占空比配置
	uint16_t CCR1_Val = 0;//Red
	uint16_t CCR2_Val = 0;//Green
	uint16_t CCR3_Val = 0;//White
	uint16_t CCR4_Val = 0;//Blue
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   //重映射必须要开AFIO时钟
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
	
//	GPIOpinInit("A15",50,'o');GPIOpinInit("B3",50,'o');//写在此处会拖慢TIM初始化导致上电微闪
//	GPIOpinInit("B10",50,'o');GPIOpinInit("B11",50,'o');
	
	// 开启定时器时钟,即内部时钟CK_INT=72M
	LED_TIM_APBxClock_FUN(LED_TIM_CLK,ENABLE);

/*--------------------时基结构体初始化-------------------------*/
  
	// 自动重装载寄存器的值，累计TIM_Period+1个频率后产生一个更新或者中断
	TIM_TimeBaseStructure.TIM_Period = LED_TIM_Period;	
	// 驱动CNT计数器的时钟 = Fck_int/(psc+1)
	TIM_TimeBaseStructure.TIM_Prescaler = LED_TIM_Prescaler;	
	// 时钟分频因子，配置死区时间时需要用到
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		
	// 计数器计数模式，设置为向上计数
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		
	// 重复计数器的值，没用到不用管
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;	
	// 初始化定时器
	TIM_TimeBaseInit(LED_TIM, &TIM_TimeBaseStructure);


	/*--------------------输出比较结构体初始化-------------------*/	
	
	// 配置为PWM模式1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	// 输出使能
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	// 输出通道电平极性配置	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	// 输出比较通道 1
	TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
	TIM_OC1Init(LED_TIM, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
	
	// 输出比较通道 2
	TIM_OCInitStructure.TIM_Pulse = CCR2_Val;
	TIM_OC2Init(LED_TIM, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
	
	// 输出比较通道 3
	TIM_OCInitStructure.TIM_Pulse = CCR3_Val;
	TIM_OC3Init(LED_TIM, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
	
	// 输出比较通道 4
	TIM_OCInitStructure.TIM_Pulse = CCR4_Val;
	TIM_OC4Init(LED_TIM, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
	
	// 使能计数器
	TIM_Cmd(LED_TIM, ENABLE);
	
	// 初始化GPIO为复用开漏，写在此处能有效改善上电微闪
	GPIOpinInit("A15",2,'o');GPIOpinInit("B3",2,'o');//2MHz功耗相对低
	GPIOpinInit("B10",2,'o');GPIOpinInit("B11",2,'o');
}

/**************************************************************************
函数功能：把TIM3初始化为编码器接口模式(复用)
入口参数：无
返回  值：无
**************************************************************************/

void TIM3_Encoder_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//根据设定参数初始化GPIOB
#if ifTimEncode
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0; // 预分频器
	TIM_TimeBaseStructure.TIM_Period = ENCODER_TIM_PERIOD; //设定计数器自动重装值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//选择时钟分频：1/2/4分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;////TIM向上计数  
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_BothEdge, TIM_ICPolarity_BothEdge);//使用编码器模式3

	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = TimFilterValue & 0x0F;//0~15
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);//根据TIM_ICInitStruct中指定的参数初始化外设TIMx
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);//清除TIM的更新标志位
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); //使能或者失能指定的TIM中断

	TIM_SetCounter(TIM3,0);
	TIM_Cmd(TIM3, ENABLE);   //使能或者失能TIMx外设
#else
	EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
	// 配置外部中断线路
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

    // 配置外部中断模式为双边沿触发
    EXTI_InitStruct.EXTI_Line = EXTI_Line5;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // 配置外部中断对应的中断通道
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
#endif
}
uint16_t IrqTimValue = 0;
volatile uint32_t IrqTime = 0;
volatile uint8_t ECold = 0;//0~3分别为A↓B0,A↓B1,A↑B0,A↑B1
// 外部中断的中断服务函数
void EXTI9_5_IRQHandler(void){
//	if(Systime - IrqTime < 10) goto EXTI9_5_IRQHandler_END;
//	IrqTime = Systime;
    if(EXTI_GetITStatus(EXTI_Line5) != RESET){
		uint8_t ECA = PBin(4), ECB = PBin(5);
		uint8_t ECnew = (ECA << 1) | ECB;//更新记录状态
		
		if(ECnew == 2 && ECold == 1) IrqTimValue += 4;//逆时针
		else if(ECnew == 3 && ECold == 0) IrqTimValue -= 4;//顺时针
		else if(ECnew == 2 && ECold == 2) IrqTimValue += 4;//逆时针
		else if(ECnew == 3 && ECold == 3) IrqTimValue -= 4;//顺时针
		else if(ECnew == 1 && ECold == 1) IrqTimValue += 4;//逆时针
		else if(ECnew == 0 && ECold == 0) IrqTimValue -= 4;//顺时针
		else if(ECnew == 2 && ECold == 3) IrqTimValue += 4;//逆时针
		else if(ECnew == 3 && ECold == 2) IrqTimValue -= 4;//顺时针
		
		
//		if(ECnew == 1) IrqTimValue += 4;//逆时针
//		else if(ECnew == 3) IrqTimValue -= 4;//顺时针
		
//		if(ECnew == 2) IrqTimValue += 4;//逆时针
//		else if(ECnew == 0) IrqTimValue -= 4;//顺时针
		
		ECold = ECnew;//更新记录状态
    }
	EXTI9_5_IRQHandler_END:
	EXTI_ClearITPendingBit(EXTI_Line5);
}


//void TIM3_IRQHandler(void)
//{ 		    		  			    
//	if(TIM3->SR&0X0001)//溢出中断
//	{    				   				     	    	
//	}				   
//	TIM3->SR&=~(1<<0);//清除中断标志位 	    
//}

//	GPIO_InitTypeDef GPIO_InitStructure;

//  // 输出比较通道 GPIO 初始化
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);

//   //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //TIM_CH1
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);

//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);// 
//	//GPIO_InitTypeDef GPIO_InitStructure;
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	TIM_OCInitTypeDef  TIM_OCInitStructure;
//	
//	TIM_TimeBaseStructure.TIM_Period = 60000; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80K
//	TIM_TimeBaseStructure.TIM_Prescaler = 10; //设置用来作为TIMx时钟频率除数的预分频值  不分频
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
//	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

// 
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
//	TIM_OCInitStructure.TIM_Pulse = 10000; //设置待装入捕获比较寄存器的脉冲值
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
//	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx

//	TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE 主输出使能	

//	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1预装载使能	 
//	
//	TIM_ARRPreloadConfig(TIM1, ENABLE); //使能TIMx在ARR上的预装载寄存器
//	
//	TIM_Cmd(TIM1, ENABLE);  //使能TIM1
	
	
	
	
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//时基结构体
//	TIM_OCInitTypeDef  TIM_OCInitStructure;//输出比较结构体
//	
//	// 占空比配置
//	uint16_t CCR1_Val = 7800;
//	
//	GPIOpinInit("A8",50,'p');
//	
//	// 开启定时器时钟,即内部时钟CK_INT=72M
//	BUZZ_TIM_APBxClock_FUN(BUZZ_TIM_CLK,ENABLE);

///*--------------------时基结构体初始化-------------------------*/
//  
//	// 自动重装载寄存器的值，累计TIM_Period+1个频率后产生一个更新或者中断
//	TIM_TimeBaseStructure.TIM_Period = BUZZ_TIM_Period;	
//	// 驱动CNT计数器的时钟 = Fck_int/(psc+1)
//	TIM_TimeBaseStructure.TIM_Prescaler = BUZZ_TIM_Prescaler;	
//	// 时钟分频因子，配置死区时间时需要用到
//	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		
//	// 计数器计数模式，设置为向上计数
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		
//	// 重复计数器的值，没用到不用管
//	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;	
//	// 初始化定时器
//	TIM_TimeBaseInit(BUZZ_TIM, &TIM_TimeBaseStructure);


//	/*--------------------输出比较结构体初始化-------------------*/	
//	
//	// 配置为PWM模式1
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
//	// 输出使能
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//	// 输出通道电平极性配置	
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
//	
//	// 输出比较通道 1
//	TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
//	TIM_OC1Init(BUZZ_TIM, &TIM_OCInitStructure);
//	TIM_OC1PreloadConfig(BUZZ_TIM, TIM_OCPreload_Enable);
//	
//	// 使能计数器
//	TIM_Cmd(BUZZ_TIM, ENABLE);
//	/* TIM1 main Output Enable */
//	TIM_CtrlPWMOutputs(BUZZ_TIM, ENABLE);
//	TIM_ARRPreloadConfig(TIM1,ENABLE);  //自动重装载预装载允许

