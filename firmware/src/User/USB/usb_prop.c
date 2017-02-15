/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : usb_prop.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : All processings related to Joystick Mouse Demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"


/**
 *	logging
**/

volatile uint32_t usb_vendor_command_buf;

extern volatile uint8_t log_seq_num;
extern volatile uint32_t log_integration_window;
extern volatile uint16_t prev_log_signal_level;


/**
 *	USB Audio Class Feature Unit controls
**/

volatile uint32_t MUTE_DATA = 0;
volatile uint32_t VOLUME_DATA = 0;


/**
 *	volume is specified using two words and in dB:

//   0x7FFF: 127.9961 dB
//   ...
//   0x0100: 1.0000 dB
//   ...
//   0x0002: 0.0078 dB
//   0x0001: 0.0039 dB
//   0x0000: 0.0000 dB
//   0xFFFF: -0.0039 dB
//   0xFFFE: -0.0078 dB
//   ...
//   0xFE00: -1.0000 dB
//   ...
//   0x8002: -127.9922 dB
//   0x8001: -127.9961 dB
//   0x8000 -inf dB (silence)
//
// steps are 1/256 dB or 0.00390625 dB (0x0001)
//
 * See.: USB Device Class Definition for Audio Devices 1.0 Table 5-18

 * the actual values are somewhat arbitrary, but should correspond to reasonable dB levels
 * note that these values do not correspond to a nice linear scaling with the volume slider in your OS
**/

const uint32_t MIN_DATA = (uint32_t)0xEFFF;		// -16 dB
const uint32_t MAX_DATA = (uint32_t)0x1000;		// +16 dB
const uint32_t RES_DATA = (uint32_t)0x001E;


/**
 *	level trackers
**/

//extern uint16_t input_level_tracker_USB;
//extern uint16_t input_level_tracker_USB_COPY;
//volatile uint16_t signal_level_decay = 0x7fff;

uint32_t ProtocolValue;
volatile uint8_t gryffindor = 0;



/* -------------------------------------------------------------------------- */
/*  Structures initializations */
/* -------------------------------------------------------------------------- */

DEVICE Device_Table = {
	EP_NUM,
	1
};

DEVICE_PROP Device_Property = {
	Joystick_init,
	Joystick_Reset,
	Joystick_Status_In,
	Joystick_Status_Out,
	Joystick_Data_Setup,
	Joystick_NoData_Setup,
	Joystick_Get_Interface_Setting,
	Joystick_GetDeviceDescriptor,
	Joystick_GetConfigDescriptor,
	Joystick_GetStringDescriptor,
	0,
	USB_MAX_PACKET_SIZE /*MAX PACKET SIZE*/
};
USER_STANDARD_REQUESTS User_Standard_Requests = {
	Joystick_GetConfiguration,
	Joystick_SetConfiguration,
	Joystick_GetInterface,
	Joystick_SetInterface,
	Joystick_GetStatus,
	(void(*)(void))Joystick_ClearFeature,
	Joystick_SetEndPointFeature,
	Joystick_SetDeviceFeature,
	Joystick_SetDeviceAddress
};

ONE_DESCRIPTOR Device_Descriptor = {
	(uint8_t*)Joystick_DeviceDescriptor,
	JOYSTICK_SIZ_DEVICE_DESC
};

ONE_DESCRIPTOR Config_Descriptor = {
	(uint8_t*)Joystick_ConfigDescriptor,
	SPEAKER_SIZ_CONFIG_DESC
};

ONE_DESCRIPTOR Joystick_Report_Descriptor = {
	(uint8_t *)Joystick_ReportDescriptor,
	JOYSTICK_SIZ_REPORT_DESC
};

ONE_DESCRIPTOR Mouse_Hid_Descriptor = {
	(uint8_t*)Joystick_ConfigDescriptor + JOYSTICK_OFF_HID_DESC,
	JOYSTICK_SIZ_HID_DESC
};

ONE_DESCRIPTOR String_Descriptor[4] = {
	{(uint8_t*)Joystick_StringLangID, JOYSTICK_SIZ_STRING_LANGID},
	{(uint8_t*)Joystick_StringVendor, JOYSTICK_SIZ_STRING_VENDOR},
	{(uint8_t*)Joystick_StringProduct, JOYSTICK_SIZ_STRING_PRODUCT},
	{(uint8_t*)Joystick_StringSerial, JOYSTICK_SIZ_STRING_SERIAL}
};

