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
extern __IO uint16_t ADC_ConvertedValue[];//ADCֵ
extern uint16_t ANA_MID_SET[];//��λ����
extern uint8_t USB_RX_BUF[64];//�Զ���HID���ջ���
extern uint8_t *cfg_data;//��������
extern uint8_t TimFilterValue;//�������˲�����
extern uint8_t KEY_FILTER_SET;//�����˲�����
uint8_t ifReceiving = 0;//�������ݱ�־λ
uint8_t savePlace = 0;//�洢λ��
uint8_t saveMid = 0;//�洢��λ��־
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
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =//�Զ���HID����������
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
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)//�Զ���HID�������ݻص�����
{
  /* USER CODE BEGIN 6 */
	static uint8_t count = 0;//���ݰ�����
	static uint16_t place = 0;//д��λ��
	uint16_t i = 0, buf_i = 0;
	uint8_t *Buf = USB_RX_BUF;//����ָ��
	
	USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef*)hUsbDeviceFS.pClassData;//�õ�USB�������ݵ����ָ��
	
	memcpy(Buf, hhid->Report_buf, 64);//��Ĭ�ϻ��濽�����û�����
	
	if(!ifReceiving){//��δ�ڽ���״̬
		if(Buf[0] == 'L' && Buf[1] == 'E' && Buf[2] >= '1' && Buf[2] <= '2'){//����ָ��
			place = (Buf[2] - '1') * 512;//ȷ��д��λ��
			Buf[0] = 'R'; Buf[1] = 'L'; Buf[2] = 'E';//������Ӧ�ֽ�
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, Buf, 64);//��Ӧ����
			count = 0;//��������
			ifReceiving = 1;//�������ݱ�־λ��λ
		}
		else if(Buf[0] == 'R' && Buf[1] == 'K' && Buf[2] == 'C'){//ҡ��У��ָ��
			Buf[0] = 'R'; Buf[1] = 'K';//������Ӧ�ֽ�
			ANA_MID_SET[0] = ADC_ConvertedValue[0];//����ǰҡ�˲���ֵ��Ϊҡ����λֵ
			ANA_MID_SET[1] = ADC_ConvertedValue[1];
			Buf[2] = ANA_MID_SET[0] >> 8;//����ҡ�˲���ֵ
			Buf[3] = ANA_MID_SET[0] & 0xFF;
			Buf[4] = ANA_MID_SET[1] >> 8;
			Buf[5] = ANA_MID_SET[1] & 0xFF;
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, Buf, 64);//��Ӧ����
			saveMid = 1;//�洢��λ��־λ��λ
		}
		else if(Buf[0] == 'E' && Buf[1] == 'C' && Buf[2] == 'F'){//�޸���ť�˲�����ָ��
			Buf[0] = 'E'; Buf[1] = 'C';//������Ӧ�ֽ�
			TimFilterValue = Buf[3];//������ť�˲�����
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, Buf, 64);//��Ӧ����
			saveMid = 1;//�洢��λ��־λ��λ
		}
		else if(Buf[0] == 'K' && Buf[1] == 'Y' && Buf[2] == 'F'){//�޸İ����˲�����ָ��
			Buf[0] = 'K'; Buf[1] = 'Y';//������Ӧ�ֽ�
			KEY_FILTER_SET = Buf[3];//���°����˲�����
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, Buf, 64);//��Ӧ����
			saveMid = 1;//�洢��λ��־λ��λ
		}
	}
	else{//���ݰ�
		for(i = count * 64; i < (count + 1) * 64; i++){
			cfg_data[place + i] = Buf[buf_i++];//���ݰ���ȡ
		}
		USB_RX_BUF[0] = count++;//�������
		USB_RX_BUF[1] = 'L'; USB_RX_BUF[2] = 'E';//������Ӧ�ֽ�
		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, USB_RX_BUF, 64);//��Ӧ����
		if(count > 7){
			count = 0;//��ֹԽ��
			savePlace = place == 0 ? 1 : 2;//ȷ���洢λ��
			ifReceiving = 0;//�������ݱ�־λ��λ
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

