/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "oled.h"
#include "i2c_hal.h"
#include "math.h"
#include "string.h"
#include "stdlib.h"
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
uint8_t recvData[4] = {0};
float threshold = 0.1f; //��ֵ
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1)
	{
		threshold = atof((const char*)recvData);
		char buffer[50];  // �����㹻��Ļ���������ž�����ַ���
		sprintf(buffer, "Thresold: %.2f m\r\n", threshold);  // ��ʽ���������ݵ��ַ���
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);  // ���͵�����
		//��������
		HAL_UART_Receive_IT(&huart1,recvData,sizeof(recvData)-1);
	}
}
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM7_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  IIC_Init();
  OLED_Init();

  OLED_Clear();
	
	OLED_ShowString(0,0, (uint8_t *)"Hello World!", 15);
	OLED_ShowString(0,10, (uint8_t *)"Festival Happy!", 15);
	OLED_ShowString(0,12, (uint8_t *)"Weekend Happy!", 15);
	OLED_ShowString(0,14, (uint8_t *)"Week Happy!", 15);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_UART_Receive_IT(&huart1,recvData,sizeof(recvData)-1);
  while (1)
  {
		// 1���ö�ʱ��CNT����������
		__HAL_TIM_SET_COUNTER(&htim1,0);
		// 2�����CCR1��CCR2��־λ
		__HAL_TIM_CLEAR_FLAG(&htim1,TIM_FLAG_CC1);
		__HAL_TIM_CLEAR_FLAG(&htim1,TIM_FLAG_CC2);
		
		// 3��������ʱ��
		HAL_TIM_IC_Start(&htim1,TIM_CHANNEL_1);
		HAL_TIM_IC_Start(&htim1,TIM_CHANNEL_2);
		
		// 4����������
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);
		for(uint16_t idx  = 0;idx <= 2000;idx++); //����11��ָ������ 
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
		
		// 5���ȴ��������� - ��ѯ����־λ
		uint8_t CC1Flag = 0;
		uint8_t IsSuccess = 0;
		uint32_t endTime = HAL_GetTick() + 50; //д50��Ϊ��ȷ������38ms
		while( endTime > HAL_GetTick() )
		{
			//���CC1��CC2�ı�־λ
			if(__HAL_TIM_GET_FLAG(&htim1,TIM_FLAG_CC1) == 1 && CC1Flag != 1)
			{
				CC1Flag = 1;
			}
			if(__HAL_TIM_GET_FLAG(&htim1,TIM_FLAG_CC2) == 1 && CC1Flag == 1)
			{
				//�����ɹ�
				IsSuccess = 1;
				//��������
				break;
			}
		}
		
		// 6���رն�ʱ��
		HAL_TIM_IC_Stop(&htim1,TIM_CHANNEL_1);
		HAL_TIM_IC_Stop(&htim1,TIM_CHANNEL_2);
		
		// 7���������
		if(IsSuccess == 1)
		{
			uint16_t ccr1 = __HAL_TIM_GET_COMPARE(&htim1,TIM_CHANNEL_1);
			uint16_t ccr2 = __HAL_TIM_GET_COMPARE(&htim1,TIM_CHANNEL_2);
			
			float pulseWidth = (ccr2 - ccr1) * 1e-6f; //������ȣ��룩
			float distance = (pulseWidth * 340.0f) / 2.0f;
//			uint16_t distance = (pulseWidth * 34000) / 2;
			if(distance < threshold)
			{
				char buffer[50];  // �����㹻��Ļ���������ž�����ַ���
				sprintf(buffer, "Distance: %.2f m\r\n", distance);  // ��ʽ���������ݵ��ַ���
				HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);  // ���͵�����
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
			}
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */