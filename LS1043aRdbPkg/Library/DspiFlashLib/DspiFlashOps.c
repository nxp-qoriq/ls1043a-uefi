/** @DspiFlashOps.c

  Functions for implementing DSI Controller functionality.

  Copyright (c) 2015, Freescale Ltd. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Library/Dspi.h"

struct DspiSlave *
DspiAllocSlave (
  IN  INT32 Offset,
  IN  INT32 Size,
  IN  UINT32 Bus,
  IN  UINT32 Cs
 )
{

  struct DspiSlave *Ptr = NULL;
  Ptr = (struct DspiSlave*)AllocatePool(Size);
  if (!Ptr)
    return Ptr;

  InternalMemZeroMem(Ptr, Size);

  Ptr->Slave.Bus = Bus;
  Ptr->Slave.Cs = Cs;
  Ptr->Slave.Wordlen = SPI_DEFAULT_WORDLEN;

  return Ptr;
}

INT32
BoardSpiIsDataflash (
  IN  UINT32 Bus,
  IN  UINT32 Cs
  )
{
  if (Bus == SPI_BUS_FSL_DSPI1 && Cs == 0)
    return 1;
  else
    return 0;
}

/*
 * PrINT32 Sizes As "Xxx KiB", "Xxx.Y KiB", "Xxx MiB", "Xxx.Y MiB",
 * Xxx GiB, Xxx.Y GiB, Etc As Needed; Allow for Optional Trailing String
 * (Like "\n")
 */
VOID
PrintSize (
  IN  UINT64 Size,
  IN  CONST INT8 *S
  )
{
  UINT64 M = 0, N;
  UINT64 F;
  static CONST INT8 Names[] = {'E', 'P', 'T', 'G', 'M', 'K'};
  UINT64 D = 10 * ARRAY_SIZE(Names);
  CHAR8 C = 0;
  UINT32 I;

  for (I = 0; I < ARRAY_SIZE(Names); I++, D -= 10) {
    if (Size >> D) {
      C = Names[I];
      break;
    }
  }

  if (!C) {
    DEBUG((EFI_D_ERROR, "%Ld Bytes,\n %a", Size, S));
    return;
  }

  N = Size >> D;
  F = Size & ((1ULL << D) - 1);

  /* if There'S A Remainder, Deal With It */
  if (F) {
    M = (10ULL * F + (1ULL << (D - 1))) >> D;

    if (M >= 10) {
           M -= 10;
           N += 1;
    }
  }

  DEBUG((EFI_D_ERROR, "%Ld", N));
  if (M) {
    DEBUG((EFI_D_ERROR, ".%Ld", M));
  }
  DEBUG((EFI_D_ERROR, " %ciB, %a ", C, S));
}

