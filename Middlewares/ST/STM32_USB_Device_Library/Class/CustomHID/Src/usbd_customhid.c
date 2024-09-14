/**
  ******************************************************************************
  * @file    usbd_customhid.c
  * @author  MCD Application Team
  * @brief   This file provides the CUSTOM_HID core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                CUSTOM_HID Class  Description
  *          ===================================================================
  *           This module manages the CUSTOM_HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (CUSTOM_HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - Usage Page : Generic Desktop
  *             - Usage : Vendor
  *             - Collection : Application
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid.h"
#include "usbd_custom_hid_if.h"
#include "usbd_ctlreq.h"
//#include "usbd_hid.h"

/* USER CODE BEGIN PV */
USBD_CUSTOM_HID_HandleTypeDef	*pCustomHidClassData;	//pClassData指向CustomHID时使用
//USBD_HID_HandleTypeDef			*pHidClassData;			//pClassData指向HID时使用

/* USER CODE END PV */


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_CUSTOM_HID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CUSTOM_HID_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_CUSTOM_HID_Private_Macros
  * @{
  */
/**
  * @}
  */
/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_CUSTOM_HID_Init(USBD_HandleTypeDef *pdev,
                                     uint8_t cfgidx);

static uint8_t  USBD_CUSTOM_HID_DeInit(USBD_HandleTypeDef *pdev,
                                       uint8_t cfgidx);

static uint8_t  USBD_CUSTOM_HID_Setup(USBD_HandleTypeDef *pdev,
                                      USBD_SetupReqTypedef *req);

static uint8_t  *USBD_CUSTOM_HID_GetFSCfgDesc(uint16_t *length);

static uint8_t  *USBD_CUSTOM_HID_GetHSCfgDesc(uint16_t *length);

static uint8_t  *USBD_CUSTOM_HID_GetOtherSpeedCfgDesc(uint16_t *length);

static uint8_t  *USBD_CUSTOM_HID_GetDeviceQualifierDesc(uint16_t *length);

static uint8_t  USBD_CUSTOM_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_CUSTOM_HID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_CUSTOM_HID_EP0_RxReady(USBD_HandleTypeDef  *pdev);

/** Usb HID report descriptor. */
__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE]  __ALIGN_END =//HID报文描述符
{
	//键盘
    0x05, 0x01,					//	USAGE_PAGE (Generic Desktop)
    0x09, 0x06,					//	USAGE (Keyboard)
    0xa1, 0x01,					//	COLLECTION (Application)
    0x85, 0x01,					//		REPORT_ID (1)
    0x05, 0x07,					//		USAGE_PAGE (Keyboard)
    0x19, 0xe0,					//		USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,					//		USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,					//		LOGICAL_MINIMUM (0)
    0x25, 0x01,					//		LOGICAL_MAXIMUM (1)
    0x75, 0x01,					//		REPORT_SIZE (1)
    0x95, 0x08,					//		REPORT_COUNT (8)
    0x81, 0x02,					//		INPUT (Data,Var,Abs)
    0x95, 0x01,					//		REPORT_COUNT (1)
    0x75, 0x08,					//		REPORT_SIZE (8)
    0x81, 0x03,					//		INPUT (Cnst,Var,Abs)
    0x95, 0x05,					//		REPORT_COUNT (5)
    0x75, 0x01,					//		REPORT_SIZE (1)
    0x05, 0x08,					//		USAGE_PAGE (LEDs)
    0x19, 0x01,					//		USAGE_MINIMUM (Num Lock)
    0x29, 0x05,					//		USAGE_MAXIMUM (Kana)
    0x91, 0x02,					//		OUTPUT (Data,Var,Abs)
    0x95, 0x01,					//		REPORT_COUNT (1)
    0x75, 0x03,					//		REPORT_SIZE (3)
    0x91, 0x03,					//		OUTPUT (Cnst,Var,Abs)
    0x95, 0x13,					//		REPORT_COUNT (19)按键数
    0x75, 0x08,					//		REPORT_SIZE (8)
    0x15, 0x00,					//		LOGICAL_MINIMUM (0)
    0x25, 0x65,					//		LOGICAL_MAXIMUM (101)
    0x05, 0x07,					//		USAGE_PAGE (Keyboard)
    0x19, 0x00,					//		USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,					//		USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,					//		INPUT (Data,Ary,Abs)
    0xc0,						//	END_COLLECTION
	
	//鼠标
    0x05, 0x01,					//	USAGE_PAGE (Generic Desktop)
    0x09, 0x02,					//	USAGE (Mouse)
    0xa1, 0x01,					//	COLLECTION (Application)
    0x85, 0x02,					//		REPORT_ID (2)
    0x09, 0x01,					//		USAGE (Pointer)
    0xa1, 0x00,					//		COLLECTION (Physical)
    0x05, 0x09,					//			USAGE_PAGE (Button)
    0x19, 0x01,					//			USAGE_MINIMUM (Button 1)
    0x29, 0x03,					//			USAGE_MAXIMUM (Button 3)
    0x15, 0x00,					//			LOGICAL_MINIMUM (0)
    0x25, 0x01,					//			LOGICAL_MAXIMUM (1)
    0x95, 0x03,					//			REPORT_COUNT (3)
    0x75, 0x01,					//			REPORT_SIZE (1)
    0x81, 0x02,					//			INPUT (Data,Var,Abs)
    0x95, 0x01,					//			REPORT_COUNT (1)
    0x75, 0x05,					//			REPORT_SIZE (5)
    0x81, 0x03,					//			INPUT (Cnst,Var,Abs)
    0x05, 0x01,					//			USAGE_PAGE (Generic Desktop)
    0x09, 0x30,					//			USAGE (X)
    0x09, 0x31,					//			USAGE (Y)
	0x09, 0x38,					//			USAGE (Wheel)	
    0x15, 0x81,					//			LOGICAL_MINIMUM (-127)
    0x25, 0x7f,					//			LOGICAL_MAXIMUM (127)
    0x75, 0x08,					//			REPORT_SIZE (8)
    0x95, 0x03,					//			REPORT_COUNT (3)
    0x81, 0x06,					//			INPUT (Data,Var,Rel)Rel相对值,Abs绝对值
    0xc0,						//		END_COLLECTION
    0xc0,						//	END_COLLECTION
	
	//指针位置
	0x05, 0x0d,					// USAGE_PAGE (Digitizers)
    //0x09, 0x02,					// USAGE (Pen)
	0x09, 0x04,					// USAGE (Touch Screen)
    0xa1, 0x01,					// COLLECTION (Application)
    0x85, 0x03,					//		REPORT_ID (3)
	0x09, 0x22,					//		USAGE (Finger)
    //0x09, 0x20,					//		USAGE (Stylus)
    0xa1, 0x00,					//		COLLECTION (Physical)
    0x09, 0x42,					//			USAGE (Tip Switch)
    0x09, 0x44,					//			USAGE (Barrel Switch)
    0x09, 0x3c,					//			USAGE (Invert)
    0x09, 0x45,					//			USAGE (Eraser Switch)
	0x09, 0x32,					//			USAGE (In Range)
    0x15, 0x00,					//			LOGICAL_MINIMUM (0)
    0x25, 0x01,					//			LOGICAL_MAXIMUM (1)
    0x75, 0x01,					//			REPORT_SIZE (1)
    0x95, 0x05,					//			REPORT_COUNT (5)
    0x81, 0x02,					//			INPUT (Data,Var,Abs)
    0x95, 0x01,					//			REPORT_COUNT (1)
    0x75, 0x03,					//			REPORT_SIZE (3)
    0x81, 0x03,					//			INPUT (Cnst,Var,Abs)
	0x75, 0x08,					//			REPORT_SIZE (8)
    0x09, 0x51,					//			USAGE (Contact Identifier)
    0x95, 0x01,					//			REPORT_COUNT (1)
    0x81, 0x02,					//			INPUT (Data,Var,Abs)
    0x05, 0x01,					//			USAGE_PAGE (Generic Desktop)
	0x75, 0x10,					//			REPORT_SIZE (16)
	0x26, 0xFF, 0x7F,			//			LOGICAL_MAXIMUM (32767)
	0x46, 0xFF, 0x7F,			//			PHYSICAL_MAXIMUM (32767)
    0x09, 0x30,					//			USAGE (X)
    0x09, 0x31,					//			USAGE (Y)
	0x95, 0x02,					//			REPORT_COUNT (2)
	0x81, 0x02,					//			INPUT (Data,Var,Abs)Rel相对值,Abs绝对值
    0xc0,						//		END_COLLECTION
    0xc0,						//	END_COLLECTION
	
	//媒体控制
	0x05,0x0C,					//	USAGE_PAGE (Consumer)
	0x09,0x01,					//	USAGE (Consumer Control)
	0xA1,0x01,					//	COLLECTION (Applicatior)
	0x85,0x04,					//		REPORT_ID (4)
	0xA1,0x00,					//		COLLECTION(Physical)
	0x09,0xE9,					//			USAGE (Volume Increment)
	0x09,0xEA,					//			USAGE (Volume Decrement)
	0x09,0xE2,					//			USAGE (Mute)
	0x09,0xCD,					//			USAGE (Play/Pause)
	0x09,0xB5,					//			USAGE (Scan Next Track)
	0x09,0xB6,					//			USAGE (Scan Previous Track)
	0x35,0x00,					//			PHYSICAL_MINIMUM (0)
	0x45,0x07,					//			PHYSICAL_MAXIMUM (7)
	0x15,0x00,					//			LOGICAL_MINIMUM (0)
	0x25,0x01,					//			LOCAL_MAXIMUM (1)
	0x75,0x01,					//			REPORT_SIZE (1)
	0x95,0x06,					//			REPORT_COUNT (6)
	0x81,0x02,					//			INPUT (Data,Var,Abs)
	0x75,0x01,					//			REPORT_SIZE (1)
	0x95,0x02,					//			REPORT_COUNT (2)
	0x81,0x01,					//			INPUT (Cnst,Ary,Abs)
	0xC0,						//		END_COLLECTION
	0xC0,						//	END_COLLECTION
};
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Variables
  * @{
  */

USBD_ClassTypeDef  USBD_CUSTOM_HID =
{
  USBD_CUSTOM_HID_Init,
  USBD_CUSTOM_HID_DeInit,
  USBD_CUSTOM_HID_Setup,
  NULL, /*EP0_TxSent*/
  USBD_CUSTOM_HID_EP0_RxReady, /*EP0_RxReady*/ /* STATUS STAGE IN */
  USBD_CUSTOM_HID_DataIn, /*DataIn*/
  USBD_CUSTOM_HID_DataOut,
  NULL, /*SOF */
  NULL,
  NULL,
  USBD_CUSTOM_HID_GetHSCfgDesc,
  USBD_CUSTOM_HID_GetFSCfgDesc,//获取全速配置描述符
  USBD_CUSTOM_HID_GetOtherSpeedCfgDesc,
  USBD_CUSTOM_HID_GetDeviceQualifierDesc,
};

/* USB CUSTOM_HID device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_CfgFSDesc[USB_CUSTOM_HID_CONFIG_DESC_SIZ] __ALIGN_END =//配置描述符
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_CUSTOM_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  USBD_MAX_NUM_INTERFACES,         /*bNumInterfaces: 2 interface*//*接口数量*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xA0,         /*bmAttributes: bus powered */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

  /*自定义HID部分*/
  /************** Descriptor of CUSTOM HID interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  USBD_CUSTOM_HID_INTERFACE,//0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: CUSTOM_HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of CUSTOM_HID *************************/
  /* 18 */
  0x09,         /*bLength: CUSTOM_HID Descriptor size*/
  CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
  0x11,         /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  USBD_CUSTOM_HID_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Custom HID endpoints ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  CUSTOM_HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  CUSTOM_HID_EPIN_SIZE, /*wMaxPacketSize: 64 Byte max */
  0x00,
  CUSTOM_HID_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */

  0x07,          /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
  CUSTOM_HID_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03, /* bmAttributes: Interrupt endpoint */
  CUSTOM_HID_EPOUT_SIZE,  /* wMaxPacketSize: 64 Bytes max  */
  0x00,
  CUSTOM_HID_FS_BINTERVAL,  /* bInterval: Polling Interval */
  /* 41 */
  
  /*HID部分*/
  /************** Descriptor of Joystick Mouse interface ****************/
  /* 41-9+09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  USBD_HID_INTERFACE,//0x01,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x01,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 41-9+18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Mouse endpoint ********************/
  /* 41-9+27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_EPIN_SIZE, /*wMaxPacketSize: 22 Byte max */
  0x00,
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 41-9+34 */
};

