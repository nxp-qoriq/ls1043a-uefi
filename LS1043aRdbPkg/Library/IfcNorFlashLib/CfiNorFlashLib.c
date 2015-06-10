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

STATIC
VOID
NorFlashReadCfiData (
  IN  UINTN                   DeviceBaseAddress,
  IN  UINTN                   CFI_Offset,
  IN  UINT32                  NumberOfBytes,
  OUT UINT32                  *Data
  )
{
  UINT32                Count;
  UINT8			*ByteData = (UINT8 *)Data;
	
  for(Count = 0; Count < NumberOfBytes; Count++, ByteData++, CFI_Offset--) 
      *ByteData = MmioRead8 ((UINTN)(DeviceBaseAddress + CFI_Offset));
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
  UINT32                  Data;
  UINT32       		  Size;
  UINT32       		  BlockSize;
  
  for (Count = 0; Count < Index; Count++) {
     // Enter the CFI Query Mode
  SEND_NOR_COMMAND (IFC_NOR_BUF_BASE, MT28EW01GABA_ENTER_CFI_QUERY_MODE_ADDR, MT28EW01GABA_ENTER_CFI_QUERY_MODE_CMD);
		
     // Query the unique QRY
		Data = 0;
     NorFlashReadCfiData(NorFlashDevices[Count]->DeviceBaseAddress, MT28EW01GABA_CFI_QUERY_UNIQUE_QRY_THIRD, 3, &Data);
     if (Data != CFI_QRY) {
			DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Not a CFI flash (QRY not recvd): Expected = 0x%x, Got = 0x%x\n", CFI_QRY, Data));
       return EFI_DEVICE_ERROR;
     }
  
     // Check we have the desired NOR flash slave (MT28EW01GABA) connected
 		 Data = 0;
     NorFlashReadCfiData(NorFlashDevices[Count]->DeviceBaseAddress, MT28EW01GABA_CFI_VENDOR_ID_MAJOR_ADDR, 1, &Data);
      if ((UINT8)Data != MT28EW01GABA_CFI_VENDOR_ID_MAJOR) {
        DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Cannot find a MT28EW01GABA flash (MAJOR ID), Expected=0x%x, Got=0x%x\n", MT28EW01GABA_CFI_VENDOR_ID_MAJOR, (UINT8)Data));
       return EFI_DEVICE_ERROR;
     }
  
 		 Data = 0;
     NorFlashReadCfiData(NorFlashDevices[Count]->DeviceBaseAddress, MT28EW01GABA_CFI_VENDOR_ID_MINOR_ADDR, 1, &Data);
      if ((UINT8)Data != MT28EW01GABA_CFI_VENDOR_ID_MINOR) {
        DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Cannot find a MT28EW01GABA flash (MINOR ID), Expected=0x%x, Got=0x%x\n", MT28EW01GABA_CFI_VENDOR_ID_MINOR, (UINT8)Data));
       return EFI_DEVICE_ERROR;
     }

     // If we reach here, we are connected to the right NOR flash slave.
     // Now, retrieve the device geometry definition
  
     // Block Size
		 BlockSize = 0;
     NorFlashReadCfiData(NorFlashDevices[Count]->DeviceBaseAddress, MT28EW01GABA_CFI_QUERY_BLOCK_SIZE, 2, &BlockSize);
     NorFlashDevices[Count]->BlockSize = 64 * 1024; //BlockSize;
  
     // Device Size
		 Size = 0;
     NorFlashReadCfiData(NorFlashDevices[Count]->DeviceBaseAddress, MT28EW01GABA_CFI_QUERY_DEVICE_SIZE, 1, &Size);
     NorFlashDevices[Count]->Size = (2 << (UINT8)Size); // 2 ^ Size

     // Put device back into Read Array mode (via Reset)
     SEND_NOR_COMMAND (NorFlashDevices[Count]->DeviceBaseAddress, 0, MT28EW01GABA_CMD_RESET);
  }
  
  return EFI_SUCCESS;
}

