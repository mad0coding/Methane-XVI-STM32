/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Methane-XVI Firmware V2.2
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
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_customhid.h"
#include "LL_convert.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;

extern uint8_t ifReceiving;//接收数据标志位
extern uint8_t savePlace;//存储位置
extern uint8_t saveMid;//存储中位标志

uint8_t USB_TX_BUF[64];//自定义HID发送缓冲
uint8_t USB_RX_BUF[64];//自定义HID接收缓冲

uint8_t KeyBrd_if_send = 0;//键盘报文是否发送
uint8_t Mouse_if_send = 0;//鼠标报文是否发送
uint8_t Point_if_send = 0;//指针报文是否发送
uint8_t Vol_if_send = 0;//媒体报文是否发送

uint8_t KeyBrd_data[KB_len] = {1,0,0,0};//编号1,功能键,保留0,其他按键
//功能键:bit7-bit0分别为为右win alt shift ctrl,左win alt shift ctrl

uint8_t Mouse_data[5] = {2,0,0,0,0};//编号2,功能键,x,y,滚轮
//功能键:bit0为左键,bit1为右键,bit2为中键,bit6为x是否溢出,bit7为y是否溢出

uint8_t Point_data[7] = {3,0x10,1,0,0,0,0};//编号3,功能键,id,x_L,x_H,y_L,y_H
//功能键:bit0为Tip Switch,bit1为Barrel Switch,bit2为Invert,bit3为Eraser Switch,bit4为In Range

uint8_t Vol_data[2] = {4,0};//编号4,功能键
//功能键:bit0音量加,bit1音量减,bit2静音,bit3播放暂停

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	LL_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
//  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
	  LL_Loop();
	  if(ifReceiving) continue;//若USB正在接收数据则跳过HID发送
	  if(savePlace){//需要更新参数
		  Update_para(savePlace - 1);
		  savePlace = 0;
//		  Light_handle(4,100);
		  continue;
	  }
	  if(saveMid){//需要更新摇杆中位等
		  Ana_mid_ctrl(1);//存储摇杆中位等
		  saveMid = 0;
		  continue;
	  }
	  
	  if(KeyBrd_if_send){
		  USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t*)&KeyBrd_data,sizeof(KeyBrd_data));
		  HAL_Delay(8);
	  }
	  if(Mouse_if_send){
		  USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t*)&Mouse_data,sizeof(Mouse_data));
		  HAL_Delay(8);
	  }
	  if(Point_if_send){
		  USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t*)&Point_data,sizeof(Point_data));
		  HAL_Delay(8);
	  }
	  if(Vol_if_send){
		  USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t*)&Vol_data,sizeof(Vol_data));
		  HAL_Delay(8);
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
