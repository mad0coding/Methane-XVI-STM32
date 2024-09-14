#include "Config_para.h"

#include <stddef.h>
#include <math.h>
#include "LL_convert.h"
#include "FLASH_para.h"

#include "GPIOpinInit.h"
#include "USARTcontrol.h"

//******************************配置数组******************************//
extern u16 STMFLASH_BUF[];//借用FLASH读写的1K缓存空间
uint8_t* cfg_data = (uint8_t*)STMFLASH_BUF;//512的u16转换为1K的u8
//********************************************************************//

//******************************报文发送******************************//
extern uint8_t KeyBrd_data[];//键盘报文
extern uint8_t Mouse_data[];//鼠标报文
extern uint8_t Point_data[];//指针报文
extern uint8_t Vol_data[];//音量报文
uint8_t KeyBrd_data_old[KB_len];//上次键盘报文
uint8_t Mouse_data_old = 0;//上次鼠标报文
uint8_t Vol_data_old = 0;//上次音量报文
extern uint8_t KeyBrd_if_send;//键盘报文是否发送
extern uint8_t Vol_if_send;//音量报文是否发送
extern uint8_t Point_if_send;//指针报文是否发送
extern uint8_t Mouse_if_send;//鼠标报文是否发送
//********************************************************************//

//******************************数据相关******************************//
extern __IO uint16_t ADC_ConvertedValue[];//ADC值
extern volatile uint32_t Systime;//系统时间

uint8_t key_state[18];//存储按下情况
uint8_t key_old[18];//存储过去的按下情况
extern const uint8_t turnL90[];
extern const uint8_t turnR90[];

const uint16_t para_start[2] = {0,512};//数据起始位置
const uint16_t mode3_start[2] = {0,452};//mode3数据起始位置
uint8_t mode3_data[904];//存储载入的mode3数据

uint8_t mode3_key = 0;//模式3按键(1-16)
uint16_t mode3_i = 0;//模式3源数据下标(访问mode3_data)
//********************************************************************//

//******************************按键配置******************************//
struct config_key{//按键配置结构体
	uint8_t mode;//模式
	//1:单按键,2:快捷键,3:顺序键,4:光标移动,5:光标点击
	//6:切换键,7:连点,8:灯控,9:串口,10:蜂鸣器
	uint8_t key;//键值
	uint8_t func;//其他功能
	uint16_t i;//mode3起始下标(访问mode3_data)(另借用存储连点周期)
	uint16_t x, y;//光标移位(另合并借用为32位计时器)
}cfg_key[2][16];
//********************************************************************//

//******************************摇杆配置******************************//
struct config_rk{
	uint8_t mode;
	//1:速度鼠标,2:位置鼠标,3:四向四按键,4:八向四按键
	uint8_t key_mode;//摇杆按键配置
	//1:单按键
	uint8_t key[5];//摇杆键值(中上下左右)
	uint8_t effect;//效果参数
	uint8_t dead;//死区半径
	int8_t dir_x, dir_y, dir_r;//方向
}cfg_rk[4];

uint8_t rk_cs[2];//摇杆配置选择
//********************************************************************//

//******************************旋钮配置******************************//
struct config_ec{//旋钮配置结构体
	uint8_t mode;//模式
	//1:音量控制,2:两向两按键,3:CTRL+鼠标滚轮,4:摇杆参数控制
	uint8_t key_mode;//旋钮按键模式
	//1:单按键,2:播放暂停,3:静音
	uint8_t key[3];//旋钮键值(中上下)
	int8_t dir;//方向
}cfg_ec[4];

uint8_t ec_cs[2];//旋钮配置选择
//********************************************************************//

//******************************系统配置******************************//
uint8_t light_mode[2],light_cycle[2];//键盘灯模式和呼吸周期
uint8_t light[2],light_rgb[2];//键盘灯和RGB灯亮度
uint8_t rgb_rk[2];//是否RGB摇杆映射
uint8_t rgb_pilot[2];//是否RGB灯光指示
uint16_t screen_w[2] = {1,1};//屏幕宽度
uint16_t screen_h[2] = {1,1};//屏幕高度
uint8_t key_turn[2];//键盘方向
uint8_t sys_cs = 0;//总配置选择
//********************************************************************//


