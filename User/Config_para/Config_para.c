#include "Config_para.h"

#include <stddef.h>
#include <math.h>
#include "LL_convert.h"
#include "FLASH_para.h"

#include "GPIOpinInit.h"
#include "USARTcontrol.h"

//******************************��������******************************//
extern u16 STMFLASH_BUF[];//����FLASH��д��1K����ռ�
uint8_t* cfg_data = (uint8_t*)STMFLASH_BUF;//512��u16ת��Ϊ1K��u8
//********************************************************************//

//******************************���ķ���******************************//
extern uint8_t KeyBrd_data[];//���̱���
extern uint8_t Mouse_data[];//��걨��
extern uint8_t Point_data[];//ָ�뱨��
extern uint8_t Vol_data[];//��������
uint8_t KeyBrd_data_old[KB_len];//�ϴμ��̱���
uint8_t Mouse_data_old = 0;//�ϴ���걨��
uint8_t Vol_data_old = 0;//�ϴ���������
extern uint8_t KeyBrd_if_send;//���̱����Ƿ���
extern uint8_t Vol_if_send;//���������Ƿ���
extern uint8_t Point_if_send;//ָ�뱨���Ƿ���
extern uint8_t Mouse_if_send;//��걨���Ƿ���
//********************************************************************//

//******************************�������******************************//
extern __IO uint16_t ADC_ConvertedValue[];//ADCֵ
extern volatile uint32_t Systime;//ϵͳʱ��

uint8_t key_state[18];//�洢�������
uint8_t key_old[18];//�洢��ȥ�İ������
extern const uint8_t turnL90[];
extern const uint8_t turnR90[];

const uint16_t para_start[2] = {0,512};//������ʼλ��
const uint16_t mode3_start[2] = {0,452};//mode3������ʼλ��
uint8_t mode3_data[904];//�洢�����mode3����

uint8_t mode3_key = 0;//ģʽ3����(1-16)
uint16_t mode3_i = 0;//ģʽ3Դ�����±�(����mode3_data)
//********************************************************************//

//******************************��������******************************//
struct config_key{//�������ýṹ��
	uint8_t mode;//ģʽ
	//1:������,2:��ݼ�,3:˳���,4:����ƶ�,5:�����
	//6:�л���,7:����,8:�ƿ�,9:����,10:������
	uint8_t key;//��ֵ
	uint8_t func;//��������
	uint16_t i;//mode3��ʼ�±�(����mode3_data)(����ô洢��������)
	uint16_t x, y;//�����λ(��ϲ�����Ϊ32λ��ʱ��)
}cfg_key[2][16];
//********************************************************************//

//******************************ҡ������******************************//
struct config_rk{
	uint8_t mode;
	//1:�ٶ����,2:λ�����,3:�����İ���,4:�����İ���
	uint8_t key_mode;//ҡ�˰�������
	//1:������
	uint8_t key[5];//ҡ�˼�ֵ(����������)
	uint8_t effect;//Ч������
	uint8_t dead;//�����뾶
	int8_t dir_x, dir_y, dir_r;//����
}cfg_rk[4];

uint8_t rk_cs[2];//ҡ������ѡ��
//********************************************************************//

//******************************��ť����******************************//
struct config_ec{//��ť���ýṹ��
	uint8_t mode;//ģʽ
	//1:��������,2:����������,3:CTRL+������,4:ҡ�˲�������
	uint8_t key_mode;//��ť����ģʽ
	//1:������,2:������ͣ,3:����
	uint8_t key[3];//��ť��ֵ(������)
	int8_t dir;//����
}cfg_ec[4];

uint8_t ec_cs[2];//��ť����ѡ��
//********************************************************************//

//******************************ϵͳ����******************************//
uint8_t light_mode[2],light_cycle[2];//���̵�ģʽ�ͺ�������
uint8_t light[2],light_rgb[2];//���̵ƺ�RGB������
uint8_t rgb_rk[2];//�Ƿ�RGBҡ��ӳ��
uint8_t rgb_pilot[2];//�Ƿ�RGB�ƹ�ָʾ
uint16_t screen_w[2] = {1,1};//��Ļ���
uint16_t screen_h[2] = {1,1};//��Ļ�߶�
uint8_t key_turn[2];//���̷���
uint8_t sys_cs = 0;//������ѡ��
//********************************************************************//


