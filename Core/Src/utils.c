#include "utils.h"
/*
 * utils.c
 *
 *  Created on: Jan 4, 2023
 *      Author: Mitja
 */


/**
* @brief  Compares two 16-bit buffers and returns the comparison result
* @param  pBuffer1: pointer to the first buffer.
* @param  pBuffer2: pointer to the second buffer.
* @retval
*    - 0: Comparison is OK (the two Buffers are identical)
*    - Value different from 0: Comparison is NOK (Buffers are different)
*/
int SPDIFBufferCmp(uint32_t* pBuffer1, uint16_t* pBuffer2, uint8_t size)
{
  while (size--)
  {
    if (((pBuffer1[size]&0x00ffff00) >>8) != (pBuffer2[size]))
    {
      return 1;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return 0;
}

void Generate1khzSineWave(int16_t* buffer, int size)
{
	float carrierFrequency = 1e3f; //carrier frequency (1khz)
	float amplitudeGain = 1.0f; // amplitude gain of the carrier frequency
	float samplingRate = 48e3f; // sampling rate of carrier generator (48khz)

    for (uint16_t i = 0; i < size; i++)
    {
        float sample = sin(2 * M_PI * i * (carrierFrequency / samplingRate)) * amplitudeGain;
        buffer[i * 2 + 0] = (int16_t)(sample * 32767);     //Left channel
        buffer[i * 2 + 1] = (int16_t)(sample * 32767);     //Right channel
    }
}
