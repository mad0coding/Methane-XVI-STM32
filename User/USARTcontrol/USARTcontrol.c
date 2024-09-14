//ʱ��2021.5.14
#include "USARTcontrol.h"
//*****************************************************************************************//
static void NVIC_Configuration(u8 UsartN,u8 Ppro,u8 Spro)//����Ƕ�������жϿ�����NVIC
{
  NVIC_InitTypeDef NVIC_InitStructure;//�жϽṹ��
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// Ƕ�������жϿ�������ѡ��
	if(UsartN<=3){	NVIC_InitStructure.NVIC_IRQChannel = 36+UsartN;	}// ����USARTΪ�ж�Դ
	else{	NVIC_InitStructure.NVIC_IRQChannel = 48+UsartN;	}
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = Ppro;// �������ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = Spro;// �����ȼ�
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;// ʹ���ж�
  NVIC_Init(&NVIC_InitStructure);// ��ʼ������NVIC
}
//*****************************************************************************************//
void USART_Config(u8 UsartN,int baudrate,u8 Ppro,u8 Spro)//���������GPIO��ʼ������ز����趨
{
	uint32_t UsartN_BASE=0;
	GPIO_InitTypeDef GPIO_InitStructure;//GPIO��ʼ���ṹ��
	USART_InitTypeDef USART_InitStructure;//���ڳ�ʼ���ṹ��
	// ���ô��ڵĹ�������:
	USART_InitStructure.USART_BaudRate = baudrate;// ���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;// ����֡�����ֳ�
	USART_InitStructure.USART_StopBits = USART_StopBits_1;// ����ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No ;// ����У��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// ����Ӳ��������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;// ���ù���ģʽ���շ�һ��
	if(UsartN==1){
		UsartN_BASE=USART1_BASE;
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA), ENABLE);// �򿪴���GPIO��ʱ��
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);// �򿪴��������ʱ��
		// ��USART Tx��GPIO����Ϊ���츴��ģʽ:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		// ��USART Rx��GPIO����Ϊ��������ģʽ:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
	if(UsartN==2){
		UsartN_BASE=USART2_BASE;
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA), ENABLE);// �򿪴���GPIO��ʱ��
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);// �򿪴��������ʱ��
		// ��USART Tx��GPIO����Ϊ���츴��ģʽ:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		// ��USART Rx��GPIO����Ϊ��������ģʽ:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
	if(UsartN==3){					//��������
		UsartN_BASE=USART3_BASE;
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOB), ENABLE);// �򿪴���GPIO��ʱ��
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);// �򿪴��������ʱ��
		// ��USART Tx��GPIO����Ϊ���츴��ģʽ:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		// ��USART Rx��GPIO����Ϊ��������ģʽ:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	}
	if(UsartN==4){					//�������
		UsartN_BASE=UART4_BASE;
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOC), ENABLE);// �򿪴���GPIO��ʱ��
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);// �򿪴��������ʱ��
		// ��USART Tx��GPIO����Ϊ���츴��ģʽ:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		// ��USART Rx��GPIO����Ϊ��������ģʽ:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
	}
	if(UsartN==5){					//�������
		UsartN_BASE=UART5_BASE;
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD), ENABLE);// �򿪴���GPIO��ʱ��
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);// �򿪴��������ʱ��
		// ��USART Tx��GPIO����Ϊ���츴��ģʽ:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		// ��USART Rx��GPIO����Ϊ��������ģʽ:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
	}
	USART_Init(((USART_TypeDef *)UsartN_BASE), &USART_InitStructure);// ��ɴ��ڵĳ�ʼ������
	NVIC_Configuration(UsartN,Ppro,Spro);// �����ж����ȼ�����
	USART_ITConfig(((USART_TypeDef *)UsartN_BASE), USART_IT_RXNE, ENABLE);// ʹ�ܴ��ڽ����ж�
	USART_Cmd(((USART_TypeDef *)UsartN_BASE), ENABLE);// ʹ�ܴ���	
}
//*****************************************************************************************//
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)//���ڷ���һ���ֽ�
{
	USART_SendData(pUSARTx,ch);//����һ���ֽ����ݵ�ָ������
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);//�ȴ��������ݼĴ���Ϊ��
}
//*****************************************************************************************//
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num)//���ڷ���8λ������
{
  uint8_t i;
	for(i=0; i<num; i++){	Usart_SendByte(pUSARTx,array[i]);	}//��������һ���ֽ����ݵ�ָ������
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);//�ȴ��������ݼĴ���Ϊ��
}
//*****************************************************************************************//
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)//���ڷ����ַ���
{
	unsigned int k=0;
  do{
    Usart_SendByte( pUSARTx, *(str + k) );
		k++;
  }while(*(str + k)!='\0');
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);//�ȴ��������ݼĴ���Ϊ��
}
//*****************************************************************************************//
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)//���ڷ���һ��16λ��
{
	uint8_t temp_h, temp_l;
	temp_h = (ch&0XFF00)>>8;//ȡ���߰�λ
	temp_l = ch&0XFF;//ȡ���Ͱ�λ
	USART_SendData(pUSARTx,temp_h);//���͸߰�λ
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);//�ȴ��������ݼĴ���Ϊ��
	USART_SendData(pUSARTx,temp_l);//���͵Ͱ�λ
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);//�ȴ��������ݼĴ���Ϊ��
}
//*****************************************************************************************//
int fputc(int ch, FILE *f)//�ض���c�⺯��printf������,�ض�����ʹ��printf����
{
		/* ����һ���ֽ����ݵ����� */
		USART_SendData(printf_USARTx, (uint8_t) ch);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(printf_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}
//*****************************************************************************************//
int fgetc(FILE *f)//�ض���c�⺯��scanf������,�ض�����ʹ��scanf��getchar�Ⱥ���
{
		/* �ȴ������������� */
		while (USART_GetFlagStatus(scanf_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(scanf_USARTx);
}
//*****************************************************************************************//
void USART1_IRQHandler(void)//����1�����ж�
{
  uint8_t ucTemp;
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{
		ucTemp = USART_ReceiveData(USART1);//��������
		
	}
}
//*****************************************************************************************//
/*void USART2_IRQHandler(void)//����2�����ж�
{
  uint8_t ucTemp;
	if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET)
	{
		ucTemp = USART_ReceiveData(USART2);//��������
		
	}
}*/
//*****************************************************************************************//
//void USART3_IRQHandler(void)//����3�����ж�
//{
//  uint8_t ucTemp;
//	if(USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET)
//	{
//		ucTemp = USART_ReceiveData(USART3);//��������
//		
//	}
//}
//*****************************************************************************************//
/*void UART4_IRQHandler(void)//����4�����ж�
{
  uint8_t ucTemp;
	if(USART_GetITStatus(UART4,USART_IT_RXNE)!=RESET)
	{
		ucTemp = USART_ReceiveData(UART4);//��������
		
	}
}*/
//*****************************************************************************************//
/*void UART5_IRQHandler(void)//����5�����ж�
{
  uint8_t ucTemp;
	if(USART_GetITStatus(UART5,USART_IT_RXNE)!=RESET)
	{
		ucTemp = USART_ReceiveData(UART5);//��������
		
	}
}*/
//*****************************************************************************************//





