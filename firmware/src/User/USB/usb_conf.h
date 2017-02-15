#ifndef __USB_CONF_H
#define __USB_CONF_H


// EP_NUM: defines how many endpoints are used by the device
#define EP_NUM     (3)

// ---------------------------------------------------------------------------
// 				Buffer Description Table
// ---------------------------------------------------------------------------

// N.B. total buffer is 256 words * 16 bits = 512 bytes
//   Buffer table memory is shared with packet buffers.

// BTABLE_ADDRESS:
//   Buffer table base address. Three lowest bits are always 000.
#define BTABLE_ADDRESS      (0x00)

// endpoint 0 (control): 8 words
#define ENDP0_TXADDR        (0x10)
#define ENDP0_RXADDR        (0x50)//(ENDP0_TXADDR+2*EP0_MAX_PACKET_SIZE)

// endpoint 1 (isochronous): 96 words
// double buffer
#define ENDP1_BUF0Addr      (0x90)
#define ENDP1_BUF1Addr      (ENDP1_BUF0Addr+EP_ISOCHRONOUS_MAX_PACKET_SIZE)

// endpoint 2 (interrupt: 4 words
#define ENDP2_TXADDR        (ENDP1_BUF1Addr+EP_ISOCHRONOUS_MAX_PACKET_SIZE)

// ---------------------------------------------------------------------------
// 				Interrupts
// ---------------------------------------------------------------------------

// IMR_MSK: mask defining which events has to be handled by the device
// application software
//#define IMR_MSK (CNTR_CTRM  | CNTR_SOFM  | CNTR_RESETM )
#define IMR_MSK (CNTR_CTRM  | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM  | CNTR_SOFM \
                 | CNTR_ESOFM | CNTR_RESETM )


// ---------------------------------------------------------------------------
// 				Callbacks
// ---------------------------------------------------------------------------

#define SOF_CALLBACK

/* CTR service routines */
/* associated to defined endpoints */
//#define  EP1_IN_Callback   NOP_Process
#define  EP2_IN_Callback   NOP_Process
#define  EP3_IN_Callback   NOP_Process
#define  EP4_IN_Callback   NOP_Process
#define  EP5_IN_Callback   NOP_Process
#define  EP6_IN_Callback   NOP_Process
#define  EP7_IN_Callback   NOP_Process

#define  EP1_OUT_Callback   NOP_Process
#define  EP2_OUT_Callback   NOP_Process
#define  EP3_OUT_Callback   NOP_Process
#define  EP4_OUT_Callback   NOP_Process
#define  EP5_OUT_Callback   NOP_Process
#define  EP6_OUT_Callback   NOP_Process
#define  EP7_OUT_Callback   NOP_Process

#endif

