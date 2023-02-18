
#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "spdifrx.h"
/*
 * tasks.h
 *
 *  Created on: 20. jan. 2023
 *      Author: Mitja
 */

#ifndef INC_TASKS_H_
#define INC_TASKS_H_

extern osThreadId_t defaultTaskHandle; //Used for displaying VU Meter or something else to LCD
extern osThreadId_t starterTaskHandle; //Used to setup SPDIF, wait for signal disconnect
extern osThreadId_t playerTaskHandle;  //Used to feeding audio to SAI

extern SemaphoreHandle_t xSemaphoreAudioOUT;
extern SemaphoreHandle_t xSemaphoreLCD;

void StartDefaultTask(void *argument);
void starterTask(void *argument);
void playerTask(void *argument);

#endif /* INC_TASKS_H_ */
