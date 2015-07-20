/** SdxcLib.C
  Sdxc Library Containing Functions for Reset Read, Write, Initialize Etc

  Copyright (C) 2014, Freescale Ltd. All Rights Reserved.

  This Program And The Accompanying Materials
  Are Licensed And Made Available Under The Terms And Conditions Of The BSD License
  Which Accompanies This Distribution.  The Full Text Of The License May Be Found At
  Http://Opensource.Org/Licenses/Bsd-License.Php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include <Library/Sdxc.h>

struct Mmc *gMmc = NULL;

/* Frequency Bases */
/* Divided By 10 To Be Nice To Platforms Without Floating Point */
static CONST INT32
Fbase[] = {
       10000,
       100000,
       1000000,
       10000000,
};

/* Multiplier Values for TRAN_SPEED.  Multiplied By 10 To Be Nice
 * To Platforms Without Floating Point.
 */
static CONST INT32
Multipliers[] = {
       0,     /* Reserved */
       10,
       12,
       13,
       15,
       20,
       25,
       30,
       35,
       40,
       45,
       50,
       55,
       60,
       70,
       80,
};

/* Wrapper for DoDiv(). Doesn'T Modify Dividend And Returns
 * The Result, Not Reminder.
 */
static inline UINT64 LlDiv (UINT64 Dividend, UINT32 Divisor)
{
  UINT64 Res = Dividend;
  DoDiv(Res, Divisor);
  return(Res);
}

INT32
MmcGetwp (
  IN  struct Mmc *Mmc
  )
{
  INT32 Wp;

  if (Mmc->Cfg->Ops->Getwp)
    Wp = Mmc->Cfg->Ops->Getwp(Mmc); //TODO return response of CMD30
  else
    Wp = 0;

  return Wp;
}

INT32
MmcSendCmd (
  IN  struct Mmc *Mmc,
  IN  struct MmcCmd *Cmd,
  IN  struct MmcData *Data
  )
{
  INT32 Ret;

  Ret = Mmc->Cfg->Ops->SendCmd(Mmc, Cmd, Data);
  return Ret;
}

EFI_STATUS
MmcSendStatus (
  IN  struct Mmc *Mmc,
  IN  INT32 Timeout
  )
{
  struct MmcCmd Cmd;
  INT32 Retries = 5;
  EFI_STATUS Err;

  Cmd.CmdIdx = MMC_CMD_SEND_STATUS;
  Cmd.RespType = MMC_RSP_R1;
  if (!MmcHostIsSpi(Mmc))
    Cmd.CmdArg = Mmc->Rca << 16;

  do {
    Err = MmcSendCmd(Mmc, &Cmd, NULL);
    if (!Err) {
      if ((Cmd.Response[0] & MMC_STATUS_RDY_FOR_DATA) &&
          ((Cmd.Response[0] & MMC_STATUS_CURR_STATE) !=
           MMC_STATE_PRG))
        break;
      else if (Cmd.Response[0] & MMC_STATUS_MASK) {
        DEBUG((EFI_D_ERROR, "Status Error: 0x%08X\n", Cmd.Response[0]));
        return EFI_DEVICE_ERROR;
      }
    } else if (--Retries < 0)
      return Err;

    MicroSecondDelay(1000);
  } while (Timeout--);

  //DEBUG((EFI_D_INFO, "Current State : %d\n",
  //     (Cmd.Response[0] & MMC_STATUS_CURR_STATE) >> 9));

  if (Timeout <= 0) {
    DEBUG((EFI_D_ERROR, "Timeout Waiting Card Ready\n"));
    return EFI_TIMEOUT;
  }
  if (Cmd.Response[0] & MMC_STATUS_SWITCH_ERROR)
    return EFI_NO_RESPONSE;

  return EFI_SUCCESS;
}

INT32
MmcReadBlks (
  IN  struct Mmc *Mmc,
  OUT VOID *Dst,
  IN  UINT64 Start,
  IN  UINT64 Blkcnt
  )
{
  struct MmcCmd Cmd;
  struct MmcData Data;

  if (Blkcnt > 1)
    Cmd.CmdIdx = MMC_CMD_READ_MULTIPLE_BLOCK;
  else
    Cmd.CmdIdx = MMC_CMD_READ_SINGLE_BLOCK;

  if (Mmc->HighCapacity)
    Cmd.CmdArg = Start;
  else
    Cmd.CmdArg = Start * Mmc->ReadBlLen;

  Cmd.RespType = MMC_RSP_R1;

  Data.Dest = Dst;
  Data.Blocks = Blkcnt;
  Data.Blocksize = Mmc->ReadBlLen;
  Data.Flags = MMC_DATA_READ;

  if (MmcSendCmd(Mmc, &Cmd, &Data))
    return 0;

  if (Blkcnt > 1) {
    Cmd.CmdIdx = MMC_CMD_STOP_TRANSMISSION;
    Cmd.CmdArg = 0;
    Cmd.RespType = MMC_RSP_R1b;

    if (MmcSendCmd(Mmc, &Cmd, NULL)) {
      DEBUG((EFI_D_ERROR, "Mmc Fail To Send Stop Cmd\n"));
      return 0;
    }
  }

  return Blkcnt;
}

INT32
MmcSetBlocklen (
  IN  struct Mmc *Mmc,
  IN  INT32 Len
  )
{
  struct MmcCmd Cmd;

  if (Mmc->DdrMode)
    return EFI_SUCCESS;

#if 0
  if (Mmc->CardCaps & MMC_MODE_DDR_52MHz)
    return 0;
#endif

  Cmd.CmdIdx = MMC_CMD_SET_BLOCKLEN;
  Cmd.RespType = MMC_RSP_R1;
  Cmd.CmdArg = Len;

  return MmcSendCmd(Mmc, &Cmd, NULL);
}

