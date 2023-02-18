//To help you implement in RTOS, look here: https://www.digikey.com/en/maker/projects/introduction-to-rtos-solution-to-part-9-hardware-interrupts/3ae7a68462584e1eb408e1638002e9ed
/*
 * spdifrx.c
 *
 *  Created on: Dec 20, 2022
 *      Author: Mitja
 */

#include "spdifrx.h"
#include <string.h>
#include <stdbool.h>
#include <tasks.h>
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_audio.h"


//check linker script for enough malloc space, here as 1-byte words!

// amount of SPDIF sub-frame per block
#define AUDIO_FRAMES		80
#define SPDIF_SAMPLE_NUM  	(48 * 2 * AUDIO_FRAMES)

SAI_HandleTypeDef           hsai;
DMA_HandleTypeDef           hSaiDma;
SPDIFRX_HandleTypeDef       SpdifrxHandle;
DMA_HandleTypeDef           hdma_spdif_rx_dt;

volatile int audio_out_buffer_state = 0;
volatile int audio_in_buffer_state = 0;

size_t spdif_tx_prev_seq_num = 0, spdif_rx_prev_seq_num = 0;
static int SPDIFState = 0;

// SPDIF RX context
struct {
  uint32_t buf[SPDIF_SAMPLE_NUM * 2];		//DoubleBuffer
  const uint32_t *readable_buf;
  size_t seq_num;
  bool active;
} spdif_rx
__attribute__((section(".dtcm")))
= { {0}, NULL, 0, false }
;

HAL_StatusTypeDef BSP_SPDIFRX_Init(void)
{
	HAL_StatusTypeDef errCode;

	/* Configure SPDIFRX Peripheral */
	memset(&SpdifrxHandle, 0, sizeof(SpdifrxHandle));
	SpdifrxHandle.Instance = SPDIFRX;
	HAL_SPDIFRX_DeInit(&SpdifrxHandle);

	SpdifrxHandle.Init.InputSelection = SPDIFRX_INPUT_IN1;
	//SpdifrxHandle.Init.Retries = SPDIFRX_MAXRETRIES_63;
	SpdifrxHandle.Init.Retries = SPDIFRX_MAXRETRIES_NONE;
	SpdifrxHandle.Init.WaitForActivity = SPDIFRX_WAITFORACTIVITY_ON;	//SPDIFRX_WAITFORACTIVITY_OFF;	//SPDIFRX_WAITFORACTIVITY_ON;
	SpdifrxHandle.Init.ChannelSelection = SPDIFRX_CHANNEL_A;
	SpdifrxHandle.Init.DataFormat = SPDIFRX_DATAFORMAT_32BITS;
	SpdifrxHandle.Init.StereoMode = SPDIFRX_STEREOMODE_ENABLE;
	/*
	SpdifrxHandle.Init.PreambleTypeMask = SPDIFRX_PREAMBLETYPEMASK_ON;
	SpdifrxHandle.Init.ChannelStatusMask = SPDIFRX_CHANNELSTATUS_ON;
	SpdifrxHandle.Init.ValidityBitMask 	= SPDIFRX_VALIDITYMASK_OFF;
	SpdifrxHandle.Init.ParityErrorMask 	= SPDIFRX_PARITYERRORMASK_OFF;
	*/
	SpdifrxHandle.Init.PreambleTypeMask = SPDIFRX_PREAMBLETYPEMASK_OFF;
	SpdifrxHandle.Init.ChannelStatusMask = SPDIFRX_CHANNELSTATUS_ON;
	SpdifrxHandle.Init.ValidityBitMask 	= SPDIFRX_VALIDITYMASK_OFF;	//SPDIFRX_VALIDITYMASK_ON;	//SPDIFRX_VALIDITYMASK_OFF;
	SpdifrxHandle.Init.ParityErrorMask 	= SPDIFRX_PARITYERRORMASK_OFF;

	/*
	 * Call the HAL init function
	 */
	errCode = HAL_SPDIFRX_Init(&SpdifrxHandle);

	HAL_NVIC_SetPriority(SPDIF_RX_IRQn, 12, 0);
	HAL_NVIC_EnableIRQ(SPDIF_RX_IRQn);

	////__HAL_SPDIFRX_ENABLE_IT(&SpdifrxHandle, SPDIFRX_IT_SYNCDIE);

	return errCode;
}

