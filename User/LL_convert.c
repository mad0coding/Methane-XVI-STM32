//2024.8.6
#include "LL_convert.h"

#include "stm32f10x.h"
#include "core_cm3.h"

#include "GPIOpinInit.h"
#include "USARTcontrol.h"
#include "SysTick.h"
#include "TIMforKB.h"
#include "ADC_DMA.h"
#include "FLASH_para.h"
#include "Config_para.h"

extern volatile uint32_t Systime;

extern __IO uint16_t ADC_ConvertedValue[];//ADC值

extern uint8_t key_state[];//存储按下情况
extern uint8_t key_old[];//存储过去的按下情况

//uint8_t CDC_TXbuf[64];//CDC发送缓存

//******************************系统配置******************************//
extern uint8_t light_mode[],light_cycle[];//键盘灯模式和呼吸周期
extern uint8_t light[],light_rgb[];//键盘灯和RGB灯亮度
extern uint8_t rgb_rk[];//是否RGB摇杆映射
extern uint8_t rgb_pilot[];//是否RGB灯光指示
extern uint8_t sys_cs;//总配置选择
//********************************************************************//
extern uint8_t key_turn[];//键盘方向

uint8_t sys_mode = 0;//总模式
//0:正常,1:灯控,2:串口,3:蜂鸣器

uint16_t ANA_MID_SET[2] = {2048,2048};//中位数组
uint8_t KEY_FILTER_SET = 10;//按键滤波设置

void LL_Init(void)
{
	//禁止JTAG,把A15,B3,B4作为普通IO使用
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	Ana_mid_ctrl(0);//读取摇杆中位
	
//	TIM1_PWM_Init();//BUZZ用TIM1初始化PWM
	TIM2_PWM_Init();//RGBLED用TIM2初始化PWM
	TIM3_Encoder_Init();//旋钮用TIM3编码器初始化
	
	Common_GPIO_Init();//普通GPIO初始化
	
	SysTick_Init();//系统定时器初始化
	
	ADCx_Init();//摇杆用ADC初始化

	Load_para();//从FLASH载入参数
	
	//USART_Config(1,115200,3,3);//串口1初始化
	
	GPIOpinInit("A11",2,'O');//模拟重新接入USB
	GPIOpinInit("A12",2,'O');
	PAout(11) = 0;
	PAout(12) = 0;
	Delay_ms(50);
}
extern uint32_t IrqTime;
extern uint8_t ECold;//0~3分别为A↓B0,A↓B1,A↑B0,A↑B1
void LL_Loop(void)
{
	Light_handle(0,0);
	Key_Get_Map();//获取按键并映射
//	Fake_IRQHandler();//伪GPIO中断
	
	if(sys_mode == 0){//正常
		sys_mode = Fill_report();//报文填写(核心函数)
	}
	else if(sys_mode == 1){//灯控
		sys_mode = Light_control();
	}
	else if(sys_mode == 2){//串口
		sys_mode = Serial_handle();
	}
	else if(sys_mode == 3){//蜂鸣器
		if(!(RCC_APB2ENR_TIM1EN & RCC->APB2ENR)) TIM1_PWM_Init();//BUZZ用TIM1初始化PWM
		sys_mode = Buzz_handle();
	}

//	Delay_ms(100);
}

const uint16_t cycle_time[8] = {250,500,1000,1500,2000,2500,3500,5000};//键盘灯呼吸周期

