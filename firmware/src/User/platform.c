#include "platform.h"

extern uint32_t SystemCoreClock;


/**
 *	circular buffer for ADC
 *
 * ``adcbuf_idx_write`` points to the next free word.
 * ``adcbuf_idx_read`` points to the next word to be read.
**/

volatile uint16_t adcbuf[ADC_BUFFER_SIZE];
volatile size_t adcbuf_idx_read;
volatile size_t adcbuf_idx_write;


/**
 *	logging
**/

volatile uint8_t log_seq_num;
volatile uint32_t log_integration_window;	// counted in number of samples at 8 kHz
volatile uint32_t log_integration_window_counter;
volatile uint16_t log_signal_level;
volatile uint16_t prev_log_signal_level;

// default integration window = 1 minute
#define DEFAULT_LOG_INTEGRATION_WINDOW 480000


/**
 *	calibration level for ADC
**/

volatile int16_t adc_dc_offset = 0;


/**
 *	alarm
**/

extern volatile uint8_t throw_alarm;
volatile uint8_t alarm_enable;

/* sensitivity potentiometer level */
volatile uint16_t alarm_lvl;

/* instantaneous input level [0..0xFFFF] */
volatile uint16_t input_level;

/* input level tracking on a fast timescale for AGC loop */
volatile uint16_t input_level_tracked_fast;

/* input level tracking on a slow timescale for logging */
volatile uint16_t input_level_tracker_LED;
//volatile uint16_t input_level_tracker_USB;
//volatile uint16_t input_level_tracker_USB_COPY;

/**
 *	current state of the digital potentiometer wiper
**/
volatile uint8_t wiper_pos;


/**
 *
 * LED PWM level
 *
 * generate a triangular waveform /\/\/\/\ in ``lvl``
 *
 * ``blink_period`` sets the frequency of oscillation
 * 0 <= blink_period <= MAX_BLINK_PERIOD
 * ``slowdown`` is for slowing down the ISR rate for rate decay
**/

extern volatile uint8_t blink_period;

extern volatile uint32_t VOLUME_DATA;
volatile uint32_t _VOLUME_DATA;


int logical_right_shift(int x, int n) {
	int size = sizeof(int) * 8;
	return (x >> n) & ~(((0x1 << size) >> n) << 1);
}


void set_alarm_enable(uint8_t _alarm_enable) {
	alarm_enable = _alarm_enable;
}


void init_alarm_led() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(LED_ALARM_GPIO_CLOCK_DOMAIN, ENABLE);
	GPIO_ResetBits(LED_ALARM_PORT, LED_ALARM_PIN);
	GPIO_InitStructure.GPIO_Pin =  LED_ALARM_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(LED_ALARM_PORT, &GPIO_InitStructure);

	blink_period = MAX_BLINK_PERIOD;
}


/**
 * 	set wiper position for digital potentiometer
 *
 * 	don't call this too often, as it's slow
**/
void set_wiper(uint8_t _val) {
	//uint8_t _val = ((uint8_t)0xff) - val;
	uint32_t i;
	GPIO_ResetBits(DIGIPOT_SPI_PORT, DIGIPOT_SPI_CS_PIN);
	for (i = 1 << 12; --i; i > 0);
	SPI_I2S_SendData(SPI2, _val);
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	for (i = 1 << 12; --i; i > 0);
	GPIO_SetBits(DIGIPOT_SPI_PORT, DIGIPOT_SPI_CS_PIN);
}


void init_adc_spi(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;


	// Configure SPI
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, ENABLE);
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SPI1_INTR_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// configure GPIO
	RCC_APB2PeriphClockCmd(EXTERNAL_ADC_GPIO_CLK_DOMAIN, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	// set #CS high (ADC inactive)
	GPIO_SetBits(EXTERNAL_ADC_PORT, EXTERNAL_ADC_CS_PIN);

	GPIO_InitStructure.GPIO_Pin = EXTERNAL_ADC_CS_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(EXTERNAL_ADC_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = EXTERNAL_ADC_CLK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(EXTERNAL_ADC_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = EXTERNAL_ADC_DOUT_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(EXTERNAL_ADC_PORT, &GPIO_InitStructure);
}


/**
 *	initialize SPI2 interface for digital potentiometer
 *
 *	init GPIO, GPIO_AF and SPI units
**/
void init_digipot_spi(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI2, &SPI_InitStructure);

	//SPI_SSOutputCmd(SPI2, ENABLE);		// Enable NSS (chip select) output
	SPI_CalculateCRC(SPI2, DISABLE);
	SPI_Cmd(SPI2, ENABLE);

	// configure GPIO and GPIO_AF
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = DIGIPOT_SPI_CLK_PIN | DIGIPOT_SPI_MOSI_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(DIGIPOT_SPI_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = DIGIPOT_SPI_CS_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(DIGIPOT_SPI_PORT, &GPIO_InitStructure);

	wiper_pos = WIPER_POS_STARTUP;
	set_wiper(wiper_pos);
}


void init_daq_timer(void) {

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_DeInit(TIM3);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0x1770;			// 48 MHz / 6000 = 8 kHz
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM3_INTR_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* enable the timer interrupt */
	TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE);

	TIM_Cmd(TIM3, ENABLE);
}


void init_boost_osc(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	// enable clock domains
	RCC_APB2PeriphClockCmd(BOOST_OSC_GPIO_CLOCK_DOMAIN, ENABLE);

	// set ``ENABLE`` pin for boost controller to output
	GPIO_StructInit(&GPIO_InitStructure); // Reset init structure
	GPIO_InitStructure.GPIO_Pin =  BOOST_OSC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(BOOST_OSC_PORT, &GPIO_InitStructure);

	// enable the boost converter (active high)
	GPIO_SetBits(BOOST_OSC_PORT, BOOST_OSC_PIN);
}


void LED_PWM_init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	// enable clock domains
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | LED_PWM_GPIO_CLOCK_DOMAIN | LED_PWM_GPIO_CLOCK_DOMAIN_AF, ENABLE);

	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);

	// connect activity LED to timer PWM output
	GPIO_StructInit(&GPIO_InitStructure); // Reset init structure
	GPIO_InitStructure.GPIO_Pin = LED_PWM_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;            // alternate function, push-pull
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LED_PWM_PORT, &GPIO_InitStructure);


	// set timer 2 for LED PWM
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_DeInit(TIM2);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0x00FF;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_OCStructInit( &TIM_OCInitStructure );
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);

	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM2, ENABLE);
	TIM_Cmd( TIM2, ENABLE );
	TIM_CtrlPWMOutputs(TIM2, ENABLE);
}


