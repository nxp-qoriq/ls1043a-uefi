#ifndef __DUART_H__
#define __DUART_H__

#include <Uefi.h>
#include <Protocol/SerialIo.h>

/*
* These are the definitions for the FIFO Control Register
*/
#define UART_FCR_FIFO_EN	0x01 /* Fifo enable */
#define UART_FCR_CLEAR_RCVR	0x02 /* Clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT	0x04 /* Clear the XMIT FIFO */
#define UART_FCR_DMA_SELECT	0x08 /* For DMA applications */
#define UART_FCR_TRIGGER_MASK	0xC0 /* Mask for the FIFO trigger range */
#define UART_FCR_TRIGGER_1	0x00 /* Mask for trigger set at 1 */
#define UART_FCR_TRIGGER_4	0x40 /* Mask for trigger set at 4 */
#define UART_FCR_TRIGGER_8	0x80 /* Mask for trigger set at 8 */
#define UART_FCR_TRIGGER_14	0xC0 /* Mask for trigger set at 14 */

#define UART_FCR_RXSR		0x02 /* Receiver soft reset */
#define UART_FCR_TXSR		0x04 /* Transmitter soft reset */

/*
* These are the definitions for the Modem Control Register
*/
#define UART_MCR_DTR	0x01		/* DTR   */
#define UART_MCR_RTS	0x02		/* RTS   */
#define UART_MCR_OUT1	0x04		/* Out 1 */
#define UART_MCR_OUT2	0x08		/* Out 2 */
#define UART_MCR_LOOP	0x10		/* Enable loopback test mode */

#define UART_MCR_DMA_EN	0x04
#define UART_MCR_TX_DFR	0x08

/*
* These are the definitions for the Line Control Register
*
* Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting
* UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
*/
#define UART_LCR_WLS_MSK 0x03		/* character length select mask */
#define UART_LCR_WLS_5	0x00		/* 5 bit character length */
#define UART_LCR_WLS_6	0x01		/* 6 bit character length */
#define UART_LCR_WLS_7	0x02		/* 7 bit character length */
#define UART_LCR_WLS_8	0x03		/* 8 bit character length */
#define UART_LCR_STB	0x04		/* # stop Bits, off=1, on=1.5 or 2) */
#define UART_LCR_PEN	0x08		/* Parity eneble */
#define UART_LCR_EPS	0x10		/* Even Parity Select */
#define UART_LCR_STKP	0x20		/* Stick Parity */
#define UART_LCR_SBRK	0x40		/* Set Break */
#define UART_LCR_BKSE	0x80		/* Bank select enable */
#define UART_LCR_DLAB	0x80		/* Divisor latch access bit */

/*
* These are the definitions for the Line Status Register
*/
#define UART_LSR_DR	0x01		/* Data ready */
#define UART_LSR_OE	0x02		/* Overrun */
#define UART_LSR_PE	0x04		/* Parity error */
#define UART_LSR_FE	0x08		/* Framing error */
#define UART_LSR_BI	0x10		/* Break */
#define UART_LSR_THRE	0x20		/* Xmit holding register empty */
#define UART_LSR_TEMT	0x40		/* Xmitter empty */
#define UART_LSR_ERR	0x80		/* Error */

#define UART_MSR_DCD	0x80		/* Data Carrier Detect */
#define UART_MSR_RI	0x40		/* Ring Indicator */
#define UART_MSR_DSR	0x20		/* Data Set Ready */
#define UART_MSR_CTS	0x10		/* Clear to Send */
#define UART_MSR_DDCD	0x08		/* Delta DCD */
#define UART_MSR_TERI	0x04		/* Trailing edge ring indicator */
#define UART_MSR_DDSR	0x02		/* Delta DSR */
#define UART_MSR_DCTS	0x01		/* Delta CTS */

/*
* These are the definitions for the Interrupt Identification Register
*/
#define UART_IIR_NO_INT	0x01	/* No interrupts pending */
#define UART_IIR_ID	0x06	/* Mask for the interrupt ID */

#define UART_IIR_MSI	0x00	/* Modem status interrupt */
#define UART_IIR_THRI	0x02	/* Transmitter holding register empty */
#define UART_IIR_RDI	0x04	/* Receiver data interrupt */
#define UART_IIR_RLSI	0x06	/* Receiver line status interrupt */

/*
* These are the definitions for the Interrupt Enable Register
*/
#define UART_IER_MSI	0x08	/* Enable Modem status interrupt */
#define UART_IER_RLSI	0x04	/* Enable receiver line status interrupt */
#define UART_IER_THRI	0x02	/* Enable Transmitter holding register int. */
#define UART_IER_RDI	0x01	/* Enable receiver data interrupt */


#ifdef CONFIG_OMAP1510
#define OSC_12M_SEL	0x01	/* selects 6.5 * current clk div */
#endif

/* useful defaults for LCR */
#define UART_LCR_8N1	0x03

#define UART_LCRVAL UART_LCR_8N1                /* 8 data, 1 stop, no parity */
#define UART_MCRVAL (UART_MCR_DTR | \
                     UART_MCR_RTS)              /* RTS/DTR */
#define UART_FCRVAL (UART_FCR_FIFO_EN | \
                     UART_FCR_RXSR |    \
                     UART_FCR_TXSR)             /* Clear & enable FIFOs */


#define URBR		0x0
#define UTHR		0x0
#define UDLB	 	0x0	 
#define UDMB	 	0x0	 
#define UIER	 	0x1 
#define UIIR	 	0x2 
#define UFCR	 	0x2 
#define UAFR	 	0x2 
#define ULCR	 	0x3 
#define UMCR	 	0x4
#define ULSR		0x5
#define UMSR		0x6
#define USCR		0x7
#define UDSR		0x10

/*

  Programmed hardware of Serial port.

  @return    Always return EFI_UNSUPPORTED.

**/
RETURN_STATUS
EFIAPI
DuartInitializePort (
  IN OUT UINTN               UartBase,
  IN OUT UINT64              *BaudRate,
  );


/**
  Write data to serial device.

  @param  Buffer           Point of data buffer which need to be written.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Write data failed.
  @retval !0               Actual number of bytes written to serial device.

**/
UINTN
EFIAPI
DuartWrite (
  IN  UINTN       UartBase,
  IN  UINT8       *Buffer,
  IN  UINTN       NumberOfBytes
  );

/**
  Read data from serial device and save the data in buffer.

  @param  Buffer           Point of data buffer which need to be written.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Read data failed.
  @retval !0               Actual number of bytes read from serial device.

**/
UINTN
EFIAPI
DuartRead (
  IN  UINTN       UartBase,
  OUT UINT8       *Buffer,
  IN  UINTN       NumberOfBytes
  );

/**
  Check to see if any data is available to be read from the debug device.

  @retval EFI_SUCCESS       At least one byte of data is available to be read
  @retval EFI_NOT_READY     No data is available to be read
  @retval EFI_DEVICE_ERROR  The serial device is not functioning properly

**/
BOOLEAN
EFIAPI
DuartPoll (
  IN  UINTN       UartBase
  );

#endif /* __DUART_H__ */



