/** SdxcLib.C
  Sdxc Library Containing Functions for Reset Read, Write, Initialize Etc

  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This Program And The Accompanying Materials
  Are Licensed And Made Available Under The Terms And Conditions Of The BSD License
  Which Accompanies This Distribution.  The Full Text Of The License May Be Found At
  Http://Opensource.Org/Licenses/Bsd-License.Php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/Sdxc.h>

extern struct Mmc *gMmc;

struct MmcOps SdxcOps = {
       .SendCmd     = SdxcSendCmd,
       .SetIos      = SdxcSetIos,
       .Init        = SdxcInit,
       .Getcd       = SdxcGetcd,
};

struct FslSdxcCfg SdxcCfg[1] = {
  {(VOID *)CONFIG_SYS_FSL_SDXC_ADDR},
};

/* return The XFERTYP Flags for A Given Command And Data Packet */
UINT32
SdxcXfertyp (
  IN  struct MmcCmd *Cmd,
  IN  struct MmcData *Data
  )
{
  UINT32 Xfertyp = 0;

  if (Data) {
    Xfertyp |= XFERTYP_DPSEL;
    if (Data->Blocks > 1) {
      Xfertyp |= XFERTYP_MSBSEL;
      Xfertyp |= XFERTYP_BCEN;
    }

    if (Data->Flags & MMC_DATA_READ)
      Xfertyp |= XFERTYP_DTDSEL;
  }

  if (Cmd->RespType & MMC_RSP_CRC)
    Xfertyp |= XFERTYP_CCCEN;
  if (Cmd->RespType & MMC_RSP_OPCODE)
    Xfertyp |= XFERTYP_CICEN;
  if (Cmd->RespType & MMC_RSP_136)
    Xfertyp |= XFERTYP_RSPTYP_136;
  else if (Cmd->RespType & MMC_RSP_BUSY)
    Xfertyp |= XFERTYP_RSPTYP_48_BUSY;
  else if (Cmd->RespType & MMC_RSP_PRESENT)
    Xfertyp |= XFERTYP_RSPTYP_48;

  if (Cmd->CmdIdx == MMC_CMD_STOP_TRANSMISSION)
    Xfertyp |= XFERTYP_CMDTYP_ABORT;

  return XFERTYP_CMD(Cmd->CmdIdx) | Xfertyp;
}

EFI_STATUS
SdxcSetupData (
  IN  struct Mmc *Mmc,
  IN  struct MmcData *Data
  )
{
  INT32 Timeout;
  struct FslSdxcCfg *Cfg = Mmc->Priv;
  struct FslSdxc *Regs = (struct FslSdxc *)Cfg->SdxcBase;

  UINT32 WmlValue;

  WmlValue = Data->Blocksize/4;

  if (Data->Flags & MMC_DATA_READ) {
    if (WmlValue > WML_RD_WML_MAX)
      WmlValue = WML_RD_WML_MAX_VAL;

    MmioClearSetBe32((UINTN)&Regs->Wml, WML_RD_WML_MASK, WmlValue);
  } else {
    if (WmlValue > WML_WR_WML_MAX)
      WmlValue = WML_WR_WML_MAX_VAL;
    if ((MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTAT_WPSPL) == 0) {
      DEBUG((EFI_D_ERROR, "The SD Card Is Locked. Can Not Write To A Locked Card.\n"));
      return EFI_TIMEOUT;
    }
    MmioClearSetBe32((UINTN)&Regs->Wml, WML_WR_WML_MASK, WmlValue << 16);
  }

  MmioWriteBe32((UINTN)&Regs->Blkattr, Data->Blocks << 16 | Data->Blocksize);

  /* Calculate The Timeout Period for Data Transactions */
  /*
   * 1)Timeout Period = (2^(Timeout+13)) SD Clock Cycles
   * 2)Timeout Period Should Be Minimum 0.250sec As Per SD Card Spec
   *  So, Number Of SD Clock Cycles for 0.25sec Should Be Minimum
   *		(SD Clock/Sec * 0.25 Sec) SD Clock Cycles
   *		= (Mmc->Clock * 1/4) SD Clock Cycles
   * As 1) >=  2)
   * => (2^(Timeout+13)) >= Mmc->Clock * 1/4
   * Taking Log2 Both The Sides
   * => Timeout + 13 >= Log2(Mmc->Clock/4)
   * Rounding Up To Next Power Of 2
   * => Timeout + 13 = Log2(Mmc->Clock/4) + 1
   * => Timeout + 13 = Fls(Mmc->Clock/4)
   */
  Timeout = GenericFls(Mmc->Clock/4);
  Timeout -= 13;

  if (Timeout > 14)
    Timeout = 14;

  if (Timeout < 0)
    Timeout = 0;

  MmioClearSetBe32((UINTN)&Regs->Sysctl, SYSCTL_TIMEOUT_MASK, Timeout << 16);

  return EFI_SUCCESS;
}