void Light_handle(uint8_t id, uint16_t on_time)//灯处理
{
	static uint32_t oldTime = 0;//记录时间
	static uint32_t time_R = 0, time_G = 0, time_B = 0, time_LED = 0;
	static uint16_t cycle_old = 0;
	static uint8_t if_rise = 1;
	
	if(oldTime > Systime) time_R = time_G = time_B = time_LED = 0;//若系统时间重置则清除灯计时
	oldTime = Systime;//记录时间更新
	
	uint16_t RGB_CCR = light_rgb[sys_cs] * light_rgb[sys_cs] * light_rgb[sys_cs] * 2;
	
	if(id == 1) time_R = Systime + on_time;
	else if(id == 2) time_G = Systime + on_time;
	else if(id == 3) time_B = Systime + on_time;
	else if(id == 4) time_LED = Systime + on_time;
	
	if(time_R > Systime && rgb_pilot[sys_cs]) TIM2->CCR1 = RGB_CCR;//Red
	else TIM2->CCR1 = 0;
	if(time_G > Systime && rgb_pilot[sys_cs]) TIM2->CCR2 = RGB_CCR;//Green
	else TIM2->CCR2 = 0;
	if(time_B > Systime && rgb_pilot[sys_cs]) TIM2->CCR4 = RGB_CCR;//Blue
	else TIM2->CCR4 = 0;
	if(time_LED > Systime && rgb_pilot[sys_cs]) PBout(8) = 0;//LED亮
	else PBout(8) = 1;//LED灭
	
	if(rgb_rk[sys_cs] && time_R <= Systime && time_G <= Systime && time_B <= Systime){//若启用RGB摇杆映射且目前无需RGB指示
		int16_t x = (ADC_ConvertedValue[0] - ANA_MID_0);
		int16_t y = (ADC_ConvertedValue[1] - ANA_MID_1);
		if(ABS(x) > 256 || ABS(y) > 256){//保留一定的死区
			int16_t xy_r = x + 256;//xy转rgb
			int16_t xy_g = - x / 2 + y * 866 / 1000 + 256;
			int16_t xy_b = - x / 2 - y * 866 / 1000 + 256;
			if(xy_r > 0) TIM2->CCR1 = xy_r * RGB_CCR / 3072;
			if(xy_g > 0) TIM2->CCR2 = xy_g * RGB_CCR / 3072;
			if(xy_b > 0) TIM2->CCR4 = xy_b * RGB_CCR / 3072;
		}
	}
	
	if(light_mode[sys_cs] == 0){//键盘灯正常
		TIM2->CCR3 = light[sys_cs] * light[sys_cs] * light[sys_cs] * 2;//White
	}
	else{//键盘灯呼吸
		uint16_t cycle_new = Systime % cycle_time[light_cycle[sys_cs]];
		if(cycle_new < cycle_old) if_rise = !if_rise;//方向切换
		cycle_old = cycle_new;
		float light_now = light[sys_cs] * 20 * (float)cycle_new / cycle_time[light_cycle[sys_cs]];
		if(!if_rise){
			//light_now = light[sys_cs] * 20 - light_now;
			if(light_now < light[sys_cs] * 10){
				light_now = light[sys_cs] * 20 - light_now * 3 / 4;
			}
			else{
				light_now = light[sys_cs] * 25 - light_now * 5 / 4;
			}
		}
		light_now = light_now * light_now * light_now / 4000;
		if(light_now <= 1) light_now = 1;
		TIM2->CCR3 = (uint16_t)light_now;
	}
}

void Fake_IRQHandler(void)//伪GPIO中断
{
	static uint8_t oldCheck = 0;
	if((ECold & 0x02) && !PBin(4)){
		if(oldCheck == 1) oldCheck = 2;
		else oldCheck = 1;
	}
	else if(!(ECold & 0x02) && PBin(4)){
		if(oldCheck == 11) oldCheck = 12;
		else oldCheck = 11;
	}
	else oldCheck = 0;
	if(oldCheck == 2 || oldCheck == 12) Light_handle(4,100);
	if(!(oldCheck == 2 || oldCheck == 12)) return;
//	return;
//	if((ECold >> 1) == PBin(4)) return;
	if(Systime - IrqTime < 10) return;
	IrqTime = Systime;
	uint8_t ECA = PBin(4), ECB = PBin(5);
	uint8_t ECnew = (ECA << 1) | ECB;//更新记录状态
	
	if(ECnew == 2 && ECold == 1) IrqTimValue += 4;//逆时针
	else if(ECnew == 3 && ECold == 0) IrqTimValue -= 4;//顺时针
	if(ECnew == 2) IrqTimValue += 4;//逆时针
	else if(ECnew == 0) IrqTimValue -= 4;//顺时针
	
//	if(ECnew == 1) IrqTimValue += 4;//逆时针
//	else if(ECnew == 3) IrqTimValue -= 4;//顺时针
	
	ECold = ECnew;//更新记录状态
}

int8_t EC_monitor(uint8_t clear)//旋钮监听
{
	static int TIM_count = 0;//编码器计数
	static int EC_count = 0;//执行计数
	
	if(clear){//清除
		TIM_count = EC_count = 0;
		return 0;
	}
	
	TIM_count += (short)(TIM3 -> CNT);//编码器计数读取
	TIM3 -> CNT = 0;//编码器清零
	
	int8_t change = TIM_count/4 - EC_count;
	EC_count = TIM_count/4;
	return change;
}

