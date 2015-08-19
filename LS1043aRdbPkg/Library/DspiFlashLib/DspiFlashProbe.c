/** @DspiFlashProbe.c

  Probing function to detect device connected to DSPI controller

  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Library/Dspi.h"

/* Read Commands Array */
static UINT8 ReadCmdsArray[] = {
  CMD_READ_ARRAY_SLOW,
  CMD_READ_ARRAY_FAST,
  CMD_READ_DUAL_OUTPUT_FAST,
  CMD_READ_DUAL_IO_FAST,
  CMD_READ_QUAD_OUTPUT_FAST,
  CMD_READ_QUAD_IO_FAST,
};

CONST struct DspiFlashParameters DspiFlashTable[] = {
#ifdef CONFIG_SIMULATOR
  {(CONST INT8 *)"SST25WF512", 0xbf2501, 0x0,
	64 * 1024, 1, 0, SECT_4K | SST_WP},
  {(CONST INT8 *)"SST25WF010", 0xbf2502, 0x0,
	64 * 1024, 2, 0, SECT_4K | SST_WP},
  {(CONST INT8 *)"SST25WF020", 0xbf2503, 0x0,
	64 * 1024, 4, 0, SECT_4K | SST_WP},
  {(CONST INT8 *)"SST25WF040", 0xbf2504, 0x0,
	64 * 1024, 8, 0, SECT_4K | SST_WP},
#endif
#ifdef CONFIG_SPI_FLASH_SPANSION	/* SPANSION */
  {(CONST INT8 *)"S25FL008A", 0x010213, 0x0,	64 * 1024,  16, RD_NORM, 0},
  {(CONST INT8 *)"S25FL016A", 0x010214, 0x0,	64 * 1024,  32, RD_NORM, 0},
  {(CONST INT8 *)"S25FL032A", 0x010215, 0x0,	64 * 1024,  64, RD_NORM, 0},
  {(CONST INT8 *)"S25FL064A", 0x010216, 0x0,	64 * 1024, 128, RD_NORM, 0},
  {(CONST INT8 *)"S25FL116K", 0x014015, 0x0,	64 * 1024, 128, RD_NORM, 0},
  {(CONST INT8 *)"S25FL164K", 0x014017, 0x0140,	64 * 1024, 128, RD_NORM, 0},
  {(CONST INT8 *)"S25FL128P_256K", 0x012018, 0x0300, 256 * 1024,  64, RD_FULL, WR_QPP},
  {(CONST INT8 *)"S25FL128P_64K",  0x012018, 0x0301,  64 * 1024, 256, RD_FULL, WR_QPP},
  {(CONST INT8 *)"S25FL032P", 0x010215, 0x4d00, 64 * 1024,  64, RD_FULL, WR_QPP},
  {(CONST INT8 *)"S25FL064P", 0x010216, 0x4d00, 64 * 1024, 128, RD_FULL, WR_QPP},
  {(CONST INT8 *)"S25FL128S_256K", 0x012018, 0x4d00, 256 * 1024,   64, RD_FULL, WR_QPP},
  {(CONST INT8 *)"S25FL128S_64K",  0x012018, 0x4d01,  64 * 1024,  256, RD_FULL, WR_QPP},
  {(CONST INT8 *)"S25FL256S_256K", 0x010219, 0x4d00, 256 * 1024,  128, RD_FULL, WR_QPP},
  {(CONST INT8 *)"S25FL256S_64K",  0x010219, 0x4d01,  64 * 1024,  512, RD_FULL, WR_QPP},
  {(CONST INT8 *)"S25FL512S_256K", 0x010220, 0x4d00, 256 * 1024,  256, RD_FULL, WR_QPP},
  {(CONST INT8 *)"S25FL512S_64K",  0x010220, 0x4d01,  64 * 1024, 1024, RD_FULL, WR_QPP},
  {(CONST INT8 *)"S25FL512S_512K", 0x010220, 0x4f00, 256 * 1024,  256, RD_FULL, WR_QPP},
#endif
  {(CONST INT8 *)"M25P10",   0x202011, 0x0,    32 * 1024, 4,    RD_NORM, 0},
  {(CONST INT8 *)"M25P20",   0x202012, 0x0,    64 * 1024, 4,    RD_NORM, 0},
  {(CONST INT8 *)"M25P40",   0x202013, 0x0,    64 * 1024, 8,    RD_NORM, 0},
  {(CONST INT8 *)"M25P80",   0x202014, 0x0,    64 * 1024, 16,   RD_NORM, 0},
  {(CONST INT8 *)"M25P16",   0x202015, 0x0,    64 * 1024, 32,   RD_NORM, 0},
  {(CONST INT8 *)"M25PE16",  0x208015, 0x1000, 64 * 1024, 32,   RD_NORM, 0},
  {(CONST INT8 *)"M25PX16",  0x207115, 0x1000, 64 * 1024, 32,   RD_EXTN, 0},
  {(CONST INT8 *)"M25P32",   0x202016, 0x0,    64 * 1024, 64,   RD_NORM, 0},
  {(CONST INT8 *)"M25P64",   0x202017, 0x0,    64 * 1024, 128,  RD_NORM, 0},
  {(CONST INT8 *)"M25P128",  0x202018, 0x0,    256 * 1024,64,   RD_NORM, 0},
  {(CONST INT8 *)"M25PX64",  0x207117, 0x0,    64 * 1024, 128,  RD_NORM, SECT_4K},
  {(CONST INT8 *)"N25Q32",   0x20ba16, 0x0,    64 * 1024, 64,   RD_FULL, WR_QPP | SECT_4K},
  {(CONST INT8 *)"N25Q32A",  0x20bb16, 0x0,    64 * 1024, 64,   RD_FULL, WR_QPP | SECT_4K},
  {(CONST INT8 *)"N25Q64",   0x20ba17, 0x0,    64 * 1024, 128,  RD_FULL, WR_QPP | SECT_4K},
  {(CONST INT8 *)"N25Q64A",  0x20bb17, 0x0,    64 * 1024, 128,  RD_FULL, WR_QPP | SECT_4K},
  {(CONST INT8 *)"N25Q128",  0x20ba18, 0x0,    64 * 1024, 256,  RD_FULL, WR_QPP},
  {(CONST INT8 *)"N25Q128A", 0x20bb18, 0x0,    64 * 1024, 256,  RD_FULL, WR_QPP},
  {(CONST INT8 *)"N25Q256",  0x20ba19, 0x0,    64 * 1024, 512,  RD_FULL, WR_QPP | SECT_4K},
  {(CONST INT8 *)"N25Q256A", 0x20bb19, 0x0,    64 * 1024, 512,  RD_FULL, WR_QPP | SECT_4K},
  {(CONST INT8 *)"N25Q512",  0x20ba20, 0x0,    64 * 1024, 1024, RD_FULL, WR_QPP | E_FSR | SECT_4K},
  {(CONST INT8 *)"N25Q512A", 0x20bb20, 0x0,    64 * 1024, 1024, RD_FULL, WR_QPP | E_FSR | SECT_4K},
  {(CONST INT8 *)"N25Q1024", 0x20ba21, 0x0,    64 * 1024, 2048, RD_FULL, WR_QPP | E_FSR | SECT_4K},
  {(CONST INT8 *)"N25Q1024A",0x20bb21, 0x0,    64 * 1024, 2048, RD_FULL, WR_QPP | E_FSR | SECT_4K},
};