EFI_STATUS
DspiFlashCmdReadStatus (
  IN  struct DspiFlash *Flash,
  OUT UINT8 *Res
  )
{
  INT32 Ret;
  UINT8 Cmd;
  Cmd = CMD_READ_STATUS;

  Ret = DspiFlashReadCommon(Flash, &Cmd, 1, Res, 1);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Fail To Read Status Register\n"));
    return Ret;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DspiFlashCmdWriteConfig (
  IN  struct DspiFlash *Flash,
  IN  UINT8 WCmd
  )
{
  UINT8 Data[2];
  UINT8 Cmd;
  INT32 Ret;

  Ret = DspiFlashCmdReadStatus(Flash, &Data[0]);
  if (Ret != EFI_SUCCESS)
    return Ret;

  Cmd = CMD_WRITE_STATUS;
  Data[1] = WCmd;
  Ret = DspiFlashWriteCommon(Flash, &Cmd, 1, &Data, 2);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Fail To Write Config Register\n"));
    return Ret;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DspiFlashCmdReadConfig (
  IN  struct DspiFlash *Flash,
  IN  UINT8 *RCmd
  )
{
  INT32 Ret;
  UINT8 Cmd;

  Cmd = CMD_READ_CONFIG;
  Ret = DspiFlashReadCommon(Flash, &Cmd, 1, RCmd, 1);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Fail To Read Config Register\n"));
    return Ret;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DspiFlashSetQebWinspan (
  IN  struct DspiFlash *Flash
  )
{
  UINT8 QebStatus;
  INT32 Ret;

  Ret = DspiFlashCmdReadConfig(Flash, &QebStatus);
  if (Ret != EFI_SUCCESS)
    return Ret;

  if (QebStatus & STATUS_QEB_WINSPAN) {
    DEBUG((EFI_D_ERROR, "SF: Winspan: QEB Is Already Set\n"));
  } else {
    Ret = DspiFlashCmdWriteConfig(Flash, STATUS_QEB_WINSPAN);
    if (Ret != EFI_SUCCESS)
      return Ret;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DspiFlashSetQeb (
  IN  struct DspiFlash *Flash,
  IN  UINT8 Idcode0
  )
{
  switch (Idcode0) {
  case SPI_FLASH_CFI_MFR_SPANSION:
  case SPI_FLASH_CFI_MFR_WINBOND:
    return DspiFlashSetQebWinspan(Flash);
  default:
    DEBUG((EFI_D_ERROR, "SF: Need Set QEB Func for %02x Flash\n",
			Idcode0));
    return EFI_UNSUPPORTED;
  }
}

VOID
DspiTx (
  IN  struct DspiSlave *Dspislave,
  IN  UINT32 Ctrl,
  IN  UINT16 Data
  )
{
  UINT32 Ret = 0;

  Ret = MmioRead32((UINTN)&Dspislave->Regs->Sr);

  while ((Ret & 0x0000F000) >= 4) {
    Ret = MmioRead32((UINTN)&Dspislave->Regs->Sr);
  }

  MmioWriteBe32((UINTN)&Dspislave->Regs->Tfr, (Ctrl | Data));
}

UINT32
DspiRx (
  IN  struct DspiSlave *Dspislave
  )
{
  UINT32 Ret = 0;
  Ret = MmioRead32((UINTN)&Dspislave->Regs->Sr);
  while ((Ret & 0x000000F0) == 0) {
    Ret = MmioRead32((UINTN)&Dspislave->Regs->Sr);
  }

  Ret = 0;
  Ret = MmioRead32((UINTN)&Dspislave->Regs->Rfr);

  return Ret;
}

EFI_STATUS
DspiXfer (
  IN  struct DspiSlave *DspiSlave,
  IN  UINT32 Bitlen,
  IN  CONST VOID *Dout,
  OUT VOID *Din,
  IN  UINT64 Flags
  )
{
  UINT16 *SpiRd16 = NULL, *SpiWr16 = NULL;
  UINT8 *SpiRd = NULL, *SpiWr = NULL;
  static UINT32 Ctrl = 0;
  UINT32 Len = Bitlen >> 3;
  INT32 ShiftCount = 0;

  if (DspiSlave->Charbit == 16) {
    Bitlen >>= 1;
    SpiWr16 = (UINT16 *) Dout;
    SpiRd16 = (UINT16 *) Din;
    ShiftCount = 16;
  } else {
    SpiWr = (UINT8 *) Dout;
    SpiRd = (UINT8 *) Din;
    ShiftCount = 24;
  }

  if ((Flags & SPI_XFER_BEGIN) == SPI_XFER_BEGIN)
    Ctrl |= DSPI_TFR_CONT;

  Ctrl = (Ctrl & 0xFF000000) | ((1 << DspiSlave->Slave.Cs) << 16); /* */

  if (Len > 1) {
    INT32 TmpLen = Len - 1;
    while (TmpLen--) {
      if (Dout != NULL) {
        if (DspiSlave->Charbit == 16)
	  DspiTx(DspiSlave, Ctrl, *SpiWr16++);
	else
	  DspiTx(DspiSlave, Ctrl, *SpiWr++);

	DspiRx(DspiSlave);
      }

      if (Din != NULL) {
	DspiTx(DspiSlave, Ctrl, CONFIG_SPI_IDLE_VAL);
	if (DspiSlave->Charbit == 16)
	  *SpiRd16++ = DspiRx(DspiSlave) >> ShiftCount;
	else
	  *SpiRd++ = DspiRx(DspiSlave) >> ShiftCount;
      }
    }
    Len = 1;	/* Remaining Byte */
  }

  if ((Flags & SPI_XFER_END) == SPI_XFER_END)
    Ctrl &= ~DSPI_TFR_CONT;

  if (Len) {
    if (Dout != NULL) {
      if (DspiSlave->Charbit == 16)
        DspiTx(DspiSlave, Ctrl, *SpiWr16);
      else
        DspiTx(DspiSlave, Ctrl, *SpiWr);
	
      DspiRx(DspiSlave);
    }

    if (Din != NULL) {
      DspiTx(DspiSlave, Ctrl, CONFIG_SPI_IDLE_VAL);
      if (DspiSlave->Charbit == 16)
        *SpiRd16 = DspiRx(DspiSlave) >> ShiftCount;
      else
	*SpiRd = DspiRx(DspiSlave) >> ShiftCount;
    }
  } else {
    /* Dummy Read */
    DspiTx(DspiSlave, Ctrl, CONFIG_SPI_IDLE_VAL);
    DspiRx(DspiSlave);
  }

  return EFI_SUCCESS;
}

struct DspiSlave *
DspiSetupSlave (
  IN  UINT32 Bus,
  IN  UINT32 Cs,
  IN  UINT32 MaxHz,
  IN  UINT32 Mode
  )
{
  /*
   * Bit Definition for Mode:
   * Bit 31 - 28: Transfer Size 3 To 16 Bits
   *     27 - 26: PCS To SCK Delay Prescaler
   *     25 - 24: After SCK Delay Prescaler
   *     23 - 22: Delay After Transfer Prescaler
   *     21     : Allow Overwrite for Bit 31-22 And Bit 20-8
   *     20     : Double Baud Rate
   *     19 - 16: PCS To SCK Delay Scaler
   *     15 - 12: After SCK Delay Scaler
   *     11 -  8: Delay After Transfer Scaler
   *      7 -  0: SPI_CPHA, SPI_CPOL, SPI_LSB_FIRST
   */
  struct DspiSlave *Dspislave;
  INT32 Prescaler[] = { 2, 3, 5, 7 };
  INT32 Scaler[] = {
    2, 4, 6, 8,
    16, 32, 64, 128,
    256, 512, 1024, 2048,
    4096, 8192, 16384, 32768
  };
  UINT32 I, J, Pbrcnt, Brcnt, Diff, Tmp, Dbr = 0;
  UINT32 BestI, BestJ, Bestmatch = 0x7FFFFFFF, BaudSpeed, BusClk;
  UINT32 BusSetup = 0;
  UINT32 Ret = 0;

  Dspislave = DspiAllocSlave(0, sizeof(struct DspiSlave), Bus, Cs);
  if (!Dspislave)
    return NULL;

  Dspislave->Baudrate = MaxHz;
  Dspislave->Regs = (VOID *)CONFIG_SYS_FSL_DSPI_ADDR;

  /* MCR : 0x80FF 0C00 */
  MmioWriteBe32((UINTN)&Dspislave->Regs->Mcr,
    DSPI_MCR_MSTR | DSPI_MCR_CSIS7 | DSPI_MCR_CSIS6 |
    DSPI_MCR_CSIS5 | DSPI_MCR_CSIS4 | DSPI_MCR_CSIS3 |
    DSPI_MCR_CSIS2 | DSPI_MCR_CSIS1 | DSPI_MCR_CSIS0 |
    DSPI_MCR_CRXF | DSPI_MCR_CTXF);

  Ret = MmioReadBe32((UINTN)&Dspislave->Regs->Mcr);

  /* default Setting In Platform Configuration 0x3800 0000 */
  MmioWriteBe32((UINTN)&Dspislave->Regs->Ctar[0], CONFIG_SYS_DSPI_CTAR0);

  Tmp = (Prescaler[3] * Scaler[15]);
  BusClk = (UINT32)GET_CLOCK;

  /* Maximum And Minimum Baudrate It Can Handle */
  if ((Dspislave->Baudrate > (BusClk >> 1)) ||
    (Dspislave->Baudrate < (BusClk / Tmp))) {
    DEBUG((EFI_D_INFO, "Exceed Baudrate Limitation: Max %d - Min %d\n",
		(INT32)(BusClk >> 1), (INT32)(BusClk / Tmp)));

    return NULL;
  }

  /* Activate Double Baud When It Exceed 1/4 The Bus Clk */
  if ((CONFIG_SYS_DSPI_CTAR0 & DSPI_CTAR_DBR) ||
    (Dspislave->Baudrate > (BusClk / (Prescaler[0] * Scaler[0])))) {
    BusSetup |= DSPI_CTAR_DBR;
    Dbr = 1;
  }

  if (Mode & SPI_CPOL)
    BusSetup |= DSPI_CTAR_CPOL;
  if (Mode & SPI_CPHA)
    BusSetup |= DSPI_CTAR_CPHA;
  if (Mode & SPI_LSB_FIRST)
    BusSetup |= DSPI_CTAR_LSBFE;

  /* Overwrite default Value Set In Platform Configuration File */
  Ret = MmioReadBe32((UINTN)&Dspislave->Regs->Ctar[Dspislave->Slave.Cs]);
  if (Mode & SPI_MODE_MOD) {
    if ((Mode & 0xF0000000) == 0) {
      BusSetup |= Ret & 0x78000000;	/* 0x0000 002F */
    } else
      BusSetup |= ((Mode & 0xF0000000) >> 1);

    /*
     * Check To See if It Is Enabled By default In Platform
     * Config, Or Manual Setting Passed By Mode Parameter
     **/
    if (Mode & SPI_DBLRATE) {
      BusSetup |= DSPI_CTAR_DBR;
      Dbr = 1;
    }

    BusSetup |= (Mode & 0x0FC00000) >> 4; /* PSCSCK, PASC, PDT */
    BusSetup |= (Mode & 0x000FFF00) >> 4; /* CSSCK, ASC, DT */
  } else
    BusSetup |= Ret & 0x78FCFFF0;

  Dspislave->Charbit = ((Ret & 0x78000000) == 0x78000000) ? 16 : 8;

  Pbrcnt = sizeof(Prescaler) / sizeof(INT32);
  Brcnt = sizeof(Scaler) / sizeof(INT32);

  /* Baudrate Calculation - To Closer Value, May Not Be Exact Match */
  for (BestI = 0, BestJ = 0, I = 0; I < Pbrcnt; I++) {
    BaudSpeed = BusClk / Prescaler[I];
    for (J = 0; J < Brcnt; J++) {
      Tmp = (BaudSpeed / Scaler[J]) * (1 + Dbr);

      if (Tmp > Dspislave->Baudrate)
	Diff = Tmp - Dspislave->Baudrate;
      else
	Diff = Dspislave->Baudrate - Tmp;

      if (Diff < Bestmatch) {
	Bestmatch = Diff;
	BestI = I;
	BestJ = J;
      }
    }
  }

  BusSetup |= (DSPI_CTAR_PBR(BestI) | DSPI_CTAR_BR(BestJ));
  MmioWriteBe32((UINTN)&Dspislave->Regs->Ctar[Dspislave->Slave.Cs], BusSetup);

  return Dspislave;
}

VOID
DspiFreeSlave (
  IN  struct DspiSlave *Slave
  )
{
  FreePool(Slave);
}

EFI_STATUS
DspiClaimBus (
  IN  struct DspiSlave *Slave
  )
{
  return EFI_SUCCESS;
}

VOID
DspiReleaseBus (
  IN  struct DspiSlave *Slave
  )
{
  return;
}

EFI_STATUS
DspiFlashReadWrite (
  IN  struct DspiSlave *Dspi,
  IN  CONST UINT8 *Cmd,
  IN  UINT32 CmdLen,
  IN  CONST UINT8 *DataOut,
  OUT UINT8 *DataIn,
  IN  UINT64 DataLen
  )
{
  UINT64 Flags = SPI_XFER_BEGIN;
  INT32 Ret;

  if (DataLen == 0)
    Flags |= SPI_XFER_END;

  Ret = DspiXfer(Dspi, CmdLen * 8, Cmd, NULL, Flags);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Failed To Send Command (%zu Bytes): %d\n",
			CmdLen, Ret));
    return Ret;
  } else if (DataLen != 0) {
    Ret = DspiXfer(Dspi, DataLen * 8, DataOut, DataIn, SPI_XFER_END);
    if (Ret != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "SF: Failed To Transfer %zu Bytes Of Data: %d\n",
			DataLen, Ret));
      return Ret;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DspiFlashCmdWaitReady (
  IN  struct DspiFlash *Flash,
  IN  UINT64 Timeout
  )
{
  struct DspiSlave *Dspi = Flash->Dspi;
  UINT64 Timebase;
  UINT64 Flags = SPI_XFER_BEGIN;
  EFI_STATUS Ret = EFI_SUCCESS;
  INT32 OutOfTime = 1;
  UINT8 Status = 0;
  UINT8 CheckStatus = 0x0;
  UINT8 PollBit = STATUS_WIP;
  UINT8 Cmd = Flash->PollCmd;
  UINT8 ClearCmd = CMD_CLEAR_FLAG_STATUS;

  if (Cmd == CMD_FLAG_STATUS || Cmd == CMD_ATMEL_FLAG_STATUS) {
    PollBit = STATUS_PEC;
    CheckStatus = PollBit;
  }

  Ret = DspiXfer(Dspi, 8, &Cmd, NULL, Flags);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Fail To Read %a Status Register\n",
          Cmd == CMD_READ_STATUS ? "Read" : "Flag"));
    return Ret;
  }

  Timebase = 0;
  do {
    //WATCHDOG_RESET();

    Ret = DspiXfer(Dspi, 8, NULL, &Status, 0);
    if (Ret != EFI_SUCCESS) {
      DspiXfer(Dspi, 0, NULL, NULL, SPI_XFER_END);
      return Ret;
    }

    if ((Status & PollBit) == CheckStatus) {
      OutOfTime = 0;
      break;
    }

  } while (++Timebase < Timeout);

  DspiXfer(Dspi, 0, NULL, NULL, SPI_XFER_END);

  if (OutOfTime) {
    /* Timed Out */
    DEBUG((EFI_D_ERROR, "SF: Time Out!\n"));
    if (Cmd == CMD_FLAG_STATUS) {
      if (DspiFlashReadWrite(Flash->Dspi, &ClearCmd, 1, NULL, NULL, 0)
			!= EFI_SUCCESS)
        DEBUG((EFI_D_ERROR, "SF: Clear Flag Status Failed\n"));
    }
    Ret = EFI_TIMEOUT;
  }
  return Ret;
}

