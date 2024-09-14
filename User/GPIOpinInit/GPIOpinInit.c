//2021.5.14
#include "GPIOpinInit.h"

char getPin(char* PinName,bool choice)//����������
{
    if(choice){   return PinName[0];  }
    else
    {
        if(PinName[2]){ return (10*(PinName[1]-48)+PinName[2]-48);  }
        else{   return (PinName[1]-48); }
    }
}

void GPIOpinInit(char* PinName,char speed,char mode)//��������
{
	char i=0;
	GPIO_InitTypeDef PinX;
	uint32_t rcc0=0x00000004;
	char PIN=getPin(PinName,1);//��ĸ
	char pin=getPin(PinName,0);//����
	switch (mode)//ѡ��ģʽ
	{
		case 'A':PinX.GPIO_Mode=GPIO_Mode_AIN;break;//ģ������
		case 'F':PinX.GPIO_Mode=GPIO_Mode_IN_FLOATING;break;//��������
		case 'D':PinX.GPIO_Mode=GPIO_Mode_IPD;break;//��������
		case 'U':PinX.GPIO_Mode=GPIO_Mode_IPU;break;//��������
		case 'O':PinX.GPIO_Mode=GPIO_Mode_Out_OD;break;//��©���
		case 'P':PinX.GPIO_Mode=GPIO_Mode_Out_PP;break;//�������
		case 'o':PinX.GPIO_Mode=GPIO_Mode_AF_OD;break;//���ÿ�©���
		case 'p':PinX.GPIO_Mode=GPIO_Mode_AF_PP;break;//�����������
	}
	switch (speed)//ѡ���ٶ�
	{
		case 2:PinX.GPIO_Speed=GPIO_Speed_2MHz;break;
		case 10:PinX.GPIO_Speed=GPIO_Speed_10MHz;break;
		case 50:PinX.GPIO_Speed=GPIO_Speed_50MHz;break;
	}
	PinX.GPIO_Pin=0x0001;
	for(i=0;i<pin;i++){	PinX.GPIO_Pin*=2;	}//ѡ�����ź�
	if(pin==16){	PinX.GPIO_Pin=0xFFFF;	}//ȫѡ
	for(i=65;i<PIN;i++){	rcc0*=2;	}//ѡ��ʱ��
	RCC_APB2PeriphClockCmd(rcc0,ENABLE);//����ʱ��
	GPIO_Init((GPIO_TypeDef *)(((uint32_t)0x40000400)+PIN*0x400),&PinX);//��ʼ��
}

void Output(char* PinName,bool state)//���
{
	uint16_t outPin=0x0001;
	char PIN=getPin(PinName,1);//��ĸ
	char pin=getPin(PinName,0);//����
	for(;pin>0;pin--){	outPin*=2;	}//ѡ�����ź�
	if(pin==16){	outPin=0xFFFF;	}//ȫѡ
	if(state){		GPIO_SetBits((GPIO_TypeDef *)(((uint32_t)0x40000400)+PIN*0x400),outPin);	}
	else{	GPIO_ResetBits((GPIO_TypeDef *)(((uint32_t)0x40000400)+PIN*0x400),outPin);	}
}

void High(char* PinName){	Output(PinName,1);	}//�ø�

void Low(char* PinName){	Output(PinName,0);	}//�õ�

bool Read(char* PinName)//��ȡ
{
	uint16_t outPin=0x0001;
	char PIN=getPin(PinName,1);//��ĸ
	char pin=getPin(PinName,0);//����
	for(;pin>0;pin--){	outPin*=2;	}//ѡ�����ź�
	return GPIO_ReadInputDataBit((GPIO_TypeDef *)(((uint32_t)0x40000400)+PIN*0x400),outPin);
}






