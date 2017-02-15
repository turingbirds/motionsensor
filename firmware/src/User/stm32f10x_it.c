#include "stm32f10x_it.h"

// X = np.tanh(np.linspace(-2., 0., 64)) / 2. + 0.5; X = X[::-1]; X /= X[0] / 20; X = np.array(X, dtype=np.int); X[0] = 63; X
static const uint8_t ACTIVITY_LED_BLINK_PERIOD_LOOKUP_TABLE[64] = {63, 19, 18, 18, 17, 16, 16, 15, 15, 14, 13, 13, 12, 12, 11, 11, 10,
																   10,  9,  9,  8,  8,  7,  7,  7,  6,  6,  6,  5,  5,  5,  4,  4,  4,
																   4,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  1,  1,  1,  1,
																   1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0
																  };


/**
 *	logging
**/

extern volatile uint8_t log_seq_num;
extern volatile uint32_t log_integration_window;
extern volatile uint32_t log_integration_window_counter;
extern volatile uint16_t log_signal_level;
extern volatile uint16_t prev_log_signal_level;
extern volatile uint32_t usb_vendor_command_buf;



/*uint8_t  sin_tbl[256] = {
  0x80, 0x83, 0x86, 0x89, 0x8C, 0x90, 0x93, 0x96,
  0x99, 0x9C, 0x9F, 0xA2, 0xA5, 0xA8, 0xAB, 0xAE,
  0xB1, 0xB3, 0xB6, 0xB9, 0xBC, 0xBF, 0xC1, 0xC4,
  0xC7, 0xC9, 0xCC, 0xCE, 0xD1, 0xD3, 0xD5, 0xD8,
  0xDA, 0xDC, 0xDE, 0xE0, 0xE2, 0xE4, 0xE6, 0xE8,
  0xEA, 0xEB, 0xED, 0xEF, 0xF0, 0xF1, 0xF3, 0xF4,
  0xF5, 0xF6, 0xF8, 0xF9, 0xFA, 0xFA, 0xFB, 0xFC,
  0xFD, 0xFD, 0xFE, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFD,
  0xFD, 0xFC, 0xFB, 0xFA, 0xFA, 0xF9, 0xF8, 0xF6,
  0xF5, 0xF4, 0xF3, 0xF1, 0xF0, 0xEF, 0xED, 0xEB,
  0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC,
  0xDA, 0xD8, 0xD5, 0xD3, 0xD1, 0xCE, 0xCC, 0xC9,
  0xC7, 0xC4, 0xC1, 0xBF, 0xBC, 0xB9, 0xB6, 0xB3,
  0xB1, 0xAE, 0xAB, 0xA8, 0xA5, 0xA2, 0x9F, 0x9C,
  0x99, 0x96, 0x93, 0x90, 0x8C, 0x89, 0x86, 0x83,
  0x80, 0x7D, 0x7A, 0x77, 0x74, 0x70, 0x6D, 0x6A,
  0x67, 0x64, 0x61, 0x5E, 0x5B, 0x58, 0x55, 0x52,
  0x4F, 0x4D, 0x4A, 0x47, 0x44, 0x41, 0x3F, 0x3C,
  0x39, 0x37, 0x34, 0x32, 0x2F, 0x2D, 0x2B, 0x28,
  0x26, 0x24, 0x22, 0x20, 0x1E, 0x1C, 0x1A, 0x18,
  0x16, 0x15, 0x13, 0x11, 0x10, 0x0F, 0x0D, 0x0C,
  0x0B, 0x0A, 0x08, 0x07, 0x06, 0x06, 0x05, 0x04,
  0x03, 0x03, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03,
  0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x0A,
  0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11, 0x13, 0x15,
  0x16, 0x18, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24,
  0x26, 0x28, 0x2B, 0x2D, 0x2F, 0x32, 0x34, 0x37,
  0x39, 0x3C, 0x3F, 0x41, 0x44, 0x47, 0x4A, 0x4D,
  0x4F, 0x52, 0x55, 0x58, 0x5B, 0x5E, 0x61, 0x64,
  0x67, 0x6A, 0x6D, 0x70, 0x74, 0x77, 0x7A, 0x7D
};*/