EFI_STATUS
DspiFlashWriteCommon (
  IN  struct DspiFlash *Flash,
  IN  CONST UINT8 *Cmd,
  IN  UINT32 CmdLen,
  IN  CONST VOID *Buf,
  IN  UINT32 BufLen
  )
{
  struct DspiSlave *Dspi = Flash->Dspi;
  UINT64 Timeout = SPI_FLASH_PROG_TIMEOUT;
  INT32 Ret;
  UINT8 *Wcmd = NULL;
  Wcmd = (UINT8 *)AllocatePool(CmdLen);
  InternalMemZeroMem(Wcmd, CmdLen);
  if (!Wcmd) {
    DEBUG((EFI_D_ERROR, "SF: Failed To Allocate Cmd\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  Wcmd[0] = CMD_WRITE_ENABLE;

  if (Buf == NULL)
    Timeout = SPI_FLASH_PAGE_ERASE_TIMEOUT;

  Ret = DspiClaimBus(Flash->Dspi);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Unable To Claim SPI Bus\n"));
    return Ret;
  }

  Ret = DspiFlashReadWrite(Dspi, Wcmd, 1, NULL, NULL, 0);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Enabling Write Failed\n"));
    return Ret;
  }

  Ret = DspiFlashReadWrite(Dspi, Cmd, CmdLen, Buf, NULL, BufLen);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Write Cmd Failed\n"));
    return Ret;
  }

  Ret = DspiFlashCmdWaitReady(Flash, Timeout);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Write %s Failed\n",
          Timeout == SPI_FLASH_PROG_TIMEOUT ?
           "Program" : "Page Erase"));
    return Ret;
  }

  DspiReleaseBus(Dspi);
  return Ret;
}

