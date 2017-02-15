/**
 *
 * USB descriptors
 *
**/

#include "usb_desc.h"


/* USB Standard Device Descriptor (little endian) */
const uint8_t Joystick_DeviceDescriptor[JOYSTICK_SIZ_DEVICE_DESC] = {
	0x12,                       // bLength
	USB_DEVICE_DESCRIPTOR_TYPE, // bDescriptorType (DEVICE)
	0x00,                       // bcdUSB (2.0)
	0x02,
	0xEF,                       // bDeviceClass (multi-interface function class)
	0x02,                       // bDeviceSubClass
	0x01,                       // bDeviceProtocol
	EP0_MAX_PACKET_SIZE,        // bMaxPacketSize0
	LO(ID_VENDOR),               // idVendor
	HI(ID_VENDOR),
	LO(ID_PRODUCT),              // idProduct
	HI(ID_PRODUCT),
	0x00,                       // bcdDevice (v1.0)
	0x01,
	1,                          // iManufacturer
	2,                          // iProduct
	3,                          // iSerialNumber
	0x01                        // bNumConfigurations
};

/* USB Configuration Descriptor */
const uint8_t Joystick_ConfigDescriptor[SPEAKER_SIZ_CONFIG_DESC] = {
	/* Configuration 1 */
	0x09,                                 // bLength
	USB_CONFIGURATION_DESCRIPTOR_TYPE,    // bDescriptorType (CONFIGURATION)
	0x6D+0x08+25+8,                                 // wTotalLength
	0x00,
	0x03,                                 // bNumInterfaces
	0x01,                                 // bConfigurationValue
	0x00,                                 // iConfiguration
	0xA0,                                 // bmAttributes (bus powered, remote wakeup)
	0x32,                                 // bMaxPower = 100 mA



	/* HID PART ----------------------------------------------------------- */
	/* Interface association descriptor */
	0x08,                                 // bLength
	USB_INTERFACE_ASSOCIATION_TYPE,       // bDescriptorType
	0,                                    // bFirstInterface
	1,                                    // bInterfaceCount
	0x03,                                    // bFunctionClass
	0x01,                                    // bFunctionSubClass
	0x02,                                    // bFunctionProtocol,
	0,                                    // iFunction



	0x09,         /*bLength: Interface Descriptor size*/
	USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
	0x00,         /*bInterfaceNumber: Number of Interface*/
	0x00,         /*bAlternateSetting: Alternate setting*/
	0x01,         /*bNumEndpoints*/
	0x03,         /*bInterfaceClass: HID*/
	0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
	0x02,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
	0,            /*iInterface: Index of string descriptor*/
	/******************** Descriptor of Joystick Mouse HID ********************/
	/* 18 */
	0x09,         /*bLength: HID Descriptor size*/
	HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
	0x00,         /*bcdHID: HID Class Spec release number*/
	0x01,
	0x00,         /*bCountryCode: Hardware target country*/
	0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
	0x22,         /*bDescriptorType*/
	JOYSTICK_SIZ_REPORT_DESC,/*wItemLength: Total length of Report descriptor*/
	0x00,
	/******************** Descriptor of Joystick Mouse endpoint ********************/
	/* 27 */
	0x07,          /*bLength: Endpoint Descriptor size*/
	USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/

	EP_HID_ADDRESS,          /*bEndpointAddress: Endpoint Address (IN)*/
	0x03,          /*bmAttributes: Interrupt endpoint*/
	0x04,          /*wMaxPacketSize: 4 Byte max */
	0x00,
	0x20,          /*bInterval: Polling Interval (32 ms)*/
	/* 34 */




	/* AUDIO PART ----------------------------------------------------------- */
	/* Interface association descriptor */
	0x08,                                 // bLength
	USB_INTERFACE_ASSOCIATION_TYPE,       // bDescriptorType
	1,                                    // bFirstInterface
	2,                                    // bInterfaceCount
	USB_DEVICE_CLASS_AUDIO,                                    // bFunctionClass
	AUDIO_SUBCLASS_AUDIOCONTROL,                                    // bFunctionSubClass
	0,                                    // bFunctionProtocol,
	0,                                    // iFunction

	/* Interface descriptor */
	0x09,                                 // bLength
	USB_INTERFACE_DESCRIPTOR_TYPE,        // bDescriptorType
	0x01,                                 // bInterfaceNumber
	0x00,                                 // bAlternateSetting
	0x00,                                 // bNumEndpoints
	USB_DEVICE_CLASS_AUDIO,               // bInterfaceClass
	AUDIO_SUBCLASS_AUDIOCONTROL,          // bInterfaceSubClass
	AUDIO_PROTOCOL_UNDEFINED,             // bInterfaceProtocol
	0x00,                                 // iInterface

	/* Interface header audio class descriptor */
	0x09,                                 // bLength
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
	AUDIO_CONTROL_HEADER,                 // bDescriptorSubtype
	0x00,                                 // bcdADC (1.00)
	0x01,
	0x27,                                 // wTotalLength (of all audio class descriptors that follow)
	0x00,
	0x01,                                 // bInCollection (1 streaming interface)
	0x02,                                 // baInterfaceNr (interface 2 is stream)

	/* Input terminal audio class descriptor */
	0x0C,                                 // bLength
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
	AUDIO_CONTROL_INPUT_TERMINAL,         // bDescriptorSubtype
	0x01,                                 // bTerminalID
	0x02,                                 // wTerminalType (desktop microphone)
	0x02,
	0x00,                                 // bAssocTerminal (none)
	0x01,                                 // bNrChannels
	0x04,                                 // wChannelConfig (center front)
	0x00,
	0x00,                                 // iChannelNames (none)
	0x00,                                 // iTerminal (none)

	/* Feature unit audio class descriptor */
	0x09,                                 // bLength
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
	AUDIO_CONTROL_FEATURE_UNIT,           // bDescriptorSubtype
	0x02,                                 // bUnitID
	0x01,                                 // bSourceID (input terminal 1)
	0x02,                                 // bControlSize (size of an element of the bmaControls array)
	0x03,                                 // bmaControls[0]: for channel 0 (Mute = 0x01, Volume = 0x02)
	0x00,
	0x00,                                 // iFeature (none)

	/* Output terminal Descriptor */
	0x09,                                 // bLength
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
	AUDIO_CONTROL_OUTPUT_TERMINAL,        // bDescriptorSubtype
	0x03,                                 // bTerminalID
	0x01,                                 // wTerminalType (USB streaming)
	0x01,
	0x00,                                 // bAssocTerminal
	0x02,                                 // bSourceID (feature unit 2)
	0x00,                                 // iTerminal

	/* Audio interface descriptor: zero bandwith */
	/* Interface 1, alternate setting 0 */
	0x09,                                 // bLength
	USB_INTERFACE_DESCRIPTOR_TYPE,        // bDescriptorType
	0x02,                                 // bInterfaceNumber
	0x00,                                 // bAlternateSetting
	0x00,                                 // bNumEndpoints
	USB_DEVICE_CLASS_AUDIO,               // bInterfaceClass
	AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass
	AUDIO_PROTOCOL_UNDEFINED,             // bInterfaceProtocol
	0x00,                                 // iInterface

	/* Audio interface descriptor: operational */
	/* Interface 1, alternate setting 1 */
	0x09,                                 // bLength
	USB_INTERFACE_DESCRIPTOR_TYPE,        // bDescriptorType
	0x02,                                 // bInterfaceNumber
	0x01,                                 // bAlternateSetting
	0x01,                                 // bNumEndpoints
	USB_DEVICE_CLASS_AUDIO,               // bInterfaceClass
	AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass
	AUDIO_PROTOCOL_UNDEFINED,             // bInterfaceProtocol
	0x00,                                 // iInterface

	/* Class-specific AS general interface descriptor */
	0x07,                                 // bLength
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
	AUDIO_STREAMING_GENERAL,              // bDescriptorSubtype
	0x03,                                 // bTerminalLink (terminal 3)
	0x00,                                 // bDelay (none)
	0x01,                                 // wFormatTag AUDIO_FORMAT_PCM
	0x00,

	/* Type I format type descriptor */
	0x0B,                                 // bLength
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
	AUDIO_STREAMING_FORMAT_TYPE,          // bDescriptorSubtype
	AUDIO_FORMAT_TYPE_I,                  // bFormatType
	0x01,                                 // bNrChannels
	0x02,                                 // bSubFrameSize (two bytes per subframe)
	0x10,                                 // bBitResolution (power of two; 16 bit PCM)
	0x01,                                 // bSamFreqType (1 sampling frequency)
	0x40,                                 // tSamFreq (8 kHz)
	0x1F,
	0x00,

	/* Isochronous endpoint descriptor */
	0x09,                                 // bLength
	USB_ENDPOINT_DESCRIPTOR_TYPE,         // bDescriptorType
	EP_ISOCHRONOUS_ADDRESS,               // bEndpointAddress (1 IN)
	EP_TYPE_ISOC | EP_ISOC_ASYNCHRONOUS,  // interrupt type data endpoint; no synchronization
	LO(EP_ISOCHRONOUS_MAX_PACKET_SIZE),   // wMaxPacketSize 48*2*2 bytes
	HI(EP_ISOCHRONOUS_MAX_PACKET_SIZE),
	0x01,                                 // bInterval (1 ms)
	0x00,                                 // bRefresh
	0x00,                                 // bSynchAddress (no synchronization)

	/* Endpoint - Audio Streaming Descriptor*/
	0x07,                                 // bLength
	AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       // bDescriptorType
	AUDIO_ENDPOINT_GENERAL,               // bDescriptor
	0x00,                                 // bmAttributes
	0x02,                                 // bLockDelayUnits (data type for wLockDelay is PCM samples)
	0x00,                                 // wLockDelay
	0x00,





};

