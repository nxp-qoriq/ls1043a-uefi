/** @CfiNorFlashLib.c

 Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

 This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php

 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

 **/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/FslIfc.h>
#include <Library/IoLib.h>
#include <Library/NorFlashLib.h>
#include <Library/TimerLib.h>

#include "Mt28ew01gabaCfiNorFlashLib.h"

VOID
FlashWrite_Data (
   FLASH_DATA Val,
   UINTN Addr
   )
{
	*(volatile FLASH_DATA *)(Addr) = (Val);
	// Wait for write to complete
	MicroSecondDelay(1000);
}

VOID
FlashWrite (
   FLASH_DATA Val,
   UINTN Addr
   )
{
	FLASH_DATA ShiftVal = Val;
	
	if(sizeof(FLASH_DATA) == sizeof(UINT16))	
		ShiftVal = SwapBytes16(Val);
	else if(sizeof(FLASH_DATA) == sizeof(UINT32))
		ShiftVal = SwapBytes32(Val);
	else if(sizeof(FLASH_DATA) == sizeof(UINT64))
		ShiftVal = SwapBytes64(Val);
	else
		ASSERT("Could Not determine FLASH_DATA size\n");
	
	*(volatile FLASH_DATA *)(Addr) = (ShiftVal);
	// Wait for write to complete
	MicroSecondDelay(1000);
}

FLASH_DATA
FlashRead_Data (
  UINTN Addr
  )
{
	FLASH_DATA Val = *(volatile FLASH_DATA *)(Addr);

	return (Val);
}

FLASH_DATA
FlashRead (
  UINTN Addr
  )
{
	FLASH_DATA Val = *(volatile FLASH_DATA *)(Addr);
	FLASH_DATA ShiftVal = Val;
		
	if(sizeof(FLASH_DATA) == sizeof(UINT16))	
		ShiftVal = SwapBytes16(Val);
	else if(sizeof(FLASH_DATA) == sizeof(UINT32))
		ShiftVal = SwapBytes32(Val);
	else if(sizeof(FLASH_DATA) == sizeof(UINT64))
		ShiftVal = SwapBytes64(Val);
	else
		ASSERT("Could Not determine FLASH_DATA size\n");

	return (ShiftVal);
}

STATIC
VOID
NorFlashReadCfiData (
  IN  UINTN                   DeviceBaseAddress,
  IN  UINTN                   CFI_Offset,
  IN  UINT32                  NumberOfShorts,
  OUT VOID                    *Data
  )
{
  UINT32                Count;
  FLASH_DATA		*TmpData = (FLASH_DATA *)Data;
	
  for(Count = 0; Count < NumberOfShorts; Count++, TmpData++) {
      *TmpData = FlashRead ((UINTN)((FLASH_DATA*)DeviceBaseAddress + CFI_Offset));
      CFI_Offset++;
  }
}

