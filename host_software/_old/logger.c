/* Name: set-led.c
 * Project: hid-custom-rq example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-10
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id$
 */

/*
General Description:
This is the host-side driver for the custom-class example device. It searches
the USB for the LEDControl device and sends the requests understood by this
device.
This program must be linked with libusb on Unix and libusb-win32 on Windows.
See http://libusb.sourceforge.net/ or http://libusb-win32.sourceforge.net/
respectively.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>        /* this is libusb */
#include "opendevice.h" /* common code moved to separate module */

#include "firmware/requests.h"   /* custom request numbers */
#include "firmware/usbconfig.h"  /* device's VID/PID and names */

#define BUFFER_LEN_SIGNAL_LEVEL		2
#define BUFFER_LEN_INTEGRATION_WINDOW		4

static void usage(char *name)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s on ....... turn on LED\n", name);
    fprintf(stderr, "  %s off ...... turn off LED\n", name);
    fprintf(stderr, "  %s status ... ask current status of LED\n", name);
}

#define COMMAND_GET_INTEGRATION_WINDOW	0x01
#define COMMAND_GET_SIGNAL_LEVEL	0x02
#define COMMAND_GET_SEQ_NUM		0x03
#define COMMAND_SET_INTEGRATION_WINDOW	0x04



void send_usb(usb_dev_handle *handle, uint8_t command, uint32_t data) {
	const uint32_t buffer_len = 4;
	uint8_t buf[buffer_len];
	uint32_t w;
	uint8_t i;
	int cnt;


	w = data;
	w &= 0x3FFFFFFF;
	w |= command << 29;

#ifdef DEBUG
	printf("Sending [w = %x] the following data:\n", w);
#endif

	for (i = 0; i < buffer_len; ++i) {
		//buf[i] = (w >> (8 * (buffer_len - i - 1))) & 0xFF;
		buf[i] = (w >> (8 * (i))) & 0xFF;
#ifdef DEBUG
		printf("%.2X ", buf[i]);
#endif
	}
#ifdef DEBUG
	printf("\n\n");
#endif

printf("YEAAAAAAAAAAAAA = %d\n", USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT);
        cnt = usb_control_msg(handle,
                              USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                              //USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_ENDPOINT_OUT,
                              CUSTOM_RQ_SET_STATUS,
                              0,
                              0,
                              buf,
                              buffer_len,
                              1000);
	if (cnt < 0) {
		fprintf(stderr, "USB error: %s\n", usb_strerror());
	}
}


uint32_t get_usb(usb_dev_handle *handle, uint8_t *buf) {

	const uint8_t buf_len = 4;
	uint8_t i;
	int cnt;
	uint32_t w;


	for (i = 0; i < buf_len; ++i) {
		buf[i] = 0;
	}
printf("YEAAAAAAA IN %d\n", USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN);
        cnt = usb_control_msg(handle,
                              USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                              CUSTOM_RQ_GET_STATUS,
                              0,
                              0,
                              buf,
                              buf_len,
                              1000);

	if (cnt < 1) {
		if (cnt < 0){
			fprintf(stderr, "USB error: %s\n", usb_strerror());
		}
		else {
			fprintf(stderr, "only %d bytes received.\n", cnt);
		}
	}
	else {
#ifdef DEBUG
		printf("Expecting %d words.\n", cnt);
		printf("Received the following data: \n");
		for (i = 0; i < buf_len; ++i) {
			printf("%.2X ", buf[i] & 0xFF);
		}
		printf("\n\n");
#endif
	}

	w = 0;
	for (i = 0; i < buf_len; ++i) {
		w |= buf[i] << (8 * i);
	}

	return w;
}





int main(int argc, char **argv)
{
usb_dev_handle      *handle = NULL;
const unsigned char rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
char                vendor[] = {USB_CFG_VENDOR_NAME, 0}, product[] = {USB_CFG_DEVICE_NAME, 0};
char                buf_signal_level[BUFFER_LEN_SIGNAL_LEVEL];
char                buf_integration_window[BUFFER_LEN_INTEGRATION_WINDOW];
uint8_t buf[4];

int                 cnt, vid, pid, isOn;
int i;

    usb_init();
    if(argc < 2){   /* we need at least one argument */
        usage(argv[0]);
        exit(1);
    }
    /* compute VID/PID from usbconfig.h so that there is a central source of information */
    vid = rawVid[1] * 256 + rawVid[0];
    pid = rawPid[1] * 256 + rawPid[0];
    /* The following function is in opendevice.c: */
    if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
        fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
        exit(1);
    }
    /* Since we use only control endpoint 0, we don't need to choose a
     * configuration and interface. Reading device descriptor and setting a
     * configuration and interface is done through endpoint 0 after all.
     * However, newer versions of Linux require that we claim an interface
     * even for endpoint 0. Enable the following code if your operating system
     * needs it: */
#if 0
    int retries = 1, usbConfiguration = 1, usbInterface = 0;
    if(usb_set_configuration(handle, usbConfiguration) && showWarnings){
        fprintf(stderr, "Warning: could not set configuration: %s\n", usb_strerror());
    }
    /* now try to claim the interface and detach the kernel HID driver on
     * Linux and other operating systems which support the call. */
    while((len = usb_claim_interface(handle, usbInterface)) != 0 && retries-- > 0){
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
        if(usb_detach_kernel_driver_np(handle, 0) < 0 && showWarnings){
            fprintf(stderr, "Warning: could not detach kernel driver: %s\n", usb_strerror());
        }
#endif
    }
#endif





	//send_usb(handle, COMMAND_SET_INTEGRATION_WINDOW, 480000 - 1);
	send_usb(handle, COMMAND_SET_INTEGRATION_WINDOW, 4*8000);
	send_usb(handle, COMMAND_GET_INTEGRATION_WINDOW, 0);
	printf("Integration window length = %d\n", get_usb(handle, buf));
	send_usb(handle, COMMAND_GET_SEQ_NUM, 0);
	printf("Sequence number = %d\n", get_usb(handle, buf));
	send_usb(handle, COMMAND_GET_SIGNAL_LEVEL, 0);
	printf("Signal level = %d\n", get_usb(handle, buf));

#ifdef LOLCATS





    if(strcasecmp(argv[1], "status") == 0){
    
            //printf("The status is %d\n", buffer[0]);
        }
    } else if ((isOn = (strcasecmp(argv[1], "on") == 0)) || strcasecmp(argv[1], "off") == 0){
	send_usb(COMMAND_SET_INTEGRATION_WINDOW, 480000);
	}


	else {
        usage(argv[0]);
        exit(1);
    }
#endif
    usb_close(handle);
    return 0;
}
