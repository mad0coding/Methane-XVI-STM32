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

void Save_para(void);//保存参数到FLASH
uint8_t Load_para(void);//从FLASH载入参数
uint8_t Load_one_para(uint8_t choose);//从FLASH缓存载入一套参数
uint8_t Update_para(uint8_t choose);//从USB写入数据更新参数

uint8_t Ana_mid_ctrl(uint8_t ifSave);//存取摇杆中位

uint8_t Fill_report(void);//报文填写

void cs_change(uint8_t change);//切换
void key_insert(uint8_t r_i, uint8_t key_v);//单键填入
void mode3_handle(void);//mode3处理
void RK_EC_key_handle(void);//摇杆旋钮按键处理
void RK_handle(uint8_t clear);//摇杆处理
void EC_handle(uint8_t clear);//旋钮处理


void Default_para(void);//载入默认参数



#endif

