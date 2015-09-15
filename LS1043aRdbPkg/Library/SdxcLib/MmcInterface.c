/** @MmcInterface.c

  Functions for providing Library interface APIs.

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
  if (Getcd() == 0) {
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
  EFI_STATUS Status;
  struct SdxcCfg *Cfg = gMmc->Private;
  struct SdxcRegs *Regs = (struct SdxcRegs *)Cfg->SdxcBase;

  Status = RecvResp(Regs, NULL, RespType, Buffer);

  return Status;
}

EFI_STATUS
MmcSendCommand (
  IN  struct SdCmd *Cmd
  )
{
  return SendCmd(Cmd, NULL);
}

EFI_STATUS
MmcSendReset (
  VOID
  )
{
  return SdxcGoIdle(gMmc);
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

VOID
DestroyMmc (
  IN VOID
  )
{
  if (gMmc) {
    FreePool(gMmc->Cfg);
    FreePool(gMmc->Private);
    FreePool(gMmc);
  }
}

VOID
SelectSdxc (
  IN VOID
  )
{
  UINT8 Data = 0;

  /* Enable soft mux */
  Data = CPLD_READ(SoftMuxOn);
  if ((Data & (ENABLE_SDXC_SOFT_MUX | ENABLE_RCW_SOFT_MUX)) 
	!= (ENABLE_SDXC_SOFT_MUX | ENABLE_RCW_SOFT_MUX))
    CPLD_WRITE(SoftMuxOn, (Data | (ENABLE_SDXC_SOFT_MUX |
					ENABLE_RCW_SOFT_MUX)));

  /* Enable sdhc */
  Data = CPLD_READ(SdhcSpiCsSel);
  if ((Data & 0x01) != 0x00)
    CPLD_WRITE(SdhcSpiCsSel, (Data & 0xFE));

  /* Enable sdhc clock */
  Data = CPLD_READ(TdmClkMuxSel);
  if ((Data & 0x01) != 0x01)
    CPLD_WRITE(TdmClkMuxSel, (Data | 0x01));

  /* configure SW4 and SW5 for SDXC/eMMC */
  Data = CPLD_READ(RcwSource1);
  if ((Data & SELECT_SW4_SDXC) != SELECT_SW4_SDXC)
    CPLD_WRITE(RcwSource1, SELECT_SW4_SDXC);

  Data = CPLD_READ(RcwSource2);
  if ((Data & SELECT_SW5_SDXC) != SELECT_SW5_SDXC)
    CPLD_WRITE(RcwSource2, SELECT_SW5_SDXC);
}

EFI_STATUS
MmcInitialize (
  VOID
  )
{
  SelectSdxc();

  if (BoardInit() < 0)
    if (SdxcMmcInit() < 0)
      return EFI_DEVICE_ERROR;

  return EFI_SUCCESS;
}
