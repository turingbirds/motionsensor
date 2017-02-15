/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : usb_prop.h
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : All processings related to Joystick Mouse demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_PROP_H
#define __USB_PROP_H
typedef enum _HID_REQUESTS {
	GET_REPORT = 1,
	GET_IDLE,
	GET_PROTOCOL,

	SET_REPORT = 9,
	SET_IDLE,
	SET_PROTOCOL
} HID_REQUESTS;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Joystick_init(void);
void Joystick_Reset(void);
void Joystick_SetConfiguration(void);
void Joystick_SetDeviceAddress (void);
void Joystick_Status_In (void);
void Joystick_Status_Out (void);
RESULT Joystick_Data_Setup(uint8_t);
RESULT Joystick_NoData_Setup(uint8_t);
RESULT Joystick_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *Joystick_GetDeviceDescriptor(uint16_t );
uint8_t *Joystick_GetConfigDescriptor(uint16_t);
uint8_t *Joystick_GetStringDescriptor(uint16_t);
RESULT Joystick_SetProtocol(void);
uint8_t *Joystick_GetProtocolValue(uint16_t Length);
RESULT Joystick_SetProtocol(void);
uint8_t *Joystick_GetReportDescriptor(uint16_t Length);
uint8_t *Get_Max_Command(uint16_t Length);
uint8_t *Get_Min_Command(uint16_t Length);
uint8_t *Get_Res_Command(uint16_t length);
uint8_t *Get_Cur_Command(uint16_t Length);
uint8_t *Set_Cur_Command(uint16_t length);
uint8_t *Vendor_Command_IN(uint16_t length);
uint8_t *Vendor_Command_OUT(uint16_t length);



// USB Device Class Definition for Audio Devices 1.0 Table A-11
#define FEATURE_UNIT_CONTROL_SELECTOR_FU_CONTROL_UNDEFINED    0x00
#define FEATURE_UNIT_CONTROL_SELECTOR_MUTE_CONTROL            0x01
#define FEATURE_UNIT_CONTROL_SELECTOR_VOLUME_CONTROL          0x02
void Joystick_SetDeviceFeature(void);


void Get_SerialNum(void);
uint8_t *Joystick_GetReportDescriptor(uint16_t Length);
uint8_t *Joystick_GetHIDDescriptor(uint16_t Length);



/* Exported define -----------------------------------------------------------*/
#define Joystick_GetConfiguration          NOP_Process
//#define Joystick_SetConfiguration          NOP_Process
#define Joystick_GetInterface              NOP_Process
#define Joystick_SetInterface              NOP_Process
#define Joystick_GetStatus                 NOP_Process//Standard_GetStatus
#define Joystick_ClearFeature              NOP_Process//Standard_ClearFeature
#define Joystick_SetEndPointFeature        NOP_Process
//#define Joystick_SetDeviceFeature          NOP_Process
//#define Joystick_SetDeviceAddress          NOP_Process


#define REPORT_DESCRIPTOR                  0x22


/**
 * 	some definitions for the USB logging interface
**/

#define COMMAND_SET_INTEGRATION_WINDOW		0x04
#define COMMAND_GET_INTEGRATION_WINDOW		0x01
#define COMMAND_GET_SIGNAL_LEVEL		0x02
#define COMMAND_GET_SEQ_NUM		0x03


/**
 *	USB Device Class Definition for Audio Devices 1.0 Table A-11
**/

#define FEATURE_UNIT_CONTROL_SELECTOR_FU_CONTROL_UNDEFINED    0x00
#define FEATURE_UNIT_CONTROL_SELECTOR_MUTE_CONTROL            0x01
#define FEATURE_UNIT_CONTROL_SELECTOR_VOLUME_CONTROL          0x02

/**
 * Values for unit control requests
 *
 * __[1] "USB Device Class Definition for Audio Devices 1.0"
 *      section C.4.2
**/
#define GET_CUR		0x81
#define GET_MIN		0x82
#define GET_MAX		0x83
#define GET_RES		0x84
#define SET_CUR		0x01




#endif /* __USB_PROP_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
