/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : usb_istr.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : ISTR events interrupt service routines
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_istr.h"


volatile uint8_t fudge;
volatile uint8_t hummus;
volatile uint8_t phase_8bit;
volatile uint16_t blinky;

__IO uint16_t wIstr;  /* ISTR register last read value */
__IO uint8_t bIntPackSOF = 0;  /* SOFs received between 2 consecutive packets */


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* function pointers to non-control endpoints service routines */
void (*pEpInt_IN[7])(void) = {
	EP1_IN_Callback,
	EP2_IN_Callback,
	EP3_IN_Callback,
	EP4_IN_Callback,
	EP5_IN_Callback,
	EP6_IN_Callback,
	EP7_IN_Callback,
};

void (*pEpInt_OUT[7])(void) = {
	EP1_OUT_Callback,
	EP2_OUT_Callback,
	EP3_OUT_Callback,
	EP4_OUT_Callback,
	EP5_OUT_Callback,
	EP6_OUT_Callback,
	EP7_OUT_Callback,
};








/*******************************************************************************
* Function Name  : EP1_OUT_Callback
* Description    : Endpoint 1 out callback routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void) {
	uint16_t pma_buf_local_addr;
	uint16_t *pma_buf_addr;
	uint16_t i;
	int16_t _data_len;
	uint16_t data_len, ch1_data_len, ch2_data_len;       /* data length*/

	// find out which buffers to use
	// cf. STM32 reference manual Table 171.
	if (GetENDPOINT(ENDP1) & EP_DTOG_TX) {
		// buf1 used by USB peripheral; buf0 used by application software
		pma_buf_local_addr = ENDP1_BUF0Addr;
	} else {
		// buf0 used by USB peripheral; buf1 used by application software
		pma_buf_local_addr = ENDP1_BUF1Addr;
	}

	_data_len = adcbuf_idx_write - adcbuf_idx_read;
	if (_data_len > 0) {
		// data in circular buffer is contiguous.
		data_len = (uint16_t)_data_len;
		data_len = MIN(data_len, EP_ISOCHRONOUS_MAX_PACKET_SIZE);

		// set up DMA transfer to the inactive USB packet buffer
		//.. from adcbuf_idx_read
		//.. length data_len
		pma_buf_addr = (uint16_t*)(pma_buf_local_addr * 2 + PMAAddr);
		for (i=0; i < data_len; ++i) {
			*(pma_buf_addr+2*i) = adcbuf[adcbuf_idx_read+i];
		}
	} else {
		// data is non-contiguous in circular buffer.
		// transfer the two chunks separately

		// set up ch0 DMA transfer to the inactive USB packet buffer
		//.. from adcbuf_idx_read
		//.. to the end of the buffer
		ch1_data_len = ADC_BUFFER_SIZE - adcbuf_idx_read;
		ch1_data_len = MIN(ch1_data_len, EP_ISOCHRONOUS_MAX_PACKET_SIZE);

		pma_buf_addr = (uint16_t*)(pma_buf_local_addr * 2 + PMAAddr);
		for (i=0; i < ch1_data_len; ++i) {
			*(pma_buf_addr+2*i) = adcbuf[adcbuf_idx_read+i];
		}

		// set up ch1 DMA transfer to the inactive USB packet buffer
		//.. from 0
		//.. to the current value of adcbuf_idx_write
		ch2_data_len = adcbuf_idx_write;
		if (ch1_data_len == EP_ISOCHRONOUS_MAX_PACKET_SIZE ||
				ch1_data_len == ADC_BUFFER_SIZE) {
			ch2_data_len = 0;
		}

		pma_buf_addr = (uint16_t*)(pma_buf_local_addr * 2 + PMAAddr);
		for (i=0; i < ch2_data_len; ++i) {
			*(pma_buf_addr+2*(i+ch1_data_len)) = adcbuf[i];
		}

		data_len = ch1_data_len + ch2_data_len;
	}
	//UserToPMABufferCopy((uint8_t*)&(adcbuf[adcbuf_idx_read]), pma_buf_addr, ch1_data_len);

	adcbuf_idx_read = (adcbuf_idx_read + data_len) % ADC_BUFFER_SIZE;

	if (GetENDPOINT(ENDP1) & EP_DTOG_TX) {
		SetEPDblBuf0Count(ENDP1, EP_DBUF_IN, (data_len << 1) + (1 << 10));
	} else {
		SetEPDblBuf1Count(ENDP1, EP_DBUF_IN, (data_len << 1) + (1 << 10));
	}
	FreeUserBuffer(ENDP1,EP_DBUF_IN);
	SetEPTxValid(ENDP1);		// should not be necessary