/*TODO*/
VOID
CheckAndInvalidateDcacheRange (
  IN  struct MmcData *Data
  )
{
  return;
//	UINT32 Start = (UINT32 *)Data->Dest;
//	UINT32 Size = RoundUp(ARCH_DMA_MINALIGN,
//				Data->Blocks*Data->Blocksize);
//	UINT32 End = Start+Size ;
//TODO	InvalidateDcacheRange(Start, End);
}

VOID
ResetCmdData (
  IN  struct MmcData *Data
  )
{
  struct FslSdxcCfg *Cfg = gMmc->Priv;
  volatile struct FslSdxc *Regs = (struct FslSdxc *)Cfg->SdxcBase;

  /* Reset CMD And DATA Portions On Error */
  MmioWriteBe32((UINTN)&Regs->Sysctl, MmioReadBe32((UINTN)&Regs->Sysctl) |
           SYSCTL_RSTC);

  while (MmioReadBe32((UINTN)&Regs->Sysctl) & SYSCTL_RSTC);

  if (Data) {
    MmioWriteBe32((UINTN)&Regs->Sysctl,
           MmioReadBe32((UINTN)&Regs->Sysctl) | SYSCTL_RSTD);
    while ((MmioReadBe32((UINTN)&Regs->Sysctl) & SYSCTL_RSTD))
     		;
  }

  MmioWriteBe32((UINTN)&Regs->Irqstat, -1);

}

static VOID
SdxcReadWrite (
 OUT struct MmcData *Data
  )
{
  struct FslSdxcCfg *Cfg = gMmc->Priv;
  struct FslSdxc *Regs = (struct FslSdxc *)Cfg->SdxcBase;
  UINT32 Blocks;
  CHAR8 *Buffer;
  UINT32 DataBuf;
  UINT32 Size;
  UINT32 Irqstat;
  UINT32 TimeOut;

  if (Data->Flags & MMC_DATA_READ) {
    Blocks = Data->Blocks;
    Buffer = Data->Dest;
    while (Blocks) {
      TimeOut = PIO_TIMEOUT;
      Size = Data->Blocksize;
      Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);
      while (!(MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTAT_BREN)
             && --TimeOut);
      if (TimeOut <= 0) {
        DEBUG((EFI_D_ERROR, "Data Read Failed in Polling Mode\n"));
        return;
      }
      while (Size && (!(Irqstat & IRQSTAT_TC))) {
        MicroSecondDelay(100); /* Wait before last byte transfer complete */
        Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);
        DataBuf = MmioReadBe32((UINTN)&Regs->Datport);
        *((UINT32 *)Buffer) = DataBuf;
        Buffer += 4;
        Size -= 4;
      }
      Blocks--;
    }
  } else {
    Blocks = Data->Blocks;
    Buffer = (CHAR8 *)Data->Src;
    while (Blocks) {
      TimeOut = PIO_TIMEOUT;
      Size = Data->Blocksize;
      Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);
      while (!(MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTAT_BWEN)
             && --TimeOut);
      if (TimeOut <= 0) {
        DEBUG((EFI_D_ERROR, "Data Write Failed in PIO Mode\n"));
        return;
      }
      while (Size && (!(Irqstat & IRQSTAT_TC))) {
        MicroSecondDelay(100); /* Wait before last byte transfer complete */
        DataBuf = *((UINT32 *)Buffer);
        Buffer += 4;
        Size -= 4;
        Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);
        MmioWriteBe32((UINTN)&Regs->Datport, DataBuf);
      }
      Blocks--;
    }
  }
}

