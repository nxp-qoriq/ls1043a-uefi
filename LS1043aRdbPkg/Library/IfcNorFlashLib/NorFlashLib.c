/** @NorFlashLib.c

  Based on NorFlash implementation available in NorFlashDxe.c
  
  Copyright (c) 2011 - 2014, ARM Ltd. All rights reserved.<BR>
  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/NorFlashLib.h>
#include <Library/NorFlash.h>
#include <Library/FslIfc.h>

#include "CfiNorFlashLib.h"
#include "Mt28ew01gabaCfiNorFlashLib.h"

#define NOR_FLASH_DEVICE_COUNT	1

#define GET_BLOCK_OFFSET(Lba) ((Instance->RegionBaseAddress)-(Instance->DeviceBaseAddress)+((UINTN)((Lba) * Instance->Media.BlockSize)))

NOR_FLASH_DESCRIPTION mNorFlashDevices[NOR_FLASH_DEVICE_COUNT] = {
  // FIXME: Add RCW, PBI, FMAN images here when available
  { // UEFI
    0, // DeviceBaseAddress
    0, // RegionBaseAddress
    0, // Size
    0, // BlockSize
    { 0x1F15DA3C, 0x37FF, 0x4070, { 0xB4, 0x71, 0xBB, 0x4A, 0xF1, 0x2A, 0x72, 0x4A } },
    0 // MultiByteWordCount
  },
};

/**
 * The following function erases a NOR flash sector.
 **/
EFI_STATUS
NorFlashPlatformEraseSector (
  IN NOR_FLASH_INSTANCE     *Instance,
  IN UINTN                  SectorAddress
  )
{
  FLASH_DATA		EraseStatus1 = 0;
  FLASH_DATA		EraseStatus2 = 0;
  UINTN           	Count;

  // Request a sector erase by writing two unlock cycles, followed by a
  // setup command and two additional unlock cycles
  
  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_SECTOR_ERASE_FIRST);

  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR, MT28EW01GABA_CMD_SECTOR_ERASE_SECOND);

  // Issue a setup command
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_SECTOR_ERASE_THIRD);

  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_SECTOR_ERASE_FOURTH);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR, MT28EW01GABA_CMD_SECTOR_ERASE_FIFTH);

  // Now send the address of the sector to be erased
  SEND_NOR_COMMAND(SectorAddress, 0, MT28EW01GABA_CMD_SECTOR_ERASE_SIXTH);
  
  // Wait for erase to complete
  // Read status register to determine ERASE DONE
  while ((EraseStatus1 = FLASH_READ(SectorAddress)) != (EraseStatus2 = FLASH_READ(SectorAddress)));
  
  // Wait for erase to complete
  for(Count = 0; Count < 2048000; Count++) {
  	// Check if we have really erased the flash
  	EraseStatus1 = FLASH_READ(SectorAddress);
	if (EraseStatus1 == 0xFFFF)
		break;
  }
  
  return EFI_SUCCESS;
}

/**
 * The following function erases the entire NOR flash chip.
 **/
EFI_STATUS
NorFlashPlatformEraseChip (
  IN NOR_FLASH_INSTANCE     *Instance
  )
{
  FLASH_DATA		EraseStatus1 = 0;
  FLASH_DATA		EraseStatus2 = 0;

  // Request a sector erase by writing two unlock cycles, followed by a
  // setup command and two additional unlock cycles, which are then
  // followed by chip erase commands
  
  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_FIRST);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_SECOND);

  // Issue the setup cmd
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_THIRD);

  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_FOURTH);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_FIFTH);

  // Issue the chip erase cmd
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_SIXTH);

  // Wait for erase to complete
  // Read status register to determine ERASE DONE
  while ((EraseStatus1 = FLASH_READ(Instance->DeviceBaseAddress)) != (EraseStatus2 = FLASH_READ(Instance->DeviceBaseAddress)));
    
  return EFI_SUCCESS;
}

