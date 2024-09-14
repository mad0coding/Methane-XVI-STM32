//2023.12.28
#ifndef _LL_CONVERT_H
#define _LL_CONVERT_H

#include <stdint.h>

#define PARA_FLASH_ADDR		0x08007C00//���ô洢��ַ
#define KB_len				22//���̱��ĳ���
#define ANA_MID_0			ANA_MID_SET[0]//ҡ����λֵ
#define ANA_MID_1			ANA_MID_SET[1]//ҡ����λֵ

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define ABS(X)   ((X) > 0 ? (X) : (-(X)))

extern uint16_t ANA_MID_SET[];//��λ��λ����
extern uint8_t TimFilterValue;//�������˲�����
extern uint8_t KEY_FILTER_SET;//�����˲�����

void LL_Init(void);//���ڱ�׼��ĳ�ʼ������
void LL_Loop(void);//���ڱ�׼���ѭ���庯��


void Light_handle(uint8_t id,uint16_t on_time);//�ƴ���

void Fake_IRQHandler(void);//αGPIO�ж�
int8_t EC_monitor(uint8_t clear);//��ť����

uint8_t Light_control(void);//�ƿ�(���豸���ֶ������ƹ�����)
uint8_t Serial_handle(void);//����(����������)
uint8_t Buzz_handle(void);//������(4���˶ȵĵ�����)

void Common_GPIO_Init(void);//��ͨIO��ʼ��
void Key_Get_Map(void);//��ȡ������ӳ��
void Key_Read_IO(void);//��ȡ������ƽ
uint8_t Key_Filter(uint8_t dt);//�����˲�

//uint8_t CDC_printf(uint8_t* Buf, uint16_t Len);//��ʽ��ӡ����

#endif

