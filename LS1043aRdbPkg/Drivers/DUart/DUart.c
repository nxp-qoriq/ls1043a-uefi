/** Duart.c
  Duart driver

Copyright (c) 2013, Freescale Ltd. All rights reserved.
Author: Bhupesh Sharma <bhupesh.sharma@freescale.com>

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

 **/

#include <LS1043aSocLib.h>

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#include <Drivers/DUart.h>

UINT32
CalculateBaudDivisor (
  OUT UINT64 *BaudRate
  )
{
	UINTN DUartClk = GetPeripheralClock(UART_CLK);
	return ((DUartClk + *BaudRate * 8)/(*BaudRate * 16));
}

/*
   Initialise the serial port to the specified settings.
   All unspecified settings will be set to the default values.

   @return    Always return EFI_SUCCESS or EFI_INVALID_PARAMETER.

 **/
RETURN_STATUS
EFIAPI
DuartInitializePort (
  IN OUT UINTN UartBase,
  IN OUT UINT64 *BaudRate
  )
{
	UINT32	BaudDivisor;

	/* BaudDivisor = CalculateBaudDivisor(BaudRate); */
	BaudDivisor = 0xD9; /* FIXME: Use hardcoded value for now */

	while (!(MmioRead8(UartBase + ULSR) & UART_LSR_TEMT))
	;

	MmioWrite8(UartBase + UIER, 0);
	MmioWrite8(UartBase + ULCR, UART_LCR_BKSE | UART_LCRVAL);
	MmioWrite8(UartBase + UDLB, 0);
	MmioWrite8(UartBase + UDMB, 0);
	MmioWrite8(UartBase + ULCR, UART_LCRVAL);
	MmioWrite8(UartBase + UMCR, UART_MCRVAL);
	MmioWrite8(UartBase + UFCR, UART_FCRVAL);
	MmioWrite8(UartBase + ULCR, UART_LCR_BKSE | UART_LCRVAL);
	MmioWrite8(UartBase + UDLB, BaudDivisor & 0xff);
	MmioWrite8(UartBase + UDMB, (BaudDivisor >> 8) & 0xff);
	MmioWrite8(UartBase + ULCR, UART_LCRVAL);
	return RETURN_SUCCESS;
}

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
		IN  UINTN    UartBase,
		IN UINT8     *Buffer,
		IN UINTN     NumberOfBytes
	       )
{
	UINT8* CONST Final = &Buffer[NumberOfBytes];

	while (Buffer < Final) {
		while ((MmioRead8(UartBase + ULSR) & UART_LSR_THRE) == 0)
			                ;
		        MmioWrite8(UartBase + UTHR, *Buffer++);
	}

	return NumberOfBytes;
}

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
		IN  UINTN     UartBase,
		OUT UINT8     *Buffer,
		IN  UINTN     NumberOfBytes
	      )
{
	UINTN   Count;

	for (Count = 0; Count < NumberOfBytes; Count++, Buffer++) {
		/*
		 * Loop while waiting for a new char(s) to arrive in the
		 * RxFIFO
		 */
		while ((MmioRead8(UartBase + ULSR) & UART_LSR_DR) == 0);

		*Buffer = MmioRead8(UartBase + URBR);
	}

	return NumberOfBytes;
}

/**
  Check to see if any data is available to be read from the debug device.

  @retval EFI_SUCCESS       At least one byte of data is available to be read
  @retval EFI_NOT_READY     No data is available to be read
  @retval EFI_DEVICE_ERROR  The serial device is not functioning properly

 **/
BOOLEAN
EFIAPI
DuartPoll (
		IN  UINTN     UartBase
	      )
{
	return ((MmioRead8 (UartBase + ULSR) & UART_LSR_DR) != 0);
}