UINT64
MmcWriteBlks (
  IN  struct Mmc *Mmc,
  IN  UINT64 Start,
  IN  UINT64 Blkcnt,
  IN  CONST VOID *Src
  )
{
  struct MmcCmd Cmd;
  struct MmcData Data;
  INT32 Timeout = 1000;

  if ((Start + Blkcnt) > Mmc->BlockDev.Lba) {
    DEBUG((EFI_D_ERROR, "MMC: Block Number %d Exceeds Max(%d)\n",
              Start + Blkcnt, Mmc->BlockDev.Lba));
    return 0;
  }

  if (Blkcnt == 0)
    return 0;
  else if (Blkcnt == 1)
    Cmd.CmdIdx = MMC_CMD_WRITE_SINGLE_BLOCK;
  else
    Cmd.CmdIdx = MMC_CMD_WRITE_MULTIPLE_BLOCK;

  if (Mmc->HighCapacity)
    Cmd.CmdArg = Start;
  else
    Cmd.CmdArg = Start * Mmc->WriteBlLen;

  Cmd.RespType = MMC_RSP_R1;

  Data.Src = Src;
  Data.Blocks = Blkcnt;
  Data.Blocksize = Mmc->WriteBlLen;
  Data.Flags = MMC_DATA_WRITE;

  if (MmcSendCmd(Mmc, &Cmd, &Data)) {
    DEBUG((EFI_D_ERROR, "Mmc Write Failed\n"));
    return 0;
  }

  /* SPI Multiblock Writes Terminate Using A Special
   * Token, Not A STOP_TRANSMISSION Request.
   */
  if (!MmcHostIsSpi(Mmc) && Blkcnt > 1) {
    Cmd.CmdIdx = MMC_CMD_STOP_TRANSMISSION;
    Cmd.CmdArg = 0;
    Cmd.RespType = MMC_RSP_R1b;
    if (MmcSendCmd(Mmc, &Cmd, NULL)) {
      DEBUG((EFI_D_ERROR, "Mmc Fail To Send Stop Cmd\n"));
      return 0;
    }
  }

  /* Waiting for The Ready Status */
  if (MmcSendStatus(Mmc, Timeout))
    return 0;

  return Blkcnt;
}

UINT64
MmcEraseBlks (
  IN  struct Mmc *Mmc,
  IN  UINT64 Start,
  IN  UINT64 Blkcnt
  )
{
  struct MmcCmd Cmd;
  UINT64 End;
  INT32 Err, StartCmd, EndCmd;

  if (Mmc->HighCapacity) {
    End = Start + Blkcnt/Mmc->EraseGrpSize - 1;
  } else {
    End = (Start + Blkcnt/Mmc->EraseGrpSize - 1) * Mmc->WriteBlLen;
    Start *= Mmc->WriteBlLen;
  }

  if (IS_SD(Mmc)) {
    StartCmd = SD_CMD_ERASE_WR_BLK_START;
    EndCmd = SD_CMD_ERASE_WR_BLK_END;
  } else {
    StartCmd = MMC_CMD_ERASE_GROUP_START;
    EndCmd = MMC_CMD_ERASE_GROUP_END;
  }

  //DEBUG((EFI_D_INFO, "Going to Erase block  start %d, end %d\n", Start, End));

  Cmd.CmdIdx = StartCmd;
  Cmd.CmdArg = Start;
  Cmd.RespType = MMC_RSP_R1;

  Err = MmcSendCmd(Mmc, &Cmd, NULL);
  if (Err)
    goto ErrOut;

  Cmd.CmdIdx = EndCmd;
  Cmd.CmdArg = End;

  Err = MmcSendCmd(Mmc, &Cmd, NULL);
  if (Err)
    goto ErrOut;

  Cmd.CmdIdx = MMC_CMD_ERASE;
  Cmd.CmdArg = SECURE_ERASE;
  Cmd.RespType = MMC_RSP_R1b;

  Err = MmcSendCmd(Mmc, &Cmd, NULL);
  if (Err)
    goto ErrOut;

  return 0;

ErrOut:
  DEBUG((EFI_D_ERROR, "Mmc Erase Failed\n"));
  return Err;
}

UINT32
MmcBRead (
  IN  UINT32 Start,
  IN  UINT32 Blkcnt,
  OUT VOID *Dst
  )
{
  UINT32 Cur, BlocksTodo = Blkcnt;

  if (Blkcnt == 0)
    return 0;

  if ((Start + Blkcnt) > gMmc->BlockDev.Lba) {
    DEBUG((EFI_D_ERROR, "MMC: Block Number 0x%x Exceeds Max(0x%x)\n",
           Start + Blkcnt, gMmc->BlockDev.Lba));
    return 0;
  }

  if (MmcSetBlocklen(gMmc, gMmc->ReadBlLen))
    return 0;

  do {
    Cur = (BlocksTodo > gMmc->Cfg->BMax) ?
           gMmc->Cfg->BMax : BlocksTodo;
    if (MmcReadBlks(gMmc, Dst, Start, Cur) != Cur)
      return 0;

    BlocksTodo -= Cur;
    Start += Cur;
    Dst += Cur * gMmc->ReadBlLen;
  } while (BlocksTodo > 0);

  return Blkcnt;
}

UINT32
MmcBWrite (
  IN  UINT32 Start,
  IN  UINT32 Blkcnt,
  IN  CONST VOID *Src
  )
{
  UINT32 Cur, BlocksTodo = Blkcnt;

  if (MmcSetBlocklen(gMmc, gMmc->WriteBlLen))
    return 0;

  do {
    Cur = (BlocksTodo > gMmc->Cfg->BMax) ?
           gMmc->Cfg->BMax : BlocksTodo;
    if (MmcWriteBlks(gMmc, Start, Cur, Src) != Cur)
      return 0;

    BlocksTodo -= Cur;
    Start += Cur;
    Src += Cur * gMmc->WriteBlLen;
  } while (BlocksTodo > 0);

  return Blkcnt;
}

UINT32
MmcBErase (
  IN  UINT32 Start,
  IN  UINT32 Blkcnt
  )
{
  INT32 Err = 0;
  UINT32 Blk = 0, BlkR = 0;
  INT32 Timeout = 1000;

  if ((Start % gMmc->EraseGrpSize) || (Blkcnt % gMmc->EraseGrpSize))
    DEBUG((EFI_D_ERROR, "\n\n Caution! Your Devices Erase Group Is 0x%x\n"
           "The Erase Range Would Be Change To 0x%x ~0x%x \n\n",
           gMmc->EraseGrpSize, Start & ~(gMmc->EraseGrpSize - 1),
           ((Start + Blkcnt + gMmc->EraseGrpSize)
           & ~(gMmc->EraseGrpSize - 1)) - 1));

  while (Blk < Blkcnt) {
    BlkR = ((Blkcnt - Blk) > gMmc->EraseGrpSize) ?
           gMmc->EraseGrpSize : (Blkcnt - Blk);

    Err = MmcEraseBlks(gMmc, Start + Blk, BlkR);
    if (Err)
      break;

    Blk += BlkR;

    /* Waiting for The Ready Status */
    if (MmcSendStatus(gMmc, Timeout))
      return 0;
  }

  return Blk;
}

INT32
MmcGetcd (
  VOID
  )
{
  INT32 Cd;

  ASSERT(gMmc->Cfg->Ops->Getcd);
  if (gMmc->Cfg->Ops->Getcd)
    Cd = gMmc->Cfg->Ops->Getcd(gMmc);
  else
    Cd = 1;

  return Cd;
}