volatile uint16_t adc_word;


volatile uint8_t sin_tbl_idx = 0;

volatile uint8_t inside_usb_hp_irq = 0;
volatile uint8_t inside_usb_lp_irq = 0;


volatile	uint8_t bitnum, adc_spi_phase;

volatile uint8_t dumbledore = 0;
volatile uint8_t slytherin = 0;
volatile uint8_t shenanigans = 0;
volatile uint16_t shebang = 0;
volatile uint8_t ledtoggle = 0;
volatile uint8_t alarm_state_machine = 0;
volatile uint8_t ghostbusters = 0;

extern volatile int16_t adc_dc_offset;

extern volatile uint32_t _VOLUME_DATA;
extern volatile uint32_t MUTE_DATA;
volatile uint8_t throw_alarm;
extern volatile uint8_t alarm_enable;

volatile uint8_t lvl;
volatile uint8_t count_dir;
volatile uint8_t blink_period;
volatile uint32_t slowdown;
volatile uint32_t slowdown2;
volatile uint32_t slowdown3;
volatile uint8_t blink_phase;

extern const uint32_t MIN_DATA;
extern const uint32_t MAX_DATA;

#define DUMMYDATA  0x0001
#define BUTTON_TEST1 20
#define BUTTON_TEST2 160

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void) {
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void) {
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1) {
	}
}

// From Joseph Yiu, minor edits by FVH
// hard fault handler in C,
// with stack frame location as input parameter
// called from HardFault_Handler in file xxx.s
void hard_fault_handler_c(unsigned int * hardfault_args) {
	unsigned int stacked_r0;
	unsigned int stacked_r1;
	unsigned int stacked_r2;
	unsigned int stacked_r3;
	unsigned int stacked_r12;
	unsigned int stacked_lr;
	unsigned int stacked_pc;
	unsigned int stacked_psr;

	stacked_r0 = ((unsigned long) hardfault_args[0]);
	stacked_r1 = ((unsigned long) hardfault_args[1]);
	stacked_r2 = ((unsigned long) hardfault_args[2]);
	stacked_r3 = ((unsigned long) hardfault_args[3]);

	stacked_r12 = ((unsigned long) hardfault_args[4]);
	stacked_lr = ((unsigned long) hardfault_args[5]);
	stacked_pc = ((unsigned long) hardfault_args[6]);
	stacked_psr = ((unsigned long) hardfault_args[7]);

	while (1);
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void) {
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1) {
	}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void) {
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1) {
	}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void) {
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1) {
	}
}

/*******************************************************************************
* Function Name  : SVC_Handler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVC_Handler(void) {
}

/*******************************************************************************
* Function Name  : DebugMon_Handler
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMon_Handler(void) {
}

/*******************************************************************************
* Function Name  : PendSV_Handler
* Description    : This function handles PendSV_Handler exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSV_Handler(void) {
}

/*******************************************************************************
* Function Name  : SysTick_Handler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_Handler(void) {
}


/*******************************************************************************
* Function Name  : WWDG_IRQHandler
* Description    : This function handles WWDG interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WWDG_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : PVD_IRQHandler
* Description    : This function handles PVD interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PVD_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : TAMPER_IRQHandler
* Description    : This function handles Tamper interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TAMPER_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles RTC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : FLASH_IRQHandler
* Description    : This function handles Flash interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : RCC_IRQHandler
* Description    : This function handles RCC interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : EXTI0_IRQHandler
* Description    : This function handles External interrupt Line 0 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI0_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : EXTI1_IRQHandler
* Description    : This function handles External interrupt Line 1 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI1_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : EXTI2_IRQHandler
* Description    : This function handles External interrupt Line 2 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI2_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : EXTI3_IRQHandler
* Description    : This function handles External interrupt Line 3 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI3_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External interrupt Line 4 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : DMA1_Channel1_IRQHandler
* Description    : This function handles DMA1 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel1_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : DMA1_Channel2_IRQHandler
* Description    : This function handles DMA1 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel2_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : DMA1_Channel3_IRQHandler
* Description    : This function handles DMA1 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel3_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : DMA1_Channel4_IRQHandler
* Description    : This function handles DMA1 Channel 4 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel4_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : DMA1_Channel5_IRQHandler
* Description    : This function handles DMA1 Channel 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel5_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : DMA1_Channel6_IRQHandler
* Description    : This function handles DMA1 Channel 6 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel6_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : DMA1_Channel7_IRQHandler
* Description    : This function handles DMA1 Channel 7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel7_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : ADC1_2_IRQHandler
* Description    : This function handles ADC1 and ADC2 global interrupt request.
* Input          : None
* Output         : None
* N.B. this IRQ service routine cannot be interrupted as there are no other IRQ
	source with higher priority
* HIGHEST PRIORITY INTERRUPT - KEEP IT LEAN OR USB WILL CRASH
*******************************************************************************/
void ADC1_2_IRQHandler(void) {
	if (ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
	}
//#ifdef READ_SENSITIVITY_LEVEL_IN_HIGH_PRIO_ISR_IS_BAD_IDEA
	if (ADC_GetITStatus(ADC2, ADC_IT_EOC)) {
		// read alarm level from ADC2
		/*if (slytherin == 0) {
			slytherin = 1;
		}
		else {
			slytherin = 0;
		}*/
		//LED2(slitherin);

		alarm_lvl = ADC2->DR << 4;		// convert to uint16_t
		/*if (alarm_lvl > 127) {

		}
		else {
			LED2(0);
		}*/
		ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);
	}
