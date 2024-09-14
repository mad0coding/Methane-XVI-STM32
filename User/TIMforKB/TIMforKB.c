#include "TIMforKB.h"
#include "GPIOpinInit.h"
//2023.12.28

extern uint32_t Systime;

uint8_t TimFilterValue = 1;//�������˲�����

void TIM1_PWM_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	//uint16_t pulse = period * vol / 1000;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);//ʹ��TIM1����ʱ��ʹ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);//ʹ��GPIO����ʱ��ʹ��
	                                                                     	

   //���ø�����Ϊ�����������,���TIM1 CH1��PWM���岨��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	TIM_TimeBaseStructure.TIM_Period = BUZZ_TIM_Period; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 80K
	TIM_TimeBaseStructure.TIM_Prescaler = BUZZ_TIM_Prescaler; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_Pulse = 0; //���ô�װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx
	
	TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE �����ʹ��	

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1Ԥװ��ʹ��	 
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	
	TIM_Cmd(TIM1, ENABLE);  //ʹ��TIM1
}

void TIM2_PWM_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//ʱ���ṹ��
	TIM_OCInitTypeDef  TIM_OCInitStructure;//����ȽϽṹ��
	
	// ռ�ձ�����
	uint16_t CCR1_Val = 0;//Red
	uint16_t CCR2_Val = 0;//Green
	uint16_t CCR3_Val = 0;//White
	uint16_t CCR4_Val = 0;//Blue
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   //��ӳ�����Ҫ��AFIOʱ��
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
	
//	GPIOpinInit("A15",50,'o');GPIOpinInit("B3",50,'o');//д�ڴ˴�������TIM��ʼ�������ϵ�΢��
//	GPIOpinInit("B10",50,'o');GPIOpinInit("B11",50,'o');
	
	// ������ʱ��ʱ��,���ڲ�ʱ��CK_INT=72M
	LED_TIM_APBxClock_FUN(LED_TIM_CLK,ENABLE);