void Save_para(void)//���������FLASH
{
	uint8_t para_i = 0;//����ѡ��
	uint8_t keyi = 0;//�����±�
	uint16_t i = 0;//����cfg_data
	uint16_t save3_i = 0;//����mode3_data
	struct config_key *save_key = NULL;
	
	for(para_i = 0; para_i < 2; para_i++){//�洢���ײ���
		i = para_start[para_i];
		save3_i = mode3_start[para_i];
		save_key = cfg_key[para_i];
		
		for(keyi = 0; keyi < 16; keyi++){
			cfg_data[i++] = keyi + 1;//�洢id
			cfg_data[i++] = save_key[keyi].mode;//�洢mode
			if(save_key[keyi].mode == 1){//����
				cfg_data[i++] = save_key[keyi].key;//�洢��ֵ
			}
			else if(save_key[keyi].mode == 2 || save_key[keyi].mode == 6){//��ݼ�\�л���
				cfg_data[i++] = save_key[keyi].key;//�洢��ֵ
				cfg_data[i++] = save_key[keyi].func;//�洢���ܼ�\�л���ʽ
			}
			else if(save_key[keyi].mode == 3){//������
				cfg_data[i++] = save_key[keyi].func;//�洢����
				for(save3_i = save_key[keyi].i; save3_i < save_key[keyi].i + save_key[keyi].func; save3_i++){
					cfg_data[i++] = mode3_data[save3_i];//�洢mode3����
				}
			}
			else if(save_key[keyi].mode == 4 || save_key[keyi].mode == 5){//����
				cfg_data[i++] = (save_key[keyi].x >> 8) & 0xFF;
				cfg_data[i++] = save_key[keyi].x & 0xFF;
				cfg_data[i++] = (save_key[keyi].y >> 8) & 0xFF;
				cfg_data[i++] = save_key[keyi].y & 0xFF;
			}
			else if(save_key[keyi].mode == 7){//����
				cfg_data[i++] = save_key[keyi].key;//�洢��ֵ
				cfg_data[i++] = save_key[keyi].func & 0x01;//�洢��������
				cfg_data[i++] = (save_key[keyi].i >> 8) & 0xFF;//���ڸ�8λ
				cfg_data[i++] = save_key[keyi].i & 0xFF;//���ڵ�8λ
			}
		}
		i = para_start[para_i] + 479;//������������������ʼλ��
		
		cfg_data[i++] = (screen_w[para_i] >> 8) & 0xFF;//��Ļ��
		cfg_data[i++] = screen_w[para_i] & 0xFF;
		cfg_data[i++] = (screen_h[para_i] >> 8) & 0xFF;//��Ļ��
		cfg_data[i++] = screen_h[para_i] & 0xFF;
		
		cfg_data[i++] = key_turn[para_i];//���̷���(483)
		
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].mode | (cfg_rk[para_i * 2 + 0].key_mode << 4);//ҡ��1ģʽ
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].key[0];//ҡ��1����
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].key[1];//ҡ��1�ϰ���
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].key[2];//ҡ��1�°���
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].key[3];//ҡ��1�󰴼�
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].key[4];//ҡ��1�Ұ���
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].effect;//ҡ��1�ٶ�
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].dead;//ҡ��1����
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].mode | (cfg_rk[para_i * 2 + 1].key_mode << 4);//ҡ��2ģʽ
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].key[0];//ҡ��2����
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].key[1];//ҡ��2�ϰ���
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].key[2];//ҡ��2�°���
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].key[3];//ҡ��2�󰴼�
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].key[4];//ҡ��2�Ұ���
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].effect;//ҡ��2�ٶ�
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].dead;//ҡ��2����
		
		cfg_data[i++] = cfg_ec[para_i * 2 + 0].mode | (cfg_ec[para_i * 2 + 0].key_mode << 4);//��ť1ģʽ
		cfg_data[i++] = cfg_ec[para_i * 2 + 0].key[0];//��ť1����
		cfg_data[i++] = cfg_ec[para_i * 2 + 0].key[1];//��ť1��ʱ�밴��
		cfg_data[i++] = cfg_ec[para_i * 2 + 0].key[2];//��ť1˳ʱ�밴��
		cfg_data[i++] = cfg_ec[para_i * 2 + 1].mode | (cfg_ec[para_i * 2 + 1].key_mode << 4);//��ť2ģʽ
		cfg_data[i++] = cfg_ec[para_i * 2 + 1].key[0];//��ť2����
		cfg_data[i++] = cfg_ec[para_i * 2 + 1].key[1];//��ť2��ʱ�밴��
		cfg_data[i++] = cfg_ec[para_i * 2 + 1].key[2];//��ť2˳ʱ�밴��
		
		
		uint8_t lightMode = light_mode[para_i] | (light_cycle[para_i] << 1)//���̵�ģʽ�ͺ�������
					| (ec_cs[para_i] << 6) | (rk_cs[para_i] << 7);//ҡ����ť���ȼ�
		lightMode |= rgb_rk[para_i] ? 0x20 : 0x00;//RGBҡ��ӳ��
		lightMode |= rgb_pilot[para_i] ? 0x10 : 0x00;//RGB�ƹ�ָʾ
		
		uint8_t dir_rkec = 0;//����
		dir_rkec |= cfg_rk[para_i * 2 + 0].dir_x ? 0x80 : 0x00;//����
		dir_rkec |= cfg_rk[para_i * 2 + 0].dir_y ? 0x40 : 0x00;
		dir_rkec |= cfg_rk[para_i * 2 + 0].dir_r ? 0x20 : 0x00;
		dir_rkec |= cfg_rk[para_i * 2 + 1].dir_x ? 0x10 : 0x00;
		dir_rkec |= cfg_rk[para_i * 2 + 1].dir_y ? 0x08 : 0x00;
		dir_rkec |= cfg_rk[para_i * 2 + 1].dir_r ? 0x04 : 0x00;
		dir_rkec |= cfg_ec[para_i * 2 + 0].dir ? 0x02 : 0x00;
		dir_rkec |= cfg_ec[para_i * 2 + 1].dir ? 0x01 : 0x00;
		
		cfg_data[i++] = light[para_i] | (light_rgb[para_i] << 4);//������
		cfg_data[i++] = lightMode;//������
		cfg_data[i++] = dir_rkec;//ҡ����ť�������ȼ�
		cfg_data[i] = sys_cs;//�����ȼ�
	}
	
	STMFLASH_Write_Del(PARA_FLASH_ADDR,(uint16_t*)cfg_data,512);//ɾ����д��1024�ֽ�(512����)
}

uint8_t Load_para(void)//��FLASH�������
{
	STMFLASH_Read(PARA_FLASH_ADDR,(uint16_t*)cfg_data,512);//��ȡ1024�ֽ�(512����)
	
	Load_one_para(0);//��һ�ײ���
	Load_one_para(1);//�ڶ��ײ���
	
	return 1;
}