/* USB CUSTOM_HID device HS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_CfgHSDesc[USB_CUSTOM_HID_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_CUSTOM_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xC0,         /*bmAttributes: bus powered */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

  /************** Descriptor of CUSTOM HID interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: CUSTOM_HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of CUSTOM_HID *************************/
  /* 18 */
  0x09,         /*bLength: CUSTOM_HID Descriptor size*/
  CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
  0x11,         /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  USBD_CUSTOM_HID_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Custom HID endpoints ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  CUSTOM_HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  CUSTOM_HID_EPIN_SIZE, /*wMaxPacketSize: 2 Byte max */
  0x00,
  CUSTOM_HID_HS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */

  0x07,          /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
  CUSTOM_HID_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03, /* bmAttributes: Interrupt endpoint */
  CUSTOM_HID_EPOUT_SIZE,  /* wMaxPacketSize: 2 Bytes max  */
  0x00,
  CUSTOM_HID_HS_BINTERVAL,  /* bInterval: Polling Interval */
  /* 41 */
};

/* USB CUSTOM_HID device Other Speed Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_OtherSpeedCfgDesc[USB_CUSTOM_HID_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_CUSTOM_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xC0,         /*bmAttributes: bus powered */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

  /************** Descriptor of CUSTOM HID interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: CUSTOM_HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of CUSTOM_HID *************************/
  /* 18 */
  0x09,         /*bLength: CUSTOM_HID Descriptor size*/
  CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
  0x11,         /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  USBD_CUSTOM_HID_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Custom HID endpoints ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  CUSTOM_HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  CUSTOM_HID_EPIN_SIZE, /*wMaxPacketSize: 2 Byte max */
  0x00,
  CUSTOM_HID_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */

  0x07,          /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
  CUSTOM_HID_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03, /* bmAttributes: Interrupt endpoint */
  CUSTOM_HID_EPOUT_SIZE,  /* wMaxPacketSize: 2 Bytes max  */
  0x00,
  CUSTOM_HID_FS_BINTERVAL,  /* bInterval: Polling Interval */
  /* 41 */
};