/* Private function prototypes -----------------------------------------------*/

void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);

/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Joystick_init.
* Description    : Joystick Mouse init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Joystick_init(void) {

	/* Update the serial number string descriptor with the data from the unique
	ID*/
	Get_SerialNum();

	pInformation->Current_Configuration = 0;
	/* Connect the device */
	PowerOn();

	/* Perform basic device initialization operations */
	USB_SIL_Init();

	bDeviceState = UNCONNECTED;
}

void Joystick_SetDeviceFeature(void) {

}

/*******************************************************************************
* Function Name  : Joystick_Reset.
* Description    : Joystick Mouse reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Joystick_Reset(void) {

	/* Set Joystick_DEVICE as not configured */
	pInformation->Current_Configuration = 0;
	pInformation->Current_Interface = 0;/*the default Interface*/

	/* Current Feature initialization */
	pInformation->Current_Feature = Joystick_ConfigDescriptor[7];




	SetBTABLE(BTABLE_ADDRESS);

	/* Initialize Endpoint 0 */
	SetEPType(ENDP0, EP_CONTROL);
	SetEPTxStatus(ENDP0, EP_TX_NAK);
	SetEPRxAddr(ENDP0, ENDP0_RXADDR);
	SetEPTxAddr(ENDP0, ENDP0_TXADDR);
	Clear_Status_Out(ENDP0);
	SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
	SetEPTxCount(ENDP0, Device_Property.MaxPacketSize);
	SetEPRxValid(ENDP0);
	//SetEPTxValid(ENDP0); // XXX added for testing vendor specific OUT

	SetEPType(ENDP1, EP_ISOCHRONOUS);
	SetEPDblBuffAddr(ENDP1, ENDP1_BUF0Addr, ENDP1_BUF1Addr);

	//SetEPRxAddr(ENDP1, ENDP1_RXADDR);
	//SetEPTxAddr(ENDP1, ENDP1_TXADDR);
	//SetEPRxCount(ENDP1, 0);
	//SetEPTxCount(ENDP1, 16);//EP_ISOCHRONOUS_MAX_PACKET_SIZE);/* packet size*/


	SetEPDblBuffCount(ENDP1, EP_DBUF_IN, 16 + (1 << 10));
	SetEPDblBuf0Count(ENDP1, EP_DBUF_IN, 16 + (1 << 10));	/* initial data */
	SetEPDblBuf1Count(ENDP1, EP_DBUF_IN, 16 + (1 << 10));

	ClearDTOG_RX(ENDP1);
	ClearDTOG_TX(ENDP1);
	ToggleDTOG_TX(ENDP1);

	SetEPTxStatus(ENDP1, EP_TX_VALID);
	SetEPRxStatus(ENDP1, EP_RX_DIS);


	/* Initialize Endpoint 2 */
	SetEPType(ENDP2, EP_INTERRUPT);
	SetEPTxAddr(ENDP2, ENDP2_TXADDR);
	SetEPTxCount(ENDP2, 4);
	SetEPRxStatus(ENDP2, EP_RX_DIS);
	SetEPTxStatus(ENDP2, EP_TX_NAK);







#ifdef LOLCATS234
	/* Initialize Endpoint 0 */
	SetEPType(ENDP0, EP_CONTROL);
	SetEPTxStatus(ENDP0, EP_TX_NAK);
	SetEPRxAddr(ENDP0, ENDP0_RXADDR);
	SetEPTxAddr(ENDP0, ENDP0_TXADDR);
	Clear_Status_Out(ENDP0);
	SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
	SetEPRxValid(ENDP0);

	/* Initialize Endpoint 1 */
	SetEPType(ENDP1, EP_ISOCHRONOUS);
	SetEPDblBuffAddr(ENDP1, ENDP1_BUF0Addr, ENDP1_BUF1Addr);
	SetEPDblBuffCount(ENDP1, EP_DBUF_OUT, 0xC0);
	ClearDTOG_RX(ENDP1);
	ClearDTOG_TX(ENDP1);
	ToggleDTOG_TX(ENDP1);
	SetEPRxStatus(ENDP1, EP_RX_VALID);
	SetEPTxStatus(ENDP1, EP_TX_DIS);