EFI_STATUS
ReceiveResponse(
  IN  struct MmcData *Data,
  IN  UINT32 RespType,
  OUT UINT32 *Response
  )
{
  UINT32	Irqstat;
  struct FslSdxcCfg *Cfg = gMmc->Priv;
  volatile struct FslSdxc *Regs = (struct FslSdxc *)Cfg->SdxcBase;

   /* Workaround for SDXC Errata ENGcm03648 */
  if (!Data && (RespType & MMC_RSP_BUSY)) {
    INT32 Timeout = 2500;

    /* Poll On DATA0 Line for Cmd With Busy Signal for 250 Ms */
    while (Timeout > 0 && !(MmioReadBe32((UINTN)&Regs->Prsstat) &
				PRSSTAT_DAT0)) {
      MicroSecondDelay(100);
      Timeout--;
    }

    if (Timeout <= 0) {
      DEBUG((EFI_D_ERROR, "Timeout Waiting for DAT0 To Go High!\n"));
      ResetCmdData(Data);
      return EFI_TIMEOUT;
    }
  }

  /* Copy The Response To The Response Buffer */
  if (RespType & MMC_RSP_136) {
    UINT32 Cmdrsp3, Cmdrsp2, Cmdrsp1, Cmdrsp0;

    Cmdrsp3 = MmioReadBe32((UINTN)&Regs->Cmdrsp3);
    Cmdrsp2 = MmioReadBe32((UINTN)&Regs->Cmdrsp2);
    Cmdrsp1 = MmioReadBe32((UINTN)&Regs->Cmdrsp1);
    Cmdrsp0 = MmioReadBe32((UINTN)&Regs->Cmdrsp0);
    Response[0] = (Cmdrsp3 << 8) | (Cmdrsp2 >> 24);
    Response[1] = (Cmdrsp2 << 8) | (Cmdrsp1 >> 24);
    Response[2] = (Cmdrsp1 << 8) | (Cmdrsp0 >> 24);
    Response[3] = (Cmdrsp0 << 8);
  } else
    Response[0] = MmioReadBe32((UINTN)&Regs->Cmdrsp0);

  /* Wait Until All Of The Blocks Are Transferred */
  if (Data) {
    do {

      SdxcReadWrite(Data);

      Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);

      if (Irqstat & IRQSTAT_DTOE) {
        ResetCmdData(Data);
        return EFI_TIMEOUT;
      }

      if (Irqstat & DATA_ERR) {
        ResetCmdData(Data);
        return EFI_DEVICE_ERROR;
      }

    } while (!(Irqstat & DATA_COMPLETE));

    if (Data->Flags & MMC_DATA_READ)
      CheckAndInvalidateDcacheRange(Data);
  }

  MmioWriteBe32((UINTN)&Regs->Irqstat, -1);

  return EFI_SUCCESS;
}


/*
 * Sends A Command Out On The Bus.  Takes The Mmc Pointer,
 * A Command Pointer, And An Optional Data Pointer.
 */
EFI_STATUS
SdxcSendCmd (
  IN  struct Mmc *Mmc,
  IN  struct MmcCmd *Cmd,
  IN  struct MmcData *Data
  )
{
  EFI_STATUS Err = 0;
  UINT32	Xfertyp;
  UINT32	Irqstat;
  struct FslSdxcCfg *Cfg = Mmc->Priv;
  volatile struct FslSdxc *Regs = (struct FslSdxc *)Cfg->SdxcBase;

  MmioWriteBe32((UINTN)&Regs->Irqstat, -1);

  asm("Dmb :");

  /* Wait for The Bus To Be Idle */
  while ((MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTAT_CICHB) ||
		(MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTAT_CIDHB))
	;

  while (MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTAT_DLA)
  	;

  /* Wait At Least 8 SD Clock Cycles Before The Next Command */
  /*
   * Note: This Is Way More Than 8 Cycles, But 1ms Seems To
   * Resolve Timing Issues With Some Cards
   */
  MicroSecondDelay(1000);

  /* Set Up for A Data Transfer if We Have One */
  if (Data) {
    Err = SdxcSetupData(Mmc, Data);
    if (Err)
      return Err;
  }

  /* Figure Out The Transfer Arguments */
  Xfertyp = SdxcXfertyp(Cmd, Data);

  /* Mask All Irqs */
  MmioWriteBe32((UINTN)&Regs->Irqsigen, 0);

  /* Send The Command */
  MmioWriteBe32((UINTN)&Regs->CmdArg, Cmd->CmdArg);
  MmioWriteBe32((UINTN)&Regs->Xfertyp, Xfertyp);

  /* Wait for The Command To Complete */
  while (!(MmioReadBe32((UINTN)&Regs->Irqstat) & (IRQSTAT_CC | IRQSTAT_CTOE)))
  	;

  Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);

  if (Irqstat & CMD_ERR) {
    Err = EFI_DEVICE_ERROR;
    goto Out;
  }

  if (Irqstat & IRQSTAT_CTOE) {
    Err = EFI_TIMEOUT;
    goto Out;
  }

  if (Cmd->RespType != 0xFF) {
    Err = ReceiveResponse(Data, Cmd->RespType, Cmd->Response);
    return Err;
  }