/*
  Currently we support only CFI flash devices; Bail-out otherwise
*/
EFI_STATUS
CfiNorFlashFlashGetAttributes (
  OUT NOR_FLASH_DESCRIPTION  *NorFlashDevices,
  IN UINT32                  Index
  )
{
  UINT32                  Count;
  FLASH_DATA              QryData[3] = {0, 0, 0};
  FLASH_DATA              MajorIdReadData;
  FLASH_DATA              MinorIdReadData;
  FLASH_DATA              BlockSize[2] = {0, 0};
  UINTN                   DeviceBaseAddress = 0;
  FLASH_DATA              MaxNumBytes[2] = {0, 0};
  FLASH_DATA              Size = 0;
  FLASH_DATA              HighByteMask = 0xFF; // Masks High byte in a UIN16 word
  FLASH_DATA              HighByteShift = 8; // Bitshifts needed to make a byte High Byte in a UIN16 word
  
  for (Count = 0; Count < Index; Count++) {

    NorFlashDevices[Count].DeviceBaseAddress = DeviceBaseAddress = PcdGet64(PcdFlashDeviceBase64);

     // Reset flash first
    SEND_NOR_COMMAND (DeviceBaseAddress, 0, MT28EW01GABA_CMD_RESET);
    SEND_NOR_COMMAND (DeviceBaseAddress, 0, 0xFF);

     // Enter the CFI Query Mode
    SEND_NOR_COMMAND (DeviceBaseAddress, MT28EW01GABA_ENTER_CFI_QUERY_MODE_ADDR, MT28EW01GABA_ENTER_CFI_QUERY_MODE_CMD);
		
     // Query the unique QRY
    NorFlashReadCfiData(DeviceBaseAddress, MT28EW01GABA_CFI_QUERY_UNIQUE_QRY_STRING, 3, &QryData);
    if (QryData[0] != (FLASH_DATA)CFI_QRY_Q || QryData[1] != (FLASH_DATA)CFI_QRY_R || QryData[2] != (FLASH_DATA)CFI_QRY_Y ) {
	DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Not a CFI flash (QRY not recvd): Got = 0x%04x, 0x%04x, 0x%04x\n", QryData[0], QryData[1], QryData[2]));
        return EFI_DEVICE_ERROR;
     }
  
    // Check we have the desired NOR flash slave (MT28EW01GABA or S29GL01GS) connected
    MajorIdReadData = 0;
    NorFlashReadCfiData(DeviceBaseAddress, MT28EW01GABA_CFI_VENDOR_ID_MAJOR_ADDR, 1, &MajorIdReadData);
    if (MajorIdReadData != (FLASH_DATA)MT28EW01GABA_CFI_VENDOR_ID_MAJOR) {
      DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Cannot find a MT28EW01GABA flash (MAJOR ID), Expected=0x%x, Got=0x%x\n", MT28EW01GABA_CFI_VENDOR_ID_MAJOR, (FLASH_DATA)MajorIdReadData));
      return EFI_DEVICE_ERROR;
    }
  
    MinorIdReadData = 0;
    NorFlashReadCfiData(DeviceBaseAddress, MT28EW01GABA_CFI_VENDOR_ID_MINOR_ADDR, 1, &MinorIdReadData);
     
    if (MinorIdReadData != (FLASH_DATA)MT28EW01GABA_CFI_VENDOR_ID_MINOR) {
      DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Cannot find a MT28EW01GABA flash (MINOR ID), Expected=0x%x, Got=0x%x\n", MT28EW01GABA_CFI_VENDOR_ID_MINOR, (FLASH_DATA)MinorIdReadData));
      return EFI_DEVICE_ERROR;
    }

    NorFlashReadCfiData(DeviceBaseAddress, MT28EW01GABA_CFI_QUERY_DEVICE_SIZE, 1, &Size);
    // Refer CFI Specification
    NorFlashDevices[Count].Size = 1 << Size;
	 
    NorFlashReadCfiData(DeviceBaseAddress, MT28EW01GABA_CFI_QUERY_BLOCK_SIZE, 2, &BlockSize);
    // Refer CFI Specification
    NorFlashDevices[Count].BlockSize = 256*((FLASH_DATA)((BlockSize[1] << HighByteShift) | (BlockSize[0] & HighByteMask)));

    NorFlashReadCfiData(DeviceBaseAddress, MT28EW01GABA_CFI_QUERY_MAX_NUM_BYTES_WRITE, 2, &MaxNumBytes);
    // Refer CFI Specification
    /* from CFI query we get the Max. number of BYTE in multi-byte write = 2^N. */
    /* But our Flash Library is able to read/write in WORD size (2 bytes in our case). */
    /* which is why we need to CONVERT MAX BYTES TO MAX WORDS by diving it by width of word size */
    NorFlashDevices[Count].MultiByteWordCount =\
    (1 << ((FLASH_DATA)((MaxNumBytes[1] << HighByteShift) | (MaxNumBytes[0] & HighByteMask))))/sizeof(FLASH_DATA);
    
    // Put device back into Read Array mode (via Reset)
    SEND_NOR_COMMAND (NorFlashDevices[Count].DeviceBaseAddress, 0, MT28EW01GABA_CMD_RESET);
  }
  
  return EFI_SUCCESS;
}