VOID
MmcSetIos (
  IN  struct Mmc *Mmc
  )
{
  if (Mmc->Cfg->Ops->SetIos)
    Mmc->Cfg->Ops->SetIos(Mmc);
}

VOID
MmcSetClock (
  IN  struct Mmc *Mmc,
  IN  UINT32 Clock
  )
{
  if (Clock > Mmc->Cfg->FMax)
    Clock = Mmc->Cfg->FMax;

  if (Clock < Mmc->Cfg->FMin)
    Clock = Mmc->Cfg->FMin;

  Mmc->Clock = Clock;

  MmcSetIos(Mmc);
}

static VOID
MmcSetBusWidth (
  IN  struct Mmc *Mmc,
  IN  UINT32 Width
  )
{
  Mmc->BusWidth = Width;

  MmcSetIos(Mmc);
}

EFI_STATUS
MmcGoIdle (
  IN  struct Mmc *Mmc
  )
{
  struct MmcCmd Cmd;
  INT32 Err;

  MicroSecondDelay(1000);

  Cmd.CmdIdx = MMC_CMD_GO_IDLE_STATE;
  Cmd.CmdArg = 0;
  Cmd.RespType = MMC_RSP_NONE;

  Err = MmcSendCmd(Mmc, &Cmd, NULL);

  if (Err)
    return Err;

  MicroSecondDelay(2000);

  return EFI_SUCCESS;
}

static EFI_STATUS
MmcSendIfCond (
  IN  struct Mmc *Mmc
  )
{
  struct MmcCmd Cmd;
  INT32 Err;

  Cmd.CmdIdx = SD_CMD_SEND_IF_COND;
  /* We Set The Bit if The Host Supports Voltages Between 2.7 And 3.6 V */
  Cmd.CmdArg = ((Mmc->Cfg->Voltages & 0xff8000) != 0) << 8 | 0xaa;
  Cmd.RespType = MMC_RSP_R7;

  Err = MmcSendCmd(Mmc, &Cmd, NULL);

  if (Err)
    return Err;

  if ((Cmd.Response[0] & 0xff) != 0xaa)
    return EFI_NO_RESPONSE;
  else
    Mmc->Version = SD_VERSION_2;
  //FIXME check for others also

  return EFI_SUCCESS;
}

static EFI_STATUS
SendAppCmd (
  IN BOOLEAN Rca
  )
{
  struct MmcCmd Cmd;

  Cmd.CmdIdx = MMC_CMD_APP_CMD;
  Cmd.RespType = MMC_RSP_R1;

  if (Rca)
    Cmd.CmdArg = gMmc->Rca << 16;
  else
    Cmd.CmdArg = 0;

  return MmcSendCmd(gMmc, &Cmd, NULL);
}

static EFI_STATUS
SdSendOpCond (
  IN  struct Mmc *Mmc
  )
{
  INT32 Timeout = 1000;
  EFI_STATUS Err;
  struct MmcCmd Cmd;

  do {
    Err = SendAppCmd(FALSE);

    if (Err)
      return Err;

    Cmd.CmdIdx = SD_CMD_APP_SEND_OP_COND;
    Cmd.RespType = MMC_RSP_R3;

    /*
     * Most Cards do Not Answer if Some Reserved Bits
     * In The Ocr Are Set. However, Some Controller
     * Can Set Bit 7 (Reserved for Low Voltages), But
     * How To Manage Low Voltages SD Card Is Not Yet
     * Specified.
     */
    Cmd.CmdArg = MmcHostIsSpi(Mmc) ? 0 :
           (Mmc->Cfg->Voltages & 0xff8000);

    if (Mmc->Version == SD_VERSION_2)
      Cmd.CmdArg |= OCR_HCS;

    Err = MmcSendCmd(Mmc, &Cmd, NULL);

    if (Err)
      return Err;

    MicroSecondDelay(1000);
  } while ((!(Cmd.Response[0] & OCR_BUSY)) && Timeout--);

  if (Timeout <= 0)
    return EFI_TIMEOUT;

  if (Mmc->Version != SD_VERSION_2)
    Mmc->Version = SD_VERSION_1_0;

  if (MmcHostIsSpi(Mmc)) { /* Read OCR for Spi */
    Cmd.CmdIdx = MMC_CMD_SPI_READ_OCR;
    Cmd.RespType = MMC_RSP_R3;
    Cmd.CmdArg = 0;

    Err = MmcSendCmd(Mmc, &Cmd, NULL);

    if (Err)
      return Err;
  }

  Mmc->Ocr = Cmd.Response[0];
  Mmc->HighCapacity = ((Mmc->Ocr & OCR_HCS) == OCR_HCS);
  Mmc->Rca = 0;

  return 0;
}

/* We Pass In The Cmd Since Otherwise The Init Seems To Fail */
static INT32
MmcSendOpCondIter (
  IN  struct Mmc *Mmc,
  IN  struct MmcCmd *Cmd,
  IN  INT32 UseArg
)
{
  INT32 Err;

  Cmd->CmdIdx = MMC_CMD_SEND_OP_COND;
  Cmd->RespType = MMC_RSP_R3;
  Cmd->CmdArg = 0;
  if (UseArg && !MmcHostIsSpi(Mmc)) {
    Cmd->CmdArg =
           (Mmc->Cfg->Voltages &
           (Mmc->Ocr & OCR_VOLTAGE_MASK)) |
           (Mmc->Ocr & OCR_ACCESS_MODE);

    if (Mmc->Cfg->HostCaps & MMC_MODE_HC)
      Cmd->CmdArg |= OCR_HCS;
  }
  Err = MmcSendCmd(Mmc, Cmd, NULL);

  if (Err)
    return Err;

  Mmc->Ocr = Cmd->Response[0];
  return EFI_SUCCESS;
}

EFI_STATUS
MmcSendOpCond (
  IN  struct Mmc *Mmc
  )
{
  struct MmcCmd Cmd;
  EFI_STATUS Err;
  INT32 I;

  /* Some Cards Seem To Need This */
  MmcGoIdle(Mmc);

  /* Asking To The Card Its Capabilities */
//  Mmc->OpCondPending = 1;
  for (I = 0; I < 2; I++) {
    Err = MmcSendOpCondIter(Mmc, &Cmd, I != 0);
    if (Err)
      return Err;

    /* Exit if Not Busy (Flag Seems To Be Inverted) */
    if (Mmc->Ocr & OCR_BUSY)
      return EFI_SUCCESS;
  }
  return EFI_ALREADY_STARTED;
}

