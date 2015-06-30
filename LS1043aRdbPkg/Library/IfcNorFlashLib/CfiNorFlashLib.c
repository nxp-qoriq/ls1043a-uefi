/** @file CfiNorFlashLib.c

 Copyright (c) 2014, Freescale Ltd. All rights reserved.

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
#include <Library/NorFlashPlatformLib.h>

#include "Mt28ew01gabaCfiNorFlashLib.h"

UINT8
FlashRead8 (
  UINTN Addr
  )
{
	UINT8 Val = *(volatile UINT8 *)(Addr);

	return Val;
}

VOID
FlashWrite8 (
  UINT8 Val,
  UINTN Addr
  )
{
	*(volatile UINT8 *)(Addr) = (Val);
}

VOID
FlashWrite16 (
   UINT16 Val,
   UINTN Addr
   )
{
	UINT16 ShiftVal = (((Val >> 8) & 0xff) | ((Val << 8) & 0xff00));

	*(volatile UINT16 *)(Addr) = (ShiftVal);
}

UINT16
FlashRead16 (
  UINTN Addr
  )
{
	UINT16 Val = *(volatile UINT16 *)(Addr);

	return (((Val) >> 8) & 0x00ff) | (((Val) << 8) & 0xff00);
}

VOID
FlashWrite32 (
   UINT32 Val,
   UINTN Addr
   )
{
	*(volatile UINT32 *)(Addr) = (Val);
}

UINT32
FlashRead32 (
  UINTN Addr
  )
{
	UINT32 Val = *(volatile UINT32 *)(Addr);

	return (Val);
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
  UINT16		*TmpData = (UINT16 *)Data;
	
  for(Count = 0; Count < NumberOfShorts; Count++, TmpData++) {
      *TmpData = FlashRead16 (DeviceBaseAddress + CFI_Offset);
      CFI_Offset -= 2;

  }
}

/*
  Currently we support only CFI flash devices; Bail-out otherwise
*/
EFI_STATUS
CfiNorFlashFlashGetAttributes (
  OUT NOR_FLASH_DESCRIPTION  **NorFlashDevices,
  IN UINT32                  Index
  )
{
  UINT32                  Count;
  UINT16                  QryData[3] = {0, 0, 0};
  UINT16                  MajorIdReadData;
  UINT16                  MinorIdReadData;
  UINT16       		  Size;
  UINT16       		  BlockSize[2] = {0, 0};
 
  for (Count = 0; Count < Index; Count++) {
     // Reset flash first
     SEND_NOR_COMMAND (NorFlashDevices[Count]->DeviceBaseAddress, 0, MT28EW01GABA_CMD_RESET);
     SEND_NOR_COMMAND (NorFlashDevices[Count]->DeviceBaseAddress, 0, 0xFF);

     // Enter the CFI Query Mode
     SEND_NOR_COMMAND (NorFlashDevices[Count]->DeviceBaseAddress, MT28EW01GABA_ENTER_CFI_QUERY_MODE_ADDR, MT28EW01GABA_ENTER_CFI_QUERY_MODE_CMD);
     DEBUG ((EFI_D_ERROR, " CFI enter mode command addr=0x%x, cmd=0x%x\n", IFC_NOR_BUF_BASE + MT28EW01GABA_ENTER_CFI_QUERY_MODE_ADDR, MT28EW01GABA_ENTER_CFI_QUERY_MODE_CMD));
		
     // Query the unique QRY
     NorFlashReadCfiData((NorFlashDevices[Count]->DeviceBaseAddress), MT28EW01GABA_CFI_QUERY_UNIQUE_QRY_THIRD, 3, &QryData);
     if (QryData[0] != (UINT16)CFI_QRY_Y || QryData[1] != (UINT16)CFI_QRY_R || QryData[2] != (UINT16)CFI_QRY_Q ) {
	DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Not a CFI flash (QRY not recvd): Got = 0x%x, 0x%x, 0x%x\n", QryData[0], QryData[1], QryData[2]));
        return EFI_DEVICE_ERROR;
     }
  
     // Check we have the desired NOR flash slave (MT28EW01GABA) connected
     MajorIdReadData = 0;
     NorFlashReadCfiData((NorFlashDevices[Count]->DeviceBaseAddress), MT28EW01GABA_CFI_VENDOR_ID_MAJOR_ADDR, 1, &MajorIdReadData);
     if (MajorIdReadData != (UINT16)MT28EW01GABA_CFI_VENDOR_ID_MAJOR) {
        DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Cannot find a MT28EW01GABA flash (MAJOR ID), Expected=0x%x, Got=0x%x\n", MT28EW01GABA_CFI_VENDOR_ID_MAJOR, (UINT16)MajorIdReadData));
       return EFI_DEVICE_ERROR;
     }
  
     MinorIdReadData = 0;
     NorFlashReadCfiData((NorFlashDevices[Count]->DeviceBaseAddress), MT28EW01GABA_CFI_VENDOR_ID_MINOR_ADDR, 1, &MinorIdReadData);
      if ((UINT8)MinorIdReadData != MT28EW01GABA_CFI_VENDOR_ID_MINOR) {
        DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Cannot find a MT28EW01GABA flash (MINOR ID), Expected=0x%x, Got=0x%x\n", MT28EW01GABA_CFI_VENDOR_ID_MINOR, (UINT8)MinorIdReadData));
       return EFI_DEVICE_ERROR;
     }

     // If we reach here, we are connected to the right NOR flash slave.
     // Now, retrieve the device geometry definition
  
     // Block Size
     NorFlashReadCfiData((NorFlashDevices[Count]->DeviceBaseAddress), MT28EW01GABA_CFI_QUERY_BLOCK_SIZE, 2, &BlockSize);
     NorFlashDevices[Count]->BlockSize = 128 * 1024; //BlockSize = 128KB;
  
     // Device Size
     Size = 0;
     NorFlashReadCfiData((NorFlashDevices[Count]->DeviceBaseAddress), MT28EW01GABA_CFI_QUERY_DEVICE_SIZE, 1, &Size);
     NorFlashDevices[Count]->Size = (2 << (UINT8)Size); // 2 ^ Size

     // Put device back into Read Array mode (via Reset)
     SEND_NOR_COMMAND (NorFlashDevices[Count]->DeviceBaseAddress, 0, MT28EW01GABA_CMD_RESET);
  }
  
  return EFI_SUCCESS;
}