uint8_t Light_control()//灯控(在设备上手动调整灯光设置)
{
	static uint8_t if_rgb = 0;
	
	if(!key_state[0] && key_old[0]) return 0;//旋钮按键释放沿退出
	if(!key_state[17] && key_old[17]){//摇杆按键释放沿退出并保存
		Save_para();
		return 0;
	}
	
	for(uint8_t i = 1; i <= 16; i++){//键盘按键
		if(key_state[i] || !key_old[i]) continue;//非释放沿则跳过
		switch(i){
			case 1:rgb_rk[sys_cs] = 0;break;//禁用摇杆RGB映射
			case 2:rgb_rk[sys_cs] = 1;break;//启用摇杆RGB映射
			case 3:rgb_pilot[sys_cs] = 0;break;//禁用RGB指示
			case 4:rgb_pilot[sys_cs] = 1;break;//启用RGB指示
			case 5:light_mode[sys_cs] = 0;break;//键盘灯正常
			case 6:light_mode[sys_cs] = 1;break;//键盘灯呼吸
			case 7:if_rgb = 0;break;//调整键盘灯亮度
			case 8:if_rgb = 1;break;//调整RGB亮度
			case 9:light_cycle[sys_cs] = 0;break;//键盘灯呼吸周期250ms
			case 10:light_cycle[sys_cs] = 1;break;//键盘灯呼吸周期500ms
			case 11:light_cycle[sys_cs] = 2;break;//键盘灯呼吸周期1000ms
			case 12:light_cycle[sys_cs] = 3;break;//键盘灯呼吸周期1500ms
			case 13:light_cycle[sys_cs] = 4;break;//键盘灯呼吸周期2000ms
			case 14:light_cycle[sys_cs] = 5;break;//键盘灯呼吸周期2500ms
			case 15:light_cycle[sys_cs] = 6;break;//键盘灯呼吸周期3500ms
			case 16:light_cycle[sys_cs] = 7;break;//键盘灯呼吸周期5000ms
			default:break;
		}
	}
	
	if(!if_rgb) light[sys_cs] += EC_monitor(0);//调整键盘灯亮度
	else{
		light_rgb[sys_cs] += EC_monitor(0);//调整RGB亮度
		Light_handle(1,10);//RGB短亮一下以看到当前亮度
		Light_handle(2,10);
		Light_handle(3,10);
	}
	
	if(light[sys_cs] > 100){//亮度值下溢则置零
		light[sys_cs] = 0;
		Light_handle(4,1);
	}
	else if(light[sys_cs] > 15){//亮度值超上限则置为上限
		light[sys_cs] = 15;
		Light_handle(4,1);
	}
	if(light_rgb[sys_cs] > 100){//亮度值下溢则置零
		light_rgb[sys_cs] = 0;
		Light_handle(4,1);
	}
	else if(light_rgb[sys_cs] > 15){//亮度值超上限则置为上限
		light_rgb[sys_cs] = 15;
		Light_handle(4,1);
	}
	
	return 1;
}

uint8_t Serial_handle()//串口(待开发功能)
{
	if(!key_state[17] && key_old[17]) return 0;//摇杆按键释放沿退出
	
	return 2;
}

const uint16_t tone[12] = {61069,57762,54422,51447,48485,45714,
							43243,40816,38462,36364,34335,32389};//音调对应定时器计数值

