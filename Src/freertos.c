/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "semphr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#include "usart.h"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern volatile uint16_t adc_buf;
extern volatile uint8_t  conv_flag;


/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId buttonHandlerHandle;
osThreadId displayTaskHandle;
osThreadId buttonPollerHandle;
osSemaphoreId xBinarySemaphoreHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartButtonTask(void const * argument);
void StartDisplayTask(void const * argument);
void StartPollerTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of xBinarySemaphore */
  osSemaphoreDef(xBinarySemaphore);
  xBinarySemaphoreHandle = osSemaphoreCreate(osSemaphore(xBinarySemaphore), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of buttonHandler */
  osThreadDef(buttonHandler, StartButtonTask, osPriorityHigh, 0, 128);
  buttonHandlerHandle = osThreadCreate(osThread(buttonHandler), NULL);

  /* definition and creation of displayTask */
  osThreadDef(displayTask, StartDisplayTask, osPriorityNormal, 0, 128);
  displayTaskHandle = osThreadCreate(osThread(displayTask), NULL);

  /* definition and creation of buttonPoller */
  osThreadDef(buttonPoller, StartPollerTask, osPriorityNormal, 0, 128);
  buttonPollerHandle = osThreadCreate(osThread(buttonPoller), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
	HAL_GPIO_TogglePin(LED4_GPIO_Port,LED4_Pin);
    osDelay(500);
  }
	vTaskDelete(NULL);
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartButtonTask */
/**
* @brief Function implementing the buttonTask thread.
* @param argument: Not used
* @retval None
*/
/*Задача 1 - обработка нажатия кнопки*/
/* USER CODE END Header_StartButtonTask */
void StartButtonTask(void const * argument)
{
  /* USER CODE BEGIN StartButtonTask */
	
	osSemaphoreWait(xBinarySemaphoreHandle, osWaitForever);
  /* Infinite loop */
  for(;;)
  {		
			osSemaphoreWait(xBinarySemaphoreHandle, osWaitForever);
			HAL_UART_Transmit(&huart3,(uint8_t*)"Semaphore2!\n",12,1000);
			 			
			taskENTER_CRITICAL();/*Критическая секция выполняемого кода*/			
			HAL_GPIO_TogglePin(SWLED_GPIO_Port,SWLED_Pin);
			HAL_UART_Transmit(&huart3,(uint8_t*)"ATTENTION!\n",11,1000);
			taskEXIT_CRITICAL();	
  }
	vTaskDelete(NULL);
  /* USER CODE END StartButtonTask */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the displayTask thread.
* @param argument: Not used
* @retval None
*/
/*Задача 2 - обработка результата преобразования и его отправка на индикатор*/
		
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void const * argument)
{
  /* USER CODE BEGIN StartDisplayTask */
	float adcVoltage = 0;
	const static float adcStep = 3.27/4096;/*vdd/2^12*/
	char adcStrVoltage[16];
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    if (conv_flag == 1)
		{
			adcVoltage = adcStep * (float)adc_buf;
			displayFloat(adcVoltage,3,1);
			//snprintf(adcStrVoltage,sizeof(adcStrVoltage),"CH1: %.3f V\n",adcVoltage);
			//HAL_UART_Transmit(&huart3,(uint8_t*)adcStrVoltage,strlen(adcStrVoltage),1000);
			conv_flag = 0;
			osDelayUntil(&xLastWakeTime,200/portTICK_RATE_MS);/*Период опроса - 200 мс*/
		}
  }
	vTaskDelete(NULL);
  /* USER CODE END StartDisplayTask */
}

/* USER CODE BEGIN Header_StartPollerTask */
/**
* @brief Function implementing the buttonPoller thread.
* @param argument: Not used
* @retval None
*/
/*Задача 3 - опрос порта GPIO, на котором висит кнопка*/
/* USER CODE END Header_StartPollerTask */
void StartPollerTask(void const * argument)
{
  /* USER CODE BEGIN StartPollerTask */
	uint8_t flag = 1;
	portTickType press_time = 0;

  /* Infinite loop */
  for(;;)
  {
		if(HAL_GPIO_ReadPin(Button_Pin_GPIO_Port, Button_Pin_Pin) == GPIO_PIN_RESET && flag) 
		{
			flag = 0;
			press_time = xTaskGetTickCount();
		}
		if(!flag && (xTaskGetTickCount() - press_time) > 300)
		{
			flag = 1;
			xSemaphoreGive(xBinarySemaphoreHandle);/*Если кнопка нажата более чем 300 мс, то отдаём семафор*/
		}
  }
  /* USER CODE END StartPollerTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