void AudioState(void)
{
	HAL_StatusTypeDef err;

	static size_t prev_spdif_seq_num = 0;

	if (SPDIFState == 2)
	{
		if (spdif_rx.seq_num == prev_spdif_seq_num)
		{
#if 1
			memset(spdif_rx.buf, 0, sizeof(spdif_rx.buf));
			SPDIFState = 1;		//STOPPED
#endif
		}
	}
	else if (SPDIFState == 1)
	{
		//try to start SPDIF again
		do {
			BSP_SPDIFRX_Init();
			while (audio_out_buffer_state != BUFFER_OFFSET_HALF) { ; }
			err = HAL_SPDIFRX_ReceiveDataFlow_DMA(&SpdifrxHandle, spdif_rx.buf, sizeof(spdif_rx.buf) / 4);
			if (err != HAL_OK)
			BSP_LED_On(LED1);
		} while (err != HAL_OK);

		//give it at least one OUT cycle to be stable on SPDIF IN
		SPDIFState = 3;			//RUNNING
	}
	else if (SPDIFState > 2)
	{
		//count down after running again
		SPDIFState--;			//RUNNING mode
	}

	prev_spdif_seq_num = spdif_rx.seq_num;
}

/*
 * SPDIF RX: DMA half completed
 */
void HAL_SPDIFRX_RxHalfCpltCallback(SPDIFRX_HandleTypeDef *ctx)
{
  //With this, always the right half of the buffer will be uvailable to be read
  spdif_rx.seq_num++;
  audio_in_buffer_state = BUFFER_OFFSET_HALF;
}

/*
 * SPDIF RX: DMA full completed
 */
void HAL_SPDIFRX_RxCpltCallback(SPDIFRX_HandleTypeDef *ctx)
{
  //With this, always the right half of the buffer will be uvailable to be read
  spdif_rx.seq_num++;
  audio_in_buffer_state = BUFFER_OFFSET_FULL;

  BSP_LED_Toggle(LED2);
}

void HAL_SPDIFRX_ErrorCallback(SPDIFRX_HandleTypeDef *ctx)
{
    //red LED should never come, this error should not happen!
    BSP_LED_Toggle(LED1);
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
	BaseType_t xHigherPriorityTaskWokenAudioOUT;
	xHigherPriorityTaskWokenAudioOUT = pdTRUE;

	//BSP_LED_Toggle(LED1);
	//We are on second half of DMA buffer
	//BSP_LED_Toggle(LED2);
	audio_out_buffer_state = BUFFER_OFFSET_HALF;

	xSemaphoreGiveFromISR(xSemaphoreAudioOUT, &xHigherPriorityTaskWokenAudioOUT);
	portYIELD_FROM_ISR(xHigherPriorityTaskWokenAudioOUT);
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
	BaseType_t xHigherPriorityTaskWokenAudioOUT;
	xHigherPriorityTaskWokenAudioOUT = pdTRUE;

	//We are on first half of the buffer or did transmission finish, idk??
	audio_out_buffer_state = BUFFER_OFFSET_FULL;

	xSemaphoreGiveFromISR(xSemaphoreAudioOUT, &xHigherPriorityTaskWokenAudioOUT);
	portYIELD_FROM_ISR(xHigherPriorityTaskWokenAudioOUT);
}

/**
  * @brief  Clock Config.
  * @param  hsai: might be required to set audio peripheral predivider if any.
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @note   This API is called by BSP_AUDIO_OUT_Init() and BSP_AUDIO_OUT_SetFrequency()
  *         Being __weak it can be overwritten by the application
  * @retval None
  */
