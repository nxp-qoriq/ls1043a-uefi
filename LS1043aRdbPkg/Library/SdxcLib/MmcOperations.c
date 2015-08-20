/** MmcOperations.C
  File Containing Functions for Erase, Read, Write, Initialize Etc

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
FreqBase[] = {
       10000,
       100000,
       1000000,
       10000000,
};

/* Multiplier Values for TRAN_SPEED.  Multiplied By 10 To Be Nice
 * To Platforms Without Floating Point.
 */
static CONST INT32
FreqMult[] = {
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
static inline UINT64 LlDiv (
  IN UINT64 Dividend,
  IN UINT32 Divisor
  )
{
  UINT64 Res = Dividend;
  DoDiv(Res, Divisor);
  return(Res);
}

INT32
MmcGetwp (
  IN  struct Mmc *Sdxc
  )
{
  INT32 WriteProt;

  if (Sdxc->Cfg->Ops->SdxcGetwp)
    WriteProt = Sdxc->Cfg->Ops->SdxcGetwp(Sdxc); //TODO return response of CMD30
  else
    WriteProt = 0;

  return WriteProt;
}

INT32
SendCmd (
  IN  struct Mmc *Mmc,
  IN  struct SdCmd *Cmd,
  IN  struct SdData *Data
  )
{
  INT32 Ret;

  Ret = Mmc->Cfg->Ops->SdxcSendCmd(Mmc, Cmd, Data);
  return Ret;
}

EFI_STATUS
SendStatus (
  IN  struct Mmc *Mmc,
  IN  INT32 Timeout
  )
{
  struct SdCmd Cmd;
  INT32 Retries = 5;
  EFI_STATUS Status;

  Cmd.CmdIdx = MMC_CMD_SEND_STATUS;
  Cmd.RespType = MMC_RSP_R1;
  if (!MmcHostIsSpi(Mmc))
    Cmd.CmdArg = Mmc->Rca << 16;

  do {
    Status = SendCmd(Mmc, &Cmd, NULL);
    if (!Status) {
      if ((Cmd.Response[0] & MMC_STATUS_RDY_FOR_DATA) &&
          ((Cmd.Response[0] & MMC_STATUS_CURR_STATE) !=
           MMC_STATE_PRG))
        break;
      else if (Cmd.Response[0] & MMC_STATUS_MASK) {
        DEBUG((EFI_D_ERROR, "Status Error: 0x%08X\n", Cmd.Response[0]));
        return EFI_DEVICE_ERROR;
      }
    } else if (--Retries < 0)
      return Status;

    MicroSecondDelay(1000);
  } while (Timeout--);

  if (Timeout <= 0) {
    DEBUG((EFI_D_ERROR, "Timeout Waiting Card Ready\n"));
    return EFI_TIMEOUT;
  }
  if (Cmd.Response[0] & MMC_STATUS_SWITCH_ERROR)
    return EFI_NO_RESPONSE;

  return EFI_SUCCESS;
}

INT32
ReadBlks (
  IN  struct Mmc *Mmc,
  OUT VOID *Dest,
  IN  UINT64 Start,
  IN  UINT64 Blkcnt
  )
{
  struct SdCmd Cmd;
  struct SdData Data;

  if (Blkcnt > 1)
    Cmd.CmdIdx = MMC_CMD_READ_MULTIPLE_BLOCK;
  else
    Cmd.CmdIdx = MMC_CMD_READ_SINGLE_BLOCK;

  if (Mmc->HighCapacity)
    Cmd.CmdArg = Start;
  else
    Cmd.CmdArg = Start * Mmc->ReadBlkLen;

  Cmd.RespType = MMC_RSP_R1;

  Data.Dest = Dest;
  Data.Blocks = Blkcnt;
  Data.Blocksize = Mmc->ReadBlkLen;
  Data.Flags = MMC_DATA_READ;

  if (SendCmd(Mmc, &Cmd, &Data))
    return 0;

  if (Blkcnt > 1) {
    Cmd.CmdIdx = MMC_CMD_STOP_TRANSMISSION;
    Cmd.CmdArg = 0;
    Cmd.RespType = MMC_RSP_R1b;

    if (SendCmd(Mmc, &Cmd, NULL)) {
      DEBUG((EFI_D_ERROR, "Mmc Fail To Send Stop Cmd\n"));
      return 0;
    }
  }

  return Blkcnt;
}

INT32
SetBlocklen (
  IN  struct Mmc *Mmc,
  IN  INT32 Len
  )
{
  struct SdCmd Cmd;

  if (Mmc->DdrMode)
    return EFI_SUCCESS;

  Cmd.CmdIdx = MMC_CMD_SET_BLOCKLEN;
  Cmd.RespType = MMC_RSP_R1;
  Cmd.CmdArg = Len;

  return SendCmd(Mmc, &Cmd, NULL);
}

UINT64
SdxcWriteBlks (
  IN  struct Mmc *Mmc,
  IN  UINT64 Start,
  IN  UINT64 Blkcnt,
  IN  CONST VOID *Src
  )
{
  struct SdCmd Cmd;
  struct SdData Data;
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
    Cmd.CmdArg = Start * Mmc->WriteBlkLen;

  Cmd.RespType = MMC_RSP_R1;

  Data.Src = Src;
  Data.Blocks = Blkcnt;
  Data.Blocksize = Mmc->WriteBlkLen;
  Data.Flags = MMC_DATA_WRITE;

  if (SendCmd(Mmc, &Cmd, &Data)) {
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
    if (SendCmd(Mmc, &Cmd, NULL)) {
      DEBUG((EFI_D_ERROR, "Mmc Fail To Send Stop Cmd\n"));
      return 0;
    }
  }

  /* Waiting for The Ready Status */
  if (SendStatus(Mmc, Timeout))
    return 0;

  return Blkcnt;
}

UINT64
SdxcEraseBlks (
  IN  struct Mmc *Mmc,
  IN  UINT64 Start,
  IN  UINT64 Blkcnt
  )
{
  struct SdCmd Cmd;
  UINT64 End;
  INT32 Status, StartCmd, EndCmd;

  if (Mmc->HighCapacity) {
    End = Start + Blkcnt/Mmc->EraseGrpSize - 1;
  } else {
    End = (Start + Blkcnt/Mmc->EraseGrpSize - 1) * Mmc->WriteBlkLen;
    Start *= Mmc->WriteBlkLen;
  }

  if (IS_SD(Mmc)) {
    StartCmd = SD_CMD_ERASE_WR_BLK_START;
    EndCmd = SD_CMD_ERASE_WR_BLK_END;
  } else {
    StartCmd = MMC_CMD_ERASE_GROUP_START;
    EndCmd = MMC_CMD_ERASE_GROUP_END;
  }

  Cmd.CmdIdx = StartCmd;
  Cmd.CmdArg = Start;
  Cmd.RespType = MMC_RSP_R1;

  Status = SendCmd(Mmc, &Cmd, NULL);
  if (Status)
    goto ErrOut;

  Cmd.CmdIdx = EndCmd;
  Cmd.CmdArg = End;

  Status = SendCmd(Mmc, &Cmd, NULL);
  if (Status)
    goto ErrOut;

  Cmd.CmdIdx = MMC_CMD_ERASE;
  Cmd.CmdArg = SECURE_ERASE;
  Cmd.RespType = MMC_RSP_R1b;

  Status = SendCmd(Mmc, &Cmd, NULL);
  if (Status)
    goto ErrOut;

  return 0;

ErrOut:
  DEBUG((EFI_D_ERROR, "Mmc Erase Failed\n"));
  return Status;
}

UINT32
SdxcBlkRead (
  IN  UINT32 Start,
  IN  UINT32 Blkcnt,
  OUT VOID *Dest
  )
{
  UINT32 CurBlk, BlocksTodo = Blkcnt;

  if (Blkcnt == 0)
    return 0;

  if ((Start + Blkcnt) > gMmc->BlockDev.Lba) {
    DEBUG((EFI_D_ERROR, "MMC: Block Number 0x%x Exceeds Max(0x%x)\n",
           Start + Blkcnt, gMmc->BlockDev.Lba));
    return 0;
  }

  if (SetBlocklen(gMmc, gMmc->ReadBlkLen))
    return 0;

  do {
    CurBlk = (BlocksTodo > gMmc->Cfg->BMax) ?
           gMmc->Cfg->BMax : BlocksTodo;
    if (ReadBlks(gMmc, Dest, Start, CurBlk) != CurBlk)
      return 0;

    BlocksTodo -= CurBlk;
    Start += CurBlk;
    Dest += CurBlk * gMmc->ReadBlkLen;
  } while (BlocksTodo > 0);

  return Blkcnt;
}

UINT32
SdxcBlkWrite (
  IN  UINT32 Start,
  IN  UINT32 Blkcnt,
  IN  CONST VOID *Src
  )
{
  UINT32 CurBlk, BlocksTodo = Blkcnt;

  if (SetBlocklen(gMmc, gMmc->WriteBlkLen))
    return 0;

  do {
    CurBlk = (BlocksTodo > gMmc->Cfg->BMax) ?
           gMmc->Cfg->BMax : BlocksTodo;
    if (SdxcWriteBlks(gMmc, Start, CurBlk, Src) != CurBlk)
      return 0;

    BlocksTodo -= CurBlk;
    Start += CurBlk;
    Src += CurBlk * gMmc->WriteBlkLen;
  } while (BlocksTodo > 0);

  return Blkcnt;
}

UINT32
SdxcBlkErase (
  IN  UINT32 Start,
  IN  UINT32 Blkcnt
  )
{
  INT32 Status = 0;
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

    Status = SdxcEraseBlks(gMmc, Start + Blk, BlkR);
    if (Status)
      break;

    Blk += BlkR;

    /* Waiting for The Ready Status */
    if (SendStatus(gMmc, Timeout))
      return 0;
  }

  return Blk;
}