EFI_STATUS
MmcStartInit (
  VOID
  )
{
  struct FslSdxcCfg *Cfg = gMmc->Priv;
  struct FslSdxc *Regs = (struct FslSdxc *)Cfg->SdxcBase;
  EFI_STATUS Err;

  /* We Pretend There'S No Card When Init Is NULL */
  if (MmcGetcd() == 0 || gMmc->Cfg->Ops->Init == NULL) {
    gMmc->HasInit = 0;
    DEBUG((EFI_D_ERROR, "MMC: No Card Present\n"));

    return EFI_NO_MEDIA;
  }

  if (gMmc->HasInit)
    return EFI_SUCCESS;

  /* Made Sure It'S Not NULL Earlier */
  Err = gMmc->Cfg->Ops->Init(gMmc);

  if (Err)
    return Err;

  gMmc->DdrMode = 0;
  MmcSetBusWidth(gMmc, 1);
  MmcSetClock(gMmc, 1);

  MmioClearBitsBe32((UINTN)&Regs->Proctl, PROCTL_BE);

  /* Reset The Card */
  Err = MmcGoIdle(gMmc);

  if (Err)
    return Err;

  /* The Internal Partition Reset To User Partition(0) At Every CMD0*/
  gMmc->PartNum = 0;

  /* Check for An MMC Card */
  Err = MmcSendOpCond(gMmc);

  if (Err && Err != EFI_ALREADY_STARTED) {
    DEBUG((EFI_D_ERROR, "Not MMC Card\n"));

    /* Test for SD Version 2 */
    Err = MmcSendIfCond(gMmc);
    if (Err != EFI_NO_RESPONSE)
      DEBUG((EFI_D_INFO, "Not SD version 2\n"));

    /* Now Try To Get The SD Card'S Operating Condition */
    Err = SdSendOpCond(gMmc);
    if (Err)
      DEBUG((EFI_D_ERROR, "Card Did Not Respond To Voltage Select!\n"));
  } else
    gMmc->OpCondPending = 1;

  if (Err == EFI_ALREADY_STARTED)
    gMmc->InitInProgress = 1;

  return Err;
}

struct Mmc *
MmcCreate (
  IN struct MmcConfig *Cfg,
  IN VOID *Priv
  )
{
  struct Mmc *Mmc;

  /* Quick Validation */
  if (Cfg == NULL || Cfg->Ops == NULL || Cfg->Ops->SendCmd == NULL ||
                Cfg->FMin == 0 || Cfg->FMax == 0 || Cfg->BMax == 0)
    return NULL;

  Mmc = (struct Mmc*)AllocatePool(sizeof(struct Mmc));
  if (Mmc == NULL)
    return NULL;

  InternalMemZeroMem(Mmc, sizeof(struct Mmc));

  Mmc->Cfg = (struct MmcConfig*)AllocatePool(sizeof(struct MmcConfig));
  if (Mmc->Cfg == NULL) {
    FreePool(Mmc);
    return NULL;
  }

  InternalMemZeroMem(Mmc->Cfg, sizeof(struct MmcConfig));

  Mmc->Signature = EFI_SDXC_SIGNATURE;
  InternalMemCopyMem(Mmc->Cfg, Cfg, sizeof(struct MmcConfig));
  Mmc->Priv = Priv;

  /* The Following Chunk Was MmcRegister() */

  /* Setup Dsr Related Values */
  Mmc->DsrImp = 0;
  Mmc->Dsr = 0xffffffff;
  /* Setup The Universal Parts Of The Block Interface Just Once */
  Mmc->BlockDev.IfType = IF_TYPE_MMC;
  Mmc->BlockDev.Removable = 1;
  Mmc->BlockDev.BlockRead = MmcBRead;
  Mmc->BlockDev.BlockWrite = MmcBWrite;
  Mmc->BlockDev.BlockErase = MmcBErase;

  /* Setup Initial Part Type */
  Mmc->BlockDev.PartType = Mmc->Cfg->PartType;

  return Mmc;
}

EFI_STATUS
MmcCompleteOpCond (
  IN  struct Mmc *Mmc
  )
{
  struct MmcCmd Cmd;
//  INT32 Timeout = 1000;
  EFI_STATUS Err;
//  UINT32 Start;

  Mmc->OpCondPending = 0;
#if 0
//  Start = GetTimer(0);
  do {
         Err = MmcSendOpCondIter(Mmc, &Cmd, 1);
         if (Err)
                return Err;
   //      if (GetTimer(Start) > Timeout)
   //             return UNUSABLE_ERR;
  	if(!Timeout--)
  		return EFI_TIMEOUT;

         MicroSecondDelay(100);
  } while (!(Mmc->Ocr & OCR_BUSY));
#endif
  if (MmcHostIsSpi(Mmc)) { /* Read OCR for Spi */
         Cmd.CmdIdx = MMC_CMD_SPI_READ_OCR;
         Cmd.RespType = MMC_RSP_R3;
         Cmd.CmdArg = 0;

         Err = MmcSendCmd(Mmc, &Cmd, NULL);

         if (Err)
                return Err;
  }

  Mmc->Version = MMC_VERSION_UNKNOWN;

  Mmc->HighCapacity = ((Mmc->Ocr & OCR_HCS) == OCR_HCS);
  Mmc->Rca = 2;

  return EFI_SUCCESS;
}

static INT32
MmcSendExtCsd (
  IN  struct Mmc *Mmc,
  IN  UINT8 *ExtCsd
  )
{
  struct MmcCmd Cmd;
  struct MmcData Data;
  INT32 Err;

  /* Get The Card Status Register */
  Cmd.CmdIdx = MMC_CMD_SEND_EXT_CSD;
  Cmd.RespType = MMC_RSP_R1;
  Cmd.CmdArg = 0;

  Data.Dest = (CHAR8 *)ExtCsd;
  Data.Blocks = 1;
  Data.Blocksize = MMC_MAX_BLOCK_LEN;
  Data.Flags = MMC_DATA_READ;

  Err = MmcSendCmd(Mmc, &Cmd, &Data);

  return Err;
}

static INT32
MmcSetCapacity (
  IN  struct Mmc *Mmc,
  IN  INT32 PartNum
  )
{
  switch (PartNum) {
  case 0:
         Mmc->Capacity = Mmc->CapacityUser;
         break;
  case 1:
  case 2:
         Mmc->Capacity = Mmc->CapacityBoot;
         break;
  case 3:
         Mmc->Capacity = Mmc->CapacityRpmb;
         break;
  case 4:
  case 5:
  case 6:
  case 7:
         Mmc->Capacity = Mmc->CapacityGp[PartNum - 4];
         break;
  default:
         return -1;
  }

  Mmc->BlockDev.Lba = LlDiv(Mmc->Capacity, Mmc->ReadBlLen);

  return 0;
}