Out:
  if (Err)
    ResetCmdData(Data);
  else
    MmioWriteBe32((UINTN)&Regs->Irqstat, -1);

  return Err;
}

static VOID
SetSysctl (
  IN  struct Mmc *Mmc,
  UINT32 Clock
  )
{
  INT32 Div, PreDiv;
  struct FslSdxcCfg *Cfg = Mmc->Priv;
  volatile struct FslSdxc *Regs = (struct FslSdxc *)Cfg->SdxcBase;
  INT32 SdhcClk = Cfg->SdhcClk;
  UINT32 Clk;

  if (Clock < Mmc->Cfg->FMin)
    Clock = Mmc->Cfg->FMin;

  if (SdhcClk / 16 > Clock) {
    for (PreDiv = 2; PreDiv < 256; PreDiv *= 2)
      if ((SdhcClk / PreDiv) <= (Clock * 16))
        break;
  } else
    PreDiv = 2;

  for (Div = 1; Div <= 16; Div++)
    if ((SdhcClk / (Div * PreDiv)) <= Clock)
      break;

  PreDiv >>= 1;
  Div -= 1;

  Clk = (PreDiv << 8) | (Div << 4);

  MmioClearBitsBe32((UINTN)&Regs->Sysctl, SYSCTL_CKEN);

  MmioClearSetBe32((UINTN)&Regs->Sysctl, SYSCTL_CLOCK_MASK, Clk);

  MicroSecondDelay(10000);

  Clk = SYSCTL_PEREN | SYSCTL_CKEN;

  MmioSetBitsBe32((UINTN)&Regs->Sysctl, Clk);
}

VOID
SdxcSetIos (
  IN  struct Mmc *Mmc
  )
{
  struct FslSdxcCfg *Cfg = Mmc->Priv;
  struct FslSdxc *Regs = (struct FslSdxc *)Cfg->SdxcBase;

  /* Set The Clock Speed */
  SetSysctl(Mmc, Mmc->Clock);

  /* Set The Bus Width */
  MmioClearBitsBe32((UINTN)&Regs->Proctl, PROCTL_DTW_4 | PROCTL_DTW_8);

  if (Mmc->BusWidth == 4)
    MmioSetBitsBe32((UINTN)&Regs->Proctl, PROCTL_DTW_4);
  else if (Mmc->BusWidth == 8)
    MmioSetBitsBe32((UINTN)&Regs->Proctl, PROCTL_DTW_8);
}

INT32
SdxcInit (
  IN  struct Mmc *Mmc
  )
{
  struct FslSdxcCfg *Cfg = Mmc->Priv;
  struct FslSdxc *Regs = (struct FslSdxc *)Cfg->SdxcBase;
  INT32 Timeout = 1000;

  /* Reset The Entire Host Controller */
  MmioSetBitsBe32((UINTN)&Regs->Sysctl, SYSCTL_RSTA);

  /* Wait Until The Controller Is Available */
  while ((MmioReadBe32((UINTN)&Regs->Sysctl) & SYSCTL_RSTA) && --Timeout)
    MicroSecondDelay(1000);

  /* Enable Cache Snooping */
  MmioWriteBe32((UINTN)&Regs->Scr, 0x00000040);

//  MmioSetBitsBe32((UINTN)&Regs->Sysctl, SYSCTL_HCKEN | SYSCTL_IPGEN);

  /* Set The Initial Clock Speed */
  MmcSetClock(Mmc, 400000);

  /* Disable The BRR And BWR Bits In IRQSTAT */
  MmioClearBitsBe32((UINTN)&Regs->Irqstaten, IRQSTATEN_BRR | IRQSTATEN_BWR);

  /* Set Little Endian mode for data Buffer */
  MmioWriteBe32((UINTN)&Regs->Proctl, PROCTL_INIT);

  /* Set Timout To The Maximum Value */
  MmioClearSetBe32((UINTN)&Regs->Sysctl, SYSCTL_TIMEOUT_MASK, 14 << 16);

  return 0;
}

INT32
SdxcGetcd (
  IN  struct Mmc *Mmc
  )
{
  struct FslSdxcCfg *Cfg = Mmc->Priv;
  struct FslSdxc *Regs = (struct FslSdxc *)Cfg->SdxcBase;
  INT32 Timeout = 1000;

  Timeout = MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTAT_CINS;

  while (!(MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTAT_CINS) && --Timeout)
    MicroSecondDelay(1000);

  return Timeout > 0;
}