INT32
Getcd (
  VOID
  )
{
  INT32 Found = 0;

  ASSERT(gMmc->Cfg->Ops->SdxcGetcd);
  if (gMmc->Cfg->Ops->SdxcGetcd)
    Found = gMmc->Cfg->Ops->SdxcGetcd(gMmc);
  else
    Found = 1;

  return Found;
}

VOID
SetIos (
  IN  struct Mmc *Sdxc
  )
{
  if (Sdxc->Cfg->Ops->SdxcSetIos)
    Sdxc->Cfg->Ops->SdxcSetIos(Sdxc);
}

VOID
SdxcSetClock (
  IN  struct Mmc *Mmc,
  IN  UINT32 Clock
  )
{
  if (Clock > Mmc->Cfg->FMax)
    Clock = Mmc->Cfg->FMax;

  if (Clock < Mmc->Cfg->FMin)
    Clock = Mmc->Cfg->FMin;

  Mmc->Clock = Clock;

  SetIos(Mmc);
}

static VOID
SdxcSetBusWidth (
  IN  struct Mmc *Mmc,
  IN  UINT32 BWidth
  )
{
  Mmc->BusWidth = BWidth;

  SetIos(Mmc);
}

EFI_STATUS
SdxcGoIdle (
  IN  struct Mmc *Mmc
  )
{
  struct SdCmd Cmd;
  INT32 Status;

  MicroSecondDelay(1000);

  Cmd.CmdIdx = MMC_CMD_GO_IDLE_STATE;
  Cmd.CmdArg = 0;
  Cmd.RespType = MMC_RSP_NONE;

  Status = SendCmd(Mmc, &Cmd, NULL);

  if (Status)
    return Status;

  MicroSecondDelay(2000);

  return EFI_SUCCESS;
}

static EFI_STATUS
SdSendIfCond (
  IN  struct Mmc *Mmc
  )
{
  struct SdCmd Cmd;
  INT32 Status;

  Cmd.CmdIdx = SD_CMD_SEND_IF_COND;
  /* We Set The Bit if The Host Supports Voltages Between 2.7 And 3.6 V */
  Cmd.CmdArg = ((Mmc->Cfg->Voltages & 0xff8000) != 0) << 8 | 0xaa;
  Cmd.RespType = MMC_RSP_R7;

  Status = SendCmd(Mmc, &Cmd, NULL);

  if (Status)
    return Status;

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
  struct SdCmd Cmd;

  Cmd.CmdIdx = MMC_CMD_APP_CMD;
  Cmd.RespType = MMC_RSP_R1;

  if (Rca)
    Cmd.CmdArg = gMmc->Rca << 16;
  else
    Cmd.CmdArg = 0;

  return SendCmd(gMmc, &Cmd, NULL);
}

static EFI_STATUS
SdSendOpCond (
  IN  struct Mmc *Mmc
  )
{
  INT32 Timeout = 1000;
  EFI_STATUS Status;
  struct SdCmd Cmd;

  do {
    Status = SendAppCmd(FALSE);

    if (Status)
      return Status;

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

    Status = SendCmd(Mmc, &Cmd, NULL);

    if (Status)
      return Status;

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

    Status = SendCmd(Mmc, &Cmd, NULL);

    if (Status)
      return Status;
  }

  Mmc->Ocr = Cmd.Response[0];
  Mmc->HighCapacity = ((Mmc->Ocr & OCR_HCS) == OCR_HCS);
  Mmc->Rca = 0;

  return 0;
}

/* We Pass In The Cmd Since Otherwise The Init Seems To Fail */
static INT32
MmcSendOpCondCmdIter (
  IN  struct Mmc *Mmc,
  IN  struct SdCmd *Cmd,
  IN  INT32 UseArg
)
{
  INT32 Status;

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
  Status = SendCmd(Mmc, Cmd, NULL);

  if (Status)
    return Status;

  Mmc->Ocr = Cmd->Response[0];
  return EFI_SUCCESS;
}

EFI_STATUS
MmcSendOpCondCmd (
  IN  struct Mmc *Mmc
  )
{
  struct SdCmd Cmd;
  EFI_STATUS Status;
  INT32 I;

  /* Some Cards Seem To Need This */
  SdxcGoIdle(Mmc);

  /* Asking To The Card Its Capabilities */
  for (I = 0; I < 2; I++) {
    Status = MmcSendOpCondCmdIter(Mmc, &Cmd, I != 0);
    if (Status)
      return Status;

    /* Exit if Not Busy (Flag Seems To Be Inverted) */
    if (Mmc->Ocr & OCR_BUSY)
      return EFI_SUCCESS;
  }
  return EFI_ALREADY_STARTED;
}

EFI_STATUS
SdStartInit (
  VOID
  )
{
  struct SdxcCfg *Cfg = gMmc->Private;
  struct SdxcRegs *Regs = (struct SdxcRegs *)Cfg->SdxcBase;
  EFI_STATUS Status;

  /* We Pretend There'S No Card When Init Is NULL */
  if (Getcd() == 0 || gMmc->Cfg->Ops->SdxcInit == NULL) {
    gMmc->HasInit = 0;
    DEBUG((EFI_D_ERROR, "MMC: No Card Present\n"));

    return EFI_NO_MEDIA;
  }

  if (gMmc->HasInit)
    return EFI_SUCCESS;

  /* Made Sure It'S Not NULL Earlier */
  Status = gMmc->Cfg->Ops->SdxcInit(gMmc);

  if (Status)
    return Status;

  gMmc->DdrMode = 0;
  SdxcSetBusWidth(gMmc, 1);
  SdxcSetClock(gMmc, 1);

  MmioClearBitsBe32((UINTN)&Regs->Proctl, PROCTL_BE);

  /* Reset The Card */
  Status = SdxcGoIdle(gMmc);

  if (Status)
    return Status;

  /* The Internal Partition Reset To User Partition(0) At Every CMD0*/
  gMmc->PartNum = 0;

  /* Check for An MMC Card */
  Status = MmcSendOpCondCmd(gMmc);

  if (Status && Status != EFI_ALREADY_STARTED) {
    DEBUG((EFI_D_INFO, "Not MMC Card\n"));

    /* Test for SD Version 2 */
    Status = SdSendIfCond(gMmc);
    if (Status != EFI_NO_RESPONSE)
      DEBUG((EFI_D_INFO, "Not SD version 2\n"));

    /* Now Try To Get The SD Card'S Operating Condition */
    Status = SdSendOpCond(gMmc);
    if (Status)
      DEBUG((EFI_D_ERROR, "Card Did Not Respond To Voltage Select!\n"));
  } else
    gMmc->OpCondPending = 1;

  if (Status == EFI_ALREADY_STARTED)
    gMmc->InitInProgress = 1;

  return Status;
}

