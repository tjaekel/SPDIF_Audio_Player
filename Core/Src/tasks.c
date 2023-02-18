/*
 * tasks.c
 *
 *  Created on: 20. jan. 2023
 *      Author: Mitja
 */

#include <tasks.h>

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */

void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */

  /* this kills the LCD: the start of line jumps and is random! */
  SPDIFRX_Start();

  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 + 10, (uint8_t *)"Default task is working", LEFT_MODE);

  osThreadSetPriority(osThreadGetId(), osPriorityIdle);

  /* Infinite loop */
  for(;;)
  {
    osDelay(10);
  }
  /* USER CODE END 5 */
}

void starterTask(void *argument)
{
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 + 40, (uint8_t *)"Starter task is working", LEFT_MODE);

  /* if we do this - the LCD Task with lower prio does not come anymore */
  osThreadSetPriority(osThreadGetId(), osPriorityHigh);

  /* Infinite loop */
  for(;;)
  {
	  SPDIFRX_Loop();

	  //we should not come back: we loop inside this called function
      osDelay(1);
  }
  /* USER CODE END 5 */
}
void playerTask(void *argument)
{
  /* Infinite loop */
  for(;;)
  {
	  //Just because I have no idea how to declare struct in a header file and use it in spdifrx.c as well as tasks.c

	  //just something to do on LCD if new audio buffer was there
	  xSemaphoreTake(xSemaphoreLCD, portMAX_DELAY);
	  SPDIF_LCD();
  }
  /* USER CODE END 5 */
}
