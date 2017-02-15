#ifndef __USB_DESC_H
#define __USB_DESC_H

#include "usb_lib.h"
#include "platform.h"


#define USB_MAX_PACKET_SIZE        0x40   // 64 bytes
#define EP0_MAX_PACKET_SIZE        0x40

#define ID_VENDOR    0x0483
#define ID_PRODUCT   0x5726
// SHOULD BE 5730

#define HID_DESCRIPTOR_TYPE                     0x21
#define JOYSTICK_SIZ_REPORT_DESC                74
#define JOYSTICK_SIZ_HID_DESC                   0x09
#define JOYSTICK_OFF_HID_DESC                   0x12

#define EP_TYPE_CTRL                           0x00
#define EP_TYPE_ISOC                           0x01
#define EP_TYPE_BULK                           0x02
#define EP_TYPE_INTR                           0x03

#define EP_ISOC_SYNCHRONOUS                    0x0C
#define EP_ISOC_ASYNCHRONOUS                   0x04

#define EP_ISOCHRONOUS_ADDRESS                 0x81
#define EP_HID_ADDRESS                         0x82
#define EP_ISOCHRONOUS_MAX_PACKET_SIZE         0x20	// 32 bytes

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05
#define USB_DEVICE_QUALIFIER_TYPE								0x06
#define USB_OTHER_SPEED_CONFIGURATION_TYPE			0x07
#define USB_INTERFACE_POWER_TYPE								0x08
#define USB_OTG_TYPE														0x09
#define USB_DEBUG_TYPE													0x0A
#define USB_INTERFACE_ASSOCIATION_TYPE					0x0B

#define JOYSTICK_SIZ_DEVICE_DESC                18
#define JOYSTICK_SIZ_CONFIG_DESC                42
#define JOYSTICK_SIZ_STRING_LANGID              4
#define JOYSTICK_SIZ_STRING_VENDOR              40
#define JOYSTICK_SIZ_STRING_PRODUCT             50
#define JOYSTICK_SIZ_STRING_SERIAL              28

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#define SPEAKER_SIZ_DEVICE_DESC                       18
#define SPEAKER_SIZ_CONFIG_DESC                       (109+8+25+8)
#define SPEAKER_SIZ_INTERFACE_DESC_SIZE               9

#define SPEAKER_SIZ_STRING_LANGID                     0x04
#define SPEAKER_SIZ_STRING_VENDOR                     0x26
#define SPEAKER_SIZ_STRING_PRODUCT                    0x1C
#define SPEAKER_SIZ_STRING_SERIAL                     0x1A

#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07
/* USB Descriptor Types */
#define USB_DEVICE_DESCRIPTOR_TYPE                    0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE             0x02
#define USB_STRING_DESCRIPTOR_TYPE                    0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE                 0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE                  0x05

#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02
#define AUDIO_PROTOCOL_UNDEFINED                      0x00
#define AUDIO_STREAMING_GENERAL                       0x01
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25


/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07

#define AUDIO_CONTROL_MUTE                            0x0001
#define AUDIO_CONTROL_VOLUME                          0x0002

#define AUDIO_FORMAT_TYPE_I                           0x01

#define USB_ENDPOINT_TYPE_ISOCHRONOUS                 0x01
#define USB_ENDPOINT_TYPE_ASYNCHRONOUS                0x05
#define AUDIO_ENDPOINT_GENERAL                        0x01

/* Exported functions ------------------------------------------------------- */
extern const uint8_t Joystick_DeviceDescriptor[JOYSTICK_SIZ_DEVICE_DESC];
extern const uint8_t Joystick_ConfigDescriptor[];
extern const uint8_t Joystick_ReportDescriptor[JOYSTICK_SIZ_REPORT_DESC];
extern const uint8_t Joystick_StringLangID[JOYSTICK_SIZ_STRING_LANGID];
extern const uint8_t Joystick_StringVendor[JOYSTICK_SIZ_STRING_VENDOR];
extern const uint8_t Joystick_StringProduct[JOYSTICK_SIZ_STRING_PRODUCT];
extern uint8_t Joystick_StringSerial[JOYSTICK_SIZ_STRING_SERIAL];

#endif /* __USB_DESC_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