#ifdef NO_USB_IAD
const uint8_t Joystick_ConfigDescriptor[JOYSTICK_SIZ_CONFIG_DESC] = {


	0x09, /* bLength: Configuation Descriptor size */
	USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
	JOYSTICK_SIZ_CONFIG_DESC,
	/* wTotalLength: Bytes returned */
	0x00,
	0x01,         /*bNumInterfaces: 1 interface*/
	0x01,         /*bConfigurationValue: Configuration value*/
	0x00,         /*iConfiguration: Index of string descriptor describing
                                     the configuration*/
	0xE0,         /*bmAttributes: bus powered */
	0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

	/* Interface association descriptor */
	0x08,                                 // bLength
	USB_INTERFACE_ASSOCIATION_TYPE,       // bDescriptorType
	0,                                    // bFirstInterface
	1,                                    // bInterfaceCount
	0x03,                                    // bFunctionClass
	0x01,                                    // bFunctionSubClass
	0x02,                                    // bFunctionProtocol,
	0,                                    // iFunction



	/************** Descriptor of Joystick Mouse interface ****************/
	/* 09 */
	0x09,         /*bLength: Interface Descriptor size*/
	USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
	0x00,         /*bInterfaceNumber: Number of Interface*/
	0x00,         /*bAlternateSetting: Alternate setting*/
	0x01,         /*bNumEndpoints*/
	0x03,         /*bInterfaceClass: HID*/
	0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
	0x02,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
	0,            /*iInterface: Index of string descriptor*/
	/******************** Descriptor of Joystick Mouse HID ********************/
	/* 18 */
	0x09,         /*bLength: HID Descriptor size*/
	HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
	0x00,         /*bcdHID: HID Class Spec release number*/
	0x01,
	0x00,         /*bCountryCode: Hardware target country*/
	0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
	0x22,         /*bDescriptorType*/
	JOYSTICK_SIZ_REPORT_DESC,/*wItemLength: Total length of Report descriptor*/
	0x00,
	/******************** Descriptor of Joystick Mouse endpoint ********************/
	/* 27 */
	0x07,          /*bLength: Endpoint Descriptor size*/
	USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/

	EP_HID_ADDRESS,          /*bEndpointAddress: Endpoint Address (IN)*/
	0x03,          /*bmAttributes: Interrupt endpoint*/
	0x04,          /*wMaxPacketSize: 4 Byte max */
	0x00,
	0x20,          /*bInterval: Polling Interval (32 ms)*/
	/* 34 */
}
;