void BSP_AUDIO_OUT_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t AudioFreq, void *Params)
{
  RCC_PeriphCLKInitTypeDef RCC_ExCLKInitStruct;

  HAL_RCCEx_GetPeriphCLKConfig(&RCC_ExCLKInitStruct);

  /* Set the PLL configuration according to the audio frequency */
  if((AudioFreq == AUDIO_FREQUENCY_11K) || (AudioFreq == AUDIO_FREQUENCY_22K) || (AudioFreq == AUDIO_FREQUENCY_44K))
  {
    /* Configure PLLSAI prescalers */
    /* PLLI2S_VCO: VCO_429M
    SAI_CLK(first level) = PLLI2S_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
    SAI_CLK_x = SAI_CLK(first level)/PLLI2SDivQ = 214.5/19 = 11.289 Mhz */

    RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
    RCC_ExCLKInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLI2S;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SP = 8;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = 429;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SQ = 2;
    RCC_ExCLKInitStruct.PLLI2SDivQ = 19;

    HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
  }
  else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K), AUDIO_FREQUENCY_96K */
  {
    /* SAI clock config
    PLLI2S_VCO: VCO_344M
    SAI_CLK(first level) = PLLI2S_VCO/PLLSAIQ = 344/7 = 49.142 Mhz
    SAI_CLK_x = SAI_CLK(first level)/PLLI2SDivQ = 49.142/1 = 49.142 Mhz */
    RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
    RCC_ExCLKInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLI2S;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SP = 8;
    //TJ: this is correct and exact 48 KHz - trim it exactly for 48 KHz!
    //fine tune on N value: 432 is actually max.! - but it seems to work best
    /*
     * trim it so that we see a stable red LED off or on, this happens just after a while
     */
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = 432;	//432;	//335;	//336   //384;	//was 344   N is multiplier
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SQ = 9;		//9;	//7;			//8;	//was 7		Q is divider
    RCC_ExCLKInitStruct.PLLI2SDivQ = 1;

    HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
  }
}

void SPDIFRX_Loop(void)
{
	xSemaphoreTake(xSemaphoreAudioOUT, portMAX_DELAY);

	if (audio_in_buffer_state == audio_out_buffer_state )
	{
		//set a breakpoint here: it should not be trigger, maybe after a while it is stable!
		BSP_LED_Toggle(LED1);
	}

	AudioState();

	xSemaphoreGive(xSemaphoreLCD);
}

int SPDIFRX_Start(void)
{
		uint8_t ret = HAL_OK;
		HAL_StatusTypeDef err;

		/* INIT AUDIO */
#if 1
		ret = BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE /*OUTPUT_DEVICE_AUTO*/, 90, 48000);
#endif

		/* Update the Audio frame slot configuration to match the PCM standard instead of TDM */
		BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);

		memset(spdif_rx.buf, 0, sizeof(spdif_rx.buf));			//Null samples as default
		BSP_AUDIO_OUT_Play((uint16_t *)spdif_rx.buf, sizeof(spdif_rx.buf));

		/* start both, IN and OUT: but IN must be there: we release OUT when 1st Half on SPDIF received */
		do {
			BSP_SPDIFRX_Init();
			while (audio_out_buffer_state != BUFFER_OFFSET_HALF) { ; }
			err = HAL_SPDIFRX_ReceiveDataFlow_DMA(&SpdifrxHandle, spdif_rx.buf, sizeof(spdif_rx.buf) / 4);
			if (err != HAL_OK)
				BSP_LED_On(LED1);
		} while (err != HAL_OK);

		SPDIFState = 2;		//RUNNING

		if (ret != HAL_OK)
		{
			/* Initialization error */
			return ret;
		}

#if 0
		//lets split: we do this loop in thread
		while (1)
		{
		  	  SPDIFRX_Loop();
		}
#endif
		return ret;
}

//Just because I have no idea how to declare struct in a header file and use it in spdifrx.c as well as tasks.c
void SPDIF_LCD(void)
{
	  char str[80];				//be careful with stack size
	  sprintf(str, "SPDIF Sequence number: %lu", (unsigned long)spdif_rx.seq_num);
	  BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)str, LEFT_MODE);
}

