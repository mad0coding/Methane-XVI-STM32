/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.c
  * @version        : v2.0_Cube
  * @brief          : USB Device Custom HID interface file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_custom_hid_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern __IO uint16_t ADC_ConvertedValue[];//ADC值
extern uint16_t ANA_MID_SET[];//中位数组
extern uint8_t USB_RX_BUF[64];//自定义HID接收缓冲
extern uint8_t *cfg_data;//配置数组
extern uint8_t TimFilterValue;//编码器滤波参数
extern uint8_t KEY_FILTER_SET;//按键滤波参数
uint8_t ifReceiving = 0;//接收数据标志位
uint8_t savePlace = 0;//存储位置
uint8_t saveMid = 0;//存储中位标志
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @addtogroup USBD_CUSTOM_HID
  * @{
  */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions USBD_CUSTOM_HID_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Defines USBD_CUSTOM_HID_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Macros USBD_CUSTOM_HID_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Variables USBD_CUSTOM_HID_Private_Variables
  * @brief Private variables.
  * @{
  */

/** Usb HID report descriptor. */
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =//自定义HID报文描述符
{
  /* USER CODE BEGIN 0 */
	0x06, 0x00, 0xff, 	// Usage page Vendor defined
	0x09, 0x01, 		// Local usage 1
	0xa1, 0x01, 		// Collation Application
	0x09, 0x02, 		// Local usage 2
	0x15, 0x00, 		// Logical min ( 0H )
	0x26, 0xff, 0x00,	// Logical max ( FFH )
	0x75, 0x08,  		// Report size ( 08H )
	0x95, 0x40, 		// Report count ( 40H )
	0x81, 0x06,  		// Input ( Data, Relative, Wrap )
	0x09, 0x02, 		// Local usage 2
	0x15, 0x00,  		// Logical min ( 0H )
	0x26, 0xff, 0x00,	// Logical max ( FFH )
	0x75, 0x08, 		// Report size ( 08H )
	0x95, 0x40, 		// Report count ( 40H )
	0x91, 0x06, 		// Output ( Data, Relative, Wrap )
  /* USER CODE END 0 */
  0xC0    /*     END_COLLECTION	             */
};

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
  * @brief Public variables.
  * @{
  */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CUSTOM_HID_Init_FS(void);
static int8_t CUSTOM_HID_DeInit_FS(void);
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state);

/**
  * @}
  */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS
};

/** @defgroup USBD_CUSTOM_HID_Private_Functions USBD_CUSTOM_HID_Private_Functions
  * @brief Private functions.
  * @{
  */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_Init_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  DeInitializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_DeInit_FS(void)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Manage the CUSTOM HID class events
  * @param  event_idx: Event index
  * @param  state: Event state
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)//自定义HID接收数据回调函数
{
  /* USER CODE BEGIN 6 */
	static uint8_t count = 0;//数据包计数
	static uint16_t place = 0;//写入位置
	uint16_t i = 0, buf_i = 0;
	uint8_t *Buf = USB_RX_BUF;//缓存指针
	
	USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef*)hUsbDeviceFS.pClassData;//得到USB接收数据的相关指针
	
	memcpy(Buf, hhid->Report_buf, 64);//从默认缓存拷贝到用户缓存
	
	if(!ifReceiving){//若未在接收状态
		if(Buf[0] == 'L' && Buf[1] == 'E' && Buf[2] >= '1' && Buf[2] <= '2'){//连接指令
			place = (Buf[2] - '1') * 512;//确定写入位置
			Buf[0] = 'R'; Buf[1] = 'L'; Buf[2] = 'E';//填入响应字节
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, Buf, 64);//响应主机
			count = 0;//计数置零
			ifReceiving = 1;//接收数据标志位置位
		}
		else if(Buf[0] == 'R' && Buf[1] == 'K' && Buf[2] == 'C'){//摇杆校正指令
			Buf[0] = 'R'; Buf[1] = 'K';//填入响应字节
			ANA_MID_SET[0] = ADC_ConvertedValue[0];//将当前摇杆采样值作为摇杆中位值
			ANA_MID_SET[1] = ADC_ConvertedValue[1];
			Buf[2] = ANA_MID_SET[0] >> 8;//填入摇杆采样值
			Buf[3] = ANA_MID_SET[0] & 0xFF;
			Buf[4] = ANA_MID_SET[1] >> 8;
			Buf[5] = ANA_MID_SET[1] & 0xFF;
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, Buf, 64);//响应主机
			saveMid = 1;//存储中位标志位置位
		}
		else if(Buf[0] == 'E' && Buf[1] == 'C' && Buf[2] == 'F'){//修改旋钮滤波参数指令
			Buf[0] = 'E'; Buf[1] = 'C';//填入响应字节
			TimFilterValue = Buf[3];//更新旋钮滤波参数
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, Buf, 64);//响应主机
			saveMid = 1;//存储中位标志位置位
		}
		else if(Buf[0] == 'K' && Buf[1] == 'Y' && Buf[2] == 'F'){//修改按键滤波参数指令
			Buf[0] = 'K'; Buf[1] = 'Y';//填入响应字节
			KEY_FILTER_SET = Buf[3];//更新按键滤波参数
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, Buf, 64);//响应主机
			saveMid = 1;//存储中位标志位置位
		}
	}
	else{//数据包
		for(i = count * 64; i < (count + 1) * 64; i++){
			cfg_data[place + i] = Buf[buf_i++];//数据包读取
		}
		USB_RX_BUF[0] = count++;//填入序号
		USB_RX_BUF[1] = 'L'; USB_RX_BUF[2] = 'E';//填入响应字节
		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, USB_RX_BUF, 64);//响应主机
		if(count > 7){
			count = 0;//防止越界
			savePlace = place == 0 ? 1 : 2;//确定存储位置
			ifReceiving = 0;//接收数据标志位复位
		}
	}
	
  return (USBD_OK);
  /* USER CODE END 6 */
}

/* USER CODE BEGIN 7 */
/**
  * @brief  Send the report to the Host
  * @param  report: The report to be sent
  * @param  len: The report length
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
/*
static int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len)
{
  return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, len);
}
*/
/* USER CODE END 7 */

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