VOID
DspiFlashAddr (
  IN  UINT32 Addr,
  OUT UINT8 *Cmd
  )
{
  /* Cmd[0] Is Actual Command */
  Cmd[1] = Addr >> 16;
  Cmd[2] = Addr >> 8;
  Cmd[3] = Addr >> 0;
}

EFI_STATUS
DspiFlashReadCommon (
  IN  struct DspiFlash *Flash,
  IN  CONST UINT8 *Cmd,
  IN  UINT32 CmdLen,
  OUT VOID *Data,
  IN  UINT64 DataLen
  )
{
  struct DspiSlave *Dspi = Flash->Dspi;
  INT32 Ret;

  Ret = DspiClaimBus(Flash->Dspi);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Unable To Claim SPI Bus\n"));
    return Ret;
  }

  Ret = DspiFlashReadWrite(Dspi, Cmd, CmdLen, NULL, Data, DataLen);
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Read Cmd Failed\n"));
    return Ret;
  }

  DspiReleaseBus(Dspi);

  return Ret;
}

EFI_STATUS
DspiFlashCmdReadOps (
  IN  struct DspiFlash *Flash,
  IN  UINT32 Offset,
  IN  UINT64 Len,
  OUT VOID *Data
  )
{
  UINT8 CmdSz = 0;
  UINT8 *Cmd = NULL;
  UINT32 RemainLen, ReadLen, ReadAddr;
  INT32 BankSel = 0;
  INT32 Ret = -1;

  /* Handle Memory-Mapped SPI */
  if (Flash->MemoryMap) {
    Ret = DspiClaimBus(Flash->Dspi);
    if (Ret != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "SF: Unable To Claim SPI Bus\n"));
      return Ret;
    }
    DspiXfer(Flash->Dspi, 0, NULL, NULL, SPI_XFER_MMAP);
    InternalMemCopyMem(Data, Flash->MemoryMap + Offset, Len);
    DspiXfer(Flash->Dspi, 0, NULL, NULL, SPI_XFER_MMAP_END);
    DspiReleaseBus(Flash->Dspi);
    return EFI_SUCCESS;
  }

  CmdSz = SPI_FLASH_CMD_LEN + Flash->DummyByte;

  Cmd = (UINT8 *)AllocatePool(CmdSz);
  if (!Cmd) {
    DEBUG((EFI_D_ERROR, "SF: Out of Memory\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  Cmd[0] = Flash->ReadCmd;
  while (Len) {
    ReadAddr = Offset;

    RemainLen = ((SPI_FLASH_16MB_BOUN << Flash->Shift) *
                 (BankSel + 1)) - Offset;
    if (Len < RemainLen)
      ReadLen = Len;
    else
      ReadLen = RemainLen;

    DspiFlashAddr(ReadAddr, Cmd);
    Ret = DspiFlashReadCommon(Flash, Cmd, CmdSz, Data, ReadLen);
    if (Ret != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "SF: Read Failed\n"));
      break;
    }

    Offset += ReadLen;
    Len -= ReadLen;
    Data += ReadLen;
  }

  return Ret;
}

