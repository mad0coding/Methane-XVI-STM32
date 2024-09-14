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

extern __IO uint16_t ADC_ConvertedValue[];//ADCֵ

extern uint8_t key_state[];//�洢�������
extern uint8_t key_old[];//�洢��ȥ�İ������

//uint8_t CDC_TXbuf[64];//CDC���ͻ���

//******************************ϵͳ����******************************//
extern uint8_t light_mode[],light_cycle[];//���̵�ģʽ�ͺ�������
extern uint8_t light[],light_rgb[];//���̵ƺ�RGB������
extern uint8_t rgb_rk[];//�Ƿ�RGBҡ��ӳ��
extern uint8_t rgb_pilot[];//�Ƿ�RGB�ƹ�ָʾ
extern uint8_t sys_cs;//������ѡ��
//********************************************************************//
extern uint8_t key_turn[];//���̷���

uint8_t sys_mode = 0;//��ģʽ
//0:����,1:�ƿ�,2:����,3:������

uint16_t ANA_MID_SET[2] = {2048,2048};//��λ����
uint8_t KEY_FILTER_SET = 10;//�����˲�����

void LL_Init(void)
{
	//��ֹJTAG,��A15,B3,B4��Ϊ��ͨIOʹ��
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	Ana_mid_ctrl(0);//��ȡҡ����λ
	
//	TIM1_PWM_Init();//BUZZ��TIM1��ʼ��PWM
	TIM2_PWM_Init();//RGBLED��TIM2��ʼ��PWM
	TIM3_Encoder_Init();//��ť��TIM3��������ʼ��
	
	Common_GPIO_Init();//��ͨGPIO��ʼ��
	
	SysTick_Init();//ϵͳ��ʱ����ʼ��
	
	ADCx_Init();//ҡ����ADC��ʼ��

	Load_para();//��FLASH�������
	
	//USART_Config(1,115200,3,3);//����1��ʼ��
	
	GPIOpinInit("A11",2,'O');//ģ�����½���USB
	GPIOpinInit("A12",2,'O');
	PAout(11) = 0;
	PAout(12) = 0;
	Delay_ms(50);
}
extern uint32_t IrqTime;
extern uint8_t ECold;//0~3�ֱ�ΪA��B0,A��B1,A��B0,A��B1
void LL_Loop(void)
{
	Light_handle(0,0);
	Key_Get_Map();//��ȡ������ӳ��
//	Fake_IRQHandler();//αGPIO�ж�
	
	if(sys_mode == 0){//����
		sys_mode = Fill_report();//������д(���ĺ���)
	}
	else if(sys_mode == 1){//�ƿ�
		sys_mode = Light_control();
	}
	else if(sys_mode == 2){//����
		sys_mode = Serial_handle();
	}
	else if(sys_mode == 3){//������
		if(!(RCC_APB2ENR_TIM1EN & RCC->APB2ENR)) TIM1_PWM_Init();//BUZZ��TIM1��ʼ��PWM
		sys_mode = Buzz_handle();
	}

//	Delay_ms(100);
}

const uint16_t cycle_time[8] = {250,500,1000,1500,2000,2500,3500,5000};//���̵ƺ�������

void Light_handle(uint8_t id, uint16_t on_time)//�ƴ���
{
	static uint32_t oldTime = 0;//��¼ʱ��
	static uint32_t time_R = 0, time_G = 0, time_B = 0, time_LED = 0;
	static uint16_t cycle_old = 0;
	static uint8_t if_rise = 1;
	
	if(oldTime > Systime) time_R = time_G = time_B = time_LED = 0;//��ϵͳʱ������������Ƽ�ʱ
	oldTime = Systime;//��¼ʱ�����
	
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
	if(time_LED > Systime && rgb_pilot[sys_cs]) PBout(8) = 0;//LED��
	else PBout(8) = 1;//LED��
	
	if(rgb_rk[sys_cs] && time_R <= Systime && time_G <= Systime && time_B <= Systime){//������RGBҡ��ӳ����Ŀǰ����RGBָʾ
		int16_t x = (ADC_ConvertedValue[0] - ANA_MID_0);
		int16_t y = (ADC_ConvertedValue[1] - ANA_MID_1);
		if(ABS(x) > 256 || ABS(y) > 256){//����һ��������
			int16_t xy_r = x + 256;//xyתrgb
			int16_t xy_g = - x / 2 + y * 866 / 1000 + 256;
			int16_t xy_b = - x / 2 - y * 866 / 1000 + 256;
			if(xy_r > 0) TIM2->CCR1 = xy_r * RGB_CCR / 3072;
			if(xy_g > 0) TIM2->CCR2 = xy_g * RGB_CCR / 3072;
			if(xy_b > 0) TIM2->CCR4 = xy_b * RGB_CCR / 3072;
		}
	}
	
	if(light_mode[sys_cs] == 0){//���̵�����
		TIM2->CCR3 = light[sys_cs] * light[sys_cs] * light[sys_cs] * 2;//White
	}
	else{//���̵ƺ���
		uint16_t cycle_new = Systime % cycle_time[light_cycle[sys_cs]];
		if(cycle_new < cycle_old) if_rise = !if_rise;//�����л�
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

void Fake_IRQHandler(void)//αGPIO�ж�
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
	uint8_t ECnew = (ECA << 1) | ECB;//���¼�¼״̬
	
	if(ECnew == 2 && ECold == 1) IrqTimValue += 4;//��ʱ��
	else if(ECnew == 3 && ECold == 0) IrqTimValue -= 4;//˳ʱ��
	if(ECnew == 2) IrqTimValue += 4;//��ʱ��
	else if(ECnew == 0) IrqTimValue -= 4;//˳ʱ��
	
//	if(ECnew == 1) IrqTimValue += 4;//��ʱ��
//	else if(ECnew == 3) IrqTimValue -= 4;//˳ʱ��
	
	ECold = ECnew;//���¼�¼״̬
}

int8_t EC_monitor(uint8_t clear)//��ť����
{
	static int TIM_count = 0;//����������
	static int EC_count = 0;//ִ�м���
	
	if(clear){//���
		TIM_count = EC_count = 0;
		return 0;
	}
	
	TIM_count += (short)(TIM3 -> CNT);//������������ȡ
	TIM3 -> CNT = 0;//����������
	
	int8_t change = TIM_count/4 - EC_count;
	EC_count = TIM_count/4;
	return change;
}

uint8_t Light_control()//�ƿ�(���豸���ֶ������ƹ�����)
{
	static uint8_t if_rgb = 0;
	
	if(!key_state[0] && key_old[0]) return 0;//��ť�����ͷ����˳�
	if(!key_state[17] && key_old[17]){//ҡ�˰����ͷ����˳�������
		Save_para();
		return 0;
	}
	
	for(uint8_t i = 1; i <= 16; i++){//���̰���
		if(key_state[i] || !key_old[i]) continue;//���ͷ���������
		switch(i){
			case 1:rgb_rk[sys_cs] = 0;break;//����ҡ��RGBӳ��
			case 2:rgb_rk[sys_cs] = 1;break;//����ҡ��RGBӳ��
			case 3:rgb_pilot[sys_cs] = 0;break;//����RGBָʾ
			case 4:rgb_pilot[sys_cs] = 1;break;//����RGBָʾ
			case 5:light_mode[sys_cs] = 0;break;//���̵�����
			case 6:light_mode[sys_cs] = 1;break;//���̵ƺ���
			case 7:if_rgb = 0;break;//�������̵�����
			case 8:if_rgb = 1;break;//����RGB����
			case 9:light_cycle[sys_cs] = 0;break;//���̵ƺ�������250ms
			case 10:light_cycle[sys_cs] = 1;break;//���̵ƺ�������500ms
			case 11:light_cycle[sys_cs] = 2;break;//���̵ƺ�������1000ms
			case 12:light_cycle[sys_cs] = 3;break;//���̵ƺ�������1500ms
			case 13:light_cycle[sys_cs] = 4;break;//���̵ƺ�������2000ms
			case 14:light_cycle[sys_cs] = 5;break;//���̵ƺ�������2500ms
			case 15:light_cycle[sys_cs] = 6;break;//���̵ƺ�������3500ms
			case 16:light_cycle[sys_cs] = 7;break;//���̵ƺ�������5000ms
			default:break;
		}
	}
	
	if(!if_rgb) light[sys_cs] += EC_monitor(0);//�������̵�����
	else{
		light_rgb[sys_cs] += EC_monitor(0);//����RGB����
		Light_handle(1,10);//RGB����һ���Կ�����ǰ����
		Light_handle(2,10);
		Light_handle(3,10);
	}
	
	if(light[sys_cs] > 100){//����ֵ����������
		light[sys_cs] = 0;
		Light_handle(4,1);
	}
	else if(light[sys_cs] > 15){//����ֵ����������Ϊ����
		light[sys_cs] = 15;
		Light_handle(4,1);
	}
	if(light_rgb[sys_cs] > 100){//����ֵ����������
		light_rgb[sys_cs] = 0;
		Light_handle(4,1);
	}
	else if(light_rgb[sys_cs] > 15){//����ֵ����������Ϊ����
		light_rgb[sys_cs] = 15;
		Light_handle(4,1);
	}
	
	return 1;
}

uint8_t Serial_handle()//����(����������)
{
	if(!key_state[17] && key_old[17]) return 0;//ҡ�˰����ͷ����˳�
	
	return 2;
}

const uint16_t tone[12] = {61069,57762,54422,51447,48485,45714,
							43243,40816,38462,36364,34335,32389};//������Ӧ��ʱ������ֵ

uint8_t Buzz_handle()//������(4���˶ȵĵ�����)
{
	static uint8_t effective = 0;//��Ч����
	static int8_t vol = 10;//����
	uint8_t count = 0;//���°�������
	uint8_t ratio_up = 4, ratio_down = 2;//���������˶ȵı�Ƶֵ
	uint16_t tone_arr = 0;
	
	if(!key_state[17] && key_old[17]){
		TIM_Cmd(TIM1, DISABLE);//�ر�TIM1������
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, DISABLE);//�ر�TIM1ʱ��
		return 0;//ҡ�˰����ͷ����˳�
	}
	
	if(vol < 30) vol += EC_monitor(0);//��������
	else if(vol < 70) vol += EC_monitor(0) * 2;
	else vol += EC_monitor(0) * 3;
	if(vol < 0){//��С����
		vol = 0;
		Light_handle(2,10);
	}
	else if(vol > 100){//�������
		vol = 100;
		Light_handle(3,10);
	}
	
	if(key_state[5]){//�������˶�
		ratio_up = 8;//���������˶�
		ratio_down = 1;//���������˶�
	}
	for(uint8_t i = 1; i <= 16; i++){
		if(i == 5 || i == 13) continue;//�������˶ȼ�����������
		if(key_state[i] && !key_old[i]) effective = i;//�����ؿ�ʼ����
		if(key_state[i]) count++;//���°�������
	}
	if(count == 0) effective = 0;//ȫ���������ͷź������Ч����
	if(effective == 0) TIM1->CCR1 = 0;//����Ч������ֹͣ����
	else{
		if(key_state[13]){//�ڼ�(ȫ��������)
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
		else{//�׼�
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


void Common_GPIO_Init(void)//��ͨIO��ʼ��
{
	GPIOpinInit("A2",2,'U');//2MHz������Ե�
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
	PBout(8) = 1;//LED��
}

const uint8_t turnL90[16] = {	4,	8,	12,	16,
								3,	7,	11,	15,
								2,	6,	10,	14,
								1,	5,	9,	13};//��������ӳ�����
const uint8_t turnR90[16] = {	13,	9,	5,	1,
								14,	10,	6,	2,
								15,	11,	7,	3,
								16,	12,	8,	4};//��������ӳ�����

uint8_t key_read[18];//��ȡIO����
uint8_t key_quies[18];//IO��Ĭ����
uint8_t key_use[18];//ʹ��IO����
void Key_Get_Map(void)//��ȡ������ƽ
{
	static uint16_t oldKeyTime = 0;//�ϴ�ʱ�� 16λ ����ϵͳʱ�����Ӱ��
	if((uint16_t)Systime != oldKeyTime){//ͬһ����ֻ����һ��
		Key_Read_IO();//��ȡ������ƽ
		Key_Filter((uint16_t)Systime - oldKeyTime);//�����˲�
		oldKeyTime = Systime;//��¼ʱ��
	}

	for(uint8_t i = 0; i < 18; i++){//��¼��ֵ ÿ�ζ�Ҫִ�� ��֤����ֻ�����ڵ���
		if(key_old[i] != key_state[i]) key_old[i] = key_state[i];
	}
	
	//����ӳ��
	key_state[0] = key_use[0];//��ť����
	key_state[17] = key_use[17];//ҡ�˰���
	
	if(key_turn[sys_cs] == 0){//��������
		for(uint8_t i = 1; i <= 16; i++){
			key_state[i] = key_use[i];
		}
	}
	else if(key_turn[sys_cs] == 1){//����90��
		for(uint8_t i = 1; i <= 16; i++){
			key_state[i] = key_use[turnR90[i - 1]];
		}
	}
	else if(key_turn[sys_cs] == 2){//��ת180��
		for(uint8_t i = 1; i <= 16; i++){
			key_state[i] = key_use[17 - i];
		}
	}
	else if(key_turn[sys_cs] == 3){//����90��
		for(uint8_t i = 1; i <= 16; i++){
			key_state[i] = key_use[turnL90[i - 1]];
		}
	}
}

void Key_Read_IO(void)//��ȡ������ƽ
{
	key_read[13] = !PAin(2);
	key_read[14] = !PAin(3);
	key_read[2] = !PAin(4);
	key_read[6] = !PAin(5);
	key_read[10] = !PAin(6);
	key_read[15] = !PAin(7);
	
	key_read[11] = !PBin(0);
	key_read[7] = !PBin(1);
	key_read[0] = !PBin(6);//��ť����
	key_read[3] = !PBin(7);
	key_read[17] = !PBin(9);//ҡ�˰���
	key_read[16] = !PBin(12);
	key_read[12] = !PBin(13);
	key_read[8] = !PBin(14);
	key_read[4] = !PBin(15);
	
	key_read[9] = !PCin(13);
	key_read[5] = !PCin(14);
	key_read[1] = !PCin(15);
}

uint8_t Key_Filter(uint8_t dt)//�����˲�
{
	uint8_t change = 0;
	//��Ĭ����:0Ϊ����״̬,���ֱ��غ��Ϊ0xFF����һ�ε�ƽȷ��,��ȷ������Ϊ��Ĭֵ���ݼ�ֱ������״̬
	//�˲�:��Ӧǰ�̶��ӳ�1��,��Ӧ��Ĭ�趨����
	for(uint8_t i = 0; i < 18; i++){
		if(key_quies[i] == 0){//����״̬
			if(key_use[i] != key_read[i]){//���Ƶ�ƽ�仯
				key_quies[i] = 0xFF;//��¼����
			}
		}
		else if(key_quies[i] == 0xFF){//����ȷ��
			if(key_use[i] != key_read[i]){//ȷ�ϵ�ƽ�仯
				key_use[i] = !!key_read[i];//�����������仯
				key_quies[i] = KEY_FILTER_SET;//������Ĭ
				change++;//��¼������Ч������
			}
			else{//�����ֱ��ȥ��,��Ϊë��
				key_quies[i] = 0;
			}
		}
		else{//��Ĭ��
			if(dt < key_quies[i]) key_quies[i] -= dt;//��ȥʱ���
			else key_quies[i] = 0;//��ʱ��������ֱ�ӻ�����״̬
		}
	}
	return change;
}

//uint8_t CDC_printf(uint8_t* Buf, uint16_t Len)//CDC���ڴ�ӡ����
//{
//	uint16_t Len_remain = Len;//ʣ�೤��
//	if(Len <= 0) return 0;
//	while(Len_remain > 64){//����64�ֽ���ְ�����
//		CDC_Transmit_FS(Buf + (Len - Len_remain), 64);
//		Len_remain -= 64;
//		Delay_us(1000);
//	}
//	if(Len_remain > 0){//�����в���һ����
//		CDC_Transmit_FS(Buf + (Len - Len_remain), Len_remain);
//		Delay_us(500 + 2 * Len_remain);
//	}
//	return 1;
//}