void set_LED_brightness(uint8_t val) {
	TIM2->CCR4 = val;
}


void init_DMA() {
	//NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable DMA clock */
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	/* Enable the DMAChannel2 Interrupt */
	/*NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA_CH1_INTR_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);*/

	/* Enable the DMAChannel1 Interrupt */
	/*NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);*/
}


void adc_buf_init(void) {
	int i;

	// initialize adc buffers
	adcbuf_idx_write = 15;
	adcbuf_idx_read = 0;
	for (i = 0; i < ADC_BUFFER_SIZE; ++i) {
		adcbuf[i] = 0;
	}

	input_level = 0;
}


/**
 *	set up ADC2 to read sensitivity level pot
**/
void init_ADC_alarm(void) {
	ADC_InitTypeDef ADC_InitStructure;
	//TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	// NVIC_InitTypeDef NVIC_InitStructure;


	RCC_ADCCLKConfig(RCC_PCLK2_Div8);		// 48 / 6 = 12 MHz ADC clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC2, &ADC_InitStructure);

	// enable vref+temp channels for debug
	//ADC_TempSensorVrefintCmd(ENABLE);

	/* ADC2 regular channel configuration */
	ADC_RegularChannelConfig(ADC2, ADC_ALARM_CHANNEL, 1, ADC_SampleTime_239Cycles5);

	/* Enable ADC2 external trigger */
	ADC_ExternalTrigConvCmd(ADC2, ENABLE);

	/* Enable EOC interupt */
	ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE);

	/* Enable ADC2 */
	ADC_Cmd(ADC2, ENABLE);

	/* Enable ADC1 reset calibaration register */
	ADC_ResetCalibration(ADC2);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC2));

	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC2);

	/* Check the end of ADC2 calibration */
	while(ADC_GetCalibrationStatus(ADC2));
}


/**
 * N.B. this is slow; use sparingly
**/
uint16_t read_alarm_setlevel(void) {
	ADC_RegularChannelConfig(ADC2, ADC_ALARM_CHANNEL, 1, ADC_SampleTime_239Cycles5);
	// Start the conversion
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);
	// Wait until conversion completion
	while(ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);
	// Get the conversion value
	return ADC_GetConversionValue(ADC2) << 4;
}


void init_ADC_timer() {

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// enable clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	/* TIM1 configuration */
	// timebase configuration
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = 0xFF;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* TIM1 channel1 configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0x7F;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);

	/* enable the timer interrupt */
	TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);//XXX

	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM1_INTR_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// enable the timer
	TIM_Cmd( TIM1, ENABLE );

	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}


/*******************************************************************************
* Function Name  : Set_USBClock
* Description    : Configures USB Clock input (48MHz).
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Set_USBClock(void) {
	/* Select USBCLK source */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1);

	/* Enable the USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/*******************************************************************************
* Function Name  : GPIO_AINConfig
* Description    : Configures all IOs as AIN to reduce the power consumption.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void GPIO_AINConfig(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable all GPIOs Clock*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ALLGPIO, ENABLE);

	/* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Disable all GPIOs Clock*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ALLGPIO, DISABLE);
}


/*******************************************************************************
* Function Name  : Enter_LowPowerMode.
* Description    : Power-off system clocks and power while entering suspend mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void) {
	/* Set the device state to suspend */
	bDeviceState = SUSPENDED;
}


/*******************************************************************************
* Function Name  : Leave_LowPowerMode.
* Description    : Restores system clocks and power while exiting suspend mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void) {
	DEVICE_INFO *pInfo = &Device_Info;

	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0) {
		/* Device configured */
		bDeviceState = CONFIGURED;
	} else {
		bDeviceState = ATTACHED;
	}
}


