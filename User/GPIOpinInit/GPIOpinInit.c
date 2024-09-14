//2021.5.14
#include "GPIOpinInit.h"

char getPin(char* PinName,bool choice)//引脚名处理
{
    if(choice){   return PinName[0];  }
    else
    {
        if(PinName[2]){ return (10*(PinName[1]-48)+PinName[2]-48);  }
        else{   return (PinName[1]-48); }
    }
}

void GPIOpinInit(char* PinName,char speed,char mode)//引脚配置
{
	char i=0;
	GPIO_InitTypeDef PinX;
	uint32_t rcc0=0x00000004;
	char PIN=getPin(PinName,1);//字母
	char pin=getPin(PinName,0);//数字
	switch (mode)//选择模式
	{
		case 'A':PinX.GPIO_Mode=GPIO_Mode_AIN;break;//模拟输入
		case 'F':PinX.GPIO_Mode=GPIO_Mode_IN_FLOATING;break;//浮空输入
		case 'D':PinX.GPIO_Mode=GPIO_Mode_IPD;break;//下拉输入
		case 'U':PinX.GPIO_Mode=GPIO_Mode_IPU;break;//上拉输入
		case 'O':PinX.GPIO_Mode=GPIO_Mode_Out_OD;break;//开漏输出
		case 'P':PinX.GPIO_Mode=GPIO_Mode_Out_PP;break;//推挽输出
		case 'o':PinX.GPIO_Mode=GPIO_Mode_AF_OD;break;//复用开漏输出
		case 'p':PinX.GPIO_Mode=GPIO_Mode_AF_PP;break;//复用推挽输出
	}
	switch (speed)//选择速度
	{
		case 2:PinX.GPIO_Speed=GPIO_Speed_2MHz;break;
		case 10:PinX.GPIO_Speed=GPIO_Speed_10MHz;break;
		case 50:PinX.GPIO_Speed=GPIO_Speed_50MHz;break;
	}
	PinX.GPIO_Pin=0x0001;
	for(i=0;i<pin;i++){	PinX.GPIO_Pin*=2;	}//选择引脚号
	if(pin==16){	PinX.GPIO_Pin=0xFFFF;	}//全选
	for(i=65;i<PIN;i++){	rcc0*=2;	}//选择时钟
	RCC_APB2PeriphClockCmd(rcc0,ENABLE);//开启时钟
	GPIO_Init((GPIO_TypeDef *)(((uint32_t)0x40000400)+PIN*0x400),&PinX);//初始化
}

void Output(char* PinName,bool state)//输出
{
	uint16_t outPin=0x0001;
	char PIN=getPin(PinName,1);//字母
	char pin=getPin(PinName,0);//数字
	for(;pin>0;pin--){	outPin*=2;	}//选择引脚号
	if(pin==16){	outPin=0xFFFF;	}//全选
	if(state){		GPIO_SetBits((GPIO_TypeDef *)(((uint32_t)0x40000400)+PIN*0x400),outPin);	}
	else{	GPIO_ResetBits((GPIO_TypeDef *)(((uint32_t)0x40000400)+PIN*0x400),outPin);	}
}

void High(char* PinName){	Output(PinName,1);	}//置高

void Low(char* PinName){	Output(PinName,0);	}//置低

bool Read(char* PinName)//读取
{
	uint16_t outPin=0x0001;
	char PIN=getPin(PinName,1);//字母
	char pin=getPin(PinName,0);//数字
	for(;pin>0;pin--){	outPin*=2;	}//选择引脚号
	return GPIO_ReadInputDataBit((GPIO_TypeDef *)(((uint32_t)0x40000400)+PIN*0x400),outPin);
}






