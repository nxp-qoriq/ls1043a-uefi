/** @MmcInterface.c

  Functions for providing interface APIs.

  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution. The full text of the license
  may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/Sdxc.h>

extern struct Mmc *gMmc;

EFI_STATUS
DetectMmcPresence (
  OUT UINT8*  Data
  )
{
  if (MmcGetcd() == 0) {
    DEBUG((EFI_D_ERROR, "MMC, No Card Present\n"));
    *Data = FALSE;
    return EFI_NO_MEDIA;
  } else {
    *Data = TRUE;
    return EFI_SUCCESS;
  }
}

EFI_STATUS
MmcRcvResp (
  IN   UINT32   RespType,
  OUT  UINT32*  Buffer
  )
{
  EFI_STATUS Err;

  Err = ReceiveResponse(NULL, RespType, Buffer);

  return Err;
}

EFI_STATUS
MmcSendCommand (
  IN  struct MmcCmd *Cmd
  )
{
  return SdxcSendCmd(gMmc, Cmd, NULL);
}

EFI_STATUS
MmcSendReset (
  VOID
  )
{
  return MmcGoIdle(gMmc);
}

EFI_STATUS
InitMmc (
  IN EFI_BLOCK_IO_MEDIA *Media
  )
{
  EFI_STATUS Status;

  Status = DoMmcInfo();

  if (Status == EFI_SUCCESS) {
    Media->BlockSize = gMmc->BlockDev.Blksz;
    Media->LastBlock = gMmc->BlockDev.Lba;
  }

  return Status;
}