EFI_STATUS
DspiFlashCmdEraseOps (
  IN  struct DspiFlash *Flash,
  IN  UINT32 Offset,
  IN  UINT64 Len
  )
{
  UINT32 EraseSize, EraseAddr;
  UINT8 Cmd[SPI_FLASH_CMD_LEN];
  INT32 Ret = -1;

  EraseSize = Flash->EraseSize;
  if (Offset % EraseSize || Len % EraseSize) {
    DEBUG((EFI_D_ERROR, "SF: Erase Offset/Length Not"\
          "Multiple Of Erase Size\n"));
    return EFI_INVALID_PARAMETER;
  }

  Cmd[0] = Flash->EraseCmd;
  while (Len) {
    EraseAddr = Offset;

    DspiFlashAddr(EraseAddr, Cmd);

    Ret = DspiFlashWriteCommon(Flash, Cmd, sizeof(Cmd), NULL, 0);
    if (Ret != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "SF: Erase Failed\n"));
      break;
    }

    Offset += EraseSize;
    Len -= EraseSize;
  }

  return Ret;
}

EFI_STATUS
DspiFlashCmdWriteOps (
  IN  struct DspiFlash *Flash,
  IN  UINT32 Offset,
  IN  UINT64 Len,
  IN  CONST VOID *Buf
  )
{
  UINT32 WriteAddr;
  UINT64 ChunkLen = 1, Actual;
  UINT8 Cmd[SPI_FLASH_CMD_LEN];
  INT32 Ret = -1;
  Cmd[0] = Flash->WriteCmd;
  for (Actual = 0; Actual < Len; Actual++) {
    WriteAddr = Offset;

    DspiFlashAddr(WriteAddr, Cmd);

    Ret = DspiFlashWriteCommon(Flash, Cmd, sizeof(Cmd),
                         Buf + Actual, ChunkLen);
    if (Ret != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "SF: Write Failed\n"));
      break;
    }

    Offset += ChunkLen;
  }

  return Ret;
}
