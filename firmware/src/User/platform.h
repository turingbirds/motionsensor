#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdlib.h>

#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_spi.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "usb_type.h"
#include "usb_desc.h"
#include "cyccnt_delay.h"


#define BIT_SET(a,b) ((a) |= (1<<(b-1)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b-1)))
#define BIT_FLIP(a,b) ((a) ^= (1<<(b-1)))
#define BIT_CHECK(a,b) ((a) & (1<<(b-1)))

#define BITMASK_SET(x,y) ((x) |= (y))
#define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
#define BITMASK_FLIP(x,y) ((x) ^= (y))
#define BITMASK_CHECK(x,y) ((x) & (y))

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#define LO(v)   ((unsigned char) (v))
#define HI(v)  ((unsigned char) (((unsigned int) (v)) >> 8))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define true		1
#define false		0


/**
 * data buffers
**/

#define ADC_BUFFER_SIZE 256

// clip level is defined with respect to the range of `input_level`, i.e. [0..FFFF].
#define CLIP_HIGHER_LEVEL			((uint16_t)(0x7FFF))
#define CLIP_LOWER_LEVEL			((uint16_t)(0x0FFF))


/**
 *	interrupt priority levels (low number = high priority)
**/

#define TIM3_INTR_PRIO				((uint8_t)0x00)
#define SPI1_INTR_PRIO 				((uint8_t)0x00)
#define USB_HP_INTR_PRIO			((uint8_t)0x01)
#define USB_LP_INTR_PRIO			((uint8_t)0x01)
#define TIM1_INTR_PRIO				((uint8_t)0x03)
#define ADC1_2_INTR_PRIO			((uint8_t)0x04)


/**
 *	activity LED
**/
#define LED_PWM_PORT										GPIOB
#define LED_PWM_PIN											GPIO_Pin_11
#define LED_PWM_GPIO_CLOCK_DOMAIN_AF		RCC_APB2Periph_AFIO
#define LED_PWM_GPIO_CLOCK_DOMAIN				RCC_APB2Periph_GPIOB


/**
 *	alarm LED
**/

#define LED_ALARM_PORT										GPIOB
#define LED_ALARM_PIN											GPIO_Pin_10
#define LED_ALARM_GPIO_CLOCK_DOMAIN				RCC_APB2Periph_GPIOB


/**
 *	 boost converter clock generator
**/

#define BOOST_OSC_PORT										GPIOB
#define BOOST_OSC_PIN											GPIO_Pin_2
#define BOOST_OSC_GPIO_CLOCK_DOMAIN_AF		RCC_APB2Periph_AFIO
#define BOOST_OSC_GPIO_CLOCK_DOMAIN				RCC_APB2Periph_GPIOB


/**
 *	output pin for turning the RF module on/off
**/

#define RF_MODULE_PWR_CONTROL_GPIO_PORT											GPIOA
#define RF_MODULE_PWR_CONTROL_GPIO_PIN											GPIO_Pin_8
#define RF_MODULE_PWR_CONTROL_GPIO_CLOCK_DOMAIN				RCC_APB2Periph_GPIOA


/**
 *	digital potentiometer via SPI2
**/

#define DIGIPOT_SPI_PORT			GPIOB
#define DIGIPOT_SPI_CLK_PIN		GPIO_Pin_13
#define DIGIPOT_SPI_MOSI_PIN	GPIO_Pin_15
#define DIGIPOT_SPI_CS_PIN		GPIO_Pin_12

#define WIPER_POS_STARTUP			0x0F
#define MAX_BLINK_PERIOD 	0x1F
#define MAX_WIPER_POS 255		// use to limit gain


/**
 *	external ADC via SPI1
**/

#define EXTERNAL_ADC_GPIO_CLK_DOMAIN	RCC_APB2Periph_GPIOA
#define EXTERNAL_ADC_PORT		GPIOA
#define EXTERNAL_ADC_CLK_PIN		GPIO_Pin_5
#define EXTERNAL_ADC_DOUT_PIN		GPIO_Pin_6
#define EXTERNAL_ADC_CS_PIN			GPIO_Pin_4


/**
 *	USB enable line
 *
 *	assumed active-high (change in platform.c:USB_Cable_Config)
**/

#define USB_ENBL_PORT										GPIOA
#define USB_ENBL_PIN										GPIO_Pin_10
//#define USB_ENBL_PIN										GPIO_Pin_2		# for PORT103Z
#define USB_ENBL_CLOCK_DOMAIN						RCC_APB2Periph_GPIOA
#define USB_GPIO_CLOCK_DOMAIN		RCC_APB2Periph_GPIOA
#define USB_PORT					GPIOA
#define USB_DN_PIN				GPIO_Pin_11

#define ADC_ALARM_PORT							GPIOA
#define ADC_ALARM_PIN									GPIO_Pin_7
#define ADC_ALARM_PIN_CLOCK_DOMAIN_AF	RCC_APB2Periph_AFIO
#define ADC_ALARM_PIN_CLOCK_DOMAIN			RCC_APB2Periph_GPIOA
#define ADC_ALARM_CHANNEL			ADC_Channel_7		// N.B. ADC_Channel_17 is internal ref

/* ------------------------------------------------------------------------- */

#define RCC_APB2Periph_ALLGPIO             (RCC_APB2Periph_GPIOA \
																						| RCC_APB2Periph_GPIOB \
																						| RCC_APB2Periph_GPIOC \
																						| RCC_APB2Periph_GPIOD \
																						| RCC_APB2Periph_GPIOE )

/* ------------------------------------------------------------------------- */

void platform_init(void);
void set_LED_brightness(uint8_t val);
void Set_USBClock(void);
void GPIO_AINConfig(void);
void GPIO_init(void);
void NVIC_Configuration(void);
int16_t grab_adc_sample(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);
void set_wiper(uint8_t val);
uint16_t read_alarm_setlevel(void);
void set_alarm_enable(uint8_t _alarm_enable);

/* ------------------------------------------------------------------------- */

#endif