uint8_t Load_one_para(uint8_t choose)//��FLASH�������밴������
{
	uint8_t keyi = 0;//�����±�
	uint16_t i = para_start[choose];//����cfg_data
	uint16_t load3_i = mode3_start[choose];//����mode3_data
	struct config_key *load_key = cfg_key[choose];
	//uint16_t len = 0;//���ݳ���
		
	for(keyi = 0; keyi < 16; keyi++){//����16������������
		if(cfg_data[i++] != keyi + 1) return 0;//��id�������˳�
		load_key[keyi].mode = cfg_data[i++];//����ģʽ
		if(load_key[keyi].mode == 1){//����
			load_key[keyi].key = cfg_data[i++];//���밴��
		}
		else if(load_key[keyi].mode == 2 || load_key[keyi].mode == 6){//��ݼ�\�л���
			load_key[keyi].key = cfg_data[i++];//���밴��
			load_key[keyi].func = cfg_data[i++];//���빦�ܼ�\�л���ʽ
		}
		else if(load_key[keyi].mode == 3){//������
			load_key[keyi].func = cfg_data[i++];//�ݴ泤��
			load_key[keyi].i = load3_i;//��¼������ʼ�±�
			for(/*load3_i*/; load3_i < load_key[keyi].i + load_key[keyi].func; load3_i++){
				mode3_data[load3_i] = cfg_data[i++];//�洢ÿ����
			}
			load_key[keyi+1].i = load3_i;//��¼��һ������ʼ�±�
		}
		else if(load_key[keyi].mode == 4 || load_key[keyi].mode == 5){//����
			load_key[keyi].key = 0;//���ô洢����
			load_key[keyi].func = 0;//���ô洢״̬
			load_key[keyi].x = (cfg_data[i + 0] << 8) | cfg_data[i + 1];
			load_key[keyi].y = (cfg_data[i + 2] << 8) | cfg_data[i + 3];
			i += 4;
		}
		else if(load_key[keyi].mode == 7){//����
			load_key[keyi].key = cfg_data[i++];//���밴��
			load_key[keyi].func = cfg_data[i++];//������������
			load_key[keyi].i = (cfg_data[i] << 8) | cfg_data[i + 1];//��������
			i += 2;
		}
		else if(load_key[keyi].mode > 13){
			load_key[keyi].mode = 0;//�����������ģʽ
			return 0;
		}
	}
	
	i = para_start[choose] + 479;//������������������ʼλ��
	
    screen_w[choose] = (cfg_data[i] << 8) | cfg_data[i + 1];//��Ļ��
    i += 2;
    screen_h[choose] = (cfg_data[i] << 8) | cfg_data[i + 1];//��Ļ��
    i += 2;
	
	key_turn[choose] = cfg_data[i++];//���̷���(483)
	
    cfg_rk[choose * 2 + 0].mode = (cfg_data[i] & 0x07);//ҡ��1ģʽ
    cfg_rk[choose * 2 + 0].key_mode = ((cfg_data[i++] >> 4) & 0x0F);//ҡ��1����ģʽ
    cfg_rk[choose * 2 + 0].key[0] = cfg_data[i++];//ҡ��1����
    cfg_rk[choose * 2 + 0].key[1] = cfg_data[i++];//ҡ��1�ϰ���
    cfg_rk[choose * 2 + 0].key[2] = cfg_data[i++];//ҡ��1�°���
    cfg_rk[choose * 2 + 0].key[3] = cfg_data[i++];//ҡ��1�󰴼�
    cfg_rk[choose * 2 + 0].key[4] = cfg_data[i++];//ҡ��1�Ұ���
    cfg_rk[choose * 2 + 0].effect = cfg_data[i++];//ҡ��1�ٶ�
    cfg_rk[choose * 2 + 0].dead = cfg_data[i++];//ҡ��1����
    
    cfg_rk[choose * 2 + 1].mode = (cfg_data[i] & 0x07);//ҡ��2ģʽ
    cfg_rk[choose * 2 + 1].key_mode = ((cfg_data[i++] >> 4) & 0x0F);//ҡ��2����ģʽ
    cfg_rk[choose * 2 + 1].key[0] = cfg_data[i++];//ҡ��2����
    cfg_rk[choose * 2 + 1].key[1] = cfg_data[i++];//ҡ��2�ϰ���
    cfg_rk[choose * 2 + 1].key[2] = cfg_data[i++];//ҡ��2�°���
    cfg_rk[choose * 2 + 1].key[3] = cfg_data[i++];//ҡ��2�󰴼�
    cfg_rk[choose * 2 + 1].key[4] = cfg_data[i++];//ҡ��2�Ұ���
    cfg_rk[choose * 2 + 1].effect = cfg_data[i++];//ҡ��2�ٶ�
    cfg_rk[choose * 2 + 1].dead = cfg_data[i++];//ҡ��2����
    
	cfg_ec[choose * 2 + 0].mode = (cfg_data[i] & 0x0F);//��ť1ģʽ
    cfg_ec[choose * 2 + 0].key_mode = ((cfg_data[i++] >> 4) & 0x0F);//��ť1����ģʽ
	cfg_ec[choose * 2 + 0].key[0] = cfg_data[i++];//��ť1����
    cfg_ec[choose * 2 + 0].key[1] = cfg_data[i++];//��ť1��ʱ�밴��
    cfg_ec[choose * 2 + 0].key[2] = cfg_data[i++];//��ť1˳ʱ�밴��
    cfg_ec[choose * 2 + 1].mode = (cfg_data[i] & 0x0F);//��ť2ģʽ
    cfg_ec[choose * 2 + 1].key_mode = ((cfg_data[i++] >> 4) & 0x0F);//��ť2����ģʽ
	cfg_ec[choose * 2 + 1].key[0] = cfg_data[i++];//��ť2����
    cfg_ec[choose * 2 + 1].key[1] = cfg_data[i++];//��ť2��ʱ�밴��
    cfg_ec[choose * 2 + 1].key[2] = cfg_data[i++];//��ť2˳ʱ�밴��
    
    light[choose] = (cfg_data[i] & 0x0F);//���̵�����
    light_rgb[choose] = ((cfg_data[i++] >> 4) & 0x0F);//RGB������
    
	rk_cs[choose] = ((cfg_data[i] >> 7) & 0x01);//ҡ�����ȼ�
    ec_cs[choose] = ((cfg_data[i] >> 6) & 0x01);//��ť���ȼ�
	light_cycle[choose] = ((cfg_data[i] >> 1) & 0x07);//��������
    light_mode[choose] = (cfg_data[i] & 0x01);//���̵�ģʽ
    rgb_rk[choose] = (cfg_data[i] & 0x20);//RGBҡ��ӳ��
    rgb_pilot[choose] = (cfg_data[i++] & 0x10);//RGB�ƹ�ָʾ
    
    cfg_rk[choose + 0].dir_x = ((cfg_data[i] >> 7) & 0x01);//����
    cfg_rk[choose + 0].dir_y = ((cfg_data[i] >> 6) & 0x01);
    cfg_rk[choose + 0].dir_r = ((cfg_data[i] >> 5) & 0x01);
    cfg_rk[choose + 1].dir_x = ((cfg_data[i] >> 4) & 0x01);
	cfg_rk[choose + 1].dir_y = ((cfg_data[i] >> 3) & 0x01);
    cfg_rk[choose + 1].dir_r = ((cfg_data[i] >> 2) & 0x01);
    cfg_ec[choose + 0].dir = ((cfg_data[i] >> 1) & 0x01);
    cfg_ec[choose + 1].dir = ((cfg_data[i++] >> 0) & 0x01);
	
	sys_cs = cfg_data[i];//�����ȼ�
	
	return 1;
}