void Save_para(void)//保存参数到FLASH
{
	uint8_t para_i = 0;//参数选择
	uint8_t keyi = 0;//按键下标
	uint16_t i = 0;//访问cfg_data
	uint16_t save3_i = 0;//访问mode3_data
	struct config_key *save_key = NULL;
	
	for(para_i = 0; para_i < 2; para_i++){//存储两套参数
		i = para_start[para_i];
		save3_i = mode3_start[para_i];
		save_key = cfg_key[para_i];
		
		for(keyi = 0; keyi < 16; keyi++){
			cfg_data[i++] = keyi + 1;//存储id
			cfg_data[i++] = save_key[keyi].mode;//存储mode
			if(save_key[keyi].mode == 1){//单键
				cfg_data[i++] = save_key[keyi].key;//存储键值
			}
			else if(save_key[keyi].mode == 2 || save_key[keyi].mode == 6){//快捷键\切换键
				cfg_data[i++] = save_key[keyi].key;//存储键值
				cfg_data[i++] = save_key[keyi].func;//存储功能键\切换方式
			}
			else if(save_key[keyi].mode == 3){//按键组
				cfg_data[i++] = save_key[keyi].func;//存储长度
				for(save3_i = save_key[keyi].i; save3_i < save_key[keyi].i + save_key[keyi].func; save3_i++){
					cfg_data[i++] = mode3_data[save3_i];//存储mode3数据
				}
			}
			else if(save_key[keyi].mode == 4 || save_key[keyi].mode == 5){//触摸
				cfg_data[i++] = (save_key[keyi].x >> 8) & 0xFF;
				cfg_data[i++] = save_key[keyi].x & 0xFF;
				cfg_data[i++] = (save_key[keyi].y >> 8) & 0xFF;
				cfg_data[i++] = save_key[keyi].y & 0xFF;
			}
			else if(save_key[keyi].mode == 7){//连点
				cfg_data[i++] = save_key[keyi].key;//存储键值
				cfg_data[i++] = save_key[keyi].func & 0x01;//存储连点配置
				cfg_data[i++] = (save_key[keyi].i >> 8) & 0xFF;//周期高8位
				cfg_data[i++] = save_key[keyi].i & 0xFF;//周期低8位
			}
		}
		i = para_start[para_i] + 479;//键盘外其他参数的起始位置
		
		cfg_data[i++] = (screen_w[para_i] >> 8) & 0xFF;//屏幕宽
		cfg_data[i++] = screen_w[para_i] & 0xFF;
		cfg_data[i++] = (screen_h[para_i] >> 8) & 0xFF;//屏幕高
		cfg_data[i++] = screen_h[para_i] & 0xFF;
		
		cfg_data[i++] = key_turn[para_i];//键盘方向(483)
		
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].mode | (cfg_rk[para_i * 2 + 0].key_mode << 4);//摇杆1模式
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].key[0];//摇杆1按键
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].key[1];//摇杆1上按键
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].key[2];//摇杆1下按键
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].key[3];//摇杆1左按键
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].key[4];//摇杆1右按键
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].effect;//摇杆1速度
		cfg_data[i++] = cfg_rk[para_i * 2 + 0].dead;//摇杆1死区
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].mode | (cfg_rk[para_i * 2 + 1].key_mode << 4);//摇杆2模式
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].key[0];//摇杆2按键
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].key[1];//摇杆2上按键
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].key[2];//摇杆2下按键
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].key[3];//摇杆2左按键
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].key[4];//摇杆2右按键
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].effect;//摇杆2速度
		cfg_data[i++] = cfg_rk[para_i * 2 + 1].dead;//摇杆2死区
		
		cfg_data[i++] = cfg_ec[para_i * 2 + 0].mode | (cfg_ec[para_i * 2 + 0].key_mode << 4);//旋钮1模式
		cfg_data[i++] = cfg_ec[para_i * 2 + 0].key[0];//旋钮1按键
		cfg_data[i++] = cfg_ec[para_i * 2 + 0].key[1];//旋钮1逆时针按键
		cfg_data[i++] = cfg_ec[para_i * 2 + 0].key[2];//旋钮1顺时针按键
		cfg_data[i++] = cfg_ec[para_i * 2 + 1].mode | (cfg_ec[para_i * 2 + 1].key_mode << 4);//旋钮2模式
		cfg_data[i++] = cfg_ec[para_i * 2 + 1].key[0];//旋钮2按键
		cfg_data[i++] = cfg_ec[para_i * 2 + 1].key[1];//旋钮2逆时针按键
		cfg_data[i++] = cfg_ec[para_i * 2 + 1].key[2];//旋钮2顺时针按键
		
		
		uint8_t lightMode = light_mode[para_i] | (light_cycle[para_i] << 1)//键盘灯模式和呼吸周期
					| (ec_cs[para_i] << 6) | (rk_cs[para_i] << 7);//摇杆旋钮优先级
		lightMode |= rgb_rk[para_i] ? 0x20 : 0x00;//RGB摇杆映射
		lightMode |= rgb_pilot[para_i] ? 0x10 : 0x00;//RGB灯光指示
		
		uint8_t dir_rkec = 0;//方向
		dir_rkec |= cfg_rk[para_i * 2 + 0].dir_x ? 0x80 : 0x00;//反向
		dir_rkec |= cfg_rk[para_i * 2 + 0].dir_y ? 0x40 : 0x00;
		dir_rkec |= cfg_rk[para_i * 2 + 0].dir_r ? 0x20 : 0x00;
		dir_rkec |= cfg_rk[para_i * 2 + 1].dir_x ? 0x10 : 0x00;
		dir_rkec |= cfg_rk[para_i * 2 + 1].dir_y ? 0x08 : 0x00;
		dir_rkec |= cfg_rk[para_i * 2 + 1].dir_r ? 0x04 : 0x00;
		dir_rkec |= cfg_ec[para_i * 2 + 0].dir ? 0x02 : 0x00;
		dir_rkec |= cfg_ec[para_i * 2 + 1].dir ? 0x01 : 0x00;
		
		cfg_data[i++] = light[para_i] | (light_rgb[para_i] << 4);//灯亮度
		cfg_data[i++] = lightMode;//灯设置
		cfg_data[i++] = dir_rkec;//摇杆旋钮方向及优先级
		cfg_data[i] = sys_cs;//总优先级
	}
	
	STMFLASH_Write_Del(PARA_FLASH_ADDR,(uint16_t*)cfg_data,512);//删除并写入1024字节(512半字)
}

uint8_t Load_para(void)//从FLASH载入参数
{
	STMFLASH_Read(PARA_FLASH_ADDR,(uint16_t*)cfg_data,512);//读取1024字节(512半字)
	
	Load_one_para(0);//第一套参数
	Load_one_para(1);//第二套参数
	
	return 1;
}

