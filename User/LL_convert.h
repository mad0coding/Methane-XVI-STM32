//2023.12.28
#ifndef _LL_CONVERT_H
#define _LL_CONVERT_H

#include <stdint.h>

#define PARA_FLASH_ADDR		0x08007C00//配置存储地址
#define KB_len				22//键盘报文长度
#define ANA_MID_0			ANA_MID_SET[0]//摇杆中位值
#define ANA_MID_1			ANA_MID_SET[1]//摇杆中位值

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define ABS(X)   ((X) > 0 ? (X) : (-(X)))

extern uint16_t ANA_MID_SET[];//中位定位数组
extern uint8_t TimFilterValue;//编码器滤波参数
extern uint8_t KEY_FILTER_SET;//按键滤波参数

void LL_Init(void);//基于标准库的初始化函数
void LL_Loop(void);//基于标准库的循环体函数


void Light_handle(uint8_t id,uint16_t on_time);//灯处理

void Fake_IRQHandler(void);//伪GPIO中断
int8_t EC_monitor(uint8_t clear);//旋钮监听

uint8_t Light_control(void);//灯控(在设备上手动调整灯光设置)
uint8_t Serial_handle(void);//串口(待开发功能)
uint8_t Buzz_handle(void);//蜂鸣器(4个八度的电子琴)

void Common_GPIO_Init(void);//普通IO初始化
void Key_Get_Map(void);//获取按键并映射
void Key_Read_IO(void);//读取按键电平
uint8_t Key_Filter(uint8_t dt);//按键滤波

//uint8_t CDC_printf(uint8_t* Buf, uint16_t Len);//格式打印函数

#endif