uint8_t Update_para(uint8_t choose)//��USBд�����ݸ��²���
{
	if(cfg_data[choose * 512 + 511]){//1Ϊ������Ϊ������
		cfg_data[511] = cfg_data[1023] = 1 - choose;//��Ϊ������λ��
	}
	else{//0Ϊ����������
		cfg_data[511] = cfg_data[1023] = choose;//��Ϊ������λ��
	}
	if(!Load_one_para(choose)){}// return 0;//�������ݼ�������
	STMFLASH_Write_Del(PARA_FLASH_ADDR,(uint16_t*)cfg_data,512);//ɾ����д��1024�ֽ�(512����)
	Systime = 0;//����ϵͳʱ��
	return 1;
}

uint8_t Ana_mid_ctrl(uint8_t ifSave)//��ȡҡ����λ��
{
	if(ifSave){//�洢
		uint16_t tmp[3];//��ʱ����
		tmp[0] = STMFLASH_BUF[0]; tmp[1] = STMFLASH_BUF[1]; tmp[2] = STMFLASH_BUF[2];//�ݴ�
		STMFLASH_BUF[0] = ANA_MID_0;//������λ
		STMFLASH_BUF[1] = ANA_MID_1;//������λ
		STMFLASH_BUF[2] = (TimFilterValue << 8) | KEY_FILTER_SET;//����������˲������Ͱ����˲�����
		STMFLASH_Write_Del(PARA_FLASH_ADDR - 1024,(uint16_t*)cfg_data,512);//ɾ����д��1024�ֽ�(512����)
		STMFLASH_BUF[0] = tmp[0]; STMFLASH_BUF[1] = tmp[1]; STMFLASH_BUF[2] = tmp[2];//�ָ�
	}
	else{//��ȡ
		ANA_MID_0 = STMFLASH_ReadHalfWord(PARA_FLASH_ADDR - 1024 + 4);//����
		TimFilterValue = ANA_MID_0 >> 8;//�������˲�����
		KEY_FILTER_SET = ANA_MID_0 & 0xFF;//�����˲�����
		if(KEY_FILTER_SET < 1 || KEY_FILTER_SET > 250) KEY_FILTER_SET = 10;//�����ݲ�����ʹ��Ĭ��ֵ
		ANA_MID_0 = STMFLASH_ReadHalfWord(PARA_FLASH_ADDR - 1024);
		ANA_MID_1 = STMFLASH_ReadHalfWord(PARA_FLASH_ADDR - 1024 + 2);
		if(ANA_MID_0 > 4095 || ANA_MID_1 > 4095) ANA_MID_0 = ANA_MID_1 = 2048;//���ж�Ϊ��δ�洢��λ��ʹ��2048
		ANA_MID_0 &= 0x0FFF;
		ANA_MID_1 &= 0x0FFF;
	}
	return 0;
}