EFI_STATUS NorFlashPlatformWritePageBuffer
(
  IN NOR_FLASH_INSTANCE      *Instance,
  IN UINTN                   PageBufferOffset,
  IN UINTN                   NumWords,
  IN FLASH_DATA              *Buffer
)
{
  UINT16        Timeout = ~0;
  UINTN         LastWrittenAddress;
  UINTN         current_offset;
  UINTN         end_offset;
  UINTN         TargetAddress;
  FLASH_DATA    Read1;
  FLASH_DATA    Read2;

  /* Initialize variables */
  current_offset   = PageBufferOffset;
  end_offset       = PageBufferOffset + NumWords - 1;
  
  TargetAddress = CREATE_NOR_ADDRESS(Instance->DeviceBaseAddress,CREATE_BYTE_OFFSET(current_offset));

  // don't try with a count of zero
  if (!NumWords)
    return EFI_SUCCESS;

  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR,
		   MT28EW01GABA_CMD_WRITE_TO_BUFFER_FIRST);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR,
		   MT28EW01GABA_CMD_WRITE_TO_BUFFER_SECOND);

  // Write the buffer load
  SEND_NOR_COMMAND(TargetAddress, 0, MT28EW01GABA_CMD_WRITE_TO_BUFFER_THIRD);

  // Write # of locations to program
  SEND_NOR_COMMAND(TargetAddress, 0, (NumWords - 1));

  // Load Data into Buffer
  while(current_offset <= end_offset)
  {
    // Write Data
    FLASH_WRITE_DATA(CREATE_NOR_ADDRESS(Instance->DeviceBaseAddress,CREATE_BYTE_OFFSET(current_offset++)), *Buffer++);
  }

  // Issue the Buffered Program Confirm command
  SEND_NOR_COMMAND (TargetAddress, 0,
		    MT28EW01GABA_CMD_WRITE_TO_BUFFER_CONFIRM);
  
  LastWrittenAddress = CREATE_NOR_ADDRESS(Instance->DeviceBaseAddress,CREATE_BYTE_OFFSET(end_offset));
 
  while (((Read1 = FLASH_READ_DATA(LastWrittenAddress)) != (Read2 = FLASH_READ_DATA(LastWrittenAddress))) && (--Timeout));

  if(!Timeout)
  {
    DEBUG((EFI_D_ERROR, "%a: ERROR - Failed to "
        "Write @ LastWrittenAddress 0x%p\n",__FUNCTION__,\
                                LastWrittenAddress));
    return EFI_DEVICE_ERROR;
  }
  else
    return EFI_SUCCESS;
}

EFI_STATUS NorFlashPlatformWriteWordAlignedAddressBuffer
(
  IN NOR_FLASH_INSTANCE     *Instance,
  IN UINTN                Offset,
  IN UINTN                NumWords,
  IN FLASH_DATA           *Buffer
  )
{
  UINTN MultiByteWordCount = mNorFlashDevices[Instance->Media.MediaId].MultiByteWordCount;
  UINTN mask = MultiByteWordCount - 1;
  UINTN IntWords = NumWords;
  EFI_STATUS Status = EFI_SUCCESS;

  if (Offset & mask)
  {
    // program only as much as necessary, so pick the lower of the two numbers
    if (NumWords < (MultiByteWordCount - (Offset & mask)) )
      IntWords = NumWords;
    else
      IntWords = MultiByteWordCount - (Offset & mask);

    // program the first few to get write buffer aligned
    Status = NorFlashPlatformWritePageBuffer(Instance, Offset, IntWords, Buffer);
    if (EFI_ERROR(Status))
      return EFI_DEVICE_ERROR;

    Offset   += IntWords; // adjust pointers and counter
    NumWords -= IntWords;
    Buffer += IntWords;
    
    if (NumWords == 0)
      return(Status);
  }

  while(NumWords >= MultiByteWordCount) /* while big chunks to do */
  {
    Status = NorFlashPlatformWritePageBuffer(Instance, Offset, MultiByteWordCount, Buffer);
    if (EFI_ERROR(Status))
      return EFI_DEVICE_ERROR;

    Offset   += MultiByteWordCount; /* adjust pointers and counter */
    NumWords -= MultiByteWordCount;
    Buffer += MultiByteWordCount;
  }
  
  if (NumWords == 0)
    return(Status);

  Status = NorFlashPlatformWritePageBuffer(Instance, Offset, NumWords, Buffer);
  
  return(Status);
}
/*
 * Writes data to the NOR Flash using the Buffered Programming method.
 *
 * Write Buffer Programming allows the system to write a maximum of 32 bytes in one
 * programming operation. Therefore this function will only handle buffers up to 32
 * bytes. To deal with larger buffers, call this function again.
 *
 */