struct DspiFlash *
ValidateParameters (
  IN  struct DspiSlave *Dspi,
  IN  UINT8 *Id
  )
{
  CONST struct DspiFlashParameters *Parameters;
  struct DspiFlash *Flash;
  UINT8 Cmd;
  UINT16 Jedec = Id[1] << 8 | Id[2];
  UINT16 ExtJedec = Id[3] << 8 | Id[4];
  UINT8 CurrBank = 0;

  Parameters = DspiFlashTable;
  for (; Parameters->Name != NULL; Parameters++) {
    if ((Parameters->Jedec >> 16) == Id[0]) {
      if ((Parameters->Jedec & 0xFFFF) == Jedec) {
	if (Parameters->ExtJedec == 0)
	  break;
	else if (Parameters->ExtJedec == ExtJedec)
	  break;
      }
    }
  }

  if (!Parameters->Name) {
    DEBUG((EFI_D_ERROR, "Unsupported DSPI Flash IDs: "));
    DEBUG((EFI_D_ERROR, "Manuf %02x, Jedec %04x, ExtJedec %04x\n",
		Id[0], Jedec, ExtJedec));
    return NULL;
  }
  Flash = (struct DspiFlash *)AllocatePool(sizeof(struct DspiFlash));
  InternalMemZeroMem(Flash, sizeof(struct DspiFlash));
  if (!Flash) {
    DEBUG((EFI_D_ERROR, "Failed To Allocate DspiFlash\n"));
    return NULL;
  }

  /* Assign Spi Data */
  Flash->Dspi = Dspi;
  Flash->Name = Parameters->Name;
  Flash->MemoryMap = Dspi->Slave.MemoryMap;
  Flash->DualFlash = Flash->Dspi->Slave.Option;

  /* Assign DspiFlash Ops */
  Flash->Write = DspiWriteOps;
  Flash->Erase = DspiEraseOps;
  Flash->Read = DspiReadOps;

  /* Compute The Flash Size */
  Flash->Shift = (Flash->DualFlash & SF_DUAL_PARALLEL_FLASH) ? 1 : 0;
  /*
   * The Spansion S25FL032P And S25FL064P Have 256b Pages, Yet Use The
   * 0x4d00 Extended JEDEC Code. The Rest Of The Spansion Flashes With
   * The 0x4d00 Extended JEDEC Code Have 512b Pages. All Of The Others
   * Have 256b Pages.
   */
  if (ExtJedec == 0x4d00) {
    if ((Jedec == 0x0215) || (Jedec == 0x216))
      Flash->PageSize = 256;
    else
      Flash->PageSize = 512;
  } else {
    Flash->PageSize = 256;
  }
  Flash->PageSize <<= Flash->Shift;
  Flash->SectorSize = Parameters->SectorSize << Flash->Shift;
  Flash->Size = Flash->SectorSize * Parameters->NoOfSectors << Flash->Shift;

  /* Compute Erase Sector And Command */
  if (Parameters->Flags & SECT_4K) {
    Flash->EraseCmd = CMD_ERASE_4K;
    Flash->EraseSize = 4096 << Flash->Shift;
  } else if (Parameters->Flags & SECT_32K) {
    Flash->EraseCmd = CMD_ERASE_32K;
    Flash->EraseSize = 32768 << Flash->Shift;
  } else {
    Flash->EraseCmd = CMD_ERASE_64K;
    Flash->EraseSize = Flash->SectorSize;
  }

  if (Parameters->Name == (CONST INT8 *)"N25Q128")
    Flash->BlockSize = Flash->SectorSize/NUM_OF_SUBSECTOR;
  else
    Flash->BlockSize = Flash->SectorSize;

  /* Look for The Fastest Read Cmd */
  Cmd = GenericFls(Parameters->EnumRdCmd & Flash->Dspi->Slave.OpModeRx);
  if (Cmd) {
    Cmd = ReadCmdsArray[Cmd - 1];
    Flash->ReadCmd = Cmd;
  } else {
    /* Go for default Supported Read Cmd */
    Flash->ReadCmd = CMD_READ_ARRAY_FAST;
  }

  /* Not Require To Look for Fastest Only Two Write Cmds Yet */
  if (Parameters->Flags & WR_QPP && Flash->Dspi->Slave.OpModeTx & SPI_OPM_TX_QPP)
    Flash->WriteCmd = CMD_QUAD_PAGE_PROGRAM;
  else
    /* Go for default Supported Write Cmd */
    Flash->WriteCmd = CMD_BYTE_PROGRAM;

  /* Read DummyByte: Dummy Byte Is Determined Based On The
   * Dummy Cycles Of A Particular Command.
   * Fast Commands - DummyByte = DummyCycles/8
   * I/O Commands- DummyByte = (DummyCycles * No.Of Lines)/8
   * for I/O Commands Except Cmd[0] Everything Goes On No.Of Lines
   * Based On Particular Command But Incase Of Fast Commands Except
   * Data All Go On Single Line Irrespective Of Command.
   */
  switch (Flash->ReadCmd) {
  case CMD_READ_QUAD_IO_FAST:
    Flash->DummyByte = 2;
    break;
  case CMD_READ_ARRAY_SLOW:
    Flash->DummyByte = 0;
    break;
  default:
    Flash->DummyByte = 1;
  }

  /* Poll Cmd Selection */
  Flash->PollCmd = CMD_READ_STATUS;


  if (Parameters->Flags & E_FSR)
    Flash->PollCmd = CMD_FLAG_STATUS;


  /* Configure the BAR - discover bank cmds and read current bank */
  if (Flash->Size > SPI_FLASH_16MB_BOUN) {
    INT32 Ret;

    Flash->BankReadCmd = (Id[0] == 0x01) ?
                         CMD_BANKADDR_BRRD : CMD_EXTNADDR_RDEAR;
    Flash->BankWriteCmd = (Id[0] == 0x01) ?
                         CMD_BANKADDR_BRWR : CMD_EXTNADDR_WREAR;

    Ret = DspiCommonRead(Flash, &Flash->BankReadCmd, 1,
                             &CurrBank, 1);
    if (Ret != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR,"Fail to read bank addr register\n"));
      return NULL;
    }
    Flash->BankCurr = CurrBank;
  } else {
    Flash->BankCurr = CurrBank;
  }
