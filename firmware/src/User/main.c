#include "platform.h"


volatile uint16_t brightness;
extern volatile uint16_t wiper_pos;
extern volatile uint8_t throw_alarm;


int main(void) {

	DWT_Init();		// init delays
	platform_init();
	USB_Interrupts_Config();
	Set_USBClock();
	USB_Init();

	DWT_Delay(100000);	// avoid alarm USB wakeup sequence from interfering with USB enumeration
	set_alarm_enable(1);

	while (1) {
		//__WFI();
		__asm("nop");
	}
}