static INT32
SdSwitch (
  IN  struct Mmc *Mmc,
  IN  INT32 Mode,
  IN  INT32 Group,
  IN  UINT8 Value,
  UINT8 *Resp
  )
{
  struct MmcCmd Cmd;
  struct MmcData Data;

  /* switch The Frequency */
  Cmd.CmdIdx = SD_CMD_SWITCH_FUNC;
  Cmd.RespType = MMC_RSP_R1;
  Cmd.CmdArg = (Mode << 31) | 0xffffff;
  Cmd.CmdArg &= ~(0xf << (Group * 4));
  Cmd.CmdArg |= Value << (Group * 4);

  Data.Dest = (CHAR8 *)Resp;
  Data.Blocksize = 64;
  Data.Blocks = 1;
  Data.Flags = MMC_DATA_READ;

  return MmcSendCmd(Mmc, &Cmd, &Data);
}

static INT32
SdChangeFreq (
  IN  struct Mmc *Mmc
  )
{
  INT32 Err;
  struct MmcCmd Cmd;
  ALLOC_CACHE_ALIGN_BUFFER(UINT32, Scr, 2);
  ALLOC_CACHE_ALIGN_BUFFER(UINT32, SwitchStatus, 16);
#if 0
  UINT32 Scr[2];
  UINT32 SwitchStatus[16];
#endif
  struct MmcData Data;
  INT32 Timeout;

  Mmc->CardCaps = 0;

  if (MmcHostIsSpi(Mmc))
    return 0;

  /* Read The SCR To Find Out if This Card Supports Higher Speeds */
  Err = SendAppCmd(TRUE);

  if (Err)
    return Err;

  Cmd.CmdIdx = SD_CMD_APP_SEND_SCR;
  Cmd.RespType = MMC_RSP_R1;
  Cmd.CmdArg = 0;

  Timeout = 5;

RetryScr:
  Data.Dest = (CHAR8 *)Scr;
  Data.Blocksize = 8;
  Data.Blocks = 1;
  Data.Flags = MMC_DATA_READ;

  Err = MmcSendCmd(Mmc, &Cmd, &Data);

  if (Err) {
    if (Timeout--)
      goto RetryScr;

    return Err;
  }

  MicroSecondDelay(100);
//TODO
//	Mmc->Scr[0] = _Be32ToCpu(Scr[0]);
//	Mmc->Scr[1] = _Be32ToCpu(Scr[1]);
	Mmc->Scr[0] = Scr[0];
	Mmc->Scr[1] = Scr[1];

  switch ((Mmc->Scr[0] >> 24) & 0xf) {
    case 0:
      Mmc->Version = SD_VERSION_1_0;
      break;
    case 1:
      Mmc->Version = SD_VERSION_1_10;
      break;
    case 2:
      Mmc->Version = SD_VERSION_2;
      if ((Mmc->Scr[0] >> 15) & 0x1)
        Mmc->Version = SD_VERSION_3;
      break;
    default:
      Mmc->Version = SD_VERSION_1_0;
      break;
  }

  if (Mmc->Scr[0] & SD_DATA_4BIT)
    Mmc->CardCaps |= MMC_MODE_4BIT;

  /* Version 1.0 Doesn'T Support Switching */
  if (Mmc->Version == SD_VERSION_1_0)
    return 0;

  Timeout = 4;
  while (Timeout--) {
    Err = SdSwitch(Mmc, SD_SWITCH_CHECK, 0, 1,
		(UINT8 *)SwitchStatus);

    if (Err)
      return Err;

    /* The High-Speed Function Is Busy.  Try Again */
    //TODO if (!(_Be32ToCpu(SwitchStatus[7]) & SD_HIGHSPEED_BUSY))
    if (!(SwitchStatus[7] & SD_HIGHSPEED_BUSY))
      break;
  }

  /* if High-Speed Isn'T Supported, We return */
  // TODO	if (!(_Be32ToCpu(SwitchStatus[3]) & SD_HIGHSPEED_SUPPORTED))
  if (!(SwitchStatus[3] & SD_HIGHSPEED_SUPPORTED))
    return 0;

  /*
   * if The Host Doesn'T Support SD_HIGHSPEED, do Not switch Card To
   * HIGHSPEED Mode Even if The Card Support SD_HIGHSPPED.
   * This Can Avoid Furthur Problem When The Card Runs In Different
   * Mode Between The Host.
   */
  if (!((Mmc->Cfg->HostCaps & MMC_MODE_HS_52MHz) &&
	(Mmc->Cfg->HostCaps & MMC_MODE_HS)))
    return 0;

  Err = SdSwitch(Mmc, SD_SWITCH_SWITCH, 0, 1, (UINT8 *)SwitchStatus);

  if (Err)
    return Err;

  //TODO	if ((_Be32ToCpu(SwitchStatus[4]) & 0x0f000000) == 0x01000000)
  if ((SwitchStatus[4] & 0x0f000000) == 0x01000000)
    Mmc->CardCaps |= MMC_MODE_HS;

  return 0;
}

static INT32
MmcSwitch (
  IN  struct Mmc *Mmc,
  IN  UINT8 Set,
  IN  UINT8 Index,
  IN UINT8 Value
  )
{
  struct MmcCmd Cmd;
  INT32 Timeout = 1000;
  INT32 Ret;

  Cmd.CmdIdx = MMC_CMD_SWITCH;
  Cmd.RespType = MMC_RSP_R1b;
  Cmd.CmdArg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
		 (Index << 16) |
  		 (Value << 8);

  Ret = MmcSendCmd(Mmc, &Cmd, NULL);

  /* Waiting for The Ready Status */
  if (!Ret)
    Ret = MmcSendStatus(Mmc, Timeout);

  return Ret;
}