/*--------------------ʱ���ṹ���ʼ��-------------------------*/
  
	// �Զ���װ�ؼĴ�����ֵ���ۼ�TIM_Period+1��Ƶ�ʺ����һ�����»����ж�
	TIM_TimeBaseStructure.TIM_Period = LED_TIM_Period;	
	// ����CNT��������ʱ�� = Fck_int/(psc+1)
	TIM_TimeBaseStructure.TIM_Prescaler = LED_TIM_Prescaler;	
	// ʱ�ӷ�Ƶ���ӣ���������ʱ��ʱ��Ҫ�õ�
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		
	// ����������ģʽ������Ϊ���ϼ���
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		
	// �ظ���������ֵ��û�õ����ù�
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;	
	// ��ʼ����ʱ��
	TIM_TimeBaseInit(LED_TIM, &TIM_TimeBaseStructure);


	/*--------------------����ȽϽṹ���ʼ��-------------------*/	
	
	// ����ΪPWMģʽ1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	// ���ʹ��
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	// ���ͨ����ƽ��������	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	// ����Ƚ�ͨ�� 1
	TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
	TIM_OC1Init(LED_TIM, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
	
	// ����Ƚ�ͨ�� 2
	TIM_OCInitStructure.TIM_Pulse = CCR2_Val;
	TIM_OC2Init(LED_TIM, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
	
	// ����Ƚ�ͨ�� 3
	TIM_OCInitStructure.TIM_Pulse = CCR3_Val;
	TIM_OC3Init(LED_TIM, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
	
	// ����Ƚ�ͨ�� 4
	TIM_OCInitStructure.TIM_Pulse = CCR4_Val;
	TIM_OC4Init(LED_TIM, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(LED_TIM, TIM_OCPreload_Enable);
	
	// ʹ�ܼ�����
	TIM_Cmd(LED_TIM, ENABLE);
	
	// ��ʼ��GPIOΪ���ÿ�©��д�ڴ˴�����Ч�����ϵ�΢��
	GPIOpinInit("A15",2,'o');GPIOpinInit("B3",2,'o');//2MHz������Ե�
	GPIOpinInit("B10",2,'o');GPIOpinInit("B11",2,'o');
}

/**************************************************************************
�������ܣ���TIM3��ʼ��Ϊ�������ӿ�ģʽ(����)
��ڲ�������
����  ֵ����
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

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;	//�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//�����趨������ʼ��GPIOB
#if ifTimEncode
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0; // Ԥ��Ƶ��
	TIM_TimeBaseStructure.TIM_Period = ENCODER_TIM_PERIOD; //�趨�������Զ���װֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//ѡ��ʱ�ӷ�Ƶ��1/2/4��Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;////TIM���ϼ���  
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_BothEdge, TIM_ICPolarity_BothEdge);//ʹ�ñ�����ģʽ3

	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = TimFilterValue & 0x0F;//0~15
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);//����TIM_ICInitStruct��ָ���Ĳ�����ʼ������TIMx
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);//���TIM�ĸ��±�־λ
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); //ʹ�ܻ���ʧ��ָ����TIM�ж�

	TIM_SetCounter(TIM3,0);
	TIM_Cmd(TIM3, ENABLE);   //ʹ�ܻ���ʧ��TIMx����
#else
	EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
	// �����ⲿ�ж���·
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

    // �����ⲿ�ж�ģʽΪ˫���ش���
    EXTI_InitStruct.EXTI_Line = EXTI_Line5;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // �����ⲿ�ж϶�Ӧ���ж�ͨ��
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
#endif
}
uint16_t IrqTimValue = 0;
volatile uint32_t IrqTime = 0;
volatile uint8_t ECold = 0;//0~3�ֱ�ΪA��B0,A��B1,A��B0,A��B1
// �ⲿ�жϵ��жϷ�����
void EXTI9_5_IRQHandler(void){
//	if(Systime - IrqTime < 10) goto EXTI9_5_IRQHandler_END;
//	IrqTime = Systime;
    if(EXTI_GetITStatus(EXTI_Line5) != RESET){
		uint8_t ECA = PBin(4), ECB = PBin(5);
		uint8_t ECnew = (ECA << 1) | ECB;//���¼�¼״̬
		
		if(ECnew == 2 && ECold == 1) IrqTimValue += 4;//��ʱ��
		else if(ECnew == 3 && ECold == 0) IrqTimValue -= 4;//˳ʱ��
		else if(ECnew == 2 && ECold == 2) IrqTimValue += 4;//��ʱ��
		else if(ECnew == 3 && ECold == 3) IrqTimValue -= 4;//˳ʱ��
		else if(ECnew == 1 && ECold == 1) IrqTimValue += 4;//��ʱ��
		else if(ECnew == 0 && ECold == 0) IrqTimValue -= 4;//˳ʱ��
		else if(ECnew == 2 && ECold == 3) IrqTimValue += 4;//��ʱ��
		else if(ECnew == 3 && ECold == 2) IrqTimValue -= 4;//˳ʱ��
		
		
//		if(ECnew == 1) IrqTimValue += 4;//��ʱ��
//		else if(ECnew == 3) IrqTimValue -= 4;//˳ʱ��
		
//		if(ECnew == 2) IrqTimValue += 4;//��ʱ��
//		else if(ECnew == 0) IrqTimValue -= 4;//˳ʱ��
		
		ECold = ECnew;//���¼�¼״̬
    }
	EXTI9_5_IRQHandler_END:
	EXTI_ClearITPendingBit(EXTI_Line5);
}


//void TIM3_IRQHandler(void)
//{ 		    		  			    
//	if(TIM3->SR&0X0001)//����ж�
//	{    				   				     	    	
//	}				   
//	TIM3->SR&=~(1<<0);//����жϱ�־λ 	    
//}

//	GPIO_InitTypeDef GPIO_InitStructure;

//  // ����Ƚ�ͨ�� GPIO ��ʼ��
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);

//   //���ø�����Ϊ�����������,���TIM1 CH1��PWM���岨��
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //TIM_CH1
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);

//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);// 
//	//GPIO_InitTypeDef GPIO_InitStructure;
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	TIM_OCInitTypeDef  TIM_OCInitStructure;
//	
//	TIM_TimeBaseStructure.TIM_Period = 60000; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 80K
//	TIM_TimeBaseStructure.TIM_Prescaler = 10; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  ����Ƶ
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
//	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

// 
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
//	TIM_OCInitStructure.TIM_Pulse = 10000; //���ô�װ�벶��ȽϼĴ���������ֵ
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
//	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx

//	TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE �����ʹ��	

//	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1Ԥװ��ʹ��	 
//	
//	TIM_ARRPreloadConfig(TIM1, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
//	
//	TIM_Cmd(TIM1, ENABLE);  //ʹ��TIM1
	
	
	
	
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//ʱ���ṹ��
//	TIM_OCInitTypeDef  TIM_OCInitStructure;//����ȽϽṹ��
//	
//	// ռ�ձ�����
//	uint16_t CCR1_Val = 7800;
//	
//	GPIOpinInit("A8",50,'p');
//	
//	// ������ʱ��ʱ��,���ڲ�ʱ��CK_INT=72M
//	BUZZ_TIM_APBxClock_FUN(BUZZ_TIM_CLK,ENABLE);

///*--------------------ʱ���ṹ���ʼ��-------------------------*/
//  
//	// �Զ���װ�ؼĴ�����ֵ���ۼ�TIM_Period+1��Ƶ�ʺ����һ�����»����ж�
//	TIM_TimeBaseStructure.TIM_Period = BUZZ_TIM_Period;	
//	// ����CNT��������ʱ�� = Fck_int/(psc+1)
//	TIM_TimeBaseStructure.TIM_Prescaler = BUZZ_TIM_Prescaler;	
//	// ʱ�ӷ�Ƶ���ӣ���������ʱ��ʱ��Ҫ�õ�
//	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		
//	// ����������ģʽ������Ϊ���ϼ���
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		
//	// �ظ���������ֵ��û�õ����ù�
//	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;	
//	// ��ʼ����ʱ��
//	TIM_TimeBaseInit(BUZZ_TIM, &TIM_TimeBaseStructure);


//	/*--------------------����ȽϽṹ���ʼ��-------------------*/	
//	
//	// ����ΪPWMģʽ1
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
//	// ���ʹ��
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//	// ���ͨ����ƽ��������	
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
//	
//	// ����Ƚ�ͨ�� 1
//	TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
//	TIM_OC1Init(BUZZ_TIM, &TIM_OCInitStructure);
//	TIM_OC1PreloadConfig(BUZZ_TIM, TIM_OCPreload_Enable);
//	
//	// ʹ�ܼ�����
//	TIM_Cmd(BUZZ_TIM, ENABLE);
//	/* TIM1 main Output Enable */
//	TIM_CtrlPWMOutputs(BUZZ_TIM, ENABLE);
//	TIM_ARRPreloadConfig(TIM1,ENABLE);  //�Զ���װ��Ԥװ������

