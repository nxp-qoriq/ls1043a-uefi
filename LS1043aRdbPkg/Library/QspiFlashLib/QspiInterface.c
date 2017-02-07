/** QspiInterface.c
  Qspi flash library containing functions for read, write, initialize etc

  Copyright (c) 2016, freescale semiconductor, inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the bsd
  license which accompanies this distribution. the full text of the license may
  be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <Uefi.h>
#include <Library/Qspi.h>
#include <Library/UefiRuntimeLib.h>

#include "QspiFlashLib.h"

struct QspiFlash *GFlash;

QSPI_FLASH_DESCRIPTION mQspiDevices[QSPI_FLASH_DEVICE_COUNT] = {
  // FIXME: Add RCW, PBI, FMAN images here when available
  { // UEFI
    0, //DeviceBaseAddress
    0, //RegionBaseAddress
    0,   //Size
    0,        //BlockSize
  },
};

EFI_STATUS
QspiFlashErase (
  IN  UINT64 Offset,
  IN  UINT64 Len
  )
{
  UINT32 EraseSize, EraseAddr;
  UINT8 Cmd[SPI_COMMON_FLASH_CMD_LEN];
  INT32 Ret = -1;

  /* Consistency Checking */
  if (Offset + Len > GFlash->Size) {
    DEBUG((EFI_D_ERROR, "ERROR: Attempting %d Past Flash Size (0x%x)\n",
		Len, GFlash->Size));
    return EFI_INVALID_PARAMETER;
  }

  EraseSize = GFlash->EraseSize;

  if (Offset % EraseSize || Len % EraseSize) {
    DEBUG((EFI_D_ERROR, "QSPI Erase Offset/Length Not"\
          "Multiple Of Erase Size\n"));
    return EFI_INVALID_PARAMETER;
  }

  Cmd[0] = GFlash->EraseCmd;
  while (Len) {
    EraseAddr = Offset;

    QspiCreateAddr(EraseAddr, Cmd);

    Ret = QspiCommonWrite(GFlash, Cmd, sizeof(Cmd), NULL, 0);
    if (Ret != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "QSPI Erase Failed\n"));
      break;
    }

    Offset += EraseSize;
    Len -= EraseSize;
  }

  return Ret;
}

EFI_STATUS
QspiFlashWrite (
  IN  UINT64 Offset,
  IN  UINT64 Len,
  IN  CONST VOID *Buf
  )
{
  UINT64 ByteAddr, PageSize;
  UINT64 WriteAddr;
  UINT64 Actual;
  UINT32 ChunkLen = 1;
  INT32 Ret = -1;
  UINT8 Cmd[SPI_COMMON_FLASH_CMD_LEN];

  /* Consistency Checking */
  if (Offset + Len > GFlash->Size) {
    DEBUG((EFI_D_ERROR, "ERROR: Attempting %d Past Flash Size (0x%x)\n",
		Len, GFlash->Size));
    return EFI_INVALID_PARAMETER;
  }

  PageSize = GFlash->PageSize;

  Cmd[0] = GFlash->WriteCmd;
  for (Actual = 0; Actual < Len; Actual += ChunkLen) {
    WriteAddr = Offset;
    ByteAddr = Offset % PageSize;
    ChunkLen = Min(Len - Actual, (PageSize - ByteAddr));

    if (GFlash->Qspi->Slave.MaxWriteSize)
      ChunkLen = Min(ChunkLen,
		GFlash->Qspi->Slave.MaxWriteSize);

    QspiCreateAddr(WriteAddr, Cmd);

    Ret = QspiCommonWrite(GFlash, Cmd, sizeof(Cmd),
                         Buf + Actual, ChunkLen);
    if (Ret != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "QSPI Write Failed\n"));
      break;
    }

    Offset += ChunkLen;
  }

  return Ret;
}