uint8_t Load_one_para(uint8_t choose)//从FLASH缓存载入按键参数
{
	uint8_t keyi = 0;//按键下标
	uint16_t i = para_start[choose];//访问cfg_data
	uint16_t load3_i = mode3_start[choose];//访问mode3_data
	struct config_key *load_key = cfg_key[choose];
	//uint16_t len = 0;//数据长度
		
	for(keyi = 0; keyi < 16; keyi++){//加载16个按键的配置
		if(cfg_data[i++] != keyi + 1) return 0;//若id不对则退出
		load_key[keyi].mode = cfg_data[i++];//载入模式
		if(load_key[keyi].mode == 1){//单键
			load_key[keyi].key = cfg_data[i++];//载入按键
		}
		else if(load_key[keyi].mode == 2 || load_key[keyi].mode == 6){//快捷键\切换键
			load_key[keyi].key = cfg_data[i++];//载入按键
			load_key[keyi].func = cfg_data[i++];//载入功能键\切换方式
		}
		else if(load_key[keyi].mode == 3){//按键组
			load_key[keyi].func = cfg_data[i++];//暂存长度
			load_key[keyi].i = load3_i;//记录本键起始下标
			for(/*load3_i*/; load3_i < load_key[keyi].i + load_key[keyi].func; load3_i++){
				mode3_data[load3_i] = cfg_data[i++];//存储每个键
			}
			load_key[keyi+1].i = load3_i;//记录下一个键起始下标
		}
		else if(load_key[keyi].mode == 4 || load_key[keyi].mode == 5){//触摸
			load_key[keyi].key = 0;//借用存储计数
			load_key[keyi].func = 0;//借用存储状态
			load_key[keyi].x = (cfg_data[i + 0] << 8) | cfg_data[i + 1];
			load_key[keyi].y = (cfg_data[i + 2] << 8) | cfg_data[i + 3];
			i += 4;
		}
		else if(load_key[keyi].mode == 7){//连点
			load_key[keyi].key = cfg_data[i++];//载入按键
			load_key[keyi].func = cfg_data[i++];//载入连点配置
			load_key[keyi].i = (cfg_data[i] << 8) | cfg_data[i + 1];//载入周期
			i += 2;
		}
		else if(load_key[keyi].mode > 13){
			load_key[keyi].mode = 0;//都不对则清除模式
			return 0;
		}
	}
	
	i = para_start[choose] + 479;//键盘外其他参数的起始位置
	
    screen_w[choose] = (cfg_data[i] << 8) | cfg_data[i + 1];//屏幕宽
    i += 2;
    screen_h[choose] = (cfg_data[i] << 8) | cfg_data[i + 1];//屏幕高
    i += 2;
	
	key_turn[choose] = cfg_data[i++];//键盘方向(483)
	
    cfg_rk[choose * 2 + 0].mode = (cfg_data[i] & 0x07);//摇杆1模式
    cfg_rk[choose * 2 + 0].key_mode = ((cfg_data[i++] >> 4) & 0x0F);//摇杆1按键模式
    cfg_rk[choose * 2 + 0].key[0] = cfg_data[i++];//摇杆1按键
    cfg_rk[choose * 2 + 0].key[1] = cfg_data[i++];//摇杆1上按键
    cfg_rk[choose * 2 + 0].key[2] = cfg_data[i++];//摇杆1下按键
    cfg_rk[choose * 2 + 0].key[3] = cfg_data[i++];//摇杆1左按键
    cfg_rk[choose * 2 + 0].key[4] = cfg_data[i++];//摇杆1右按键
    cfg_rk[choose * 2 + 0].effect = cfg_data[i++];//摇杆1速度
    cfg_rk[choose * 2 + 0].dead = cfg_data[i++];//摇杆1死区
    
    cfg_rk[choose * 2 + 1].mode = (cfg_data[i] & 0x07);//摇杆2模式
    cfg_rk[choose * 2 + 1].key_mode = ((cfg_data[i++] >> 4) & 0x0F);//摇杆2按键模式
    cfg_rk[choose * 2 + 1].key[0] = cfg_data[i++];//摇杆2按键
    cfg_rk[choose * 2 + 1].key[1] = cfg_data[i++];//摇杆2上按键
    cfg_rk[choose * 2 + 1].key[2] = cfg_data[i++];//摇杆2下按键
    cfg_rk[choose * 2 + 1].key[3] = cfg_data[i++];//摇杆2左按键
    cfg_rk[choose * 2 + 1].key[4] = cfg_data[i++];//摇杆2右按键
    cfg_rk[choose * 2 + 1].effect = cfg_data[i++];//摇杆2速度
    cfg_rk[choose * 2 + 1].dead = cfg_data[i++];//摇杆2死区
    
	cfg_ec[choose * 2 + 0].mode = (cfg_data[i] & 0x0F);//旋钮1模式
    cfg_ec[choose * 2 + 0].key_mode = ((cfg_data[i++] >> 4) & 0x0F);//旋钮1按键模式
	cfg_ec[choose * 2 + 0].key[0] = cfg_data[i++];//旋钮1按键
    cfg_ec[choose * 2 + 0].key[1] = cfg_data[i++];//旋钮1逆时针按键
    cfg_ec[choose * 2 + 0].key[2] = cfg_data[i++];//旋钮1顺时针按键
    cfg_ec[choose * 2 + 1].mode = (cfg_data[i] & 0x0F);//旋钮2模式
    cfg_ec[choose * 2 + 1].key_mode = ((cfg_data[i++] >> 4) & 0x0F);//旋钮2按键模式
	cfg_ec[choose * 2 + 1].key[0] = cfg_data[i++];//旋钮2按键
    cfg_ec[choose * 2 + 1].key[1] = cfg_data[i++];//旋钮2逆时针按键
    cfg_ec[choose * 2 + 1].key[2] = cfg_data[i++];//旋钮2顺时针按键
    
    light[choose] = (cfg_data[i] & 0x0F);//键盘灯亮度
    light_rgb[choose] = ((cfg_data[i++] >> 4) & 0x0F);//RGB灯亮度
    
	rk_cs[choose] = ((cfg_data[i] >> 7) & 0x01);//摇杆优先级
    ec_cs[choose] = ((cfg_data[i] >> 6) & 0x01);//旋钮优先级
	light_cycle[choose] = ((cfg_data[i] >> 1) & 0x07);//呼吸周期
    light_mode[choose] = (cfg_data[i] & 0x01);//键盘灯模式
    rgb_rk[choose] = (cfg_data[i] & 0x20);//RGB摇杆映射
    rgb_pilot[choose] = (cfg_data[i++] & 0x10);//RGB灯光指示
    
    cfg_rk[choose + 0].dir_x = ((cfg_data[i] >> 7) & 0x01);//反向
    cfg_rk[choose + 0].dir_y = ((cfg_data[i] >> 6) & 0x01);
    cfg_rk[choose + 0].dir_r = ((cfg_data[i] >> 5) & 0x01);
    cfg_rk[choose + 1].dir_x = ((cfg_data[i] >> 4) & 0x01);
	cfg_rk[choose + 1].dir_y = ((cfg_data[i] >> 3) & 0x01);
    cfg_rk[choose + 1].dir_r = ((cfg_data[i] >> 2) & 0x01);
    cfg_ec[choose + 0].dir = ((cfg_data[i] >> 1) & 0x01);
    cfg_ec[choose + 1].dir = ((cfg_data[i++] >> 0) & 0x01);
	
	sys_cs = cfg_data[i];//总优先级
	
	return 1;
}