EFI_STATUS
NorFlashPlatformWriteBuffer (
  IN        NOR_FLASH_INSTANCE     *Instance,
  IN        EFI_LBA                Lba,
  IN        UINTN                  Offset,
  IN OUT    UINTN                  *NumBytes,
  IN        UINT8                  *Buffer
  )
{
  EFI_STATUS        Status;
  FLASH_DATA        *pSrcBuffer = (FLASH_DATA *)Buffer;
  UINTN             TargetOffsetinBytes = 0;
  UINTN             WordsToWrite = 0;
  UINTN             mask = sizeof(FLASH_DATA) - 1;
  UINTN             BufferSizeInBytes = *NumBytes;
  UINTN             IntBytes = BufferSizeInBytes;// Intermediate Bytes needed to copy for alignment
  UINTN             Counter = 0;
  UINT8             *CopyFrom, *CopyTo;
  FLASH_DATA        TempWrite = 0;
  UINTN             BlockSize = Instance->Media.BlockSize;

  DEBUG ((DEBUG_BLKIO, "%a(Parameters: Lba=%ld, Offset=0x%x, *NumBytes=0x%x, Buffer @ 0x%08x)\n",\
                        __FUNCTION__, Lba, Offset, *NumBytes, Buffer));

  // Detect WriteDisabled state
  if (Instance->Media.ReadOnly == TRUE) {
    DEBUG ((EFI_D_ERROR, "%a: ERROR - Can not write: Device is in WriteDisabled state.\n",__FUNCTION__));
    // It is in WriteDisabled state, return an error right away
    return EFI_ACCESS_DENIED;
  }
  
  // The buffer must be valid
  if (Buffer == NULL)
    return EFI_INVALID_PARAMETER;
  
  // We must have some bytes to write
  if (*NumBytes == 0) {
    DEBUG ((EFI_D_ERROR, "%a: ERROR - EFI_BAD_BUFFER_SIZE: (Offset=0x%x + NumBytes=0x%x) > BlockSize=0x%x\n",\
                            __FUNCTION__, Offset, *NumBytes, BlockSize ));
    return EFI_BAD_BUFFER_SIZE;
  }

  TargetOffsetinBytes = GET_BLOCK_OFFSET(Lba) + (UINTN)(Offset);
 
  if (((Lba * BlockSize) + *NumBytes) > Instance->Size) {
    DEBUG ((EFI_D_ERROR, "%a: ERROR - Write will exceed device size.\n",__FUNCTION__));
    return EFI_INVALID_PARAMETER;
  }

  if(TargetOffsetinBytes & mask)
  {
    // Write only as much as necessary, so pick the lower of the two numbers and call it Intermediate bytes to write to make alignment proper
    if (BufferSizeInBytes < (sizeof(FLASH_DATA) - (TargetOffsetinBytes & mask)) )
      IntBytes = BufferSizeInBytes;
    else
      IntBytes = sizeof(FLASH_DATA) - (TargetOffsetinBytes & mask);
    // Read the first few to get Read buffer aligned
    NorFlashPlatformRead(Instance, Lba, (TargetOffsetinBytes & ~mask)-GET_BLOCK_OFFSET(Lba), sizeof(TempWrite), (UINT8*)&TempWrite);

    CopyTo = (UINT8*)&TempWrite;
    CopyTo += (TargetOffsetinBytes & mask);
    CopyFrom = (UINT8*)Buffer;
    
    for(Counter=IntBytes; (0 != Counter); Counter--)
        *(CopyTo++) = *(CopyFrom++);
    
    Status = NorFlashPlatformWriteWordAlignedAddressBuffer (Instance,\
                                (UINTN)((TargetOffsetinBytes & ~mask)/sizeof(FLASH_DATA)),\
                                1,\
                                &TempWrite);
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "%a: ERROR - Failed to "
        "Write @ TargetOffset 0x%x Status=%d\n",__FUNCTION__,TargetOffsetinBytes,Status));
      return EFI_DEVICE_ERROR;
    }
    
    TargetOffsetinBytes += IntBytes; /* adjust pointers and counter */
    BufferSizeInBytes -= IntBytes;
    Buffer += IntBytes;
    
    if (BufferSizeInBytes == 0)
      return EFI_SUCCESS;
  }
  
  // Write the bytes to CFI width aligned address. Note we can Write number of bytes=CFI width in one operation
  WordsToWrite = BufferSizeInBytes/sizeof(FLASH_DATA);
  pSrcBuffer = (FLASH_DATA*)Buffer;

  Status = NorFlashPlatformWriteWordAlignedAddressBuffer (Instance,\
                                                        (UINTN)(TargetOffsetinBytes/sizeof(FLASH_DATA)),\
                                                        WordsToWrite,\
                                                        pSrcBuffer);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "%a: ERROR - Failed to "
            "Write @ TargetOffset 0x%x Status=%d\n",__FUNCTION__,TargetOffsetinBytes,Status));
    return EFI_DEVICE_ERROR;
  }
  
  BufferSizeInBytes -= (WordsToWrite*sizeof(FLASH_DATA));
  Buffer += (WordsToWrite*sizeof(FLASH_DATA));
  TargetOffsetinBytes += (WordsToWrite*sizeof(FLASH_DATA));
  
  if (BufferSizeInBytes == 0)
    return EFI_SUCCESS;
  
  // Now Write bytes that are remaining and are less than CFI width.
  // Read the first few to get Read buffer aligned
  NorFlashPlatformRead(Instance, Lba, TargetOffsetinBytes-GET_BLOCK_OFFSET(Lba), sizeof(TempWrite), (UINT8*)&TempWrite);
  
  CopyFrom = (UINT8*)Buffer;
  CopyTo = (UINT8*)&TempWrite;
  
  while(BufferSizeInBytes--)
    *(CopyTo++) = *(CopyFrom++);
  
  Status = NorFlashPlatformWriteWordAlignedAddressBuffer (Instance,\
                            (UINTN)(TargetOffsetinBytes/sizeof(FLASH_DATA)),\
                            1,\
                            &TempWrite);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "%a: ERROR - Failed to "
    "Write @ TargetOffset 0x%x Status=%d\n",__FUNCTION__,TargetOffsetinBytes,Status));
    return EFI_DEVICE_ERROR;
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
NorFlashPlatformRead (
  IN  NOR_FLASH_INSTANCE  *Instance,
  IN  EFI_LBA             Lba,
  IN  UINTN               Offset,
  IN  UINTN               BufferSizeInBytes,
  OUT UINT8               *Buffer
  )
{
  UINTN               IntBytes = BufferSizeInBytes;// Intermediate Bytes needed to copy for alignment
  UINTN               mask = sizeof(FLASH_DATA) - 1;
  FLASH_DATA          TempRead = 0;
  UINT8               *CopyFrom, *CopyTo;
  UINTN               TargetOffsetinBytes = (UINTN)(GET_BLOCK_OFFSET(Lba)+Offset);
  FLASH_DATA          *pReadData;
  UINTN               BlockSize = Instance->Media.BlockSize;
  UINTN               Counter = 0;
  
  DEBUG ((DEBUG_BLKIO, "%a(Parameters: Lba=%ld, Offset=0x%x, BufferSizeInBytes=0x%x, Buffer @ 0x%p)\n",\
                        __FUNCTION__, Lba, Offset, BufferSizeInBytes, Buffer));

  // The buffer must be valid
  if (Buffer == NULL)
    return EFI_INVALID_PARAMETER;

  // Return if we have not any byte to read
  if (BufferSizeInBytes == 0)
    return EFI_SUCCESS;

  if (((Lba * BlockSize) + BufferSizeInBytes) > Instance->Size) {
    DEBUG ((EFI_D_ERROR, "%a: ERROR - Read will exceed device size.\n",__FUNCTION__));
    return EFI_INVALID_PARAMETER;
  }

  // Put device back into Read Array mode (via Reset)
  NorFlashPlatformReset(Instance);
  
  // First Read bytes to make buffer aligned to CFI width
  if(TargetOffsetinBytes & mask)
  {
    // Read only as much as necessary, so pick the lower of the two numbers
    if (BufferSizeInBytes < (sizeof(FLASH_DATA) - (TargetOffsetinBytes & mask)) )
      IntBytes = BufferSizeInBytes;
    else
      IntBytes = sizeof(FLASH_DATA) - (TargetOffsetinBytes & mask);
    // Read the first few to get Read buffer aligned
    TempRead = FLASH_READ_DATA(CREATE_NOR_ADDRESS(Instance->DeviceBaseAddress,\
                                                CREATE_BYTE_OFFSET((TargetOffsetinBytes & ~mask)/sizeof(FLASH_DATA))));

    CopyFrom = (UINT8*)&TempRead;
    CopyFrom += (TargetOffsetinBytes & mask);
    CopyTo = (UINT8*)Buffer;
    
    for(Counter=IntBytes; (0 != Counter); Counter--)
        *(CopyTo++) = *(CopyFrom++);
    
    TargetOffsetinBytes += IntBytes; // adjust pointers and counter
    BufferSizeInBytes -= IntBytes;
    Buffer += IntBytes;
    if (BufferSizeInBytes == 0)
      return EFI_SUCCESS;
  }

  pReadData = (FLASH_DATA*)Buffer;

  // Readout the bytes from CFI width aligned address. Note we can read number of bytes=CFI width in one operation
  while(BufferSizeInBytes >= sizeof(FLASH_DATA)) {
    *pReadData = FLASH_READ_DATA(CREATE_NOR_ADDRESS(Instance->DeviceBaseAddress,\
                                                 CREATE_BYTE_OFFSET(TargetOffsetinBytes/sizeof(FLASH_DATA))));
    pReadData += 1;
    BufferSizeInBytes -= sizeof(FLASH_DATA);
    TargetOffsetinBytes += sizeof(FLASH_DATA);
  }
  if (BufferSizeInBytes == 0)
    return EFI_SUCCESS;

  // Now read bytes that are remaining and are less than CFI width.
  CopyTo = (UINT8*)pReadData;
  // Read the first few to get Read buffer aligned
  TempRead = FLASH_READ_DATA(CREATE_NOR_ADDRESS(Instance->DeviceBaseAddress,
                                              CREATE_BYTE_OFFSET(TargetOffsetinBytes/sizeof(FLASH_DATA))));
  CopyFrom = (UINT8*)&TempRead;
  
  while(BufferSizeInBytes--)
    *(CopyTo++) = *(CopyFrom++);
  
  return EFI_SUCCESS;
}