//#else
//		ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);

//#endif
}

/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void) {
	//inside_usb_hp_irq = 1;
	//GPIO_SetBits(LED_ALARM_PORT, LED_ALARM_PIN);
	CTR_HP();
	//inside_usb_hp_irq = 0;
	//GPIO_ResetBits(LED_ALARM_PORT, LED_ALARM_PIN);
}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void) {
	inside_usb_lp_irq = 1;
	USB_LP_IRQHandler();
	inside_usb_lp_irq = 0;
}

/*******************************************************************************
* Function Name  : CAN_RX1_IRQHandler
* Description    : This function handles CAN RX1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_RX1_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : CAN_SCE_IRQHandler
* Description    : This function handles CAN SCE interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_SCE_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI9_5_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : TIM1_BRK_IRQHandler
* Description    : This function handles TIM1 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_BRK_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : TIM1_UP_IRQHandler
* Description    : This function handles TIM1 overflow and update interrupt
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_UP_IRQHandler(void) {

}

/*******************************************************************************
* Function Name  : TIM1_TRG_COM_IRQHandler
* Description    : This function handles TIM1 Trigger and commutation interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_TRG_COM_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : TIM1_CC_IRQHandler
* Description    : This function handles TIM1 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_CC_IRQHandler(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	uint16_t _tmp, d;


	/**
	 *	read new alarm trigger level
	**/
	alarm_lvl = read_alarm_setlevel();


	/**
	 *	if connected via USB, set frontend potentiometer according to volume setting
	**/
	if (~(bDeviceState == ATTACHED || bDeviceState == UNCONNECTED)) {
		d = VOLUME_DATA;
		if (_tmp & 0x8000) {
			// if sign bit is set, convert to absolute value
			d = ~d;
			++d;
			d = 0x1000 - d;
		} else {
			// higher (positive) end of the range; add offset
			d += 0x1000;
		}

		d >>= 5;
		d &= 0xFF;

		set_wiper(d);
	}

	// toggle the alarm LED

	if (throw_alarm) {

		if (alarm_state_machine % 2 == 0) {
			GPIO_SetBits(LED_ALARM_PORT, LED_ALARM_PIN);
		} else {
			GPIO_ResetBits(LED_ALARM_PORT, LED_ALARM_PIN);
		}


		blink_period = 0x01;

		if (alarm_state_machine == 0) {

			// initiate the USB wakeup sequence, or
			// if USB is not connected, pull-down pins
			if (bDeviceState == ATTACHED || bDeviceState == UNCONNECTED) {
				//GPIO_SetBits(LED_ALARM_PORT, LED_ALARM_PIN);

				SetCNTR(GetCNTR() | USB_CNTR_PDWN);

				// pull down USB_DP and USB_DN using GPIO
				// enable clock domain
				RCC_APB2PeriphClockCmd(USB_GPIO_CLOCK_DOMAIN, ENABLE);

				// USB enable line (push-pull)
				GPIO_InitStructure.GPIO_Pin =  USB_DN_PIN;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
				GPIO_Init(USB_ENBL_PORT, &GPIO_InitStructure);

				GPIO_SetBits(USB_PORT, USB_DN_PIN);
			} else {
				/* Initiate external resume sequence (1 step) */

				// check that the remote wakeup feature is enabled
				// n.b. this bit is set and cleared by the standard request handlers in usb_core
				if (pInformation->Current_Feature & (1<<5)) {
					if (bDeviceState == SUSPENDED) {
						//GPIO_SetBits(LED_ALARM_PORT, LED_ALARM_PIN);
						//*CNTR &= ~CNTR_FSUSP;
						Resume(RESUME_INTERNAL);
					}
				}
			}
		}

//#endif

		// update current state
		++alarm_state_machine;


		if (alarm_state_machine > 0x08) {
			// terminate alarm
			GPIO_ResetBits(LED_ALARM_PORT, LED_ALARM_PIN);
			throw_alarm = 0;
			alarm_state_machine = 0;

			if (bDeviceState == ATTACHED) {
				// release USB_DN using GPIO
				GPIO_ResetBits(USB_PORT, USB_DN_PIN);

				// USB enable line (push-pull)
				GPIO_InitStructure.GPIO_Pin =  USB_DN_PIN;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
				GPIO_Init(USB_ENBL_PORT, &GPIO_InitStructure);

				// bring USB section out of power down
				SetCNTR(GetCNTR() & ~USB_CNTR_PDWN);
				// disable clock domain
				//RCC_APB2PeriphClockCmd(USB_GPIO_CLOCK_DOMAIN, DISABLE);
			}
		}
	}


	/*
	 * input level tracking: USB log value
	**/

	/*_tmp = log_signal_level;
	if (VOLUME_DATA == MAX_DATA) {
		// AGC is enabled.
		_tmp = MAX(_tmp, ((uint16_t)(0xff - wiper_pos)) << 8); //(uint16_t)((((uint32_t)_input_level_tracked_slow) + wiper_pos) >> 8);
	}
	log_signal_level = _tmp;*/




	/*
	 * input level tracking: LED pulse rate value
	**/

	_tmp = input_level_tracker_LED;
	_tmp = (uint16_t)((((uint32_t)_tmp) * 0xc0) >> 8);	// decay
	if (VOLUME_DATA == MAX_DATA) {
		// AGC is enabled.
		//_tmp = MAX(_tmp, ((uint16_t)(0xff - wiper_pos)) << 8);
	}
	input_level_tracker_LED = _tmp;

	TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : This function handles TIM2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM2_IRQHandler(void) {
	// actually, disable TIM2 IRQ
	TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
}
#define ValBit(VAR,Place)    (VAR & (1 << Place))