static INT32
MmcChangeFreq (
  IN  struct Mmc *Mmc
  )
{
  ALLOC_CACHE_ALIGN_BUFFER(UINT8, ExtCsd, MMC_MAX_BLOCK_LEN);
#if 0
  UINT8 ExtCsd[MMC_MAX_BLOCK_LEN];
#endif
  CHAR8 Cardtype;
  INT32 Err;

  Mmc->CardCaps = MMC_MODE_4BIT | MMC_MODE_8BIT;

  if (MmcHostIsSpi(Mmc))
    return 0;

  /* Only Version 4 Supports High-Speed */
  if (Mmc->Version < MMC_VERSION_4)
    return 0;

  Err = MmcSendExtCsd(Mmc, ExtCsd);
  if (Err)
    return Err;

  Cardtype = ExtCsd[EXT_CSD_CARD_TYPE] & 0xf;

  Err = MmcSwitch(Mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);

  if (Err)
    return Err == EFI_NO_RESPONSE ? 0 : Err;

  /* Now Check To See That It Worked */
  Err = MmcSendExtCsd(Mmc, ExtCsd);

  if (Err)
    return Err;

  /* No High-Speed Support */
  if (!ExtCsd[EXT_CSD_HS_TIMING])
    return 0;

  /* High Speed Is Set, There Are Two Types: 52MHz And 26MHz */
  if (Cardtype & EXT_CSD_CARD_TYPE_52) {
    if (Cardtype & EXT_CSD_CARD_TYPE_DDR_52)
      Mmc->CardCaps |= MMC_MODE_DDR_52MHz;
    Mmc->CardCaps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
  } else {
     Mmc->CardCaps |= MMC_MODE_HS;
  }

  return 0;
}