VOID
SdxcReset (
  IN struct FslSdxc *Regs
  )
{
  UINT64 Timeout = 100; /* Wait Max 100 Ms */

  /* Reset The Controller */
  MmioWriteBe32((UINTN)&Regs->Sysctl, SYSCTL_RSTA);

  /* Hardware Clears The Bit When It Is Done */
  while ((MmioReadBe32((UINTN)&Regs->Sysctl) & SYSCTL_RSTA) && --Timeout)
    MicroSecondDelay(1000);

  if (!Timeout)
    DEBUG((EFI_D_ERROR, "MMC/SD: Reset Never Completed.\n"));
}


EFI_STATUS
FslSdxcInitialize (
  IN struct FslSdxcCfg *Cfg
  )
{
  struct FslSdxc *Regs;

  UINT32 Caps, VoltageCaps;

  if (!Cfg)
    return EFI_INVALID_PARAMETER;

  Regs = (struct FslSdxc *)Cfg->SdxcBase;

  /* First Reset The SDXC Controller */
  SdxcReset(Regs);

 // MmioSetBitsBe32((UINTN)&Regs->Sysctl, SYSCTL_PEREN | SYSCTL_HCKEN
 //                      | SYSCTL_IPGEN | SYSCTL_CKEN);

  InternalMemZeroMem(&Cfg->Cfg, sizeof(Cfg->Cfg));

  VoltageCaps = 0;
  Caps = MmioReadBe32((UINTN)&Regs->Hostcapblt);

  if (Caps & SDXC_HOSTCAPBLT_VS18)
    VoltageCaps |= MMC_VDD_165_195;
  if (Caps & SDXC_HOSTCAPBLT_VS30)
    VoltageCaps |= MMC_VDD_29_30 | MMC_VDD_30_31;
  if (Caps & SDXC_HOSTCAPBLT_VS33)
    VoltageCaps |= MMC_VDD_32_33 | MMC_VDD_33_34;

  Cfg->Cfg.Name = "FSL_SDXC";
  Cfg->Cfg.Ops = &SdxcOps;
  Cfg->Cfg.Voltages = MMC_VDD_32_33 | MMC_VDD_33_34;

  if ((Cfg->Cfg.Voltages & VoltageCaps) == 0) {
    DEBUG((EFI_D_ERROR, "Voltage Not Supported By Controller\n"));
    return -1;
  }

  Cfg->Cfg.HostCaps = MMC_MODE_4BIT | MMC_MODE_8BIT | MMC_MODE_HC;

  if (Cfg->MaxBusWidth > 0) {
    if (Cfg->MaxBusWidth < 8)
      Cfg->Cfg.HostCaps &= ~MMC_MODE_8BIT;
      if (Cfg->MaxBusWidth < 4)
        Cfg->Cfg.HostCaps &= ~MMC_MODE_4BIT;
  }

  if (Caps & SDXC_HOSTCAPBLT_HSS)
    Cfg->Cfg.HostCaps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

  Cfg->Cfg.FMin = 400000;
  Cfg->Cfg.FMax = Min(GET_CLOCK/2, 52000000);
  //TODO Cfg->Cfg.FMax = Min(Gd->Arch.SdhcClk, 52000000);

  Cfg->Cfg.BMax = CONFIG_SYS_MMC_MAX_BLK_COUNT;

  if (gMmc) {
    FreePool(gMmc->Cfg);
    FreePool(gMmc->Priv);
    FreePool(gMmc);
  }

  gMmc = (struct Mmc *)MmcCreate(&Cfg->Cfg, Cfg);
  if (gMmc == NULL)
    return -1;

  return 0;
}

INT32
SdxcMmcInit (
  )
{
 struct FslSdxcCfg *Cfg;

 Cfg = (struct FslSdxcCfg *)AllocatePool(sizeof(struct FslSdxcCfg));
 InternalMemZeroMem(Cfg, sizeof(struct FslSdxcCfg));
 Cfg->SdxcBase = (VOID *)CONFIG_SYS_FSL_SDXC_ADDR;
 Cfg->SdhcClk = (UINT32)(GET_CLOCK/2); //Gd->Arch.SdhcClk;

 return FslSdxcInitialize(Cfg);
}

INT32
BoardMmcInit (
  VOID
  )
{
  SdxcCfg[0].SdhcClk = (UINT32)(GET_CLOCK/2);
  return FslSdxcInitialize(&SdxcCfg[0]);
}