#if 0
  if (IsDataflash(Dspi->Slave.Bus, Dspi->Slave.Cs)) {
    Flash->PollCmd = CMD_ATMEL_READ_STATUS;
    Flash->WriteCmd = CMD_ATMEL_PAGE_PROGRAM;
    if (Parameters->Flags & SECT_32K)
      Flash->EraseCmd = CMD_ATMEL_ERASE_32K;
  }
#endif
  return Flash;
}

/* enable the W#/Vpp signal to disable writing to the status register */
EFI_STATUS
DspiEnableWritePin (
  IN  struct DspiFlash *Flash
  )
{
  return EFI_SUCCESS;
}

/**
 * DspiProbeDevice() - Probe for a SPI flash device on a bus
 *
 * @Dspi: Bus to probe
 */
struct DspiFlash *
DspiProbeDevice (
  IN  struct DspiSlave *Dspi
  )
{
  struct DspiFlash *Flash = NULL;
  UINT8 Id[5] = {0};
  INT32 Ret;
  UINT8 Cmd;

  /* Claim Dspi Bus */
  Ret = DspiClaimBus(Dspi);
  if (Ret) {
    DEBUG((EFI_D_ERROR, "SF: Failed To Claim SPI Bus: %d\n", Ret));
    goto ErrClaimBus;
  }

  /* Read The ID Codes */
  Cmd = CMD_READ_JEDEC_ID;
  Ret = DspiReadWrite(Dspi, &Cmd, 1, NULL, Id,
		sizeof(Id));
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Failed To Get Ids\n"));
    goto ErrClaimBus;
  }

  /* Validate Parameters From SpiFlashParameters Table */
  Flash = ValidateParameters(Dspi, Id);
  if (!Flash)
    goto ErrClaimBus;

  /* Set The Quad Enable Bit - Only for Quad Commands */
  if ((Flash->ReadCmd == CMD_READ_QUAD_OUTPUT_FAST) ||
      (Flash->ReadCmd == CMD_READ_QUAD_IO_FAST) ||
      (Flash->WriteCmd == CMD_QUAD_PAGE_PROGRAM)) {
    if (DspiSetQeb(Flash, Id[0]) != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "Fail To Set QEB for %02x\n", Id[0]));
      return NULL;
    }
  }

  DEBUG((EFI_D_RELEASE, "Detected DSPI Flash %a With Page Size ", Flash->Name));
  PrintMemorySize(Flash->PageSize, (CONST INT8 *)"Erase Size ");
  PrintMemorySize(Flash->EraseSize, (CONST INT8 *)"Total ");
  PrintMemorySize(Flash->Size, (CONST INT8 *)"");
  DEBUG((EFI_D_RELEASE, "\n"));

  if (Flash->MemoryMap)
    DEBUG((EFI_D_INFO, ", Mapped At %p\n", Flash->MemoryMap));

  if (((Flash->DualFlash == SF_SINGLE_FLASH) &&
       (Flash->Size > SPI_FLASH_16MB_BOUN)) ||
       ((Flash->DualFlash > SF_SINGLE_FLASH) &&
       (Flash->Size > SPI_FLASH_16MB_BOUN << 1))) {
    DEBUG((EFI_D_WARN, "Warning - Only Lower 16MiB Accessible, "\
  		"Full Access #define CONFIG_SPI_FLASH_BAR\n"));
  }

  if (DspiEnableWritePin(Flash) != EFI_SUCCESS)
    DEBUG((EFI_D_WARN,"Enable WP pin failed\n"));

  /* Release Dspi Bus */
  DspiReleaseBus(Dspi);
  return Flash;

ErrClaimBus:
  DspiReleaseBus(Dspi);
  DspiFreeSlave(Dspi);
  return NULL;
}