uint8_t Buzz_handle()//蜂鸣器(4个八度的电子琴)
{
	static uint8_t effective = 0;//有效按键
	static int8_t vol = 10;//音量
	uint8_t count = 0;//按下按键计数
	uint8_t ratio_up = 4, ratio_down = 2;//上下两个八度的倍频值
	uint16_t tone_arr = 0;
	
	if(!key_state[17] && key_old[17]){
		TIM_Cmd(TIM1, DISABLE);//关闭TIM1计数器
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, DISABLE);//关闭TIM1时钟
		return 0;//摇杆按键释放沿退出
	}
	
	if(vol < 30) vol += EC_monitor(0);//音量调节
	else if(vol < 70) vol += EC_monitor(0) * 2;
	else vol += EC_monitor(0) * 3;
	if(vol < 0){//最小音量
		vol = 0;
		Light_handle(2,10);
	}
	else if(vol > 100){//最大音量
		vol = 100;
		Light_handle(3,10);
	}
	
	if(key_state[5]){//上下跳八度
		ratio_up = 8;//上面上跳八度
		ratio_down = 1;//下面下跳八度
	}
	for(uint8_t i = 1; i <= 16; i++){
		if(i == 5 || i == 13) continue;//跳过跳八度键和升半音键
		if(key_state[i] && !key_old[i]) effective = i;//按下沿开始发音
		if(key_state[i]) count++;//按下按键计数
	}
	if(count == 0) effective = 0;//全部发音键释放后空置有效按键
	if(effective == 0) TIM1->CCR1 = 0;//无有效按键则停止发音
	else{
		if(key_state[13]){//黑键(全部升半音)
			switch(effective){
				case 1:tone_arr = tone[6]/ratio_up;break;//+4#
				case 2:tone_arr = tone[8]/ratio_up;break;//+5#
				case 3:tone_arr = tone[10]/ratio_up;break;//+6#
				case 4:tone_arr = tone[0]/ratio_up/2;break;//+7#(++1)
				case 6:tone_arr = tone[1]/ratio_up;break;//+1#
				case 7:tone_arr = tone[3]/ratio_up;break;//+2#
				case 8:tone_arr = tone[5]/ratio_up;break;//+3#(+4)
				case 9:tone_arr = tone[6]/ratio_down;break;//4#
				case 10:tone_arr = tone[8]/ratio_down;break;//5#
				case 11:tone_arr = tone[10]/ratio_down;break;//6#
				case 12:tone_arr = tone[0]/ratio_down/2;break;//7#(+1)
				case 14:tone_arr = tone[1]/ratio_down;break;//1#
				case 15:tone_arr = tone[3]/ratio_down;break;//2#
				case 16:tone_arr = tone[5]/ratio_down;break;//3#(4)
				default:tone_arr = 0;break;
			}
		}
		else{//白键
			switch(effective){
				case 1:tone_arr = tone[5]/ratio_up;break;//+4
				case 2:tone_arr = tone[7]/ratio_up;break;//+5
				case 3:tone_arr = tone[9]/ratio_up;break;//+6
				case 4:tone_arr = tone[11]/ratio_up;break;//+7
				case 6:tone_arr = tone[0]/ratio_up;break;//+1
				case 7:tone_arr = tone[2]/ratio_up;break;//+2
				case 8:tone_arr = tone[4]/ratio_up;break;//+3
				case 9:tone_arr = tone[5]/ratio_down;break;//4
				case 10:tone_arr = tone[7]/ratio_down;break;//5
				case 11:tone_arr = tone[9]/ratio_down;break;//6
				case 12:tone_arr = tone[11]/ratio_down;break;//7
				case 14:tone_arr = tone[0]/ratio_down;break;//1
				case 15:tone_arr = tone[2]/ratio_down;break;//2
				case 16:tone_arr = tone[4]/ratio_down;break;//3
				default:tone_arr = 0;break;
			}
		}
		if(tone_arr > 0){
			TIM1->ARR = tone_arr - 1;
			TIM1->CCR1 = tone_arr * vol / 1000;
		}
		else TIM1->CCR1 = 0;
	}
	
	return 3;
}


void Common_GPIO_Init(void)//普通IO初始化
{
	GPIOpinInit("A2",2,'U');//2MHz功耗相对低
	GPIOpinInit("A3",2,'U');
	GPIOpinInit("A4",2,'U');
	GPIOpinInit("A5",2,'U');
	GPIOpinInit("A6",2,'U');
	GPIOpinInit("A7",2,'U');
	
	GPIOpinInit("B0",2,'U');
	GPIOpinInit("B1",2,'U');
	GPIOpinInit("B6",2,'U');
	GPIOpinInit("B7",2,'U');
	GPIOpinInit("B9",2,'U');
	GPIOpinInit("B12",2,'U');
	GPIOpinInit("B13",2,'U');
	GPIOpinInit("B14",2,'U');
	GPIOpinInit("B15",2,'U');
	
	GPIOpinInit("C13",2,'U');
	GPIOpinInit("C14",2,'U');
	GPIOpinInit("C15",2,'U');
	
	GPIOpinInit("B8",2,'O');//LED
	PBout(8) = 1;//LED灭
}

const uint8_t turnL90[16] = {	4,	8,	12,	16,
								3,	7,	11,	15,
								2,	6,	10,	14,
								1,	5,	9,	13};//左旋按键映射矩阵
const uint8_t turnR90[16] = {	13,	9,	5,	1,
								14,	10,	6,	2,
								15,	11,	7,	3,
								16,	12,	8,	4};//右旋按键映射矩阵