static INT32
MmcStartup (
  IN  struct Mmc *Mmc
  )
{
  INT32   Err, I;
  UINT32 Mult, Freq;
  UINT64 CMult, CSize, Capacity;
  struct MmcCmd Cmd;

  ALLOC_CACHE_ALIGN_BUFFER(UINT8, ExtCsd, MMC_MAX_BLOCK_LEN);
  ALLOC_CACHE_ALIGN_BUFFER(UINT8, TestCsd, MMC_MAX_BLOCK_LEN);
#if 0
  UINT8 ExtCsd[MMC_MAX_BLOCK_LEN];
  UINT8 TestCsd[MMC_MAX_BLOCK_LEN];
#endif
  INT32 Timeout = 10000;


  /* Put The Card In Identify Mode */
  Cmd.CmdIdx = MmcHostIsSpi(Mmc) ? MMC_CMD_SEND_CID :
         MMC_CMD_ALL_SEND_CID; /* Cmd Not Supported In Spi */
  Cmd.RespType = MMC_RSP_R2;
  Cmd.CmdArg = 0;

  Err = MmcSendCmd(Mmc, &Cmd, NULL);

  if (Err)
    return Err;

  Mmc->Cid[0] = Cmd.Response[0];
  Mmc->Cid[1] = Cmd.Response[1];
  Mmc->Cid[2] = Cmd.Response[2];
  Mmc->Cid[3] = Cmd.Response[3];
  /*
   * for MMC Cards, Set The Relative Address.
   * for SD Cards, Get The Relatvie Address.
   * This Also Puts The Cards Into Standby State
  */
  if (!MmcHostIsSpi(Mmc)) { /* Cmd Not Supported In Spi */
    if (IS_SD(Mmc)) {
      Cmd.CmdIdx = SD_CMD_SEND_RELATIVE_ADDR;
      Cmd.CmdArg = Mmc->Rca << 16;
      Cmd.RespType = MMC_RSP_R6;
    } else {
      Cmd.CmdIdx = SD_CMD_SEND_RELATIVE_ADDR;
      Cmd.CmdArg = Mmc->Rca << 16;
      Cmd.RespType = MMC_RSP_R1;
    }

    Err = MmcSendCmd(Mmc, &Cmd, NULL);

    if (Err)
      return Err;

    if (IS_SD(Mmc))
      Mmc->Rca = (Cmd.Response[0] >> 16) & 0xffff;
  }

  /* Get The Card-Specific Data */
  Cmd.CmdIdx = MMC_CMD_SEND_CSD;
  Cmd.RespType = MMC_RSP_R2;
  Cmd.CmdArg = Mmc->Rca << 16;

  Err = MmcSendCmd(Mmc, &Cmd, NULL);

  /* Waiting for The Ready Status */
  MmcSendStatus(Mmc, Timeout);

  if (Err)
    return Err;

  Mmc->Csd[0] = Cmd.Response[0];
  Mmc->Csd[1] = Cmd.Response[1];
  Mmc->Csd[2] = Cmd.Response[2];
  Mmc->Csd[3] = Cmd.Response[3];

  if (Mmc->Version == MMC_VERSION_UNKNOWN) {
    INT32 Version = (Cmd.Response[0] >> 26) & 0xf;

    switch (Version) {
      case 0:
        Mmc->Version = MMC_VERSION_1_2;
        break;
      case 1:
        Mmc->Version = MMC_VERSION_1_4;
        break;
      case 2:
        Mmc->Version = MMC_VERSION_2_2;
        break;
      case 3:
        Mmc->Version = MMC_VERSION_3;
        break;
      case 4:
        Mmc->Version = MMC_VERSION_4;
        break;
      default:
        Mmc->Version = MMC_VERSION_1_2;
        break;
    }
  }

  /* Divide Frequency By 10, Since The Mults Are 10x Bigger */
  Freq = Fbase[(Cmd.Response[0] & 0x7)];
  Mult = Multipliers[((Cmd.Response[0] >> 3) & 0xf)];

  Mmc->TranSpeed = Freq * Mult;

  Mmc->DsrImp = ((Cmd.Response[1] >> 12) & 0x1);
  Mmc->ReadBlLen = 1 << ((Cmd.Response[1] >> 16) & 0xf);

  if (IS_SD(Mmc))
    Mmc->WriteBlLen = Mmc->ReadBlLen;
  else
    Mmc->WriteBlLen = 1 << ((Cmd.Response[3] >> 22) & 0xf);

  if (Mmc->HighCapacity) {
    CSize = (Mmc->Csd[1] & 0x3f) << 16
            | (Mmc->Csd[2] & 0xffff0000) >> 16;
    CMult = 8;
  } else {
    CSize = (Mmc->Csd[1] & 0x3ff) << 2
            | (Mmc->Csd[2] & 0xc0000000) >> 30;
    CMult = (Mmc->Csd[2] & 0x00038000) >> 15;
  }

  Mmc->CapacityUser = (CSize + 1) << (CMult + 2);
  Mmc->CapacityUser *= Mmc->ReadBlLen;
  Mmc->CapacityBoot = 0;
  Mmc->CapacityRpmb = 0;
  for (I = 0; I < 4; I++)
    Mmc->CapacityGp[I] = 0;

  if (Mmc->ReadBlLen > MMC_MAX_BLOCK_LEN)
    Mmc->ReadBlLen = MMC_MAX_BLOCK_LEN;

  if (Mmc->WriteBlLen > MMC_MAX_BLOCK_LEN)
    Mmc->WriteBlLen = MMC_MAX_BLOCK_LEN;

  if ((Mmc->DsrImp) && (0xffffffff != Mmc->Dsr)) {
    Cmd.CmdIdx = MMC_CMD_SET_DSR;
    Cmd.CmdArg = (Mmc->Dsr & 0xffff) << 16;
    Cmd.RespType = MMC_RSP_NONE;
    if (MmcSendCmd(Mmc, &Cmd, NULL))
      DEBUG((EFI_D_ERROR, "MMC: SET_DSR Failed\n"));
  }

  /* Select The Card, And Put It Into Transfer Mode */
  if (!MmcHostIsSpi(Mmc)) { /* Cmd Not Supported In Spi */
    Cmd.CmdIdx = MMC_CMD_SELECT_CARD;
    Cmd.RespType = MMC_RSP_R1;
    Cmd.CmdArg = Mmc->Rca << 16;
    Err = MmcSendCmd(Mmc, &Cmd, NULL);

    if (Err)
      return Err;
  }
  /*
   * for SD, Its Erase Group Is Always One Sector
   */
  Mmc->EraseGrpSize = 1;
  Mmc->PartConfig = MMCPART_NOAVAILABLE;
  if (!IS_SD(Mmc) && (Mmc->Version >= MMC_VERSION_4)) {
    /* Check  ExtCsd Version And Capacity */
    Err = MmcSendExtCsd(Mmc, ExtCsd);
    if (!Err && (ExtCsd[EXT_CSD_REV] >= 2)) {
      /*
       * According To The JEDEC Standard, The Value Of
       * ExtCsd'S Capacity Is Valid if The Value Is More
       * Than 2GB
       */
      Capacity = ExtCsd[EXT_CSD_SEC_CNT] << 0
                 | ExtCsd[EXT_CSD_SEC_CNT + 1] << 8
                 | ExtCsd[EXT_CSD_SEC_CNT + 2] << 16
                 | ExtCsd[EXT_CSD_SEC_CNT + 3] << 24;
      Capacity *= MMC_MAX_BLOCK_LEN;
      if ((Capacity >> 20) > 2 * 1024)
        Mmc->CapacityUser = Capacity;
    }

    switch (ExtCsd[EXT_CSD_REV]) {
    case 1:
      Mmc->Version = MMC_VERSION_4_1;
      break;
    case 2:
      Mmc->Version = MMC_VERSION_4_2;
      break;
    case 3:
      Mmc->Version = MMC_VERSION_4_3;
      break;
    case 5:
      Mmc->Version = MMC_VERSION_4_41;
      break;
    case 6:
      Mmc->Version = MMC_VERSION_4_5;
      break;
    case 7:
      Mmc->Version = MMC_VERSION_5_0;
      break;
    }

    /*
     * Host Needs To Enable ERASE_GRP_DEF Bit if Device Is
     * Partitioned. This Bit Will Be Lost Every Time After A Reset
     * Or Power Off. This Will Affect Erase Size.
     */
    if ((ExtCsd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) &&
        (ExtCsd[EXT_CSD_PARTITIONS_ATTRIBUTE] & PART_ENH_ATTRIB)) {
      Err = MmcSwitch(Mmc, EXT_CSD_CMD_SET_NORMAL,
                  EXT_CSD_ERASE_GROUP_DEF, 1);

      if (Err)
        return Err;
      else
        ExtCsd[EXT_CSD_ERASE_GROUP_DEF] = 1;

      /* Read Out Group Size From ExtCsd */
      Mmc->EraseGrpSize =
             ExtCsd[EXT_CSD_HC_ERASE_GRP_SIZE] *
                         MMC_MAX_BLOCK_LEN * 1024;
      /*
       * if high capacity and partition setting completed
       * SEC_COUNT is valid even if it is smaller than 2 GiB
	* JEDEC Standard JESD84-B45, 6.2.4
	*/
      if (Mmc->HighCapacity &&
	 (ExtCsd[EXT_CSD_PARTITION_SETTING] &
	     EXT_CSD_PARTITION_SETTING_COMPLETED)) {
	 Capacity = (ExtCsd[EXT_CSD_SEC_CNT]) |
		     (ExtCsd[EXT_CSD_SEC_CNT + 1] << 8) |
		     (ExtCsd[EXT_CSD_SEC_CNT + 2] << 16) |
		     (ExtCsd[EXT_CSD_SEC_CNT + 3] << 24);
	 Capacity *= MMC_MAX_BLOCK_LEN;
	 Mmc->CapacityUser = Capacity;
      }
    } else {
      /* Calculate The Group Size From The Csd Value. */
      INT32 EraseGsz, EraseGmul;
      EraseGsz = (Mmc->Csd[2] & 0x00007c00) >> 10;
      EraseGmul = (Mmc->Csd[2] & 0x000003e0) >> 5;
      Mmc->EraseGrpSize = (EraseGsz + 1)
                       * (EraseGmul + 1);
    }

    /* Store The Partition Info Of Emmc */
    if ((ExtCsd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) ||
        ExtCsd[EXT_CSD_BOOT_MULT])
      Mmc->PartConfig = ExtCsd[EXT_CSD_PART_CONF];

    Mmc->CapacityBoot = ExtCsd[EXT_CSD_BOOT_MULT] << 17;

    Mmc->CapacityRpmb = ExtCsd[EXT_CSD_RPMB_MULT] << 17;

    for (I = 0; I < 4; I++) {
      INT32 Idx = EXT_CSD_GP_SIZE_MULT + I * 3;
      Mmc->CapacityGp[I] = (ExtCsd[Idx + 2] << 16) +
             (ExtCsd[Idx + 1] << 8) + ExtCsd[Idx];
      Mmc->CapacityGp[I] *=
             ExtCsd[EXT_CSD_HC_ERASE_GRP_SIZE];
      Mmc->CapacityGp[I] *= ExtCsd[EXT_CSD_HC_WP_GRP_SIZE];
    }
  }

  Err = MmcSetCapacity(Mmc, Mmc->PartNum);
  if (Err)
    return Err;
  if (IS_SD(Mmc))
    Err = SdChangeFreq(Mmc);
  else
    Err = MmcChangeFreq(Mmc);

  if (Err)
    return Err;

  /* Restrict Card'S Capabilities By What The Host Can do */
  Mmc->CardCaps &= Mmc->Cfg->HostCaps;

  if (IS_SD(Mmc)) {
    if (Mmc->CardCaps & MMC_MODE_4BIT) {
      Err = SendAppCmd(TRUE);
      if (Err)
        return Err;

      Cmd.CmdIdx = SD_CMD_APP_SET_BUS_WIDTH;
      Cmd.RespType = MMC_RSP_R1;
      Cmd.CmdArg = 2;
      Err = MmcSendCmd(Mmc, &Cmd, NULL);
      if (Err)
        return Err;

      MmcSetBusWidth(Mmc, 4);
    }

    if (Mmc->CardCaps & MMC_MODE_HS)
      Mmc->TranSpeed = 50000000;
    else
      Mmc->TranSpeed = 25000000;
  } else {
    INT32 Idx;

    /* An Array Of Possible Bus Widths In Order Of Preference */
    static UINT32 ExtCsdBits[] = {
           EXT_CSD_DDR_BUS_WIDTH_8,
           EXT_CSD_DDR_BUS_WIDTH_4,
           EXT_CSD_BUS_WIDTH_8,
           EXT_CSD_BUS_WIDTH_4,
           EXT_CSD_BUS_WIDTH_1,
    };

    /* An Array To Map CSD Bus Widths To Host Cap Bits */
    static UINT32 ExtToHostcaps[] = {
           [EXT_CSD_DDR_BUS_WIDTH_4] = MMC_MODE_DDR_52MHz | MMC_MODE_4BIT,
           [EXT_CSD_DDR_BUS_WIDTH_8] = MMC_MODE_DDR_52MHz | MMC_MODE_8BIT,
           [EXT_CSD_BUS_WIDTH_4] = MMC_MODE_4BIT,
           [EXT_CSD_BUS_WIDTH_8] = MMC_MODE_8BIT,
         };

    /* An Array To Map Chosen Bus Width To An Integer */
    static UINT32 Widths[] = {
           8, 4, 8, 4, 1,
    };

    for (Idx=0; Idx < ARRAY_SIZE(ExtCsdBits); Idx++) {
      UINT32 Extw = ExtCsdBits[Idx];
      UINT32 Caps = ExtToHostcaps[Extw];

      /*
       * Check To Make Sure The Controller Supports
       * This Bus Width, if It'S More Than 1
       */
#if 0
      if (Extw != EXT_CSD_BUS_WIDTH_1 &&
                    !(Mmc->Cfg->HostCaps & ExtToHostcaps[Extw]))
        continue;
#endif
      if ((Mmc->CardCaps & Caps) != Caps)
        continue;

      Err = MmcSwitch(Mmc, EXT_CSD_CMD_SET_NORMAL,
                    EXT_CSD_BUS_WIDTH, Extw);

      if (Err)
        continue;

      Mmc->DdrMode = (Caps & MMC_MODE_DDR_52MHz) ? 1 : 0;
      MmcSetBusWidth(Mmc, Widths[Idx]); //2

      Err = MmcSendExtCsd(Mmc, TestCsd);
      if (Err)
        continue;

      if (ExtCsd[EXT_CSD_PARTITIONING_SUPPORT] \
                 == TestCsd[EXT_CSD_PARTITIONING_SUPPORT]
              && ExtCsd[EXT_CSD_HC_WP_GRP_SIZE] \
                 == TestCsd[EXT_CSD_HC_WP_GRP_SIZE] \
              && ExtCsd[EXT_CSD_REV] \
                 == TestCsd[EXT_CSD_REV]
              && ExtCsd[EXT_CSD_HC_ERASE_GRP_SIZE] \
                 == TestCsd[EXT_CSD_HC_ERASE_GRP_SIZE]
              && InternalMemCompareMem(&ExtCsd[EXT_CSD_SEC_CNT], \
                    &TestCsd[EXT_CSD_SEC_CNT], 4) == 0)

        break;
      else
        Err = -1; 
    }

    if (Err)
      return Err;

    if (Mmc->CardCaps & MMC_MODE_HS) {
      if (Mmc->CardCaps & MMC_MODE_HS_52MHz)
        Mmc->TranSpeed = 52000000;
      else
        Mmc->TranSpeed = 26000000;
    }
  }

  MmcSetClock(Mmc, Mmc->TranSpeed);

  /* Fix the block length for DDR mode */
  if (Mmc->DdrMode) {
    Mmc->ReadBlLen = MMC_MAX_BLOCK_LEN;
    Mmc->WriteBlLen = MMC_MAX_BLOCK_LEN;
  }

  /* Fill In Device Description */
  Mmc->BlockDev.Lun = 0;
  Mmc->BlockDev.Type = 0;
  Mmc->BlockDev.Blksz = Mmc->ReadBlLen;
  Mmc->BlockDev.Log2blksz = LOG2(Mmc->BlockDev.Blksz);
  Mmc->BlockDev.Lba = LlDiv(Mmc->Capacity, Mmc->ReadBlLen);

  AsciiSPrint(Mmc->BlockDev.Vendor,sizeof(Mmc->BlockDev.Vendor),
         "Man %06x Snr %04x%04x", Mmc->Cid[0] >> 24,
         (Mmc->Cid[2] & 0xffff), (Mmc->Cid[3] >> 16) & 0xffff);
  AsciiSPrint(Mmc->BlockDev.Product,sizeof(Mmc->BlockDev.Product),
         "%c%c%c%c%c%c", Mmc->Cid[0] & 0xff, (Mmc->Cid[1] >> 24),
         (Mmc->Cid[1] >> 16) & 0xff, (Mmc->Cid[1] >> 8) & 0xff,
          Mmc->Cid[1] & 0xff, (Mmc->Cid[2] >> 24) & 0xff);
  AsciiSPrint(Mmc->BlockDev.Revision, sizeof(Mmc->BlockDev.Revision),
         "%d.%d", (Mmc->Cid[2] >> 20) & 0xf, (Mmc->Cid[2] >> 16) & 0xf);

  return 0;
}

static INT32
MmcCompleteInit (
  IN  struct Mmc *Mmc
  )
{
  INT32 Err = EFI_SUCCESS;
  if (Mmc->OpCondPending)
    Err = MmcCompleteOpCond(Mmc);

  if (Err == EFI_SUCCESS)
    Err = MmcStartup(Mmc);

  if (Err)
    Mmc->HasInit = 0;
  else
    Mmc->HasInit = 1;

  Mmc->InitInProgress = 0;
  return Err;
}

EFI_STATUS
MmcInit (
  IN  struct Mmc *Mmc
  )
{
  EFI_STATUS Err = EFI_ALREADY_STARTED;

  if (Mmc->HasInit)
    return EFI_SUCCESS;

  if (!Mmc->InitInProgress)
    Err = MmcStartInit();

  if (!Err || Err == EFI_ALREADY_STARTED)
    Err = MmcCompleteInit(Mmc);

  return Err;
}