uint8_t Fill_report(void)//������д
{
	static uint32_t oldTime = 0;//��¼ʱ��
	static uint8_t switch_i = 0xFF, switch_count = 0;//�л���ѡ��ͼ���
	static uint8_t switch_key = 0, switch_func = 0;//�л�������
	static uint8_t mode3_pulse = 0;//ģʽ3���
	uint8_t mode1_num = 0, mode2_num = 0, mode7_num = 0;
	uint8_t i = 0;//������
	struct config_key *fill_key = cfg_key[sys_cs];
	
	//***********************************�����ļ����ͱ�־��ʼ��***********************************//
	KeyBrd_if_send = Mouse_if_send = Point_if_send = Vol_if_send = 0;//���ͱ�־����
	for(i = 1; i < KB_len; i++){
		KeyBrd_data_old[i] = KeyBrd_data[i];//��¼��һ�α���
		KeyBrd_data[i] = 0;//������м�
	}
	Mouse_data_old = Mouse_data[1];//��¼��һ�α���
	for(i = 1; i < 5; i++){
		Mouse_data[i] = 0;//�����걨��
	}
	Point_data[1] = 0x10;
	for(i = 3; i < 7; i++){
		Point_data[i] = 0xFF;//���ָ�뱨��
	}
	Vol_data_old = Vol_data[1];//��¼��һ�α���
	Vol_data[1] = 0;//�����������
	//********************************************************************************************//
	
	//****************************************���̰�������****************************************//
	if(mode3_key){//������mode3δ���״̬
		if(!mode3_pulse) mode3_handle();
		mode3_pulse = !mode3_pulse;
	}
	else{//����״̬
		for(i = 0; i < 16; i++){//ͳ�ư��µĸ�ģʽ����
			if(key_state[i+1]){
				if(fill_key[i].mode == 1 && i != switch_i) mode1_num++;
				else if(fill_key[i].mode == 2 && i != switch_i) mode2_num++;
				else if(fill_key[i].mode == 7 && i != switch_i) mode7_num++;
			}
		}
		for(i = 0; i < 16; i++){//����16������
			if(i == switch_i){//����������Ч����ʱ�л���
				if(!key_state[i+1] && !key_old[i+1]){//�ͷ���֮��һ��
					switch_count++;//�����˲�
					if(switch_count > 100){
						switch_count = 0;//��������
						key_insert(i + 3,switch_key);//�����ֵ
						cs_change(switch_func);//�л�����
						switch_i = 0xFF;//��λ
					}
				}
				continue;//������ʱ�л���
			}
			if(key_state[i+1]){//������
				if(fill_key[i].mode == 1){//ģʽ1:����
					key_insert(i + 3,fill_key[i].key);//�����ֵ
				}
				else if(fill_key[i].mode == 2){//ģʽ2:��ݼ�
					key_insert(i + 3,fill_key[i].key);//�����ֵ
					KeyBrd_data[1] |= fill_key[i].func;//���빦�ܼ�
				}
				else if(fill_key[i].mode == 3){//ģʽ3:˳���
					if(mode1_num == 0 && mode2_num == 0 && mode7_num == 0 && !key_old[i+1]){//������1,2,7ģʽ�����Ұ�����
						mode3_key = i + 1;//��¼mode3����
						mode3_i = fill_key[i].i;//��ȡ��ʼ�±�
						mode3_handle();
						mode3_pulse = 1;//������
					}
				}
				else if(fill_key[i].mode == 4 || fill_key[i].mode == 5){//ģʽ4:�����λ,ģʽ5:�����
					if(!key_old[i+1]) fill_key[i].func = 2;//������̬
					
					uint16_t x = fill_key[i].x * 32768 / screen_w[sys_cs];
					uint16_t y = fill_key[i].y * 32768 / screen_h[sys_cs];
					Point_data[3] = x & 0xFF;
					Point_data[4] = (x >> 8) & 0xFF;
					Point_data[5] = y & 0xFF;
					Point_data[6] = (y >> 8) & 0xFF;
					if(fill_key[i].mode == 5){
						Point_data[1] |= 0x01;
					}
					
					Point_data[2] = i;
				}
				else if(fill_key[i].mode == 6){//ģʽ6:�л���
					if((fill_key[i].func & 0x08) && switch_i == 0xFF && !key_old[i+1]){//��Ϊ��ʱ�л���Ϊ�����Ұ�����
						uint8_t turn_old = key_turn[sys_cs];//�ɼ��̷���
						cs_change(fill_key[i].func);//�л�
						uint8_t turn_dif = (key_turn[sys_cs] + 4 - turn_old) % 4;//��Ծɼ��̷���
						if(turn_dif == 0) switch_i = i;
						else if(turn_dif == 1) switch_i = turnL90[i] - 1;
						else if(turn_dif == 2) switch_i = 16 - i;
						else if(turn_dif == 3) switch_i = turnR90[i] - 1;
						switch_key = fill_key[i].key;//�����ֵ
						switch_func = fill_key[i].func;//�����л���ʽ
						break;//��������ѭ��
					}
				}
				else if(fill_key[i].mode == 7){//ģʽ7:��������
					if(!key_old[i+1] && !(fill_key[i].func & 0x01)){//��Ϊ�������ҷ��Զ�����
						fill_key[i].func |= 0x02;//��������
						fill_key[i].x = fill_key[i].y = 0;//��ʱ������
					}
				}
			}
			else if(key_old[i+1]){//����̧��(�ͷ���)
				if(fill_key[i].mode == 5){//ģʽ5:�����
					if(key_old[i+1]){//�ͷ���
						fill_key[i].func = 1;//��̧��̬
					}
				}
				else if(fill_key[i].mode == 6){//ģʽ6:�л���
					if(!(fill_key[i].func & 0x08)){//��Ϊ����ʱ�л�
						cs_change(fill_key[i].func);//�л�
					}
				}
				else if(fill_key[i].mode == 7){//ģʽ7:��������
					if((fill_key[i].func & 0x01) && !(fill_key[i].func & 0x02)){//�Զ������ҵ�ǰδ������
						fill_key[i].func |= 0x02;//��������
						fill_key[i].x = fill_key[i].y = 0;//��ʱ������
					}
					else fill_key[i].func &= ~0x02;//ֹͣ����
				}
				else if(fill_key[i].mode == 8){//ģʽ8:�ƿ�
					return 1;
				}
				else if(fill_key[i].mode == 9){//ģʽ9:����
					USART_Config(1,115200,3,3);//����1��ʼ��
					return 2;
				}
				else if(fill_key[i].mode == 10){//ģʽ10:������
					return 3;
				}
			}
		}//������16����������Ҫ����
		
		for(i = 0; i < 16; i++){//����16�������Ĵ���
			if(fill_key[i].mode == 4 || fill_key[i].mode == 5){//ģʽ4:�����λ,ģʽ5:�����
				if(fill_key[i].func == 1 || fill_key[i].func == 2){
					uint16_t x = fill_key[i].x * 32768 / screen_w[sys_cs];
					uint16_t y = fill_key[i].y * 32768 / screen_h[sys_cs];
					Point_data[3] = x & 0xFF;
					Point_data[4] = (x >> 8) & 0xFF;
					Point_data[5] = y & 0xFF;
					Point_data[6] = (y >> 8) & 0xFF;
					
					Point_data[2] = i;
					
					if(fill_key[i].func == 2){
						if(fill_key[i].mode == 5) Point_data[1] |= 0x01;
						else Point_data[1] &= ~0x01;
						fill_key[i].func = 3;
					}
					else{
						Point_data[1] = 0x00;
						fill_key[i].func = 0;
					}
					break;
				}
			}
		}//������16�������Ĵ���
		
		int8_t auto_num = 0;//����ִ���Զ�����İ�������
		for(i = 0; i < 16; i++){//����16������������
			if(fill_key[i].mode == 7){//ģʽ7:��������
				if(oldTime > Systime){//��ϵͳʱ������
					fill_key[i].x = fill_key[i].y = 0;//����趨ʱ��
					fill_key[i].func &= ~0x04;//��λ�����־
					fill_key[i].func &= ~0x02;//ֹͣ����
				}
				else if(fill_key[i].func & 0x02){//����������
					if(fill_key[i].i == 0){//������Ϊ0�򳤰�
						key_insert(i + 3,fill_key[i].key);//�����ֵ
					}
					else if(fill_key[i].func & 0x04){//���ϴ��ѵ��
						fill_key[i].func &= ~0x04;//��λ�����־
					}
					else{//���ϴ�δ���
						uint32_t setTime = (fill_key[i].x << 16) | fill_key[i].y;//�趨ʱ��
						if(setTime <= Systime){//����ʱ�ѵ�
							key_insert(i + 3,fill_key[i].key);//�����ֵ
							setTime = Systime + fill_key[i].i;//�����趨ʱ��
							fill_key[i].x = (setTime >> 16) & 0xFFFF;
							fill_key[i].y = setTime & 0xFFFF;
							fill_key[i].func |= 0x04;//��λ�����־
						}
					}
					if(fill_key[i].func & 0x01) auto_num++;//�Զ����㰴������
				}
			}
		}//������16������������
		oldTime = Systime;//��¼ʱ�����
		if(auto_num-- > 0) Light_handle(3,0xFF);//B
		//else Light_handle(3,0);
		if(auto_num-- > 0) Light_handle(1,0xFF);//R
		//else Light_handle(1,0);
		if(auto_num-- > 0) Light_handle(2,0xFF);//G
		//else Light_handle(2,0);
	}
	//********************************************************************************************//
	
	//***********************************ҡ����ť����***********************************//
	RK_handle(0);//ҡ�˴���
	EC_handle(0);//��ť����
	if(!mode3_key) RK_EC_key_handle();//ҡ����ť��������
	//**********************************************************************************//
	
	//***********************************�жϸ������Ƿ�Ҫ����***********************************//
	for(i = 1; i < KB_len; i++){
		if(KeyBrd_data_old[i] != KeyBrd_data[i]){//���̱�������һ�β�ͬ����
			KeyBrd_if_send = 1;
			break;
		}
	}
	if(Mouse_data[1] != Mouse_data_old) Mouse_if_send = 1;//��갴�����ϴβ�ͬ����
	else{
		for(i = 2; i < 5; i++){
			if(Mouse_data[i] != 0){//�������ƶ����������
				Mouse_if_send = 1;
				break;
			}
		}
	}
	for(i = 3; i < 7; i++){
		if(Point_data[i] != 0xFF){//����������Ч��������
			Point_if_send = 1;
			break;
		}
	}
	if(Vol_data[1] != Vol_data_old) Vol_if_send = 1;//ý�屨�����ϴβ�ͬ����
	//******************************************************************************************//
	
	if(switch_i != 0xFF 
		&& (KeyBrd_if_send || Mouse_if_send || Point_if_send || Vol_if_send)){
		switch_key = 0;//����ʱ�л��������ڼ�����������������л��������ֵ
	}
	
	return 0;
}