#endif
	/* Set this device to response on default address */
	SetDeviceAddress(0);

	bDeviceState = ATTACHED;

}
/*******************************************************************************
* Function Name  : Joystick_SetConfiguration.
* Description    : Udpade the device state to configured.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Joystick_SetConfiguration(void) {
	DEVICE_INFO *pInfo = &Device_Info;

	if (pInfo->Current_Configuration != 0) {
		/* Device configured */
		bDeviceState = CONFIGURED;
	}
}
/*******************************************************************************
* Function Name  : Joystick_SetConfiguration.
* Description    : Udpade the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Joystick_SetDeviceAddress (void) {
	bDeviceState = ADDRESSED;
}
/*******************************************************************************
* Function Name  : Joystick_Status_In.
* Description    : Joystick status IN routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Joystick_Status_In(void) {
}

/*******************************************************************************
* Function Name  : Joystick_Status_Out
* Description    : Joystick status OUT routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Joystick_Status_Out (void) {
}

/*******************************************************************************
* Function Name  : Joystick_Data_Setup
* Description    : Handle the data class specific requests.
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
/* All class specified requests with data stage are processed in Class_Data_Setup
 Class_Data_Setup()
  responses to check all special requests and fills ENDPOINT_INFO
  according to the request
  If IN tokens are expected, then wLength & wOffset will be filled
  with the total transferring bytes and the starting position
  If OUT tokens are expected, then rLength & rOffset will be filled
  with the total expected bytes and the starting position in the buffer

  If the request is valid, Class_Data_Setup returns SUCCESS, else UNSUPPORT

 CAUTION:
  Since GET_CONFIGURATION & GET_INTERFACE are highly related to
  the individual classes, they will be checked and processed here.
*/
RESULT Joystick_Data_Setup(uint8_t RequestNo) {
	uint8_t *(*CopyRoutine)(uint16_t);
	uint8_t req_type = pInformation->USBbmRequestType;
	CopyRoutine = NULL;

	if ((req_type & REQUEST_TYPE) == VENDOR_REQUEST) {
		// vendor-specific request
		if (req_type & 0x80) {
			// direction is IN (device to host)
			CopyRoutine = Vendor_Command_IN;
		} else {
			// direction is OUT (host to device)
			CopyRoutine = Vendor_Command_OUT;
		}
	} else if (RequestNo == GET_CUR) {
		CopyRoutine = Get_Cur_Command;
	} else if (RequestNo == SET_CUR) {
		CopyRoutine = Set_Cur_Command;
	} else if (RequestNo == GET_MIN) {
		CopyRoutine = Get_Min_Command;
	} else if (RequestNo == GET_MAX) {
		CopyRoutine = Get_Max_Command;
	} else if (RequestNo == GET_RES) {
		CopyRoutine = Get_Res_Command;
	} else  if ((RequestNo == GET_DESCRIPTOR)
				&& (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
				&& (pInformation->USBwIndex0 == 0)) {
		if (pInformation->USBwValue1 == REPORT_DESCRIPTOR) {
			CopyRoutine = Joystick_GetReportDescriptor;
		} else if (pInformation->USBwValue1 == HID_DESCRIPTOR_TYPE) {
			CopyRoutine = Joystick_GetHIDDescriptor;
		}
	} else if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
			   && RequestNo == GET_PROTOCOL) {
		CopyRoutine = Joystick_GetProtocolValue;
	} else {
		return USB_UNSUPPORT;
	}

	pInformation->Ctrl_Info.CopyData = CopyRoutine;
	pInformation->Ctrl_Info.Usb_wOffset = 0;
	(*CopyRoutine)(0);


	/**
	 * update log time integration window if required
	**/

	if (usb_vendor_command_buf >> 29 == COMMAND_SET_INTEGRATION_WINDOW) {
		log_integration_window = usb_vendor_command_buf & 0x3FFFFFFF;
	}

	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Joystick_NoData_Setup
* Description    : handle the no data class specific requests
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT Joystick_NoData_Setup(uint8_t RequestNo) {
	if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
			&& (RequestNo == SET_PROTOCOL)) {
		return Joystick_SetProtocol();
	}

	else {
		return USB_UNSUPPORT;
	}
}

