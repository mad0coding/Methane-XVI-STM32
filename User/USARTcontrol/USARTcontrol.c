//时间2021.5.14
#include "USARTcontrol.h"
//*****************************************************************************************//
static void NVIC_Configuration(u8 UsartN,u8 Ppro,u8 Spro)//配置嵌套向量中断控制器NVIC
{
  NVIC_InitTypeDef NVIC_InitStructure;//中断结构体
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 嵌套向量中断控制器组选择
	if(UsartN<=3){	NVIC_InitStructure.NVIC_IRQChannel = 36+UsartN;	}// 配置USART为中断源
	else{	NVIC_InitStructure.NVIC_IRQChannel = 48+UsartN;	}
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = Ppro;// 抢断优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = Spro;// 子优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;// 使能中断
  NVIC_Init(&NVIC_InitStructure);// 初始化配置NVIC
}
//*****************************************************************************************//
void USART_Config(u8 UsartN,int baudrate,u8 Ppro,u8 Spro)//串口外设和GPIO初始化及相关参数设定
{
	uint32_t UsartN_BASE=0;
	GPIO_InitTypeDef GPIO_InitStructure;//GPIO初始化结构体
	USART_InitTypeDef USART_InitStructure;//串口初始化结构体
	// 配置串口的工作参数:
	USART_InitStructure.USART_BaudRate = baudrate;// 配置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;// 配置帧数据字长
	USART_InitStructure.USART_StopBits = USART_StopBits_1;// 配置停止位
	USART_InitStructure.USART_Parity = USART_Parity_No ;// 配置校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// 配置硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;// 配置工作模式，收发一起
	if(UsartN==1){
		UsartN_BASE=USART1_BASE;
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA), ENABLE);// 打开串口GPIO的时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);// 打开串口外设的时钟
		// 将USART Tx的GPIO配置为推挽复用模式:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		// 将USART Rx的GPIO配置为浮空输入模式:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
	if(UsartN==2){
		UsartN_BASE=USART2_BASE;
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA), ENABLE);// 打开串口GPIO的时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);// 打开串口外设的时钟
		// 将USART Tx的GPIO配置为推挽复用模式:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		// 将USART Rx的GPIO配置为浮空输入模式:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
	if(UsartN==3){					//需中容量
		UsartN_BASE=USART3_BASE;
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOB), ENABLE);// 打开串口GPIO的时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);// 打开串口外设的时钟
		// 将USART Tx的GPIO配置为推挽复用模式:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		// 将USART Rx的GPIO配置为浮空输入模式:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	}
	if(UsartN==4){					//需大容量
		UsartN_BASE=UART4_BASE;
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOC), ENABLE);// 打开串口GPIO的时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);// 打开串口外设的时钟
		// 将USART Tx的GPIO配置为推挽复用模式:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		// 将USART Rx的GPIO配置为浮空输入模式:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
	}
	if(UsartN==5){					//需大容量
		UsartN_BASE=UART5_BASE;
		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD), ENABLE);// 打开串口GPIO的时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);// 打开串口外设的时钟
		// 将USART Tx的GPIO配置为推挽复用模式:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		// 将USART Rx的GPIO配置为浮空输入模式:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
	}
	USART_Init(((USART_TypeDef *)UsartN_BASE), &USART_InitStructure);// 完成串口的初始化配置
	NVIC_Configuration(UsartN,Ppro,Spro);// 串口中断优先级配置
	USART_ITConfig(((USART_TypeDef *)UsartN_BASE), USART_IT_RXNE, ENABLE);// 使能串口接收中断
	USART_Cmd(((USART_TypeDef *)UsartN_BASE), ENABLE);// 使能串口	
}
//*****************************************************************************************//
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)//串口发送一个字节
{
	USART_SendData(pUSARTx,ch);//发送一个字节数据到指定串口
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);//等待发送数据寄存器为空
}
//*****************************************************************************************//
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num)//串口发送8位的数组
{
  uint8_t i;
	for(i=0; i<num; i++){	Usart_SendByte(pUSARTx,array[i]);	}//连续发送一个字节数据到指定串口
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);//等待发送数据寄存器为空
}
//*****************************************************************************************//
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)//串口发送字符串
{
	unsigned int k=0;
  do{
    Usart_SendByte( pUSARTx, *(str + k) );
		k++;
  }while(*(str + k)!='\0');
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);//等待发送数据寄存器为空
}
//*****************************************************************************************//
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)//串口发送一个16位数
{
	uint8_t temp_h, temp_l;
	temp_h = (ch&0XFF00)>>8;//取出高八位
	temp_l = ch&0XFF;//取出低八位
	USART_SendData(pUSARTx,temp_h);//发送高八位
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);//等待发送数据寄存器为空
	USART_SendData(pUSARTx,temp_l);//发送低八位
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);//等待发送数据寄存器为空
}
//*****************************************************************************************//
int fputc(int ch, FILE *f)//重定向c库函数printf到串口,重定向后可使用printf函数
{
		/* 发送一个字节数据到串口 */
		USART_SendData(printf_USARTx, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(printf_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}
//*****************************************************************************************//
int fgetc(FILE *f)//重定向c库函数scanf到串口,重定向后可使用scanf、getchar等函数
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(scanf_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(scanf_USARTx);
}
//*****************************************************************************************//
void USART1_IRQHandler(void)//串口1接收中断
{
  uint8_t ucTemp;
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{
		ucTemp = USART_ReceiveData(USART1);//接收数据
		
	}
}
//*****************************************************************************************//
/*void USART2_IRQHandler(void)//串口2接收中断
{
  uint8_t ucTemp;
	if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET)
	{
		ucTemp = USART_ReceiveData(USART2);//接收数据
		
	}
}*/
//*****************************************************************************************//
//void USART3_IRQHandler(void)//串口3接收中断
//{
//  uint8_t ucTemp;
//	if(USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET)
//	{
//		ucTemp = USART_ReceiveData(USART3);//接收数据
//		
//	}
//}
//*****************************************************************************************//
/*void UART4_IRQHandler(void)//串口4接收中断
{
  uint8_t ucTemp;
	if(USART_GetITStatus(UART4,USART_IT_RXNE)!=RESET)
	{
		ucTemp = USART_ReceiveData(UART4);//接收数据
		
	}
}*/
//*****************************************************************************************//
/*void UART5_IRQHandler(void)//串口5接收中断
{
  uint8_t ucTemp;
	if(USART_GetITStatus(UART5,USART_IT_RXNE)!=RESET)
	{
		ucTemp = USART_ReceiveData(UART5);//接收数据
		
	}
}*/
//*****************************************************************************************//





