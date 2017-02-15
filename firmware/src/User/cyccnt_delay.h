#ifndef CYCCNT_DELAY_H
#define CYCCNT_DELAY_H

#include <stdlib.h>

#include "stm32f10x.h"

void DWT_Init(void);
void DWT_Delay(uint32_t us);   // microseconds

#endif