#define DELAY_CLK		 __asm("nop"); __asm("nop");__asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");__asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");__asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");__asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");__asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");
#define DELAY_DATA_VALID		__asm("nop"); __asm("nop"); __asm("nop"); __asm("nop");


/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles TIM3 global interrupt request.
* Trigger read from external ADC by sending dummy word
*******************************************************************************/
volatile uint16_t _adc_word;

void TIM3_IRQHandler(void) {

	/**
	 * start grabbing new word
	**/

	GPIO_ResetBits(EXTERNAL_ADC_PORT, EXTERNAL_ADC_CS_PIN);			// set #CS high (ADC inactive)
	SPI_Cmd(SPI1, ENABLE);
	SPI_I2S_SendData(SPI1, 0x0000);

	TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
	NVIC_ClearPendingIRQ(TIM3_IRQn);
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : This function handles TIM4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : I2C1_EV_IRQHandler
* Description    : This function handles I2C1 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_EV_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : I2C1_ER_IRQHandler
* Description    : This function handles I2C1 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_ER_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : I2C2_EV_IRQHandler
* Description    : This function handles I2C2 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_EV_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : I2C2_ER_IRQHandler
* Description    : This function handles I2C2 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_ER_IRQHandler(void) {
}


/*******************************************************************************
* Function Name  : SPI1_IRQHandler
* Description    : This function handles SPI1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI1_IRQHandler(void) {
	uint16_t _input_level_tracked_fast;
	int16_t _adc_word;
	uint16_t w;


	/**
	 *	grab the ADC word from SPI buffer
	**/

	SPI_Cmd(SPI1, DISABLE);
	w = SPI_I2S_ReceiveData(SPI1);
	GPIO_SetBits(EXTERNAL_ADC_PORT, EXTERNAL_ADC_CS_PIN);			// set #CS high (ADC inactive)

	w &= 0x1FFE;	// hi-z, hi-z, null bit, data word (12 bits), hi-z
	w <<= 3;


	/**
	* convert ``_adc_word`` into two's complement format
	 * i.e. map the range [0..FFFF] onto [-8000..7FFF]
	**/

	if (w >= 0x7FFF) {
		w -= 0x7FFF;
	} else {
		w =  ~(0x7FFF - w) + 1;
	}

	_adc_word = (int16_t)w;


	/**
	 *	calculate the signal level [0..FFFF]
	**/

	if (!BIT_CHECK(_adc_word, 16)) {
		input_level = _adc_word << 1;
	} else if (_adc_word == 0x8000) {
		input_level = 0xFFFF;
	} else {
		input_level = (~_adc_word + 1) << 1;
	}


	/**
	 * input level tracking
	**/

	input_level_tracker_LED = MAX(input_level, input_level_tracker_LED);
	log_signal_level = MAX(input_level, log_signal_level);
	_input_level_tracked_fast = MAX(input_level, input_level_tracked_fast);
	if (_input_level_tracked_fast > 0) {
		--_input_level_tracked_fast;
	}
	input_level_tracked_fast = _input_level_tracked_fast;

	++log_integration_window_counter;
	if (log_integration_window_counter >= log_integration_window) {
		log_integration_window_counter = 0;
		++log_seq_num;
		prev_log_signal_level = log_signal_level;
		log_signal_level = 0;
	}


	/**
	*	throw the alarm?
	**/

	if (input_level > alarm_lvl) {

		/**
		 *	throw the alarm!
		 *	TIM1 ISR does the rest
		**/

		if (alarm_enable) {
			throw_alarm = 1;
		}
	}

	if (MUTE_DATA & 0x00000001 || VOLUME_DATA == 0x00008000) {

		/**
		 *	if muting or volume is at -inf dB...
		**/

		adcbuf[adcbuf_idx_write] = (uint16_t)0x0000;
	} else {
		adcbuf[adcbuf_idx_write] = _adc_word;

		/*adcbuf[adcbuf_idx_write] = sin_tbl[++sin_tbl_idx] << 6;
		if (sin_tbl_idx == 0xff) {
			sin_tbl_idx = 0;
		}*/
	}
	adcbuf_idx_write = (adcbuf_idx_write + 1) % ADC_BUFFER_SIZE;


	++ghostbusters;

	if (ghostbusters > 0x08) {

		/**
		 * blink the activity LED
		**/

		blink_period = (input_level_tracker_LED >> 10);
		blink_period &= 1 + 2 + 4 + 8 + 16 + 32;
		blink_period = ACTIVITY_LED_BLINK_PERIOD_LOOKUP_TABLE[blink_period];

		// set the PWM level
		++blink_phase;
		if (blink_phase >= blink_period) {
			blink_phase = 0;
			if (count_dir == 1) {
				// count
				--lvl;

				// bounce?
				if (lvl == 0) {
					count_dir = 0;
				}
			} else {
				// count
				++lvl;

				// bounce?
				if (lvl == 255) {
					count_dir = 1;
				}
			}
			set_LED_brightness(lvl);
		}
	}

	if (ghostbusters > 0x1F) {
		ghostbusters = 0;

		/**
		 * AGC control loop
		**/

		if (VOLUME_DATA == MAX_DATA) {
			/**
			 *	AGC is enabled.
			**/

			if (input_level > CLIP_HIGHER_LEVEL) {
				// amplitude too high
				// update the wiper position
				++slowdown3;
				if (slowdown3 > (uint32_t)(1<<8)) {
					slowdown3 = 0;
					if (wiper_pos >= 2) {
						--wiper_pos;
					}
				}
			} else if (input_level < CLIP_LOWER_LEVEL) { //(CLIP_LEVEL>>1)) {
				// amplitude too low
				++slowdown2;
				if (slowdown2 > (uint32_t)(1<<8)) { //UINT32_MAX >> 14) {
					slowdown2 = 0;
					// decay clip level
					if (wiper_pos < MAX_WIPER_POS) {
						++wiper_pos;
						//LED2(0);
					}
				}
			} else {
				// amplitude OK
				/*if (wiper_pos > 0x1F) {
					blink_period = MIN(blink_period, (input_level - (CLIP_LEVEL>>1)) >> 7);
				}*/
			}
		} else {
			// AGC is disabled. Host volume controls potentiometer.
			wiper_pos = (((int16_t)(VOLUME_DATA & 0xFFFF)) - ((int16_t)MIN_DATA)) >> 5;
		}
	}
}

