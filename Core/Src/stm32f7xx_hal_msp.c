/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32f7xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
RCC_PeriphCLKInitTypeDef            PeriphClkInitStruct;
extern DMA_HandleTypeDef            hSaiDma;
extern DMA_HandleTypeDef            hdma_spdif_rx_dt;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* System interrupt init*/
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/* USER CODE BEGIN 1 */
/**
  * @brief  SAI MSP Init.
  * @param  hsai : pointer to a SAI_HandleTypeDef structure that contains
  *                the configuration information for SAI module.
  * @retval None
  */
void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
  GPIO_InitTypeDef  GPIO_Init;

  /* Enable SAI2 clock */
  __HAL_RCC_SAI2_CLK_ENABLE();

  /* Configure GPIOs used for SAI2 */
  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_NOPULL;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_Init.Alternate = GPIO_AF10_SAI2;
  GPIO_Init.Pin       = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOD, &GPIO_Init);

  /* Configure DMA used for SAI2 */
  __HAL_RCC_DMA2_CLK_ENABLE();

  if(hsai->Instance == SAI2_Block_A)
  //if(hsai->Instance ==  AUDIO_OUT_SAIx)
  {
    hSaiDma.Init.Channel             = DMA_CHANNEL_3;
    hSaiDma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hSaiDma.Init.PeriphInc           = DMA_PINC_DISABLE;
    hSaiDma.Init.MemInc              = DMA_MINC_ENABLE;
    hSaiDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hSaiDma.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hSaiDma.Init.Mode                = DMA_CIRCULAR;
    hSaiDma.Init.Priority            = DMA_PRIORITY_HIGH;
    hSaiDma.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    hSaiDma.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hSaiDma.Init.MemBurst            = DMA_MBURST_SINGLE;
    hSaiDma.Init.PeriphBurst         = DMA_PBURST_SINGLE;

    /* Select the DMA instance to be used for the transfer : DMA2_Stream4 */
    hSaiDma.Instance                 = DMA2_Stream4;

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmatx, hSaiDma);

    /* Deinitialize the Stream for new transfer */
    HAL_DMA_DeInit(&hSaiDma);

    /* Configure the DMA Stream */
    if (HAL_OK != HAL_DMA_Init(&hSaiDma))
    {
      Error_Handler();
    }
  }

  HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 13, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

  PeriphClkInitStruct.PLLSAI.PLLSAIN = 344;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 7;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
  PeriphClkInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);			//XXXX here the LCD jumps!!!
  __HAL_RCC_SAI2_CLK_ENABLE();
  HAL_NVIC_SetPriority(SAI2_IRQn, 14, 0);
  HAL_NVIC_EnableIRQ(SAI2_IRQn);
}

/**
* @brief SPDIFRX MSP Init
* @param hspdif: SPDIFRX handle
* @retval None
*/

void HAL_SPDIFRX_MspInit(SPDIFRX_HandleTypeDef *hspdif)
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_PeriphCLKInitTypeDef rcc_clkex;

  __HAL_RCC_SPDIFRX_CLK_ENABLE();

  rcc_clkex.PeriphClockSelection = RCC_PERIPHCLK_SPDIFRX;
  rcc_clkex.PLLI2S.PLLI2SN = 192;
  rcc_clkex.PLLI2S.PLLI2SP = RCC_PLLP_DIV2;
  rcc_clkex.PLLI2S.PLLI2SR = 2;
  rcc_clkex.PLLI2S.PLLI2SQ = 2;
  rcc_clkex.PLLI2SDivQ = 1;
  HAL_RCCEx_PeriphCLKConfig(&rcc_clkex);

  /* GPIOs Configuration */
  /* RX1   <->   PG12 */

  /*configure SPDIFRX_IN1 PG12 pin */

  /* Enable SPDIF GPIO IN */
  __HAL_RCC_GPIOG_CLK_ENABLE();

  GPIO_InitStructure.Pin       = GPIO_PIN_12;
  GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull      = GPIO_NOPULL;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate = GPIO_AF7_SPDIFRX;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* SPDIFRX DMA Init */
  /* SPDIF_RX_DT Init */
  hdma_spdif_rx_dt.Instance = DMA1_Stream1;
  hdma_spdif_rx_dt.Init.Channel = DMA_CHANNEL_0;
  hdma_spdif_rx_dt.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_spdif_rx_dt.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_spdif_rx_dt.Init.MemInc = DMA_MINC_ENABLE;
  hdma_spdif_rx_dt.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_spdif_rx_dt.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_spdif_rx_dt.Init.Mode = DMA_CIRCULAR;
  hdma_spdif_rx_dt.Init.Priority = DMA_PRIORITY_MEDIUM;
  hdma_spdif_rx_dt.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&hdma_spdif_rx_dt) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_LINKDMA(hspdif, hdmaDrRx, hdma_spdif_rx_dt);

  /* Enable SPDIF DMA */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 12, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

  /* Enable and set SPDIF Interrupt */
  HAL_NVIC_SetPriority(SPDIF_RX_IRQn, 13, 1);
  HAL_NVIC_EnableIRQ(SPDIF_RX_IRQn);
}
/* USER CODE END 1 */
