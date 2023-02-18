/*
 * spdifrx.h
 *
 *  Created on: Dec 20, 2022
 *      Author: Mitja
 */

#ifndef INC_SPDIFRX_H_
#define INC_SPDIFRX_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*
 * Enum that will signal on which half of the the output buffer is free
 */

typedef enum
{
    BUFFER_OFFSET_NONE = 0,
    BUFFER_OFFSET_HALF = 1,
    BUFFER_OFFSET_FULL = 2,
} BUFFER_StateTypeDef;

typedef enum
{
    PLAYER_OK       = 0x00U,
	PLAYER_ERROR    = 0x01U,
	PLAYER_BUSY     = 0x02U,
	PLAYER_TIMEOUT  = 0x03U,
	PLAYING      = 0x04U,
	NO_SIGNAL    = 0x05U,
	UNKNOWN      = 0x06U
} Player_StateTypeDef;

typedef enum
{
	NOTINIT       = 0x00U,
	STOPPED       = 0x01U,
	STEADY        = 0x02U,
	RUNNING       = 0x03U
} SPDIF_States;

void HAL_SPDIFRX_MspInit(SPDIFRX_HandleTypeDef *hspdif);

/*
 * Exported functions
 */

HAL_StatusTypeDef BSP_SPDIFRX_Init(void);

void SPDIFRX_Start_IT(void);
int SPDIFRX_Start(void);
void  SPDIFRX_Loop(void);

void SPDIF_LCD(void);

#endif /* INC_SPDIFRX_H_ */
