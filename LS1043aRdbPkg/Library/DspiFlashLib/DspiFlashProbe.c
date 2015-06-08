/** @DspiFlashProbe.c

  Probing function to detect device connected to DSPI controller

  Copyright (c) 2015, Freescale Ltd. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Library/Dspi.h"

/* Read Commands Array */
static UINT8 SpiReadCmdsArray[] = {
  CMD_READ_ARRAY_SLOW,
  CMD_READ_DUAL_OUTPUT_FAST,
  CMD_READ_DUAL_IO_FAST,
  CMD_READ_QUAD_OUTPUT_FAST,
  CMD_READ_QUAD_IO_FAST,
};

CONST struct SpiFlashParams SpiFlashParamsTable[] = {
  {(CONST INT8 *)"SST25WF512", 0xbf2501, 0x0,
	64 * 1024, 1, 0, SECT_4K | SST_WP},
  {(CONST INT8 *)"SST25WF010", 0xbf2502, 0x0,
	64 * 1024, 2, 0, SECT_4K | SST_WP},
  {(CONST INT8 *)"SST25WF020", 0xbf2503, 0x0,
	64 * 1024, 4, 0, SECT_4K | SST_WP},
  {(CONST INT8 *)"SST25WF040", 0xbf2504, 0x0,
	64 * 1024, 8, 0, SECT_4K | SST_WP},
};

struct DspiFlash *
DspiFlashValidateParams (
  IN  struct DspiSlave *Dspi,
  IN  UINT8 *Idcode
  )
{
  CONST struct SpiFlashParams *Params;
  struct DspiFlash *Flash;
  UINT8 Cmd;
  UINT16 Jedec = Idcode[1] << 8 | Idcode[2];
  UINT16 ExtJedec = Idcode[3] << 8 | Idcode[4];

  Params = SpiFlashParamsTable;
  for (; Params->Name != NULL; Params++) {
    if ((Params->Jedec >> 16) == Idcode[0]) {
      if ((Params->Jedec & 0xFFFF) == Jedec) {
	if (Params->ExtJedec == 0)
	  break;
	else if (Params->ExtJedec == ExtJedec)
	  break;
      }
    }
  }

  if (!Params->Name) {
    DEBUG((EFI_D_ERROR, "SF: Unsupported Flash IDs: "));
    DEBUG((EFI_D_ERROR, "Manuf %02x, Jedec %04x, ExtJedec %04x\n",
		Idcode[0], Jedec, ExtJedec));
    return NULL;
  }
  Flash = (struct DspiFlash *)AllocatePool(sizeof(struct DspiFlash));
  InternalMemZeroMem(Flash, sizeof(struct DspiFlash));
  if (!Flash) {
    DEBUG((EFI_D_ERROR, "SF: Failed To Allocate DspiFlash\n"));
    return NULL;
  }

  /* Assign Spi Data */
  Flash->Dspi = Dspi;
  Flash->Name = Params->Name;
  Flash->MemoryMap = Dspi->Slave.MemoryMap;
  Flash->DualFlash = Flash->Dspi->Slave.Option;

  /* Assign DspiFlash Ops */
  Flash->Write = DspiFlashCmdWriteOps;
  Flash->Erase = DspiFlashCmdEraseOps;
  Flash->Read = DspiFlashCmdReadOps;

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
  Flash->SectorSize = Params->SectorSize << Flash->Shift;
  Flash->Size = Flash->SectorSize * Params->NrSectors << Flash->Shift;

  /* Compute Erase Sector And Command */
  if (Params->Flags & SECT_4K) {
    Flash->EraseCmd = CMD_ERASE_4K;
    Flash->EraseSize = 4096 << Flash->Shift;
  } else if (Params->Flags & SECT_32K) {
    Flash->EraseCmd = CMD_ERASE_32K;
    Flash->EraseSize = 32768 << Flash->Shift;
  } else {
    Flash->EraseCmd = CMD_ERASE_64K;
    Flash->EraseSize = Flash->SectorSize;
  }

  Flash->BlockSize = Flash->EraseSize;

  /* Look for The Fastest Read Cmd */
  Cmd = GenericFls(Params->ERdCmd & Flash->Dspi->Slave.OpModeRx);
  if (Cmd) {
    Cmd = SpiReadCmdsArray[Cmd - 1];
    Flash->ReadCmd = Cmd;
  } else {
    /* Go for default Supported Read Cmd */
    Flash->ReadCmd = CMD_READ_ARRAY_FAST;
  }

  /* Not Require To Look for Fastest Only Two Write Cmds Yet */
  if (Params->Flags & WR_QPP && Flash->Dspi->Slave.OpModeTx & SPI_OPM_TX_QPP)
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
#if 0
  if (BoardSpiIsDataflash(Dspi->Slave.Bus, Dspi->Slave.Cs)) {
    Flash->PollCmd = CMD_ATMEL_READ_STATUS;
    Flash->WriteCmd = CMD_ATMEL_PAGE_PROGRAM;
    if (Params->Flags & SECT_32K)
      Flash->EraseCmd = CMD_ATMEL_ERASE_32K;
  }
#endif
  return Flash;
}

struct DspiFlash *
DspiFlashProbeSlave (
  IN  struct DspiSlave *Dspi
  )
{
  struct DspiFlash *Flash = NULL;
  UINT8 Idcode[5] = {0};
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
  Ret = DspiFlashReadWrite(Dspi, &Cmd, 1, NULL, Idcode,
		sizeof(Idcode));
  if (Ret != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR, "SF: Failed To Get Idcodes\n"));
    goto ErrClaimBus;
  }

  /* Validate Params From SpiFlashParams Table */
  Flash = DspiFlashValidateParams(Dspi, Idcode);
  if (!Flash)
    goto ErrClaimBus;

  /* Set The Quad Enable Bit - Only for Quad Commands */
  if ((Flash->ReadCmd == CMD_READ_QUAD_OUTPUT_FAST) ||
      (Flash->ReadCmd == CMD_READ_QUAD_IO_FAST) ||
      (Flash->WriteCmd == CMD_QUAD_PAGE_PROGRAM)) {
    if (DspiFlashSetQeb(Flash, Idcode[0]) != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "SF: Fail To Set QEB for %02x\n", Idcode[0]));
      return NULL;
    }
  }

  DEBUG((EFI_D_INFO, "SF: Detected %a With Page Size ", Flash->Name));
  PrintSize(Flash->PageSize, (CONST INT8 *)"Erase Size ");
  PrintSize(Flash->EraseSize, (CONST INT8 *)"Total ");
  PrintSize(Flash->Size, (CONST INT8 *)"");
  DEBUG((EFI_D_INFO, "\n"));

  if (Flash->MemoryMap)
    DEBUG((EFI_D_INFO, ", Mapped At %p\n", Flash->MemoryMap));

  if (((Flash->DualFlash == SF_SINGLE_FLASH) &&
       (Flash->Size > SPI_FLASH_16MB_BOUN)) ||
       ((Flash->DualFlash > SF_SINGLE_FLASH) &&
       (Flash->Size > SPI_FLASH_16MB_BOUN << 1))) {
    DEBUG((EFI_D_WARN, "SF: Warning - Only Lower 16MiB Accessible, "\
  		"Full Access #define CONFIG_SPI_FLASH_BAR\n"));
  }

  /* Release Dspi Bus */
  DspiReleaseBus(Dspi);
  return Flash;

ErrClaimBus:
  DspiReleaseBus(Dspi);
  DspiFreeSlave(Dspi);
  return NULL;
}