EFI_STATUS
NorFlashPlatformReset (
  IN  NOR_FLASH_INSTANCE *Instance
  )
{
  SEND_NOR_COMMAND (Instance->DeviceBaseAddress, 0, MT28EW01GABA_CMD_RESET);
  return EFI_SUCCESS;
}

EFI_STATUS
NorFlashPlatformGetDevices (
  OUT NOR_FLASH_DESCRIPTION   **NorFlashDevices,
  OUT UINT32                  *Count
  )
{
  if ((NorFlashDevices == NULL) || (Count == NULL))
    return EFI_INVALID_PARAMETER;

  // Get the number of NOR flash devices supported
  *NorFlashDevices = mNorFlashDevices;
  *Count = NOR_FLASH_DEVICE_COUNT;

  return EFI_SUCCESS;
}

EFI_STATUS
NorFlashPlatformFlashGetAttributes (
  OUT NOR_FLASH_DESCRIPTION  **NorFlashDevices,
  IN UINT32                  Count
  )
{
  EFI_STATUS              Status;
  UINT32                  Index;
  
  if ((NorFlashDevices == NULL) || (Count == 0))
    return EFI_INVALID_PARAMETER;

  // Check the attributes of the NOR flash slave we are connected to.
  // Currently we support only CFI flash devices. Bail-out otherwise.
  Status = CfiNorFlashFlashGetAttributes (NorFlashDevices, Count);
  // Limit the Size of Nor Flash that can be programmed
  for (Index = 0; Index < Count; Index++) {
    NorFlashDevices[Index]->RegionBaseAddress = PcdGet64(PcdFlashReservedRegionBase64);
    // Since we are in Secondary Bank the flash size available is half of total flash size
    NorFlashDevices[Index]->Size >>= 1;
    NorFlashDevices[Index]->Size -= (NorFlashDevices[Index]->RegionBaseAddress - NorFlashDevices[Index]->DeviceBaseAddress);
    if((NorFlashDevices[Index]->RegionBaseAddress - NorFlashDevices[Index]->DeviceBaseAddress) % NorFlashDevices[Index]->BlockSize) {
      DEBUG ((EFI_D_ERROR, "%a ERROR: Reserved Region(0x%p) doesn't start from block boundry(0x%08x)\n", __FUNCTION__,\
				(UINTN)NorFlashDevices[Index]->RegionBaseAddress, (UINT32)NorFlashDevices[Index]->BlockSize));
      return EFI_DEVICE_ERROR;
    }
  }
  return Status;
}

EFI_STATUS
NorFlashPlatformControllerInitialization (
  VOID
  )
{
  // IFC NOR is enabled by default on reset and available on CS0, but
  // still re-init the IFC NOR settings
  // nothing here

  IfcNorInit ();
  return EFI_SUCCESS;
}