#endif
const uint8_t Joystick_ReportDescriptor[JOYSTICK_SIZ_REPORT_DESC] = {
	0x05,          /*Usage Page(Generic Desktop)*/
	0x01,
	0x09,          /*Usage(Mouse)*/
	0x02,
	0xA1,          /*Collection(Logical)*/
	0x01,
	0x09,          /*Usage(Pointer)*/
	0x01,
	/* 8 */
	0xA1,          /*Collection(Linked)*/
	0x00,
	0x05,          /*Usage Page(Buttons)*/
	0x09,
	0x19,          /*Usage Minimum(1)*/
	0x01,
	0x29,          /*Usage Maximum(3)*/
	0x03,
	/* 16 */
	0x15,          /*Logical Minimum(0)*/
	0x00,
	0x25,          /*Logical Maximum(1)*/
	0x01,
	0x95,          /*Report Count(3)*/
	0x03,
	0x75,          /*Report Size(1)*/
	0x01,
	/* 24 */
	0x81,          /*Input(Variable)*/
	0x02,
	0x95,          /*Report Count(1)*/
	0x01,
	0x75,          /*Report Size(5)*/
	0x05,
	0x81,          /*Input(Constant,Array)*/
	0x01,
	/* 32 */
	0x05,          /*Usage Page(Generic Desktop)*/
	0x01,
	0x09,          /*Usage(X axis)*/
	0x30,
	0x09,          /*Usage(Y axis)*/
	0x31,
	0x09,          /*Usage(Wheel)*/
	0x38,
	/* 40 */
	0x15,          /*Logical Minimum(-127)*/
	0x81,
	0x25,          /*Logical Maximum(127)*/
	0x7F,
	0x75,          /*Report Size(8)*/
	0x08,
	0x95,          /*Report Count(3)*/
	0x03,
	/* 48 */
	0x81,          /*Input(Variable, Relative)*/
	0x06,
	0xC0,          /*End Collection*/
	0x09,
	0x3c,
	0x05,
	0xff,
	0x09,
	/* 56 */
	0x01,
	0x15,
	0x00,
	0x25,
	0x01,
	0x75,
	0x01,
	0x95,
	/* 64 */
	0x02,
	0xb1,
	0x22,
	0x75,
	0x06,
	0x95,
	0x01,
	0xb1,
	/* 72 */
	0x01,
	0xc0
}
; /* Joystick_ReportDescriptor */