/* USB CUSTOM_HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_Desc[USB_CUSTOM_HID_DESC_SIZ] __ALIGN_END =
{
  /* 18 */
  0x09,         /*bLength: CUSTOM_HID Descriptor size*/
  CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
  0x11,         /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  USBD_CUSTOM_HID_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         Initialize the CUSTOM_HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_HID_Init(USBD_HandleTypeDef *pdev,
                                     uint8_t cfgidx)
{
  uint8_t ret = 0U;
  USBD_CUSTOM_HID_HandleTypeDef     *hhid;

  /* Open EP IN */
  USBD_LL_OpenEP(pdev, CUSTOM_HID_EPIN_ADDR, USBD_EP_TYPE_INTR,
                 CUSTOM_HID_EPIN_SIZE);

  pdev->ep_in[CUSTOM_HID_EPIN_ADDR & 0xFU].is_used = 1U;

  /* Open EP OUT */
  USBD_LL_OpenEP(pdev, CUSTOM_HID_EPOUT_ADDR, USBD_EP_TYPE_INTR,
                 CUSTOM_HID_EPOUT_SIZE);

  pdev->ep_out[CUSTOM_HID_EPOUT_ADDR & 0xFU].is_used = 1U;
	
	/* Open HID EP OUT */
	USBD_LL_OpenEP(pdev, HID_EPIN_ADDR, USBD_EP_TYPE_INTR,HID_EPIN_SIZE);//启动HID的端点
	pdev->ep_in[HID_EPIN_ADDR & 0xFU].is_used = 1U;//该端点的使用标志位置位

  pCustomHidClassData = (USBD_CUSTOM_HID_HandleTypeDef *)USBD_malloc(sizeof(USBD_CUSTOM_HID_HandleTypeDef));//分配空间
//  pHidClassData = (USBD_HID_HandleTypeDef *)USBD_HID_malloc(sizeof(USBD_HID_HandleTypeDef));
//  pHidClassData->state = CUSTOM_HID_IDLE;
  pdev->pClassData = pCustomHidClassData;//让pClassData先指向CustomHID
  


  if (pdev->pClassData == NULL)
  {
    ret = 1U;
  }
  else
  {
    hhid = (USBD_CUSTOM_HID_HandleTypeDef *) pdev->pClassData;

    hhid->state = CUSTOM_HID_IDLE;
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->Init();

    /* Prepare Out endpoint to receive 1st packet */
    USBD_LL_PrepareReceive(pdev, CUSTOM_HID_EPOUT_ADDR, hhid->Report_buf,
                           USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
  }

  return ret;
}

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         DeInitialize the CUSTOM_HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_HID_DeInit(USBD_HandleTypeDef *pdev,
                                       uint8_t cfgidx)
{
  /* Close CUSTOM_HID EP IN */
  USBD_LL_CloseEP(pdev, CUSTOM_HID_EPIN_ADDR);
  pdev->ep_in[CUSTOM_HID_EPIN_ADDR & 0xFU].is_used = 0U;

  /* Close CUSTOM_HID EP OUT */
  USBD_LL_CloseEP(pdev, CUSTOM_HID_EPOUT_ADDR);
  pdev->ep_out[CUSTOM_HID_EPOUT_ADDR & 0xFU].is_used = 0U;
	
	/* Close HID EP OUT */
	USBD_LL_CloseEP(pdev, HID_EPIN_ADDR);//关闭HID的端点
	pdev->ep_in[HID_EPIN_ADDR & 0xFU].is_used = 0U;//该端点的使用标志位清零

  /* FRee allocated memory */
  if (pdev->pClassData != NULL)
  {
    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->DeInit();
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_Setup
  *         Handle the CUSTOM_HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_CUSTOM_HID_Setup(USBD_HandleTypeDef *pdev,
                                      USBD_SetupReqTypedef *req)
{
//  if((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE 
//	  && req->wIndex == USBD_HID_INTERFACE 
//	  || (req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_ENDPOINT 
//      && req->wIndex == HID_EPIN_ADDR){//若为HID
//	pdev->pClassData = pHidClassData;
//	pdev->pUserData = NULL;
//	return (USBD_HID.Setup(pdev, req));
//  }//否则为CustomHID
  pdev->pClassData = pCustomHidClassData;
  pdev->pUserData = &USBD_CustomHID_fops_FS;
  
  USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassData;
  uint16_t len = 0U;
  uint8_t  *pbuf = NULL;
  uint16_t status_info = 0U;
  uint8_t ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS :
      switch (req->bRequest)
      {
        case CUSTOM_HID_REQ_SET_PROTOCOL:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;

        case CUSTOM_HID_REQ_GET_PROTOCOL:
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->Protocol, 1U);
          break;

        case CUSTOM_HID_REQ_SET_IDLE:
          hhid->IdleState = (uint8_t)(req->wValue >> 8);
          break;

        case CUSTOM_HID_REQ_GET_IDLE:
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->IdleState, 1U);
          break;

        case CUSTOM_HID_REQ_SET_REPORT:
          hhid->IsReportAvailable = 1U;
          USBD_CtlPrepareRx(pdev, hhid->Report_buf, req->wLength);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if (req->wValue >> 8 == CUSTOM_HID_REPORT_DESC)//若要获取报文描述符
          {
			  if(req->wIndex == USBD_CUSTOM_HID_INTERFACE){//接口0使用自定义HID报文描述符
				len = MIN(USBD_CUSTOM_HID_REPORT_DESC_SIZE, req->wLength);
				pbuf = USBD_CustomHID_fops_FS.pReport; 
//				pbuf = ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->pReport; 
			  }
			  if(req->wIndex == USBD_HID_INTERFACE){//接口1使用HID报文描述符
				len = MIN(HID_MOUSE_REPORT_DESC_SIZE, req->wLength);
				pbuf = HID_MOUSE_ReportDesc;
			  }
//            len = MIN(USBD_CUSTOM_HID_REPORT_DESC_SIZE, req->wLength);
//            pbuf = ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->pReport;
          }
          else
          {
            if (req->wValue >> 8 == CUSTOM_HID_DESCRIPTOR_TYPE)
            {
              pbuf = USBD_CUSTOM_HID_Desc;
              len = MIN(USB_CUSTOM_HID_DESC_SIZ, req->wLength);
            }
          }

          USBD_CtlSendData(pdev, pbuf, len);
          break;

        case USB_REQ_GET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->AltSetting, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hhid->AltSetting = (uint8_t)(req->wValue);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }
  return ret;
}

/**
  * @brief  USBD_HID_SendReport
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport(USBD_HandleTypeDef  *pdev,
                            uint8_t *report,
                            uint16_t len)
{
  //USBD_HID_HandleTypeDef *hhid = pHidClassData;
	USBD_CUSTOM_HID_HandleTypeDef *hhid = pCustomHidClassData;
  
  if (pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if (hhid->state == CUSTOM_HID_IDLE)
    {
      hhid->state = CUSTOM_HID_BUSY;
      USBD_LL_Transmit(pdev, HID_EPIN_ADDR, report, len);
    }
    else
    {
      return USBD_BUSY;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_SendReport
  *         Send CUSTOM_HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_CUSTOM_HID_SendReport(USBD_HandleTypeDef  *pdev,
                                   uint8_t *report,
                                   uint16_t len)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid = pCustomHidClassData;//(USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassData;

  if (pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if (hhid->state == CUSTOM_HID_IDLE)
    {
      hhid->state = CUSTOM_HID_BUSY;
      USBD_LL_Transmit(pdev, CUSTOM_HID_EPIN_ADDR, report, len);
    }
    else
    {
      return USBD_BUSY;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_GetFSCfgDesc
  *         return FS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CUSTOM_HID_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CUSTOM_HID_CfgFSDesc);
  return USBD_CUSTOM_HID_CfgFSDesc;
}

/**
  * @brief  USBD_CUSTOM_HID_GetHSCfgDesc
  *         return HS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CUSTOM_HID_GetHSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CUSTOM_HID_CfgHSDesc);
  return USBD_CUSTOM_HID_CfgHSDesc;
}

/**
  * @brief  USBD_CUSTOM_HID_GetOtherSpeedCfgDesc
  *         return other speed configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CUSTOM_HID_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CUSTOM_HID_OtherSpeedCfgDesc);
  return USBD_CUSTOM_HID_OtherSpeedCfgDesc;
}

/**
  * @brief  USBD_CUSTOM_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_HID_DataIn(USBD_HandleTypeDef *pdev,
                                       uint8_t epnum)//数据上传函数
{
  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
	if(1/*epnum == (CUSTOM_HID_EPIN_ADDR & 0x0F)*/){//自定义HID上传
		pdev->pUserData = &USBD_CustomHID_fops_FS;
        pdev->pClassData = pCustomHidClassData;
		((USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassData)->state = CUSTOM_HID_IDLE;
	}
	else if(epnum == (HID_EPIN_ADDR & 0x0F)){//HID上传
//		pdev->pUserData = NULL;
//        pdev->pClassData = pHidClassData;
//		((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = CUSTOM_HID_IDLE;
	}
  
  return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_HID_DataOut(USBD_HandleTypeDef *pdev,
                                        uint8_t epnum)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid = pCustomHidClassData;//(USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassData;

//  ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->OutEvent(hhid->Report_buf[0],
//                                                            hhid->Report_buf[1]);
	USBD_CustomHID_fops_FS.OutEvent(hhid->Report_buf[0],hhid->Report_buf[1]);

  USBD_LL_PrepareReceive(pdev, CUSTOM_HID_EPOUT_ADDR, hhid->Report_buf,
                         USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);

  return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_CUSTOM_HID_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_CUSTOM_HID_HandleTypeDef *hhid = pCustomHidClassData;//(USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassData;

  if (hhid->IsReportAvailable == 1U)
  {
//    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->OutEvent(hhid->Report_buf[0],
//                                                              hhid->Report_buf[1]);
	  USBD_CustomHID_fops_FS.OutEvent(hhid->Report_buf[0],hhid->Report_buf[1]);
    hhid->IsReportAvailable = 0U;
  }

  return USBD_OK;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_CUSTOM_HID_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = sizeof(USBD_CUSTOM_HID_DeviceQualifierDesc);
  return USBD_CUSTOM_HID_DeviceQualifierDesc;
}

/**
* @brief  USBD_CUSTOM_HID_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CUSTOMHID Interface callback
  * @retval status
  */
uint8_t  USBD_CUSTOM_HID_RegisterInterface(USBD_HandleTypeDef   *pdev,
                                           USBD_CUSTOM_HID_ItfTypeDef *fops)
{
  uint8_t  ret = USBD_FAIL;

  if (fops != NULL)
  {
    pdev->pUserData = fops;
    ret = USBD_OK;
  }

  return ret;
}
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