void cs_change(uint8_t change)//�л�
{
	if(change & 0x01){//����
		sys_cs = 1 - sys_cs;
		Light_handle(sys_cs,200);//Red
	}
	if(change & 0x02){//ҡ��
		rk_cs[sys_cs] = 1 - rk_cs[sys_cs];
		Light_handle(rk_cs[sys_cs] ? 3 : 0,200);//Blue
	}
	if(change & 0x04){//��ť
		ec_cs[sys_cs] = 1 - ec_cs[sys_cs];
		Light_handle(ec_cs[sys_cs] ? 2 : 0,200);//Green
	}
	Light_handle(4,1);
	RK_handle(1);
	EC_handle(1);
}

void key_insert(uint8_t r_i, uint8_t key_v)//��������
{
	if(key_v == 240) Mouse_data[4] += 1;//��������
	else if(key_v == 241) Mouse_data[4] += -1;//��������
	else if(key_v == 242) Vol_data[1] |= 0x01;//������
	else if(key_v == 243) Vol_data[1] |= 0x02;//������
	else if(key_v == 244) Vol_data[1] |= 0x04;//����
	else if(key_v == 245) Vol_data[1] |= 0x08;//������ͣ
	else if(key_v == 237) Vol_data[1] |= 0x10;//��һ��
	else if(key_v == 238) Vol_data[1] |= 0x20;//��һ��
	else if(key_v == 246) Mouse_data[1] |= 0x01;//������
	else if(key_v == 247) Mouse_data[1] |= 0x04;//����м�
	else if(key_v == 248) Mouse_data[1] |= 0x02;//����Ҽ�
	else if(key_v == 249) KeyBrd_data[1] |= 0x01;//ctrl
	else if(key_v == 250) KeyBrd_data[1] |= 0x02;//shift
	else if(key_v == 251) KeyBrd_data[1] |= 0x04;//alt
	else if(key_v == 252) KeyBrd_data[1] |= 0x08;//win
	else KeyBrd_data[r_i] = key_v;//�����ֵ
}