struct Mmc *
CreateMmcNode (
  IN struct MmcConfig *Cfg,
  IN VOID *Private
  )
{
  struct Mmc *Mmc;

  /* Quick Validation */
  if (Cfg == NULL || Cfg->Ops == NULL || Cfg->Ops->SdxcSendCmd == NULL ||
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

  InternalMemCopyMem(Mmc->Cfg, Cfg, sizeof(struct MmcConfig));
  Mmc->Private = Private;

  /* Setup Dsr Related Values */
  Mmc->DsrImp = 0;
  Mmc->Dsr = 0xffffffff;
  /* Setup The Universal Parts Of The Block Interface Just Once */
  Mmc->BlockDev.BlkRead = SdxcBlkRead;
  Mmc->BlockDev.BlkWrite = SdxcBlkWrite;
  Mmc->BlockDev.BlkErase = SdxcBlkErase;

  return Mmc;
}

EFI_STATUS
SdxcCompleteOpCond (
  IN  struct Mmc *Mmc
  )
{
  struct SdCmd Cmd;
  EFI_STATUS Status;

  Mmc->OpCondPending = 0;

  if (MmcHostIsSpi(Mmc)) { /* Read OCR for Spi */
         Cmd.CmdIdx = MMC_CMD_SPI_READ_OCR;
         Cmd.RespType = MMC_RSP_R3;
         Cmd.CmdArg = 0;

         Status = SendCmd(Mmc, &Cmd, NULL);

         if (Status)
                return Status;
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
  struct SdCmd Cmd;
  struct SdData Data;
  INT32 Status;

  /* Get The Card Status Register */
  Cmd.CmdIdx = MMC_CMD_SEND_EXT_CSD;
  Cmd.RespType = MMC_RSP_R1;
  Cmd.CmdArg = 0;

  Data.Blocksize = MMC_MAX_BLOCK_LEN;
  Data.Flags = MMC_DATA_READ;
  Data.Dest = (CHAR8 *)ExtCsd;
  Data.Blocks = 1;

  Status = SendCmd(Mmc, &Cmd, &Data);

  return Status;
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

  Mmc->BlockDev.Lba = LlDiv(Mmc->Capacity, Mmc->ReadBlkLen);

  return 0;
}

#ifdef GET_SD_REVISION
static INT32
SdxcSwitch (
  IN  struct Mmc *Mmc,
  IN  INT32 Mode,
  IN  INT32 Group,
  IN  UINT8 Value,
  UINT8 *Resp
  )
{
  struct SdCmd Cmd;
  struct SdData Data;

  /* switch The Frequency */
  Cmd.CmdIdx = SD_CMD_SWITCH_FUNC;
  Cmd.RespType = MMC_RSP_R1;
  Cmd.CmdArg = (Mode << 31) | 0xffffff;
  Cmd.CmdArg &= ~(0xf << (Group * 4));
  Cmd.CmdArg |= Value << (Group * 4);

  Data.Blocks = 1;
  Data.Flags = MMC_DATA_READ;
  Data.Dest = (CHAR8 *)Resp;
  Data.Blocksize = 64;

  return SendCmd(Mmc, &Cmd, &Data);
}

static INT32
SdxcChangeFreq (
  IN  struct Mmc *Mmc
  )
{
  INT32 Status;
  struct SdCmd Cmd;
  ALLOC_CACHE_ALIGN_BUFFER(UINT32, Scr, 2);
  ALLOC_CACHE_ALIGN_BUFFER(UINT32, SwitchStatus, 16);
  struct SdData Data;
  INT32 Timeout;

  Mmc->CardCaps = 0;

  if (MmcHostIsSpi(Mmc))
    return 0;

  /* Read The SCR To Find Out if This Card Supports Higher Speeds */
  Status = SendAppCmd(TRUE);

  if (Status)
    return Status;

  Cmd.CmdIdx = SD_CMD_APP_SEND_SCR;
  Cmd.RespType = MMC_RSP_R1;
  Cmd.CmdArg = 0;

  Timeout = 5;

RetryScr:
  Data.Blocks = 1;
  Data.Flags = MMC_DATA_READ;
  Data.Dest = (CHAR8 *)Scr;
  Data.Blocksize = 8;

  Status = SendCmd(Mmc, &Cmd, &Data);

  if (Status) {
    if (Timeout--)
      goto RetryScr;

    return Status;
  }

  MicroSecondDelay(100);

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
    Status = SdxcSwitch(Mmc, SD_SWITCH_CHECK, 0, 1,
		(UINT8 *)SwitchStatus);

    if (Status)
      return Status;

    /* The High-Speed Function Is Busy.  Try Again */
    if (!(SwitchStatus[7] & SD_HIGHSPEED_BUSY))
      break;
  }

  /* if High-Speed Isn'T Supported, We return */
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

  Status = SdxcSwitch(Mmc, SD_SWITCH_SWITCH, 0, 1, (UINT8 *)SwitchStatus);

  if (Status)
    return Status;

  if ((SwitchStatus[4] & 0x0f000000) == 0x01000000)
    Mmc->CardCaps |= MMC_MODE_HS;

  return 0;
}
#endif

static INT32
MmcSwitch (
  IN  struct Mmc *Mmc,
  IN  UINT8 Set,
  IN  UINT8 Index,
  IN UINT8 Value
  )
{
  struct SdCmd Cmd;
  INT32 Timeout = 1000;
  INT32 Ret;

  Cmd.CmdIdx = MMC_CMD_SWITCH;
  Cmd.RespType = MMC_RSP_R1b;
  Cmd.CmdArg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
		 (Index << 16) |
  		 (Value << 8);

  Ret = SendCmd(Mmc, &Cmd, NULL);

  /* Waiting for The Ready Status */
  if (!Ret)
    Ret = SendStatus(Mmc, Timeout);

  return Ret;
}

#ifdef GET_SD_REVISION
static INT32
MmcChangeFreq (
  IN  struct Mmc *Mmc
  )
{
  ALLOC_CACHE_ALIGN_BUFFER(UINT8, ExtCsd, MMC_MAX_BLOCK_LEN);
  CHAR8 Cardtype;
  INT32 Status;

  Mmc->CardCaps = MMC_MODE_4BIT | MMC_MODE_8BIT;

  if (MmcHostIsSpi(Mmc))
    return 0;

  /* Only Version 4 Supports High-Speed */
  if (Mmc->Version < MMC_VERSION_4)
    return 0;

  Status = MmcSendExtCsd(Mmc, ExtCsd);
  if (Status)
    return Status;

  Cardtype = ExtCsd[EXT_CSD_CARD_TYPE] & 0xf;

  Status = MmcSwitch(Mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);

  if (Status)
    return Status == EFI_NO_RESPONSE ? 0 : Status;

  /* Now Check To See That It Worked */
  Status = MmcSendExtCsd(Mmc, ExtCsd);

  if (Status)
    return Status;

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
#endif

static INT32
SdxcStartup (
  IN  struct Mmc *Mmc
  )
{
  INT32   Status, I;
  UINT32 Mult, Freq;
  UINT64 CsdMult, CsdSize, Capacity;
  struct SdCmd Cmd;

  ALLOC_CACHE_ALIGN_BUFFER(UINT8, ExtCsd, MMC_MAX_BLOCK_LEN);
  ALLOC_CACHE_ALIGN_BUFFER(UINT8, TestCsd, MMC_MAX_BLOCK_LEN);
  INT32 Timeout = 10000;

  /* Put The Card In Identify Mode */
  Cmd.CmdIdx = MmcHostIsSpi(Mmc) ? MMC_CMD_SEND_CID :
         MMC_CMD_ALL_SEND_CID; /* Cmd Not Supported In Spi */
  Cmd.RespType = MMC_RSP_R2;
  Cmd.CmdArg = 0;

  Status = SendCmd(Mmc, &Cmd, NULL);

  if (Status)
    return Status;

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

    Status = SendCmd(Mmc, &Cmd, NULL);

    if (Status)
      return Status;

    if (IS_SD(Mmc))
      Mmc->Rca = (Cmd.Response[0] >> 16) & 0xffff;
  }

  /* Get The Card-Specific Data */
  Cmd.CmdIdx = MMC_CMD_SEND_CSD;
  Cmd.RespType = MMC_RSP_R2;
  Cmd.CmdArg = Mmc->Rca << 16;

  Status = SendCmd(Mmc, &Cmd, NULL);

  /* Waiting for The Ready Status */
  SendStatus(Mmc, Timeout);

  if (Status)
    return Status;

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
  Freq = FreqBase[(Cmd.Response[0] & 0x7)];
  Mult = FreqMult[((Cmd.Response[0] >> 3) & 0xf)];

  Mmc->TranSpeed = Freq * Mult;

  Mmc->DsrImp = ((Cmd.Response[1] >> 12) & 0x1);
  Mmc->ReadBlkLen = 1 << ((Cmd.Response[1] >> 16) & 0xf);

  if (IS_SD(Mmc))
    Mmc->WriteBlkLen = Mmc->ReadBlkLen;
  else
    Mmc->WriteBlkLen = 1 << ((Cmd.Response[3] >> 22) & 0xf);

  if (Mmc->HighCapacity) {
    CsdSize = (Mmc->Csd[1] & 0x3f) << 16
            | (Mmc->Csd[2] & 0xffff0000) >> 16;
    CsdMult = 8;
  } else {
    CsdSize = (Mmc->Csd[1] & 0x3ff) << 2
            | (Mmc->Csd[2] & 0xc0000000) >> 30;
    CsdMult = (Mmc->Csd[2] & 0x00038000) >> 15;
  }

  Mmc->CapacityUser = (CsdSize + 1) << (CsdMult + 2);
  Mmc->CapacityUser *= Mmc->ReadBlkLen;
  Mmc->CapacityBoot = 0;
  Mmc->CapacityRpmb = 0;
  for (I = 0; I < 4; I++)
    Mmc->CapacityGp[I] = 0;

  if (Mmc->ReadBlkLen > MMC_MAX_BLOCK_LEN)
    Mmc->ReadBlkLen = MMC_MAX_BLOCK_LEN;

  if (Mmc->WriteBlkLen > MMC_MAX_BLOCK_LEN)
    Mmc->WriteBlkLen = MMC_MAX_BLOCK_LEN;

  if ((Mmc->DsrImp) && (0xffffffff != Mmc->Dsr)) {
    Cmd.CmdIdx = MMC_CMD_SET_DSR;
    Cmd.CmdArg = (Mmc->Dsr & 0xffff) << 16;
    Cmd.RespType = MMC_RSP_NONE;
    if (SendCmd(Mmc, &Cmd, NULL))
      DEBUG((EFI_D_ERROR, "MMC: SET_DSR Failed\n"));
  }

  /* Select The Card, And Put It Into Transfer Mode */
  if (!MmcHostIsSpi(Mmc)) { /* Cmd Not Supported In Spi */
    Cmd.CmdIdx = MMC_CMD_SELECT_CARD;
    Cmd.RespType = MMC_RSP_R1;
    Cmd.CmdArg = Mmc->Rca << 16;
    Status = SendCmd(Mmc, &Cmd, NULL);

    if (Status)
      return Status;
  }
  /*
   * for SD, Its Erase Group Is Always One Sector
   */
  Mmc->EraseGrpSize = 1;
  Mmc->PartConfig = MMCPART_NOAVAILABLE;
  if (!IS_SD(Mmc) && (Mmc->Version >= MMC_VERSION_4)) {
    /* Check  ExtCsd Version And Capacity */
    Status = MmcSendExtCsd(Mmc, ExtCsd);
    if (!Status && (ExtCsd[EXT_CSD_REV] >= 2)) {
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
      Status = MmcSwitch(Mmc, EXT_CSD_CMD_SET_NORMAL,
                  EXT_CSD_ERASE_GROUP_DEF, 1);

      if (Status)
        return Status;
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

  Status = MmcSetCapacity(Mmc, Mmc->PartNum);
  if (Status)
    return Status;

#ifdef GET_SD_REVISION
  if (IS_SD(Mmc))
    Status = SdxcChangeFreq(Mmc);
  else
    Status = MmcChangeFreq(Mmc);

  if (Status)
    return Status;
#endif

  /* Restrict Card'S Capabilities By What The Host Can do */
  Mmc->CardCaps &= Mmc->Cfg->HostCaps;

  if (IS_SD(Mmc)) {
    if (Mmc->CardCaps & MMC_MODE_4BIT) {
      Status = SendAppCmd(TRUE);
      if (Status)
        return Status;

      Cmd.CmdIdx = SD_CMD_APP_SET_BUS_WIDTH;
      Cmd.RespType = MMC_RSP_R1;
      Cmd.CmdArg = 2;
      Status = SendCmd(Mmc, &Cmd, NULL);
      if (Status)
        return Status;

      SdxcSetBusWidth(Mmc, 4);
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

      Status = MmcSwitch(Mmc, EXT_CSD_CMD_SET_NORMAL,
                    EXT_CSD_BUS_WIDTH, Extw);

      if (Status)
        continue;

      Mmc->DdrMode = (Caps & MMC_MODE_DDR_52MHz) ? 1 : 0;
      SdxcSetBusWidth(Mmc, Widths[Idx]);

      Status = MmcSendExtCsd(Mmc, TestCsd);
      if (Status)
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
        Status = -1; 
    }

    if (Status)
      return Status;

    if (Mmc->CardCaps & MMC_MODE_HS) {
      if (Mmc->CardCaps & MMC_MODE_HS_52MHz)
        Mmc->TranSpeed = 52000000;
      else
        Mmc->TranSpeed = 26000000;
    }
  }

  SdxcSetClock(Mmc, Mmc->TranSpeed);

  /* Fix the block length for DDR mode */
  if (Mmc->DdrMode) {
    Mmc->ReadBlkLen = MMC_MAX_BLOCK_LEN;
    Mmc->WriteBlkLen = MMC_MAX_BLOCK_LEN;
  }

  /* Fill In Device Description */
  Mmc->BlockDev.Blksz = Mmc->ReadBlkLen;
  Mmc->BlockDev.Lba = LlDiv(Mmc->Capacity, Mmc->ReadBlkLen);

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
SdxcCompleteInit (
  IN  struct Mmc *Mmc
  )
{
  INT32 Status = EFI_SUCCESS;
  if (Mmc->OpCondPending)
    Status = SdxcCompleteOpCond(Mmc);

  if (Status == EFI_SUCCESS)
    Status = SdxcStartup(Mmc);

  if (Status)
    Mmc->HasInit = 0;
  else
    Mmc->HasInit = 1;

  Mmc->InitInProgress = 0;
  return Status;
}

EFI_STATUS
MmcInit (
  IN  struct Mmc *Mmc
  )
{
  EFI_STATUS Status = EFI_ALREADY_STARTED;

  if (Mmc->HasInit)
    return EFI_SUCCESS;

  if (!Mmc->InitInProgress)
    Status = SdStartInit();

  if (!Status || Status == EFI_ALREADY_STARTED)
    Status = SdxcCompleteInit(Mmc);

  return Status;
}