#ifdef LOLCATS
	// enter critical section
	// copy ADC buffer to packet memory area
	//UserToPMABufferCopy(adc_buffer, ENDP1_BUF0Addr, adc_buffer_offset);
	//UserToPMABufferCopy(adc_buffer, ENDP1_BUF1Addr, adc_buffer_offset);
	pma_buf_addr = (uint16_t *)(ENDP1_BUF0Addr * 2 + PMAAddr);
	for (i=0; i < 16; ++i) {
		*(pma_buf_addr+2*i) = (uint16_t)((adcbuf)[i]<<2);
	}
	pma_buf_addr = (uint16_t *)(ENDP1_BUF1Addr * 2 + PMAAddr);
	for (i=0; i < 16; ++i) {
		*(pma_buf_addr+2*i) = (uint16_t)((adcbuf)[i]<<2);
	}
	//*(pma_buf_addr) = 0xFFFF - (uint16_t)((*adc_buffer)[0]<<2);//0x02ff;
	/*for (i=0; i < adc_buffer_offset; ++i) {
		*(pma_buf_addr) = (uint16_t)(adc_buffer[0] & 0xff);
	}*/
	/*pma_buf_addr = (uint16_t *)(ENDP1_BUF1Addr * 2 + PMAAddr);
	for (i=0; i < adc_buffer_offset; ++i) {
		*(pma_buf_addr+2*i) = 0;//adc_buffer[0];
	}*/
	data_len = 16;//adc_buffer_offset;
	//adc_buffer_offset = 0;
	// exit critical section

#endif
#ifdef FOOTESTING
	pma_buf_addr = (uint16_t *)(ENDP1_BUF0Addr * 2 + PMAAddr);

	*pma_buf_addr = 0xFFFF - 0x0003;//(((uint16_t)phase_8bit) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+2) = 0xFFFF - 0x0002;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+4) = 0xFFFF - 0x0001;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+6) = 0xFFFF;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	//*(pma_buf_addr+8) = 0x0000;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+10) = 0x0001;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+12) = 0x0002;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+14) = 0x0003;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);

	pma_buf_addr = (uint16_t *)(ENDP1_BUF1Addr * 2 + PMAAddr);

	*pma_buf_addr = 0x0003;//(((uint16_t)phase_8bit) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+2) = 0x0002;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+4) = 0x0001;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+6) = 0x0000;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+8) = 0xFFFF;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+10) = 0xFFFF - 0x0001;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+12) = 0xFFFF - 0x0002;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);
	*(pma_buf_addr+14) = 0xFFFF - 0x0003;//(((uint16_t)(0xff-phase_8bit)) << 8) + (0xff-phase_8bit);

	//*(pma_buf_addr+8) = adc_buffer[0];
	*(pma_buf_addr+8) = (uint16_t)(adc_buffer[0]);
	//adc_buffer[0] = adc_buffer[1];
#endif

	++phase_8bit;


	//if (*pma_buf_addr % 2) {		// if an ADC sample is ready
	//SetEPDblBuffCount(ENDP1, EP_DBUF_OUT, data_len);
	//}
	//else {
	//		SetEPTxCount(ENDP1, 0);/* packet size*/
	//}
	//ToggleDTOG_TX(ENDP1);
	//SetEPTxValid(ENDP1);		// should not be necessary