void init_NVIC(void) {
	/* 3 bits for pre-emption priority, 1 bit for subpriority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
}


/*******************************************************************************
* Function Name  : USB_Interrupts_Config.
* Description    : Configures the USB interrupts.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_Interrupts_Config(void) {
	NVIC_InitTypeDef NVIC_InitStructure;

	/* USB high priority IRQ channel (used for isochronous transfers) */
	NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USB_HP_INTR_PRIO;  //0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* USB low priority IRQ channel (triggered by all USB events) */
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USB_LP_INTR_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable ADC1_1 IRQChannel */
	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ADC1_2_INTR_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void perform_usb_remote_wakeup(void) {
	/* A device may require to exit from suspend mode as an answer to particular events not
		directly related to the USB protocol (e.g. a mouse movement wakes up the whole system).
		In this case, the resume sequence can be started by setting the RESUME bit in the
		USB_CNTR register to ‘1 and resetting it to 0 after an interval between 1mS and 15mS (this
		interval can be timed using ESOF interrupts, occurring with a 1mS period when the system
		clock is running at nominal frequency). Once the RESUME bit is clear, the resume
		sequence will be completed by the host PC and its end can be monitored again using the
		RXDP and RXDM bits in the USB_FNR register.
		Note: The RESUME bit must be anyway used only after the USB peripheral has been put in
		suspend mode, setting the FSUSP bit in USB_CNTR register to 1.
	*/
	//bmUsbState &= ~SUSPEND; // Reset "Go Suspend"
	//bmUsbState |= REMOTE_WAKEUP; // Set "Remote Wake-up"

}

/*******************************************************************************
* Function Name  : USB_Cable_Config.
* Description    : Software Connection/Disconnection of USB Cable. Active high.
* Input          : NewState: new state.
* Output         : None.
* Return         : None
*******************************************************************************/
void USB_Cable_Config(FunctionalState NewState) {
	if (NewState != DISABLE) {
		GPIO_SetBits(USB_ENBL_PORT, USB_ENBL_PIN);
	} else {
		GPIO_ResetBits(USB_ENBL_PORT, USB_ENBL_PIN);
	}
}


void init_rf_module_power_management() {
	GPIO_InitTypeDef GPIO_InitStructure;

	// enable clock domains
	RCC_APB2PeriphClockCmd(RF_MODULE_PWR_CONTROL_GPIO_CLOCK_DOMAIN, ENABLE);

	// set ``ENABLE`` pin for boost controller to output
	GPIO_StructInit(&GPIO_InitStructure); // Reset init structure
	GPIO_InitStructure.GPIO_Pin =  RF_MODULE_PWR_CONTROL_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(RF_MODULE_PWR_CONTROL_GPIO_PORT, &GPIO_InitStructure);

	// enable the boost converter (active high)
	GPIO_SetBits(RF_MODULE_PWR_CONTROL_GPIO_PORT, RF_MODULE_PWR_CONTROL_GPIO_PIN);
}


/**
 * busy waiting - use only when SPI interrupts disabled
**/
int16_t grab_adc_sample(void) {
	uint16_t w;

	GPIO_ResetBits(EXTERNAL_ADC_PORT, EXTERNAL_ADC_CS_PIN);	// set #CS low (ADC active)
	//SPI_Cmd(SPI1, ENABLE);
	//SPI_I2S_SendData(SPI1, 0x0000);

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, 0x0000);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
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

	return (int16_t)w;
}


void calibrate_adc_offset() {
	const unsigned int n_samples = 4096;
	const unsigned int log_n_samples = 12;
	unsigned int i;
	int32_t avg = 0;

	for (i = 0; i < n_samples; ++i) {
		avg += grab_adc_sample();
		DWT_Delay(10);
	}

	adc_dc_offset = (int16_t)(logical_right_shift(avg, log_n_samples) & 0xFFFF);
}


void init_usb_enable_pin() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(USB_ENBL_CLOCK_DOMAIN, ENABLE);
	GPIO_InitStructure.GPIO_Pin =  USB_ENBL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(USB_ENBL_PORT, &GPIO_InitStructure);
}


void init_log() {
	log_seq_num = 0;
	log_integration_window = DEFAULT_LOG_INTEGRATION_WINDOW;
	log_signal_level = 0x0000;
}


/**
 *	main platform initialisation
**/
void platform_init(void) {
	throw_alarm = 0;
	alarm_enable = 0;

	GPIO_AINConfig();
	init_NVIC();
	init_alarm_led();
	init_usb_enable_pin();
	init_rf_module_power_management();
	init_boost_osc();
	init_adc_spi();
	init_digipot_spi();
	init_log();

	DWT_Delay(1000000);			// avoid alarm on start-up
	// calibrate_adc_offset();

	adc_buf_init();
	init_ADC_alarm();
	init_ADC_timer();
	//init_DMA();					// XXX not supported yet
	init_daq_timer();
	LED_PWM_init();
}