/* USB string descriptors */
const uint8_t Joystick_StringLangID[JOYSTICK_SIZ_STRING_LANGID] = {
	JOYSTICK_SIZ_STRING_LANGID,           // bLength
	USB_STRING_DESCRIPTOR_TYPE,           // bDescriptorType
	0x09,                                 // LangID = 0x0409: U.S. English
	0x04
};

const uint8_t Joystick_StringVendor[JOYSTICK_SIZ_STRING_VENDOR] = {
	JOYSTICK_SIZ_STRING_VENDOR,           // bLength
	USB_STRING_DESCRIPTOR_TYPE,           // bDescriptorType
	'w', 0, 'w', 0, 'w', 0, '.', 0, 't', 0, 'u', 0, 'r', 0, 'i', 0,
	'n', 0, 'g', 0, 'b', 0, 'i', 0, 'r', 0, 'd', 0, 's', 0, '.', 0,
	'c', 0, 'o', 0, 'm', 0
};

const uint8_t Joystick_StringProduct[JOYSTICK_SIZ_STRING_PRODUCT] = {
	JOYSTICK_SIZ_STRING_PRODUCT,          // bLength
	USB_STRING_DESCRIPTOR_TYPE,           // bDescriptorType
	'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'w', 0, 'a', 0,
	'v', 0, 'e', 0, ' ', 0, 'd', 0, 'o', 0, 'p', 0, 'p', 0,
	'l', 0, 'e', 0, 'r', 0, ' ', 0, 's', 0, 'e', 0, 'n', 0,
	's', 0, 'o', 0, 'r', 0
};

uint8_t Joystick_StringSerial[JOYSTICK_SIZ_STRING_SERIAL] = {
	JOYSTICK_SIZ_STRING_SERIAL,           // bLength
	USB_STRING_DESCRIPTOR_TYPE,           // bDescriptorType
	'J', 0, 'o', 0, 'y', 0, 'S', 0, 't', 0, 'i', 0, 'c', 0,
	'k', 0, 'M', 0, 'o', 0, 'u', 0, 's', 0, 'e'
};




