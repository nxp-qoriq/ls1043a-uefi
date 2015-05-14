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
#include <Library/IoLib.h>
#include <Library/NorFlashPlatformLib.h>

#include "Am29LV065muCfiNorFlashLib.h"

STATIC
VOID
NorFlashReadCfiData (
  IN  UINTN                   DeviceBaseAddress,
  IN  UINTN                   CFI_Offset,
  IN  UINT32                  NumberOfBytes,
  OUT UINT32                  *Data
  )
{
  UINTN                 Count;
  UINT8			*ByteData = (UINT8 *)Data;
	
  for(Count = 0; Count < NumberOfBytes; Count++, ByteData++, CFI_Offset--) 
      *ByteData = MmioRead32 ((UINTN)(DeviceBaseAddress + CFI_Offset));
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
     SEND_NOR_COMMAND (AM29LV065MU_ENTER_CFI_QUERY_MODE_ADDR, 0, AM29LV065MU_ENTER_CFI_QUERY_MODE_CMD);
  
     // Query the unique QRY
     NorFlashReadCfiData(NorFlashDevices[Count]->DeviceBaseAddress, AM29LV065MU_CFI_QUERY_UNIQUE_QRY_THIRD, 3, &Data);
     if (Data != CFI_QRY) {
       DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Not a CFI flash\n"));
       return EFI_DEVICE_ERROR;
     }
  
     // Check we have the desired NOR flash slave (AM29LV065MU) connected
     NorFlashReadCfiData(NorFlashDevices[Count]->DeviceBaseAddress, AM29LV065MU_CFI_VENDOR_ID_MAJOR_ADDR, 1, &Data);
     if (Data != AM29LV065MU_CFI_VENDOR_ID_MAJOR) {
       DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Cannot find a AM29LV065MU flash\n"));
       return EFI_DEVICE_ERROR;
     }
  
     NorFlashReadCfiData(NorFlashDevices[Count]->DeviceBaseAddress, AM29LV065MU_CFI_VENDOR_ID_MINOR_ADDR, 1, &Data);
     if (Data != AM29LV065MU_CFI_VENDOR_ID_MINOR) {
       DEBUG ((EFI_D_ERROR, "CfiNorFlashFlashGetAttributes: ERROR - Cannot find a AM29LV065MU flash\n"));
       return EFI_DEVICE_ERROR;
     }

     // If we reach here, we are connected to the right NOR flash slave.
     // Now, retrieve the device geometry definition
  
     // Block Size
     NorFlashReadCfiData(NorFlashDevices[Count]->DeviceBaseAddress, AM29LV065MU_CFI_QUERY_BLOCK_SIZE, 2, &BlockSize);
     NorFlashDevices[Count]->BlockSize = (UINTN)BlockSize;
  
     // Device Size
     NorFlashReadCfiData(NorFlashDevices[Count]->DeviceBaseAddress, AM29LV065MU_CFI_QUERY_DEVICE_SIZE, 1, &Size);
     NorFlashDevices[Count]->Size = (UINTN)Size;

     // Put device back into Read Array mode (via Reset)
     SEND_NOR_COMMAND (NorFlashDevices[Count]->DeviceBaseAddress, 0, AM29LV065MU_CMD_RESET);
  }
  
  return EFI_SUCCESS;
}