uint8_t Update_para(uint8_t choose)//从USB写入数据更新参数
{
	if(cfg_data[choose * 512 + 511]){//1为新配置为不优先
		cfg_data[511] = cfg_data[1023] = 1 - choose;//置为旧配置位置
	}
	else{//0为新配置优先
		cfg_data[511] = cfg_data[1023] = choose;//置为新配置位置
	}
	if(!Load_one_para(choose)){}// return 0;//从新数据加载配置
	STMFLASH_Write_Del(PARA_FLASH_ADDR,(uint16_t*)cfg_data,512);//删除并写入1024字节(512半字)
	Systime = 0;//重置系统时间
	return 1;
}

uint8_t Ana_mid_ctrl(uint8_t ifSave)//存取摇杆中位等
{
	if(ifSave){//存储
		uint16_t tmp[3];//临时缓存
		tmp[0] = STMFLASH_BUF[0]; tmp[1] = STMFLASH_BUF[1]; tmp[2] = STMFLASH_BUF[2];//暂存
		STMFLASH_BUF[0] = ANA_MID_0;//填入中位
		STMFLASH_BUF[1] = ANA_MID_1;//填入中位
		STMFLASH_BUF[2] = (TimFilterValue << 8) | KEY_FILTER_SET;//填入编码器滤波参数和按键滤波参数
		STMFLASH_Write_Del(PARA_FLASH_ADDR - 1024,(uint16_t*)cfg_data,512);//删除并写入1024字节(512半字)
		STMFLASH_BUF[0] = tmp[0]; STMFLASH_BUF[1] = tmp[1]; STMFLASH_BUF[2] = tmp[2];//恢复
	}
	else{//读取
		ANA_MID_0 = STMFLASH_ReadHalfWord(PARA_FLASH_ADDR - 1024 + 4);//借用
		TimFilterValue = ANA_MID_0 >> 8;//编码器滤波参数
		KEY_FILTER_SET = ANA_MID_0 & 0xFF;//按键滤波参数
		if(KEY_FILTER_SET < 1 || KEY_FILTER_SET > 250) KEY_FILTER_SET = 10;//若数据不对则使用默认值
		ANA_MID_0 = STMFLASH_ReadHalfWord(PARA_FLASH_ADDR - 1024);
		ANA_MID_1 = STMFLASH_ReadHalfWord(PARA_FLASH_ADDR - 1024 + 2);
		if(ANA_MID_0 > 4095 || ANA_MID_1 > 4095) ANA_MID_0 = ANA_MID_1 = 2048;//若判断为尚未存储中位则使用2048
		ANA_MID_0 &= 0x0FFF;
		ANA_MID_1 &= 0x0FFF;
	}
	return 0;
}