#ifdef LOLCATS0000

	if (GetENDPOINT(ENDP1) & EP_DTOG_TX) {
		/*read from ENDP1_BUF0Addr buffer*/
		Data_Len = GetEPDblBuf0Count(ENDP1);
		PMAToUserBufferCopy(Stream_Buff, ENDP1_BUF0Addr, Data_Len);
	} else {
		/*read from ENDP1_BUF1Addr buffer*/
		Data_Len = GetEPDblBuf1Count(ENDP1);
		PMAToUserBufferCopy(Stream_Buff, ENDP1_BUF1Addr, Data_Len);
	}
	FreeUserBuffer(ENDP1, EP_DBUF_OUT);
	In_Data_Offset += Data_Len;
#endif
}



/*******************************************************************************
* Function Name  : USB_LP_IRQHandler
* Description    : ISTR events interrupt service routine
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_LP_IRQHandler(void) {
//  CTR_LP();
	wIstr = _GetISTR();

#if (IMR_MSK & ISTR_CTR)
	if (wIstr & ISTR_CTR & wInterrupt_Mask) {
		/* servicing of the endpoint correct transfer interrupt */
		/* clear of the CTR flag into the sub */
		CTR_LP();
#ifdef CTR_CALLBACK
		CTR_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_RESET)
	if (wIstr & ISTR_RESET & wInterrupt_Mask) {
		_SetISTR((uint16_t)CLR_RESET);
		Device_Property.Reset();
#ifdef RESET_CALLBACK
		RESET_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_DOVR)
	if (wIstr & ISTR_DOVR & wInterrupt_Mask) {
		_SetISTR((uint16_t)CLR_DOVR);
#ifdef DOVR_CALLBACK
		DOVR_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_ERR)
	if (wIstr & ISTR_ERR & wInterrupt_Mask) {
		_SetISTR((uint16_t)CLR_ERR);
#ifdef ERR_CALLBACK
		ERR_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_WKUP)
	if (wIstr & ISTR_WKUP & wInterrupt_Mask) {
		_SetISTR((uint16_t)CLR_WKUP);
		Resume(RESUME_EXTERNAL);
#ifdef WKUP_CALLBACK
		WKUP_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_SUSP)
	if (wIstr & ISTR_SUSP & wInterrupt_Mask) {

		/* check if SUSPEND is possible */
		if (fSuspendEnabled) {
			Suspend();
		} else {
			/* if not possible then resume after xx ms */
			Resume(RESUME_LATER);
		}
		/* clear of the ISTR bit must be done after setting of CNTR_FSUSP */
		_SetISTR((uint16_t)CLR_SUSP);
#ifdef SUSP_CALLBACK
		SUSP_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_SOF)
	if (wIstr & ISTR_SOF & wInterrupt_Mask) {
		_SetISTR((uint16_t)CLR_SOF);
		bIntPackSOF++;

#ifdef SOF_CALLBACK
		SOF_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_ESOF)
	if (wIstr & ISTR_ESOF & wInterrupt_Mask) {
		_SetISTR((uint16_t)CLR_ESOF);
		/* resume handling timing is made with ESOFs */
		Resume(RESUME_ESOF); /* request without change of the machine state */

#ifdef ESOF_CALLBACK
		ESOF_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

} /* USB_Istr */
/*******************************************************************************
* Function Name  : USB_Istr
* Description    : Start of frame callback function.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SOF_Callback(void) {

	if (hummus == 0) {
		//LED4(1);
		hummus = 1;
	} else {
		//LED4(0);
		hummus= 0;
	}

	/*  if(!Samples) Out_Data_Offset = 390;
	  Samples = 0;
	*/
}



/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