EFI_STATUS
QspiFlashRead (
  IN  UINT64 Offset,
  IN  UINT64 Len,
  OUT VOID *Buf
  )
{
  UINT8 CmdSz = 0;
  UINT8 Cmd[SPI_COMMON_FLASH_CMD_LEN + GFlash->DummyByte];
  UINT32 RemainLen, ReadLen, ReadAddr;
  INT32 BankSel = 0;
  INT32 Ret = -1;

  /* Consistency Checking */
  if (Offset + Len > GFlash->Size) {
    DEBUG((EFI_D_ERROR, "ERROR: Attempting %d Past Flash Size (0x%x)\n",
		Len, GFlash->Size));
    return EFI_INVALID_PARAMETER;
  }
  /* Handle Memory-Mapped SPI */
  if (GFlash->MemoryMap) {
    Ret = QspiClaimBus(GFlash->Qspi);
    if (Ret != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "Unable To Claim QSPI Bus\n"));
      return Ret;
    }
    QspiXfer(GFlash->Qspi, 0, NULL, NULL, SPI_COMMON_XFER_MMAP);
    InternalMemCopyMem(Buf, GFlash->MemoryMap + Offset, Len);
    QspiXfer(GFlash->Qspi, 0, NULL, NULL, SPI_COMMON_XFER_MMAP_END);
    QspiReleaseBus(GFlash->Qspi);
    return EFI_SUCCESS;
  }

  CmdSz = SPI_COMMON_FLASH_CMD_LEN + GFlash->DummyByte;

  Cmd[0] = GFlash->ReadCmd;
  while (Len) {
    ReadAddr = Offset;

    RemainLen = ((SPI_COMMON_FLASH_16MB_BOUN << GFlash->Shift) *
                 (BankSel + 1)) - Offset;
    if (Len < RemainLen)
      ReadLen = Len;
    else
      ReadLen = RemainLen;

    QspiCreateAddr(ReadAddr, Cmd);
    Ret = QspiCommonRead(GFlash, Cmd, CmdSz, Buf, ReadLen);
    if (Ret != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "QSPI Read Failed\n"));
      break;
    }

    Offset += ReadLen;
    Len -= ReadLen;
    Buf += ReadLen;
  }

  return Ret;
}

VOID
QspiFlashFree (
  VOID
  )
{
  FreePool(GFlash->Qspi);
  FreePool(GFlash);
}

VOID
QspiFlashRelocate (
  VOID
  )
{
  EfiConvertPointer (0x0, (VOID**)&(GFlash->Qspi->Regs));
  EfiConvertPointer (0x0, (VOID**)&(GFlash->Qspi));
  EfiConvertPointer (0x0, (VOID**)&GFlash);
}

  EFI_STATUS
QspiDetect(
    VOID
    )
{
  struct QspiSlave *Qspi;

  UINT32 Bus = DEFAULT_BUS;
  UINT32 Cs = DEFAULT_CS;
  UINT32 Speed = DEFAULT_SPEED;
  UINT32 Mode = DEFAULT_MODE;
  struct QspiFlash *New = NULL;


  Qspi = QspiSetupSlave(Bus, Cs, Speed, Mode);
  if (!Qspi) {
    DEBUG((EFI_D_ERROR, "QSPI : Failed To Set Up Slave\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  New = QspiProbeDevice(Qspi);

  if (!New)
    return EFI_OUT_OF_RESOURCES;

  if (GFlash)
    QspiFlashFree();

  GFlash = New;

  return EFI_SUCCESS;
}

  EFI_STATUS
QspiPlatformInitialization (
    VOID
    )
{
  EFI_STATUS  Status;

  Status = QspiDetect();

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Qspi Initialization Failure: Status: %x\n", Status));
    return Status;
  }

  return Status;
}

  EFI_STATUS
QspiPlatformGetDevices (
    OUT QSPI_FLASH_DESCRIPTION   **QspiDevices,
    OUT UINT32                  *Count
    )
{
  if ((QspiDevices == NULL) || (Count == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  // Get the number of Qspi flash devices supported
  *QspiDevices = mQspiDevices;
  *Count = QSPI_FLASH_DEVICE_COUNT;

  return EFI_SUCCESS;
}

  EFI_STATUS
QspiPlatformFlashGetAttributes (
    OUT QSPI_FLASH_DESCRIPTION  *QspiDevices,
    IN UINT32                  Count
    )
{
  UINT32                  Index;

  if ((QspiDevices == NULL) || (Count == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < Count; Index++) {
    QspiDevices[Index].BlockSize = GFlash->BlockSize;
    QspiDevices[Index].Size = GFlash->Size;
    QspiDevices[Index].DeviceBaseAddress = PcdGet64(PcdFlashDeviceBase64);
    QspiDevices[Index].RegionBaseAddress = 
      PcdGet64(PcdFlashReservedRegionBase64);
    // Limit the Size of QSPI Flash that can be programmed
    QspiDevices[Index].Size -= (QspiDevices[Index].RegionBaseAddress - 
        QspiDevices[Index].DeviceBaseAddress);
    if((QspiDevices[Index].RegionBaseAddress - 
          QspiDevices[Index].DeviceBaseAddress) % QspiDevices[Index].BlockSize) 
    {
      DEBUG ((EFI_D_ERROR, "%a ERROR: Reserved Region(0x%p) doesn't start from"
            "block boundry(0x%08x)\n", __FUNCTION__,\
            (UINTN)QspiDevices[Index].RegionBaseAddress,
            (UINT32)QspiDevices[Index].BlockSize));
      return EFI_DEVICE_ERROR;
    }
  }

  return EFI_SUCCESS;
}