/*******************************************************************************
* Function Name  : Joystick_GetDeviceDescriptor.
* Description    : Gets the device descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the device descriptor.
*******************************************************************************/
uint8_t *Joystick_GetDeviceDescriptor(uint16_t Length) {
	return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/*******************************************************************************
* Function Name  : Joystick_GetConfigDescriptor.
* Description    : Gets the configuration descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *Joystick_GetConfigDescriptor(uint16_t Length) {
	return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*******************************************************************************
* Function Name  : Joystick_GetStringDescriptor
* Description    : Gets the string descriptors according to the needed index
* Input          : Length
* Output         : None.
* Return         : The address of the string descriptors.
*******************************************************************************/
uint8_t *Joystick_GetStringDescriptor(uint16_t Length) {
	uint8_t wValue0 = pInformation->USBwValue0;
	if (wValue0 > 4) {
		return NULL;
	} else {
		return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
	}
}


/*******************************************************************************
* Function Name  : Joystick_GetReportDescriptor.
* Description    : Gets the HID report descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *Joystick_GetReportDescriptor(uint16_t Length) {
	return Standard_GetDescriptorData(Length, &Joystick_Report_Descriptor);
}

/*******************************************************************************
* Function Name  : Joystick_GetHIDDescriptor.
* Description    : Gets the HID descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *Joystick_GetHIDDescriptor(uint16_t Length) {
	return Standard_GetDescriptorData(Length, &Mouse_Hid_Descriptor);
}
/*******************************************************************************
* Function Name  : Joystick_Get_Interface_Setting.
* Description    : tests the interface and the alternate setting according to the
*                  supported one.
* Input          : - Interface : interface number.
*                  - AlternateSetting : Alternate Setting number.
* Output         : None.
* Return         : USB_SUCCESS or USB_UNSUPPORT.
*******************************************************************************/
RESULT Joystick_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting) {
	if (AlternateSetting > 1) {
		return USB_UNSUPPORT;
	} else if (Interface > 2) {
		return USB_UNSUPPORT;
	}
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Joystick_SetProtocol
* Description    : Joystick Set Protocol request routine.
* Input          : None.
* Output         : None.
* Return         : USB SUCCESS.
*******************************************************************************/
RESULT Joystick_SetProtocol(void) {
	uint8_t wValue0 = pInformation->USBwValue0;
	ProtocolValue = wValue0;
	return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Joystick_GetProtocolValue
* Description    : get the protocol value
* Input          : Length.
* Output         : None.
* Return         : address of the protcol value.
*******************************************************************************/
uint8_t *Joystick_GetProtocolValue(uint16_t Length) {
	if (Length == 0) {
		pInformation->Ctrl_Info.Usb_wLength = 1;
		return NULL;
	} else {
		return (uint8_t *)(&ProtocolValue);
	}
}

uint8_t *Set_Cur_Command(uint16_t length) {
	//LED3(1);
	// if length is 0, return the available data length
	if (length == 0) {
		// if volume control is addressed, data length is two words
		// if mute control is addressed, data length is one word
		pInformation->Ctrl_Info.Usb_wLength = pInformation->USBwLengths.w;
		return (uint8_t*)(pInformation->Ctrl_Info.Usb_wLength);
	} else {
		// if Length is not 0, return user buffer address
		if (pInformation->USBwValues.bw.bb1 & 0x02) {
			// Control Selector = 2, i.e. volume control
			return((uint8_t*)(&VOLUME_DATA));
		} else {
			// Control Selector = 1, i.e. mute control
			return((uint8_t*)(&MUTE_DATA));
		}
	}
}

uint8_t *Get_Cur_Command(uint16_t length) {
	//LED3(1);
	if (length == 0) {
		// if length is 0, return the available data length
		// if volume control is addressed, data length is two words
		// if mute control is addressed, data length is one word
		pInformation->Ctrl_Info.Usb_wLength = pInformation->USBwLengths.w;
		return  (uint8_t*)(pInformation->Ctrl_Info.Usb_wLength);

	} else {
		// if Length is not 0, return user buffer address
		if (pInformation->USBwValues.bw.bb1 & FEATURE_UNIT_CONTROL_SELECTOR_VOLUME_CONTROL) {	// XXX: replace ``&`` with ``==``?
			// Control Selector = 2, i.e. volume control
			/*if (gryffindor == 0) {
				gryffindor = 1;
			}
			else {
				gryffindor = 0;
			}
			//LED3(gryffindor); //led flashes as volume is adjusted
			*/
			return((uint8_t*)(&VOLUME_DATA));
		} else {
			// Control Selector = 1, i.e. mute control
			return((uint8_t*)(&MUTE_DATA));
		}
	}
}


/**
 * Vendor_Command_IN
 *
 * Send signal level data to the host
**/
uint8_t *Vendor_Command_IN(uint16_t length) {
	if (length == 0) {
		// if length is 0, return the available data length
		// data length is two words
		pInformation->Ctrl_Info.Usb_wLength = 4;//1*pInformation->USBwLengths.w;
		return (uint8_t*)(pInformation->Ctrl_Info.Usb_wLength);
	} else {
		// if Length is not 0, return user buffer address

		if (usb_vendor_command_buf >> 29 == COMMAND_GET_INTEGRATION_WINDOW) {
			usb_vendor_command_buf = log_integration_window;
		} else if (usb_vendor_command_buf >> 29 == COMMAND_GET_SEQ_NUM) {
			usb_vendor_command_buf = log_seq_num;
		} else if (usb_vendor_command_buf >> 29 == COMMAND_GET_SIGNAL_LEVEL) {
			usb_vendor_command_buf = prev_log_signal_level;
		} else {
			usb_vendor_command_buf = 0;
		}

		return(uint8_t*)(&usb_vendor_command_buf);
	}
}


/**
 * Vendor_Command_OUT
 *
 * Read signal level integration time constant from the host
**/
uint8_t *Vendor_Command_OUT(uint16_t length) {
	if (length == 0) {
		// if length is 0, return the available data length
		// data length is two words
		pInformation->Ctrl_Info.Usb_wLength = 4;//1*pInformation->USBwLengths.w;
		return (uint8_t*)(pInformation->Ctrl_Info.Usb_wLength);
	} else {
		// if Length is not 0, return user buffer address
		return( uint8_t*)(&usb_vendor_command_buf);
	}
}


uint8_t *Get_Max_Command(uint16_t length) {
	//LED3(1);
	if (length == 0) {
		// if length is 0, return the available data length
		// data length is two words
		pInformation->Ctrl_Info.Usb_wLength = 2*pInformation->USBwLengths.w;
		return (uint8_t*)(pInformation->Ctrl_Info.Usb_wLength);
	} else {
		// if Length is not 0, return user buffer address
		return((uint8_t*)(&MAX_DATA));
	}
}

uint8_t *Get_Min_Command(uint16_t length) {

	if (length == 0) {
		// if length is 0, return the available data length
		// data length is one word
		pInformation->Ctrl_Info.Usb_wLength = pInformation->USBwLengths.w;
		return (uint8_t*)(pInformation->Ctrl_Info.Usb_wLength);
	} else {
		// if Length is not 0, return user buffer address
		return((uint8_t*)(&MIN_DATA));
	}
}

/**
 *
**/
uint8_t *Get_Res_Command(uint16_t length) {

	if (length == 0) {
		// if length is 0, return the available data length
		// data length is one word
		pInformation->Ctrl_Info.Usb_wLength = pInformation->USBwLengths.w;
		return (uint8_t*)(pInformation->Ctrl_Info.Usb_wLength);
	} else {
		// if Length is not 0, return user buffer address
		return((uint8_t*)(&RES_DATA));
	}
}

/*******************************************************************************
* Function Name  : HexToChar.
* Description    : Convert Hex 32Bits value into char.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len) {
	uint8_t idx = 0;

	for( idx = 0 ; idx < len ; idx ++) {
		if( ((value >> 28)) < 0xA ) {
			pbuf[ 2* idx] = (value >> 28) + '0';
		} else {
			pbuf[2* idx] = (value >> 28) + 'A' - 10;
		}

		value = value << 4;

		pbuf[ 2* idx + 1] = 0;
	}
}


/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void) {
	uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

	/* copy serial number from stm32 Unique device ID register */
	Device_Serial0 = *(__IO uint32_t*)(0x1FFFF7E8);
	Device_Serial1 = *(__IO uint32_t*)(0x1FFFF7EC);
	Device_Serial2 = *(__IO uint32_t*)(0x1FFFF7F0);

	Device_Serial0 += Device_Serial2;

	if (Device_Serial0 != 0) {
		IntToUnicode (Device_Serial0, &Joystick_StringSerial[2] , 8);
		IntToUnicode (Device_Serial1, &Joystick_StringSerial[18], 4);
	}
}