void mode3_handle(void)//mode3����(�����鴦��)
{
	static uint32_t setTime = 0, oldTime = 0;//�趨ʱ�估��¼ʱ��
	uint8_t report_i = 3;//����д��λ��
	
	if(oldTime > Systime) setTime = 0;//��ϵͳʱ����������ֹ��ʱ
	oldTime = Systime;//��¼ʱ�����
	if(setTime > Systime) return;//��ʱδ�������˳��ȴ�
	
	uint16_t end_i = 0;//����λ��
	if(mode3_key) end_i = cfg_key[sys_cs][mode3_key - 1].i + cfg_key[sys_cs][mode3_key - 1].func;
	
	while(report_i < KB_len){//������δ����
		if(mode3_data[mode3_i] == kv_shortcut){//���п�ݼ�
			if(report_i > 3 || Mouse_data[1] != 0) break;//�����α��������������˳�����һ��
			key_insert(report_i,mode3_data[++mode3_i]);//�����ֵ
			KeyBrd_data[1] = mode3_data[++mode3_i];//���빦�ܼ�
			if(++mode3_i == end_i) mode3_key = 0;//����������
			break;//��ռ���α���
		}
		else if(mode3_data[mode3_i] == kv_point){//���й����λ
			if(report_i > 3 || Mouse_data[1] != 0) break;//�����α��������������˳�����һ��
			uint16_t x = (mode3_data[mode3_i + 1] << 8) | mode3_data[mode3_i + 2];
			uint16_t y = (mode3_data[mode3_i + 3] << 8) | mode3_data[mode3_i + 4];
			x = x * 32768 / screen_w[sys_cs];
			y = y * 32768 / screen_h[sys_cs];
			Point_data[3] = x & 0xFF;
			Point_data[4] = (x >> 8) & 0xFF;
			Point_data[5] = y & 0xFF;
			Point_data[6] = (y >> 8) & 0xFF;
			if(1){
				Point_data[1] |= 0x01;
			}
			mode3_i += 4;
			if(++mode3_i == end_i) mode3_key = 0;//����������
			break;//��ռ���α���
		}
		else if(mode3_data[mode3_i] == kv_delay){//������ʱ
			uint16_t delayTime = (mode3_data[mode3_i + 1] << 8) | mode3_data[mode3_i + 2];
			setTime = Systime + delayTime;
			mode3_i += 2;
			if(++mode3_i == end_i) mode3_key = 0;//����������
			break;//��ռ���α���
		}
		else if(mode3_data[mode3_i] == kv_shift){//����shift
			uint8_t check_i = 3;
			for(check_i = 3; check_i < report_i; check_i++){//����Ƿ�������ͬ��ֵ
				if(KeyBrd_data[check_i] == mode3_data[mode3_i]){
					check_i = 0;
					break;
				}
			}
			if(!check_i) break;//��������ͬ��ֵ���˳�����һ��
			if(Mouse_data[1] != 0) break;//����걨�������������˳�����һ��
			if(KeyBrd_data[1]){//�������Ѻ�shift
				key_insert(report_i++,mode3_data[++mode3_i]);//�����ֵ
			}
			else if(report_i == 3){//������ʼ
				key_insert(report_i++,mode3_data[++mode3_i]);//�����ֵ
				KeyBrd_data[1] |= 0x02;//����shift
			}
			else break;//��ǰ���ų�shift���޷�����
		}
		else{//����shift
			uint8_t check_i = 3;
			for(check_i = 3; check_i < report_i; check_i++){//����Ƿ�������ͬ��ֵ
				if(KeyBrd_data[check_i] == mode3_data[mode3_i]){
					check_i = 0;
					break;
				}
			}
			if(!check_i) break;//��������ͬ��ֵ���˳�����һ��
			if(Mouse_data[1] != 0) break;//����걨�������������˳�����һ��
			if(!KeyBrd_data[1]){//������δ��shift
				key_insert(report_i++,mode3_data[mode3_i]);//�����ֵ
			}
			else if(report_i == 3){//������ʼ
				key_insert(report_i++,mode3_data[mode3_i]);//�����ֵ
			}
			else break;//��ǰ������shift���޷�����
		}
		mode3_i++;
		if(mode3_i == end_i){//����������
			mode3_key = 0;
			break;
		}
	}
}

void RK_EC_key_handle(void)//ҡ����ť��������
{
	uint8_t rk_ec_cs = sys_cs * 2 + rk_cs[sys_cs];//����ѡ��
	uint8_t cfg_rk_ec = cfg_rk[rk_ec_cs].key_mode;//����ģʽ
	uint8_t key_rk_ec = cfg_rk[rk_ec_cs].key[0];//����ֵ
	uint8_t keyState = key_state[17];//����״̬
	for(uint8_t rk_ec = 0; rk_ec < 2; rk_ec++){
		if(rk_ec == 1){
			rk_ec_cs = sys_cs * 2 + ec_cs[sys_cs];//����ѡ��
			cfg_rk_ec = cfg_ec[rk_ec_cs].key_mode;//����ģʽ
			key_rk_ec = cfg_ec[rk_ec_cs].key[0];//����ֵ
			keyState = key_state[0];//����״̬
		}
		if(!keyState) continue;
		switch(cfg_rk_ec){
			case 1:{//������
				for(uint8_t i = 3; i < KB_len; i++){
					if(!KeyBrd_data[i]){
						key_insert(i,key_rk_ec);//�����ֵ
						break;
					}
				}
				break;
			}
			default:break;
		}
	}
	
}

//1:�ٶ����,2:λ�����,3:�����İ���,4:�����İ���
//1:������,2:������ͣ,3:����

void RK_handle(uint8_t clear)//ҡ�˴���
{
	static int16_t RK_pulse = 0;//�����־
	static int16_t x_pic = 0, y_pic = 0;
	
	if(clear){//���
		RK_pulse = x_pic = y_pic = 0;
		return;
	}
	
	uint8_t key_rk_cs = sys_cs * 2 + rk_cs[sys_cs];//����ѡ��
	int16_t dx = 0, dy = 0;
	int16_t x = (cfg_rk[key_rk_cs].dir_x*2 - 1);
	int16_t y = (cfg_rk[key_rk_cs].dir_y*2 - 1);
	
	if(cfg_rk[key_rk_cs].dir_r){//��ת90��
		x *= (ADC_ConvertedValue[0] - ANA_MID_0);
		y *= (ADC_ConvertedValue[1] - ANA_MID_1);
	}
	else{
		x *= (ANA_MID_1 - ADC_ConvertedValue[1]);//����Ϊ��
		y *= (ADC_ConvertedValue[0] - ANA_MID_0);//����Ϊ��
	}
	
	uint16_t equal_r = MAX(ABS(x), ABS(y));//��Ч�뾶
	
	if(equal_r <= cfg_rk[key_rk_cs].dead * 21) return;//���������˳�
	
	
	switch(cfg_rk[key_rk_cs].mode){
		case 1:{//�ٶ����
			dx = (int16_t)(x * cfg_rk[key_rk_cs].effect / 2000);
			dy = (int16_t)(y * cfg_rk[key_rk_cs].effect/ 2000);
			dx = dx < 127 ? dx : 127;
			dy = dy < 127 ? dy : 127;
			dx = dx > -128 ? dx : -128;
			dy = dy > -128 ? dy : -128;
			Mouse_data[2] = (int8_t)dx;
			Mouse_data[3] = -(int8_t)dy;
			break;
		}
		case 2:{//λ�����
			dx = (int16_t)(x * cfg_rk[key_rk_cs].effect / 1000) - x_pic;
			dy = (int16_t)(y * cfg_rk[key_rk_cs].effect / 1000) - y_pic;
			dx = dx < 127 ? dx : 127;
			dy = dy < 127 ? dy : 127;
			dx = dx > -128 ? dx : -128;
			dy = dy > -128 ? dy : -128;
			x_pic += dx;
			y_pic += dy;
			Mouse_data[2] = (int8_t)dx;
			Mouse_data[3] = -(int8_t)dy;
			break;
		}
		case 3:{//�����İ���
			if(RK_pulse){//���ϴ��ѷ����򱾴μ��
				RK_pulse--;//��־�ݼ�
				break;
			}
			RK_pulse = 2000 - equal_r * (100 - cfg_rk[key_rk_cs].effect) / 100;
			RK_pulse = RK_pulse >= 1 ? RK_pulse : 1;
			RK_pulse *= (cfg_rk[key_rk_cs].effect + 19) / 20;//�����ѷ������´μ��
			if(y > x){
				if(y > -x) key_insert(19,cfg_rk[key_rk_cs].key[1]);//�����ϼ�ֵ
				else key_insert(19,cfg_rk[key_rk_cs].key[3]);//�������ֵ
			}
			else{
				if(y > -x) key_insert(19,cfg_rk[key_rk_cs].key[4]);//�����Ҽ�ֵ
				else key_insert(19,cfg_rk[key_rk_cs].key[2]);//�����¼�ֵ
			}
			break;
		}
		case 4:{//�����İ���
			if(RK_pulse){//���ϴ��ѷ����򱾴μ��
				RK_pulse--;//��־�ݼ�
				break;
			}
			RK_pulse = 2000 - equal_r * (100 - cfg_rk[key_rk_cs].effect) / 100;
			RK_pulse = RK_pulse >= 1 ? RK_pulse : 1;
			RK_pulse *= (cfg_rk[key_rk_cs].effect + 19) / 20;//�����ѷ������´μ��
			if(y*12 > x*5 && y*12 > -x*5){//��
				key_insert(19,cfg_rk[key_rk_cs].key[1]);//�����ֵ
			}
			else if(y*12 < x*5 && y*12 < -x*5){//��
				key_insert(19,cfg_rk[key_rk_cs].key[2]);//�����ֵ
			}
			if(y*5 > x*12 && y*5 < -x*12){//��
				key_insert(20,cfg_rk[key_rk_cs].key[3]);//�����ֵ
			}
			else if(y*5 < x*12 && y*5 > -x*12){//��
				key_insert(20,cfg_rk[key_rk_cs].key[4]);//�����ֵ
			}
			break;
		}
		default:break;
	}
}

//1:��������,2:����������,3:CTRL+������,4:ҡ�˲�������
//1:������
void EC_handle(uint8_t clear)//��ť����
{
	static uint32_t oldTime = 0;//��¼ʱ��
	static int TIM_old = 0;//�������ɼ���
	static int TIM_count = 0;//����������
	static int EC_count = 0;//ִ�м���
	static uint8_t EC_flag = 0;//ִ�б�־
	static uint8_t EC_pulse = 0;//�����־
	
	if(clear){//���
		TIM_count = EC_count = 0;
		EC_flag = EC_pulse = 0;
		return;
	}
	
	uint8_t key_ec_cs = sys_cs * 2 + ec_cs[sys_cs];//����ѡ��
	
#if ifTimEncode
	TIM_count -= (cfg_ec[key_ec_cs].dir * 2 - 1) * (short)(TIM3 -> CNT);//������������ȡ
	TIM3 -> CNT = 0;//����������
#else
	TIM_count -= (cfg_ec[key_ec_cs].dir * 2 - 1) * (short)(IrqTimValue);//������������ȡ
	IrqTimValue = 0;//����������
#endif
	
	if(TIM_count/4 > EC_count) EC_flag = 1;
	else if(TIM_count/4 < EC_count) EC_flag = 2;
	else EC_flag = 0;
	
	if(TIM_old != TIM_count){
		TIM_old = TIM_count;
		oldTime = Systime;//������ֵ��һ�²Ÿ���ʱ��
	}
	if(Systime - oldTime > 5000){//����ʱ��������޶���
		TIM_old = TIM_count = EC_count = EC_flag = 0;
		oldTime = Systime;
	}
	
	switch(cfg_ec[key_ec_cs].mode){
		case 1:{//����������
			if(EC_flag){
				if(EC_pulse){//���ϴ��ѷ����򱾴μ��
					EC_pulse = 0;//��ձ�־
					KeyBrd_data[20] = 0;
					EC_count -= EC_flag * 2 - 3;//��������
				}
				else{
					key_insert(21,cfg_ec[key_ec_cs].key[EC_flag]);//�����ֵ
					EC_pulse = 1;//�����ѷ������´μ��
				}
			}
			break;
		}
		case 2:{//CTRL+������
			if(EC_flag){
				KeyBrd_data[1] |= 0x01;
				Mouse_data[4] = 3 - EC_flag * 2;
				EC_count -= EC_flag * 2 - 3;//��������
				Mouse_if_send = 1;
			}
			else Mouse_data[4] = 0;
			break;
		}
		case 3:{//ҡ�˲�������
			uint8_t key_rk_cs = sys_cs + rk_cs[sys_cs];//ҡ�˲���ѡ��
			if(EC_flag){
				cfg_rk[key_rk_cs].effect += 3 - EC_flag * 2;
				if(cfg_rk[key_rk_cs].effect > 150) cfg_rk[key_rk_cs].effect = 0;
				else if(cfg_rk[key_rk_cs].effect > 50) cfg_rk[key_rk_cs].effect = 50;
				EC_count -= EC_flag * 2 - 3;//��������
			}
			break;
		}
		default:{
			EC_count = TIM_count/4;
			break;
		}
	}
}


void Default_para(void)//����Ĭ�ϲ���(�ݲ�ʹ��)
{
	
}














