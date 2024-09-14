#ifndef _CONFIG_PARA_H
#define _CONFIG_PARA_H

#include "stm32f10x.h"
#include "TIMforKB.h"

#define kv_vol_next     237
#define kv_vol_prev     238
#define kv_point		239
#define kv_wheel_up     240
#define kv_wheel_down   241
#define kv_vol_up       242
#define kv_vol_down     243
#define kv_vol_mute     244
#define kv_vol_stop     245
#define kv_mouse_l      246
#define kv_mouse_m      247
#define kv_mouse_r      248
#define kv_ctrl         249
#define kv_shift        250
#define kv_alt          251
#define kv_win          252
#define kv_shortcut     253
#define kv_delay		254

void Save_para(void);//���������FLASH
uint8_t Load_para(void);//��FLASH�������
uint8_t Load_one_para(uint8_t choose);//��FLASH��������һ�ײ���
uint8_t Update_para(uint8_t choose);//��USBд�����ݸ��²���

uint8_t Ana_mid_ctrl(uint8_t ifSave);//��ȡҡ����λ

uint8_t Fill_report(void);//������д

void cs_change(uint8_t change);//�л�
void key_insert(uint8_t r_i, uint8_t key_v);//��������
void mode3_handle(void);//mode3����
void RK_EC_key_handle(void);//ҡ����ť��������
void RK_handle(uint8_t clear);//ҡ�˴���
void EC_handle(uint8_t clear);//��ť����


void Default_para(void);//����Ĭ�ϲ���



#endif