/*******************************************************************************
* Function Name  : SPI2_IRQHandler
* Description    : This function handles SPI2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI2_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART1_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : This function handles USART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : This function handles USART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : EXTI15_10_IRQHandler
* Description    : This function handles External lines 15 to 10 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI15_10_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : RTCAlarm_IRQHandler
* Description    : This function handles RTC Alarm interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTCAlarm_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : USBWakeUp_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBWakeUp_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : TIM8_BRK_IRQHandler
* Description    : This function handles TIM8 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_BRK_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : TIM8_UP_IRQHandler
* Description    : This function handles TIM8 overflow and update interrupt
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_UP_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : TIM8_TRG_COM_IRQHandler
* Description    : This function handles TIM8 Trigger and commutation interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_TRG_COM_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : TIM8_CC_IRQHandler
* Description    : This function handles TIM8 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_CC_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : ADC3_IRQHandler
* Description    : This function handles ADC3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC3_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : FSMC_IRQHandler
* Description    : This function handles FSMC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FSMC_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void) {
	/* Process All SDIO Interrupt Sources */
}

/*******************************************************************************
* Function Name  : TIM5_IRQHandler
* Description    : This function handles TIM5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM5_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : SPI3_IRQHandler
* Description    : This function handles SPI3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI3_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : UART4_IRQHandler
* Description    : This function handles UART4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART4_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : UART5_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART5_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : TIM6_IRQHandler
* Description    : This function handles TIM6 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM6_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : TIM7_IRQHandler
* Description    : This function handles TIM7 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM7_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : DMA2_Channel1_IRQHandler
* Description    : This function handles DMA2 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel1_IRQHandler(void) {
	/* DMA2 channel 1 used by MEM2MEM copy  */
	/* Test on DMA2 Channel1 Transfer Complete or Half Transfer interrupt */
	/*  if(DMA_GetITStatus(DMA2_IT_TC1))
	  {
	    if(B0_Transfer)
	    {
	      B0_Transfer = 0;
	      B0_Ready = 1;
	    }
	    else if(B1_Transfer)
	    {
	      B1_Transfer = 0;
	      B1_Ready = 1;
	    }
	  }*/

	/* Clear DMA Channel1 Half Transfer, Transfer Complete and Global interrupt pending bits */
	//DMA_ClearITPendingBit(DMA2_IT_GL1);
}

/*******************************************************************************
* Function Name  : DMA2_Channel2_IRQHandler
* Description    : This function handles DMA2 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel2_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : DMA2_Channel3_IRQHandler
* Description    : This function handles DMA2 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel3_IRQHandler(void) {
}

/*******************************************************************************
* Function Name  : DMA2_Channel4_5_IRQHandler
* Description    : This function handles DMA2 Channel 4 and DMA2 Channel 5
*                  interrupts requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel4_5_IRQHandler(void) {
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