uint8_t Fill_report(void)//报文填写
{
	static uint32_t oldTime = 0;//记录时间
	static uint8_t switch_i = 0xFF, switch_count = 0;//切换键选择和计数
	static uint8_t switch_key = 0, switch_func = 0;//切换键缓存
	static uint8_t mode3_pulse = 0;//模式3间隔
	uint8_t mode1_num = 0, mode2_num = 0, mode7_num = 0;
	uint8_t i = 0;//公共用
	struct config_key *fill_key = cfg_key[sys_cs];
	
	//***********************************各报文及发送标志初始化***********************************//
	KeyBrd_if_send = Mouse_if_send = Point_if_send = Vol_if_send = 0;//发送标志置零
	for(i = 1; i < KB_len; i++){
		KeyBrd_data_old[i] = KeyBrd_data[i];//记录上一次报文
		KeyBrd_data[i] = 0;//清除所有键
	}
	Mouse_data_old = Mouse_data[1];//记录上一次报文
	for(i = 1; i < 5; i++){
		Mouse_data[i] = 0;//清除鼠标报文
	}
	Point_data[1] = 0x10;
	for(i = 3; i < 7; i++){
		Point_data[i] = 0xFF;//清除指针报文
	}
	Vol_data_old = Vol_data[1];//记录上一次报文
	Vol_data[1] = 0;//清除音量报文
	//********************************************************************************************//
	
	//****************************************键盘按键处理****************************************//
	if(mode3_key){//若处于mode3未完成状态
		if(!mode3_pulse) mode3_handle();
		mode3_pulse = !mode3_pulse;
	}
	else{//空闲状态
		for(i = 0; i < 16; i++){//统计按下的各模式数量
			if(key_state[i+1]){
				if(fill_key[i].mode == 1 && i != switch_i) mode1_num++;
				else if(fill_key[i].mode == 2 && i != switch_i) mode2_num++;
				else if(fill_key[i].mode == 7 && i != switch_i) mode7_num++;
			}
		}
		for(i = 0; i < 16; i++){//对于16个按键
			if(i == switch_i){//若有正在生效的临时切换键
				if(!key_state[i+1] && !key_old[i+1]){//释放沿之后一拍
					switch_count++;//计数滤波
					if(switch_count > 100){
						switch_count = 0;//计数清零
						key_insert(i + 3,switch_key);//填入键值
						cs_change(switch_func);//切换回来
						switch_i = 0xFF;//复位
					}
				}
				continue;//屏蔽临时切换键
			}
			if(key_state[i+1]){//若按下
				if(fill_key[i].mode == 1){//模式1:单键
					key_insert(i + 3,fill_key[i].key);//填入键值
				}
				else if(fill_key[i].mode == 2){//模式2:快捷键
					key_insert(i + 3,fill_key[i].key);//填入键值
					KeyBrd_data[1] |= fill_key[i].func;//填入功能键
				}
				else if(fill_key[i].mode == 3){//模式3:顺序键
					if(mode1_num == 0 && mode2_num == 0 && mode7_num == 0 && !key_old[i+1]){//不存在1,2,7模式按键且按下沿
						mode3_key = i + 1;//记录mode3按键
						mode3_i = fill_key[i].i;//读取起始下标
						mode3_handle();
						mode3_pulse = 1;//插入间隔
					}
				}
				else if(fill_key[i].mode == 4 || fill_key[i].mode == 5){//模式4:光标移位,模式5:光标点击
					if(!key_old[i+1]) fill_key[i].func = 2;//待按下态
					
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
				else if(fill_key[i].mode == 6){//模式6:切换键
					if((fill_key[i].func & 0x08) && switch_i == 0xFF && !key_old[i+1]){//若为临时切换且为独有且按下沿
						uint8_t turn_old = key_turn[sys_cs];//旧键盘方向
						cs_change(fill_key[i].func);//切换
						uint8_t turn_dif = (key_turn[sys_cs] + 4 - turn_old) % 4;//相对旧键盘方向
						if(turn_dif == 0) switch_i = i;
						else if(turn_dif == 1) switch_i = turnL90[i] - 1;
						else if(turn_dif == 2) switch_i = 16 - i;
						else if(turn_dif == 3) switch_i = turnR90[i] - 1;
						switch_key = fill_key[i].key;//缓存键值
						switch_func = fill_key[i].func;//缓存切换方式
						break;//跳出本次循环
					}
				}
				else if(fill_key[i].mode == 7){//模式7:按键连点
					if(!key_old[i+1] && !(fill_key[i].func & 0x01)){//若为按下沿且非自动连点
						fill_key[i].func |= 0x02;//启动连点
						fill_key[i].x = fill_key[i].y = 0;//计时器清零
					}
				}
			}
			else if(key_old[i+1]){//若刚抬起(释放沿)
				if(fill_key[i].mode == 5){//模式5:光标点击
					if(key_old[i+1]){//释放沿
						fill_key[i].func = 1;//待抬起态
					}
				}
				else if(fill_key[i].mode == 6){//模式6:切换键
					if(!(fill_key[i].func & 0x08)){//若为非临时切换
						cs_change(fill_key[i].func);//切换
					}
				}
				else if(fill_key[i].mode == 7){//模式7:按键连点
					if((fill_key[i].func & 0x01) && !(fill_key[i].func & 0x02)){//自动连点且当前未在连点
						fill_key[i].func |= 0x02;//启动连点
						fill_key[i].x = fill_key[i].y = 0;//计时器清零
					}
					else fill_key[i].func &= ~0x02;//停止连点
				}
				else if(fill_key[i].mode == 8){//模式8:灯控
					return 1;
				}
				else if(fill_key[i].mode == 9){//模式9:串口
					USART_Config(1,115200,3,3);//串口1初始化
					return 2;
				}
				else if(fill_key[i].mode == 10){//模式10:蜂鸣器
					return 3;
				}
			}
		}//处理完16个按键的主要内容
		
		for(i = 0; i < 16; i++){//对于16个按键的触摸
			if(fill_key[i].mode == 4 || fill_key[i].mode == 5){//模式4:光标移位,模式5:光标点击
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
		}//处理完16个按键的触摸
		
		int8_t auto_num = 0;//正在执行自动连点的按键数量
		for(i = 0; i < 16; i++){//对于16个按键的连点
			if(fill_key[i].mode == 7){//模式7:按键连点
				if(oldTime > Systime){//若系统时间重置
					fill_key[i].x = fill_key[i].y = 0;//清空设定时间
					fill_key[i].func &= ~0x04;//复位点击标志
					fill_key[i].func &= ~0x02;//停止连点
				}
				else if(fill_key[i].func & 0x02){//若连点启用
					if(fill_key[i].i == 0){//若周期为0则长按
						key_insert(i + 3,fill_key[i].key);//填入键值
					}
					else if(fill_key[i].func & 0x04){//若上次已点击
						fill_key[i].func &= ~0x04;//复位点击标志
					}
					else{//若上次未点击
						uint32_t setTime = (fill_key[i].x << 16) | fill_key[i].y;//设定时间
						if(setTime <= Systime){//若定时已到
							key_insert(i + 3,fill_key[i].key);//填入键值
							setTime = Systime + fill_key[i].i;//更新设定时间
							fill_key[i].x = (setTime >> 16) & 0xFFFF;
							fill_key[i].y = setTime & 0xFFFF;
							fill_key[i].func |= 0x04;//置位点击标志
						}
					}
					if(fill_key[i].func & 0x01) auto_num++;//自动连点按键计数
				}
			}
		}//处理完16个按键的连点
		oldTime = Systime;//记录时间更新
		if(auto_num-- > 0) Light_handle(3,0xFF);//B
		//else Light_handle(3,0);
		if(auto_num-- > 0) Light_handle(1,0xFF);//R
		//else Light_handle(1,0);
		if(auto_num-- > 0) Light_handle(2,0xFF);//G
		//else Light_handle(2,0);
	}
	//********************************************************************************************//
	
	//***********************************摇杆旋钮处理***********************************//
	RK_handle(0);//摇杆处理
	EC_handle(0);//旋钮处理
	if(!mode3_key) RK_EC_key_handle();//摇杆旋钮按键处理
	//**********************************************************************************//
	
	//***********************************判断各报文是否要发送***********************************//
	for(i = 1; i < KB_len; i++){
		if(KeyBrd_data_old[i] != KeyBrd_data[i]){//键盘报文与上一次不同则发送
			KeyBrd_if_send = 1;
			break;
		}
	}
	if(Mouse_data[1] != Mouse_data_old) Mouse_if_send = 1;//鼠标按键与上次不同则发送
	else{
		for(i = 2; i < 5; i++){
			if(Mouse_data[i] != 0){//鼠标存在移动或滚动则发送
				Mouse_if_send = 1;
				break;
			}
		}
	}
	for(i = 3; i < 7; i++){
		if(Point_data[i] != 0xFF){//触摸存在有效坐标则发送
			Point_if_send = 1;
			break;
		}
	}
	if(Vol_data[1] != Vol_data_old) Vol_if_send = 1;//媒体报文与上次不同则发送
	//******************************************************************************************//
	
	if(switch_i != 0xFF 
		&& (KeyBrd_if_send || Mouse_if_send || Point_if_send || Vol_if_send)){
		switch_key = 0;//若临时切换键按下期间有其他操作则清除切换键自身键值
	}
	
	return 0;
}

void cs_change(uint8_t change)//切换
{
	if(change & 0x01){//键盘
		sys_cs = 1 - sys_cs;
		Light_handle(sys_cs,200);//Red
	}
	if(change & 0x02){//摇杆
		rk_cs[sys_cs] = 1 - rk_cs[sys_cs];
		Light_handle(rk_cs[sys_cs] ? 3 : 0,200);//Blue
	}
	if(change & 0x04){//旋钮
		ec_cs[sys_cs] = 1 - ec_cs[sys_cs];
		Light_handle(ec_cs[sys_cs] ? 2 : 0,200);//Green
	}
	Light_handle(4,1);
	RK_handle(1);
	EC_handle(1);
}

void key_insert(uint8_t r_i, uint8_t key_v)//单键填入
{
	if(key_v == 240) Mouse_data[4] += 1;//滚轮向上
	else if(key_v == 241) Mouse_data[4] += -1;//滚轮向下
	else if(key_v == 242) Vol_data[1] |= 0x01;//音量加
	else if(key_v == 243) Vol_data[1] |= 0x02;//音量减
	else if(key_v == 244) Vol_data[1] |= 0x04;//静音
	else if(key_v == 245) Vol_data[1] |= 0x08;//播放暂停
	else if(key_v == 237) Vol_data[1] |= 0x10;//下一个
	else if(key_v == 238) Vol_data[1] |= 0x20;//上一个
	else if(key_v == 246) Mouse_data[1] |= 0x01;//鼠标左键
	else if(key_v == 247) Mouse_data[1] |= 0x04;//鼠标中键
	else if(key_v == 248) Mouse_data[1] |= 0x02;//鼠标右键
	else if(key_v == 249) KeyBrd_data[1] |= 0x01;//ctrl
	else if(key_v == 250) KeyBrd_data[1] |= 0x02;//shift
	else if(key_v == 251) KeyBrd_data[1] |= 0x04;//alt
	else if(key_v == 252) KeyBrd_data[1] |= 0x08;//win
	else KeyBrd_data[r_i] = key_v;//填入键值
}

void mode3_handle(void)//mode3处理(按键组处理)
{
	static uint32_t setTime = 0, oldTime = 0;//设定时间及记录时间
	uint8_t report_i = 3;//报文写入位置
	
	if(oldTime > Systime) setTime = 0;//若系统时间重置则终止延时
	oldTime = Systime;//记录时间更新
	if(setTime > Systime) return;//延时未结束则退出等待
	
	uint16_t end_i = 0;//结束位置
	if(mode3_key) end_i = cfg_key[sys_cs][mode3_key - 1].i + cfg_key[sys_cs][mode3_key - 1].func;
	
	while(report_i < KB_len){//当报文未填满
		if(mode3_data[mode3_i] == kv_shortcut){//若有快捷键
			if(report_i > 3 || Mouse_data[1] != 0) break;//若本次报文已有内容则退出等下一次
			key_insert(report_i,mode3_data[++mode3_i]);//填入键值
			KeyBrd_data[1] = mode3_data[++mode3_i];//填入功能键
			if(++mode3_i == end_i) mode3_key = 0;//当读完数据
			break;//独占本次报文
		}
		else if(mode3_data[mode3_i] == kv_point){//若有光标移位
			if(report_i > 3 || Mouse_data[1] != 0) break;//若本次报文已有内容则退出等下一次
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
			if(++mode3_i == end_i) mode3_key = 0;//当读完数据
			break;//独占本次报文
		}
		else if(mode3_data[mode3_i] == kv_delay){//若有延时
			uint16_t delayTime = (mode3_data[mode3_i + 1] << 8) | mode3_data[mode3_i + 2];
			setTime = Systime + delayTime;
			mode3_i += 2;
			if(++mode3_i == end_i) mode3_key = 0;//当读完数据
			break;//独占本次报文
		}
		else if(mode3_data[mode3_i] == kv_shift){//若有shift
			uint8_t check_i = 3;
			for(check_i = 3; check_i < report_i; check_i++){//检查是否已有相同键值
				if(KeyBrd_data[check_i] == mode3_data[mode3_i]){
					check_i = 0;
					break;
				}
			}
			if(!check_i) break;//若已有相同键值则退出等下一次
			if(Mouse_data[1] != 0) break;//若鼠标报文已有内容则退出等下一次
			if(KeyBrd_data[1]){//若报文已含shift
				key_insert(report_i++,mode3_data[++mode3_i]);//填入键值
			}
			else if(report_i == 3){//当是起始
				key_insert(report_i++,mode3_data[++mode3_i]);//填入键值
				KeyBrd_data[1] |= 0x02;//填入shift
			}
			else break;//因前面排斥shift而无法填入
		}
		else{//若无shift
			uint8_t check_i = 3;
			for(check_i = 3; check_i < report_i; check_i++){//检查是否已有相同键值
				if(KeyBrd_data[check_i] == mode3_data[mode3_i]){
					check_i = 0;
					break;
				}
			}
			if(!check_i) break;//若已有相同键值则退出等下一次
			if(Mouse_data[1] != 0) break;//若鼠标报文已有内容则退出等下一次
			if(!KeyBrd_data[1]){//若报文未含shift
				key_insert(report_i++,mode3_data[mode3_i]);//填入键值
			}
			else if(report_i == 3){//当是起始
				key_insert(report_i++,mode3_data[mode3_i]);//填入键值
			}
			else break;//因前面已有shift而无法填入
		}
		mode3_i++;
		if(mode3_i == end_i){//当读完数据
			mode3_key = 0;
			break;
		}
	}
}

void RK_EC_key_handle(void)//摇杆旋钮按键处理
{
	uint8_t rk_ec_cs = sys_cs * 2 + rk_cs[sys_cs];//参数选择
	uint8_t cfg_rk_ec = cfg_rk[rk_ec_cs].key_mode;//按键模式
	uint8_t key_rk_ec = cfg_rk[rk_ec_cs].key[0];//按键值
	uint8_t keyState = key_state[17];//按键状态
	for(uint8_t rk_ec = 0; rk_ec < 2; rk_ec++){
		if(rk_ec == 1){
			rk_ec_cs = sys_cs * 2 + ec_cs[sys_cs];//参数选择
			cfg_rk_ec = cfg_ec[rk_ec_cs].key_mode;//按键模式
			key_rk_ec = cfg_ec[rk_ec_cs].key[0];//按键值
			keyState = key_state[0];//按键状态
		}
		if(!keyState) continue;
		switch(cfg_rk_ec){
			case 1:{//单按键
				for(uint8_t i = 3; i < KB_len; i++){
					if(!KeyBrd_data[i]){
						key_insert(i,key_rk_ec);//填入键值
						break;
					}
				}
				break;
			}
			default:break;
		}
	}
	
}

//1:速度鼠标,2:位置鼠标,3:四向四按键,4:八向四按键
//1:单按键,2:播放暂停,3:静音

void RK_handle(uint8_t clear)//摇杆处理
{
	static int16_t RK_pulse = 0;//间隔标志
	static int16_t x_pic = 0, y_pic = 0;
	
	if(clear){//清除
		RK_pulse = x_pic = y_pic = 0;
		return;
	}
	
	uint8_t key_rk_cs = sys_cs * 2 + rk_cs[sys_cs];//参数选择
	int16_t dx = 0, dy = 0;
	int16_t x = (cfg_rk[key_rk_cs].dir_x*2 - 1);
	int16_t y = (cfg_rk[key_rk_cs].dir_y*2 - 1);
	
	if(cfg_rk[key_rk_cs].dir_r){//若转90度
		x *= (ADC_ConvertedValue[0] - ANA_MID_0);
		y *= (ADC_ConvertedValue[1] - ANA_MID_1);
	}
	else{
		x *= (ANA_MID_1 - ADC_ConvertedValue[1]);//向右为正
		y *= (ADC_ConvertedValue[0] - ANA_MID_0);//向上为正
	}
	
	uint16_t equal_r = MAX(ABS(x), ABS(y));//等效半径
	
	if(equal_r <= cfg_rk[key_rk_cs].dead * 21) return;//在死区则退出
	
	
	switch(cfg_rk[key_rk_cs].mode){
		case 1:{//速度鼠标
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
		case 2:{//位置鼠标
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
		case 3:{//四向四按键
			if(RK_pulse){//若上次已发送则本次间隔
				RK_pulse--;//标志递减
				break;
			}
			RK_pulse = 2000 - equal_r * (100 - cfg_rk[key_rk_cs].effect) / 100;
			RK_pulse = RK_pulse >= 1 ? RK_pulse : 1;
			RK_pulse *= (cfg_rk[key_rk_cs].effect + 19) / 20;//本次已发送则下次间隔
			if(y > x){
				if(y > -x) key_insert(19,cfg_rk[key_rk_cs].key[1]);//填入上键值
				else key_insert(19,cfg_rk[key_rk_cs].key[3]);//填入左键值
			}
			else{
				if(y > -x) key_insert(19,cfg_rk[key_rk_cs].key[4]);//填入右键值
				else key_insert(19,cfg_rk[key_rk_cs].key[2]);//填入下键值
			}
			break;
		}
		case 4:{//八向四按键
			if(RK_pulse){//若上次已发送则本次间隔
				RK_pulse--;//标志递减
				break;
			}
			RK_pulse = 2000 - equal_r * (100 - cfg_rk[key_rk_cs].effect) / 100;
			RK_pulse = RK_pulse >= 1 ? RK_pulse : 1;
			RK_pulse *= (cfg_rk[key_rk_cs].effect + 19) / 20;//本次已发送则下次间隔
			if(y*12 > x*5 && y*12 > -x*5){//上
				key_insert(19,cfg_rk[key_rk_cs].key[1]);//填入键值
			}
			else if(y*12 < x*5 && y*12 < -x*5){//下
				key_insert(19,cfg_rk[key_rk_cs].key[2]);//填入键值
			}
			if(y*5 > x*12 && y*5 < -x*12){//左
				key_insert(20,cfg_rk[key_rk_cs].key[3]);//填入键值
			}
			else if(y*5 < x*12 && y*5 > -x*12){//右
				key_insert(20,cfg_rk[key_rk_cs].key[4]);//填入键值
			}
			break;
		}
		default:break;
	}
}

//1:音量控制,2:两向两按键,3:CTRL+鼠标滚轮,4:摇杆参数控制
//1:单按键
void EC_handle(uint8_t clear)//旋钮处理
{
	static uint32_t oldTime = 0;//记录时间
	static int TIM_old = 0;//编码器旧计数
	static int TIM_count = 0;//编码器计数
	static int EC_count = 0;//执行计数
	static uint8_t EC_flag = 0;//执行标志
	static uint8_t EC_pulse = 0;//间隔标志
	
	if(clear){//清除
		TIM_count = EC_count = 0;
		EC_flag = EC_pulse = 0;
		return;
	}
	
	uint8_t key_ec_cs = sys_cs * 2 + ec_cs[sys_cs];//参数选择
	
#if ifTimEncode
	TIM_count -= (cfg_ec[key_ec_cs].dir * 2 - 1) * (short)(TIM3 -> CNT);//编码器计数读取
	TIM3 -> CNT = 0;//编码器清零
#else
	TIM_count -= (cfg_ec[key_ec_cs].dir * 2 - 1) * (short)(IrqTimValue);//编码器计数读取
	IrqTimValue = 0;//编码器清零
#endif
	
	if(TIM_count/4 > EC_count) EC_flag = 1;
	else if(TIM_count/4 < EC_count) EC_flag = 2;
	else EC_flag = 0;
	
	if(TIM_old != TIM_count){
		TIM_old = TIM_count;
		oldTime = Systime;//若计数值不一致才更新时间
	}
	if(Systime - oldTime > 5000){//若长时间编码器无动作
		TIM_old = TIM_count = EC_count = EC_flag = 0;
		oldTime = Systime;
	}
	
	switch(cfg_ec[key_ec_cs].mode){
		case 1:{//两向两按键
			if(EC_flag){
				if(EC_pulse){//若上次已发送则本次间隔
					EC_pulse = 0;//清空标志
					KeyBrd_data[20] = 0;
					EC_count -= EC_flag * 2 - 3;//计数跟进
				}
				else{
					key_insert(21,cfg_ec[key_ec_cs].key[EC_flag]);//填入键值
					EC_pulse = 1;//本次已发送则下次间隔
				}
			}
			break;
		}
		case 2:{//CTRL+鼠标滚轮
			if(EC_flag){
				KeyBrd_data[1] |= 0x01;
				Mouse_data[4] = 3 - EC_flag * 2;
				EC_count -= EC_flag * 2 - 3;//计数跟进
				Mouse_if_send = 1;
			}
			else Mouse_data[4] = 0;
			break;
		}
		case 3:{//摇杆参数控制
			uint8_t key_rk_cs = sys_cs + rk_cs[sys_cs];//摇杆参数选择
			if(EC_flag){
				cfg_rk[key_rk_cs].effect += 3 - EC_flag * 2;
				if(cfg_rk[key_rk_cs].effect > 150) cfg_rk[key_rk_cs].effect = 0;
				else if(cfg_rk[key_rk_cs].effect > 50) cfg_rk[key_rk_cs].effect = 50;
				EC_count -= EC_flag * 2 - 3;//计数跟进
			}
			break;
		}
		default:{
			EC_count = TIM_count/4;
			break;
		}
	}
}


void Default_para(void)//载入默认参数(暂不使用)
{
	
}