uint8_t key_read[18];//读取IO输入
uint8_t key_quies[18];//IO静默计数
uint8_t key_use[18];//使用IO输入
void Key_Get_Map(void)//读取按键电平
{
	static uint16_t oldKeyTime = 0;//上次时间 16位 不受系统时间溢出影响
	if((uint16_t)Systime != oldKeyTime){//同一毫秒只采样一次
		Key_Read_IO();//读取按键电平
		Key_Filter((uint16_t)Systime - oldKeyTime);//按键滤波
		oldKeyTime = Systime;//记录时间
	}

	for(uint8_t i = 0; i < 18; i++){//记录旧值 每次都要执行 保证边沿只存在于单拍
		if(key_old[i] != key_state[i]) key_old[i] = key_state[i];
	}
	
	//按键映射
	key_state[0] = key_use[0];//旋钮按键
	key_state[17] = key_use[17];//摇杆按键
	
	if(key_turn[sys_cs] == 0){//正常方向
		for(uint8_t i = 1; i <= 16; i++){
			key_state[i] = key_use[i];
		}
	}
	else if(key_turn[sys_cs] == 1){//右旋90度
		for(uint8_t i = 1; i <= 16; i++){
			key_state[i] = key_use[turnR90[i - 1]];
		}
	}
	else if(key_turn[sys_cs] == 2){//旋转180度
		for(uint8_t i = 1; i <= 16; i++){
			key_state[i] = key_use[17 - i];
		}
	}
	else if(key_turn[sys_cs] == 3){//左旋90度
		for(uint8_t i = 1; i <= 16; i++){
			key_state[i] = key_use[turnL90[i - 1]];
		}
	}
}

void Key_Read_IO(void)//读取按键电平
{
	key_read[13] = !PAin(2);
	key_read[14] = !PAin(3);
	key_read[2] = !PAin(4);
	key_read[6] = !PAin(5);
	key_read[10] = !PAin(6);
	key_read[15] = !PAin(7);
	
	key_read[11] = !PBin(0);
	key_read[7] = !PBin(1);
	key_read[0] = !PBin(6);//旋钮按键
	key_read[3] = !PBin(7);
	key_read[17] = !PBin(9);//摇杆按键
	key_read[16] = !PBin(12);
	key_read[12] = !PBin(13);
	key_read[8] = !PBin(14);
	key_read[4] = !PBin(15);
	
	key_read[9] = !PCin(13);
	key_read[5] = !PCin(14);
	key_read[1] = !PCin(15);
}

uint8_t Key_Filter(uint8_t dt)//按键滤波
{
	uint8_t change = 0;
	//静默计数:0为正常状态,出现边沿后变为0xFF等下一次电平确认,若确认则置为静默值并递减直到正常状态
	//滤波:响应前固定延迟1拍,响应后静默设定拍数
	for(uint8_t i = 0; i < 18; i++){
		if(key_quies[i] == 0){//正常状态
			if(key_use[i] != key_read[i]){//疑似电平变化
				key_quies[i] = 0xFF;//记录边沿
			}
		}
		else if(key_quies[i] == 0xFF){//边沿确认
			if(key_use[i] != key_read[i]){//确认电平变化
				key_use[i] = !!key_read[i];//输出跟随输入变化
				key_quies[i] = KEY_FILTER_SET;//启动静默
				change++;//记录本拍有效边沿数
			}
			else{//输入又变回去了,视为毛刺
				key_quies[i] = 0;
			}
		}
		else{//静默中
			if(dt < key_quies[i]) key_quies[i] -= dt;//减去时间差
			else key_quies[i] = 0;//若时间差过大则直接回正常状态
		}
	}
	return change;
}

//uint8_t CDC_printf(uint8_t* Buf, uint16_t Len)//CDC串口打印函数
//{
//	uint16_t Len_remain = Len;//剩余长度
//	if(Len <= 0) return 0;
//	while(Len_remain > 64){//大于64字节需分包发送
//		CDC_Transmit_FS(Buf + (Len - Len_remain), 64);
//		Len_remain -= 64;
//		Delay_us(1000);
//	}
//	if(Len_remain > 0){//若还有不满一包的
//		CDC_Transmit_FS(Buf + (Len - Len_remain), Len_remain);
//		Delay_us(500 + 2 * Len_remain);
//	}
//	return 1;
//}
