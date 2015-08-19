/** @NorFlashLib.c

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
#include <Library/NorFlashPlatformLib.h>
#include <Library/NorFlash.h>
#include <Library/FslIfc.h>

#include "CfiNorFlashLib.h"
#include "Mt28ew01gabaCfiNorFlashLib.h"

#define NOR_FLASH_DEVICE_COUNT	1

NOR_FLASH_DESCRIPTION mNorFlashDevices[NOR_FLASH_DEVICE_COUNT] = {
  // FIXME: Add RCW, PBI, FMAN images here when available
  { // UEFI
    IFC_NOR_RESERVED_REGION_BASE, //DeviceBaseAddress
    IFC_NOR_RESERVED_REGION_BASE, //RegionBaseAddress
    SIZE_64KB * 128,  //Size
    SIZE_64KB,	      //BlockSize
    { 0x1F15DA3C, 0x37FF, 0x4070, { 0xB4, 0x71, 0xBB, 0x4A, 0xF1, 0x2A, 0x72, 0x4A } }
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
  UINT16		EraseStatus1 = 0;
  UINT16		EraseStatus2 = 0;
  UINTN           	Count;

  // Request a sector erase by writing two unlock cycles, followed by a
  // setup command and two additional unlock cycles
  
  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_SECTOR_ERASE_FIRST);
  // Wait for write to complete
  MicroSecondDelay(1000);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR, MT28EW01GABA_CMD_SECTOR_ERASE_SECOND);
  // Wait for write to complete
  MicroSecondDelay(1000);

  // Issue a setup command
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_SECTOR_ERASE_THIRD);
  // Wait for write to complete
  MicroSecondDelay(1000);

  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_SECTOR_ERASE_FOURTH);
  // Wait for write to complete
  MicroSecondDelay(1000);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR, MT28EW01GABA_CMD_SECTOR_ERASE_FIFTH);
  // Wait for write to complete
  MicroSecondDelay(1000);

  // Now send the address of the sector to be erased
  SEND_NOR_COMMAND(SectorAddress, 0, MT28EW01GABA_CMD_SECTOR_ERASE_SIXTH);
  // Wait for write to complete
  MicroSecondDelay(1000);
  
  // Wait for erase to complete
  // Read status register to determine ERASE DONE
  while ((EraseStatus1 = FLASH_READ_16(SectorAddress)) != (EraseStatus2 = FLASH_READ_16(SectorAddress)));
  
  // Wait for erase to complete
  for(Count = 0; Count < 2048000; Count++) {
  	// Check if we have really erased the flash
  	EraseStatus1 = FLASH_READ_16(SectorAddress);
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
  UINT16		EraseStatus1 = 0;
  UINT16		EraseStatus2 = 0;

  // Request a sector erase by writing two unlock cycles, followed by a
  // setup command and two additional unlock cycles, which are then
  // followed by chip erase commands
  
  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_FIRST);
  // Wait for erase to complete
  MicroSecondDelay(100);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_SECOND);
  // Wait for erase to complete
  MicroSecondDelay(100);

  // Issue the setup cmd
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_THIRD);
  // Wait for erase to complete
  MicroSecondDelay(100);

  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_FOURTH);
  // Wait for erase to complete
  MicroSecondDelay(100);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_FIFTH);
  // Wait for erase to complete
  MicroSecondDelay(100);

  // Issue the chip erase cmd
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_CHIP_ERASE_SIXTH);
  // Wait for erase to complete
  MicroSecondDelay(100);

  // Wait for erase to complete
  // Read status register to determine ERASE DONE
  while ((EraseStatus1 = FLASH_READ_16(Instance->DeviceBaseAddress)) != (EraseStatus2 = FLASH_READ_16(Instance->DeviceBaseAddress)));
  
  // Wait for erase to complete
  MicroSecondDelay(100);
  
  return EFI_SUCCESS;
}

EFI_STATUS
NorFlashPlatformWriteSingleWord (
  IN NOR_FLASH_INSTANCE     *Instance,
  IN UINTN                  WordAddress,
  IN UINT8                  WriteData
  )
{
  UINT16		WriteStatus1 = 0;
  UINT16		WriteStatus2 = 0;
  
  // Request a write program command sequence
  
  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_PROGRAM_FIRST);
  // Wait for erase to complete
  MicroSecondDelay(100);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR, MT28EW01GABA_CMD_PROGRAM_SECOND);
  // Wait for erase to complete
  MicroSecondDelay(100);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR, MT28EW01GABA_CMD_PROGRAM_THIRD);
  // Wait for erase to complete
  MicroSecondDelay(100);

  // Store the word into NOR Flash;
  FLASH_WRITE_32 (WordAddress, WriteData);

  // Wait for write to complete
  // Read status register to determine WRITE DONE
  while ((WriteStatus1 = FLASH_READ_16(Instance->DeviceBaseAddress)) != (WriteStatus2 = FLASH_READ_16(Instance->DeviceBaseAddress)));
  
  // Wait for write to complete
  MicroSecondDelay(100);
  
  return EFI_SUCCESS;
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
  IN NOR_FLASH_INSTANCE     *Instance,
  IN UINTN                  TargetAddress,
  IN UINTN                  BufferSizeInBytes,
  IN UINT16                 *pSrcBuffer
  )
{
  UINT16		WriteStatus1 = 0;
  UINT16		WriteStatus2 = 0;
  UINTN			Count;
  volatile UINT16	*ProgramAddress;
  UINT16		*BufferU16;
  UINTN			BufferSizeInWords;
  UINTN			DestAddress = TargetAddress;
  INTN			EraseCheckFlag = 1;
  UINTN			U16Cnt;
  UINT16		*pSrcBufferCpy = pSrcBuffer;

  // Check there are some data to program
  if (BufferSizeInBytes == 0) {
    return EFI_BUFFER_TOO_SMALL;
  }

  // Ensure that requested buffer size does not exceed the maximum supported buffer size
  if (BufferSizeInBytes > MT28EW01GABA_BUFFER_SIZE_IN_BYTES) {
    return EFI_BAD_BUFFER_SIZE;
  }

  // Check that the buffer size is a multiple of 32-bit words
  if ((BufferSizeInBytes % 4) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }
  
  BufferSizeInWords = (BufferSizeInBytes / 4);
  U16Cnt = BufferSizeInWords;

  DEBUG((DEBUG_BLKIO, "NorFlashPlatformWriteBuffer: TargetAddress=0x%x, "
	"pSrcBufferCpy=0x%x, U16Cnt=0x%x\n", TargetAddress, pSrcBufferCpy,
	U16Cnt));
  
  // Wait for any previous write operation to complete
  MicroSecondDelay(1000);

  // Check if the destination block in the flash is erased or not
  while ((U16Cnt-- > 0) && (EraseCheckFlag == 1)) {
  	EraseCheckFlag = ((FLASH_READ_16((UINTN)DestAddress) &
			  FLASH_READ_16((UINTN)pSrcBufferCpy)) ==
			  FLASH_READ_16((UINTN)pSrcBufferCpy));
	pSrcBufferCpy += 1;
	DestAddress += 2;
  }
  
  if (!EraseCheckFlag) {
	DEBUG((EFI_D_ERROR, "NorFlashPlatformWriteBuffer: ERROR - Flash block "
	      "not erased properly: BlockAddr = 0x%x\n", TargetAddress));
	return EFI_DEVICE_ERROR;
  }

  // Prepare the destination program address
  ProgramAddress = (UINT16 *)TargetAddress;
  
  // Pre-programming conditions checked, now start the algorithm.

  // Issue the Unlock cmds
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_1_ADDR,
		   MT28EW01GABA_CMD_WRITE_TO_BUFFER_FIRST);
  // Wait for write to complete
  MicroSecondDelay(100);
  
  SEND_NOR_COMMAND(Instance->DeviceBaseAddress, MT28EW01GABA_CMD_UNLOCK_2_ADDR,
		   MT28EW01GABA_CMD_WRITE_TO_BUFFER_SECOND);
  // Wait for write to complete
  MicroSecondDelay(100);

  // Write the buffer load
  SEND_NOR_COMMAND(TargetAddress, 0, MT28EW01GABA_CMD_WRITE_TO_BUFFER_THIRD);
  // Wait for write to complete
  MicroSecondDelay(100);

  // Write the word count, which is (BufferSizeInWords - 1),
  // because word count 0 means one word.
  SEND_NOR_COMMAND(TargetAddress, 0, (BufferSizeInWords - 1));
  // Wait for write to complete
  MicroSecondDelay(100);

  pSrcBufferCpy = pSrcBuffer;
  BufferU16 = (UINT16 *)pSrcBufferCpy;
  U16Cnt = BufferSizeInWords;

  // Write the data to the NOR Flash, advancing each address by 1 word
  for(Count = 0; Count < U16Cnt; Count++, ProgramAddress++, BufferU16++) {
    FLASH_WRITE_16 ((UINTN)ProgramAddress, FLASH_READ_16((UINTN)BufferU16));
    // Wait for write to complete
    MicroSecondDelay(100);
  }

  // Issue the Buffered Program Confirm command
  SEND_NOR_COMMAND ((UINTN)TargetAddress, 0,
		    MT28EW01GABA_CMD_WRITE_TO_BUFFER_CONFIRM);

  // Wait for write to complete
  MicroSecondDelay(100);
  
  // Wait for write to complete
  // Read status register to determine WRITE DONE
  while ((WriteStatus1 = FLASH_READ_16(Instance->DeviceBaseAddress)) !=
	 (WriteStatus2 = FLASH_READ_16(Instance->DeviceBaseAddress)));
 
  // Wait for write to complete
  MicroSecondDelay(100);

  return EFI_SUCCESS;
}

EFI_STATUS
NorFlashPlatformWriteFullBlock (
  IN NOR_FLASH_INSTANCE     *Instance,
  IN EFI_LBA                Lba,
  IN UINT32                 *DataBuffer,
  IN UINT32                 BlockSizeInWords
  )
{
  EFI_STATUS    Status;
  UINTN         WordAddress;
  UINT32        WordIndex;
  UINTN         BufferIndex;
  UINTN         BlockAddress;
  UINTN         BuffersInBlock;
  UINTN         RemainingWords;
  UINTN         Cnt;
  UINT16        *pSrcBuffer = (UINT16 *)DataBuffer;

  Status = EFI_SUCCESS;

  // Get the physical address of the block
  BlockAddress = GET_NOR_BLOCK_ADDRESS (Instance->RegionBaseAddress, Lba,
					BlockSizeInWords * 4);

  // Start writing from the first address at the start of the block
  WordAddress = BlockAddress;

  // Erase the Sector/Block to be written-to
  Status = NorFlashPlatformEraseSector (Instance, BlockAddress);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "NorFlashPlatformWriteFullBlock: ERROR - Failed to "
			"erase block 0x%x\n", BlockAddress));
    goto EXIT;
  }

  // To speed up the programming operation, NOR Flash is programmed using the
  // Buffered Programming method.

  // Check that the address starts at a 32-word boundary, i.e. last 7 bits must
  // be zero
  if ((WordAddress & BOUNDARY_OF_32_WORDS) == 0x00) {
    // First, break the entire block into buffer-sized chunks.
    // This product supports a 512-word (x16) maximum program buffer.
    BuffersInBlock = (BlockSizeInWords / MT28EW01GABA_MAX_BUFFER_SIZE_IN_WORDS) * 2;
  
    // Then feed each buffer chunk to the NOR Flash
    // If a buffer does not contain any data, don't write it.
    for(BufferIndex = 0; BufferIndex < BuffersInBlock; BufferIndex++) {
      // Check the source buffer to see if it contains any data (not set all 1s).
      for (Cnt = 0; Cnt < MT28EW01GABA_MAX_BUFFER_SIZE_IN_WORDS; Cnt++) {
        if (~DataBuffer[Cnt] != 0)
		continue;
	else
		break;
      }

      // Some data found, write the buffer.
      Status = NorFlashPlatformWriteBuffer (Instance, WordAddress,
		MT28EW01GABA_BUFFER_SIZE_IN_BYTES, pSrcBuffer);
      if (EFI_ERROR(Status)) {
      	goto EXIT;
      }

      WordAddress += MT28EW01GABA_MAX_BUFFER_SIZE_IN_WORDS * 2;
      pSrcBuffer += MT28EW01GABA_MAX_BUFFER_SIZE_IN_WORDS;
     }

    // Finally, finish off any remaining words that are less than the
    // maximum size of the buffer.
    RemainingWords = (UINTN)(BlockSizeInWords % MT28EW01GABA_MAX_BUFFER_SIZE_IN_WORDS);

    if(RemainingWords != 0) {
      Status = NorFlashPlatformWriteBuffer (Instance, WordAddress,
		(RemainingWords * 4), pSrcBuffer);
      if (EFI_ERROR(Status)) {
        goto EXIT;
      }
    }
  } else {
    // For now, use the single word programming algorithm
    // It is unlikely that the NOR Flash will exist in an address which falls
    // within a 32 word boundary range,
    // i.e. which ends in the range 0x......01 - 0x......7F.
    for(WordIndex = 0; WordIndex < BlockSizeInWords; WordIndex++, DataBuffer++,
	WordAddress = WordAddress + 4) {
      Status = NorFlashPlatformWriteSingleWord (Instance, WordAddress, *DataBuffer);
      if (EFI_ERROR(Status)) {
        goto EXIT;
      }
    }
  }

EXIT:
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "NorFlashPlatformWriteFullBlock - Failed at addr 0x%x, "
			"Exit Status = \"%r\".\n", WordAddress, Status));
  }

  return Status;
}

EFI_STATUS
NorFlashPlatformWriteBlocks (
  IN NOR_FLASH_INSTANCE     *Instance,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSizeInBytes,
  IN VOID                   *Buffer
  )
{
  UINT32          *pWriteBuffer;
  EFI_STATUS      Status = EFI_SUCCESS;
  EFI_LBA         CurrentBlock;
  UINT32          BlockSizeInWords;
  UINT32          NumBlocks;
  UINT32          BlockCount;

  // The buffer must be valid
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // We must have some bytes to read
  DEBUG((DEBUG_BLKIO, "NorFlashPlatformWriteBlocks: BufferSizeInBytes=0x%x\n", BufferSizeInBytes));
  if(BufferSizeInBytes == 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  // The size of the buffer must be a multiple of the block size
  DEBUG((DEBUG_BLKIO, "NorFlashPlatformWriteBlocks: BlockSize in bytes =0x%x\n", Instance->Media.BlockSize));
  if ((BufferSizeInBytes % Instance->Media.BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  // All blocks must be within the device
  NumBlocks = ((UINT32)BufferSizeInBytes) / Instance->Media.BlockSize ;

  DEBUG((DEBUG_BLKIO, "NorFlashPlatformWriteBlocks: NumBlocks=%d, LastBlock=%ld, Lba=%ld.\n", NumBlocks, Instance->Media.LastBlock, Lba));

  if ((Lba + NumBlocks) > (Instance->Media.LastBlock + 1)) {
    DEBUG((EFI_D_ERROR, "NorFlashPlatformWriteBlocks: ERROR - Write will exceed last block.\n"));
    return EFI_INVALID_PARAMETER;
  }

  BlockSizeInWords = Instance->Media.BlockSize / 4;
  
  pWriteBuffer = (UINT32 *)Buffer;

  CurrentBlock = Lba;

  for (BlockCount = 0; BlockCount < NumBlocks; BlockCount++, CurrentBlock++,
	pWriteBuffer = pWriteBuffer + BlockSizeInWords) {
    DEBUG((DEBUG_BLKIO, "NorFlashPlatformWriteBlocks: Writing block #%d\n",
	  (UINTN)CurrentBlock));
    
    Status = NorFlashPlatformWriteFullBlock (Instance, CurrentBlock,
		pWriteBuffer, BlockSizeInWords);
    if (EFI_ERROR(Status)) {
      break;
    }
  }

  DEBUG((DEBUG_BLKIO, "NorFlashPlatformWriteBlocks: Exit Status = \"%r\".\n", Status));
  return Status;
}

EFI_STATUS
NorFlashPlatformReadFullBlock (
  IN NOR_FLASH_INSTANCE   *Instance,
  IN EFI_LBA              Lba,
  IN UINT32               *DataBuffer,
  IN UINTN		  BufferSizeInU16
  )
{
  UINTN               StartAddress;
  UINTN               Count;
  UINT16 	      *pReadData = (UINT16 *)DataBuffer;
  
  // Get the address to start reading from
  StartAddress = GET_NOR_BLOCK_ADDRESS (Instance->RegionBaseAddress,
                                        Lba,
                                        Instance->Media.BlockSize
                                       );

  // Readout the data
  for (Count = 0; Count < BufferSizeInU16; Count++) {
  	*pReadData = *(volatile UINT16 *)(StartAddress);
	pReadData += 1;
	StartAddress += 2;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
NorFlashPlatformReadBlocks (
  IN NOR_FLASH_INSTANCE   *Instance,
  IN EFI_LBA              Lba,
  IN UINTN                BufferSizeInBytes,
  OUT VOID                *Buffer
  )
{
  UINT32              NumBlocks;
  UINTN               BufferSizeInU16;
  EFI_STATUS	      Status = EFI_INVALID_PARAMETER;
  EFI_LBA             CurrentBlock;
  UINTN		      BlockCount;
  UINTN               BlockSizeInWords;

  DEBUG((DEBUG_BLKIO, "NorFlashPlatformReadBlocks: BufferSize=0x%x, "
	"BlockSize=0x%x, LastBlock=%ld, Lba=%ld.\n", BufferSizeInBytes,
	Instance->Media.BlockSize, Instance->Media.LastBlock, Lba));

  // The buffer must be valid
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Return if we have not any byte to read
  if (BufferSizeInBytes == 0) {
    return EFI_SUCCESS;
  }

  // The size of the buffer must be a multiple of the block size
  if ((BufferSizeInBytes % Instance->Media.BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  // All blocks must be within the device
  NumBlocks = ((UINT32)BufferSizeInBytes) / Instance->Media.BlockSize ;

  if ((Lba + NumBlocks) > (Instance->Media.LastBlock + 1)) {
    DEBUG((EFI_D_ERROR, "NorFlashPlatformReadBlocks: ERROR - Read will exceed last block\n"));
    return EFI_INVALID_PARAMETER;
  }

  // Put device back into Read Array mode (via Reset)
  SEND_NOR_COMMAND (Instance->DeviceBaseAddress, 0, MT28EW01GABA_CMD_RESET);

  // Wait for flash to enter READ ARRAY mode
  MicroSecondDelay(100);
  
  BufferSizeInU16 = BufferSizeInBytes / 2;
  
  BlockSizeInWords = Instance->Media.BlockSize / 4;

  CurrentBlock = Lba;

  for (BlockCount = 0; BlockCount < NumBlocks; BlockCount++, CurrentBlock++,
	Buffer = Buffer + BlockSizeInWords) {
    DEBUG((DEBUG_BLKIO, "NorFlashPlatformReadBlocks: Reading block #%d\n",
	  (UINTN)CurrentBlock));
    
    Status = NorFlashPlatformReadFullBlock (Instance, CurrentBlock, Buffer,
		BufferSizeInU16);
    if (EFI_ERROR(Status)) {
      break;
    }
  }

  DEBUG((DEBUG_BLKIO, "NorFlashPlatforReadBlocks: Exit Status = \"%r\".\n", Status));
  return Status;
}

EFI_STATUS
NorFlashPlatformRead (
  IN NOR_FLASH_INSTANCE   *Instance,
  IN EFI_LBA              Lba,
  IN UINTN                Offset,
  IN UINTN                BufferSizeInBytes,
  OUT VOID                *Buffer
  )
{
  UINTN               BufferSizeInU16;
  EFI_STATUS	      Status;

  // The buffer must be valid
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Return if we have not any byte to read
  if (BufferSizeInBytes == 0) {
    return EFI_SUCCESS;
  }

  // Put the device into Read Array mode (via RESET)
  SEND_NOR_COMMAND (Instance->DeviceBaseAddress, 0, MT28EW01GABA_CMD_RESET);

  // Wait for flash to enter READ ARRAY mode
  MicroSecondDelay(100);
 
  BufferSizeInU16 = BufferSizeInBytes / 2;
  
  // Readout the data
  Status = NorFlashPlatformReadFullBlock (Instance, 0, Buffer,
		BufferSizeInU16);
  
  return Status;
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
  if ((NorFlashDevices == NULL) || (Count == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

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
  
  if ((NorFlashDevices == NULL) || (Count == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  // Check the attributes of the NOR flash slave we are connected to.
  // Currently we support only CFI flash devices. Bail-out otherwise.
  Status = CfiNorFlashFlashGetAttributes (NorFlashDevices, Count);

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

/* Block IO layer */

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.Reset
//
EFI_STATUS
EFIAPI
NorFlashPlatformBlockIoReset (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN BOOLEAN                ExtendedVerification
  )
{
  NOR_FLASH_INSTANCE *Instance;

  Instance = INSTANCE_FROM_BLKIO_THIS(This);

  DEBUG ((DEBUG_ERROR, "NorFlashBlockIoReset(MediaId=0x%x)\n", This->Media->MediaId));

  return NorFlashPlatformReset (Instance);
}

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.ReadBlocks
//
EFI_STATUS
EFIAPI
NorFlashPlatformBlockIoReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSizeInBytes,
  OUT VOID                    *Buffer
  )
{
  NOR_FLASH_INSTANCE  *Instance;
  EFI_STATUS          Status;
  EFI_BLOCK_IO_MEDIA  *Media;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = INSTANCE_FROM_BLKIO_THIS(This);
  Media = This->Media;

  DEBUG ((DEBUG_RELEASE, "NorFlashBlockIoReadBlocks(MediaId=0x%x, Lba=%ld, BufferSize=0x%x bytes (%d kB), BufferPtr @ 0x%08x)\n", MediaId, Lba, BufferSizeInBytes, Buffer));

  if (!Media) {
    Status = EFI_INVALID_PARAMETER;
  } else if (!Media->MediaPresent) {
    Status = EFI_NO_MEDIA;
  } else if (Media->MediaId != MediaId) {
    Status = EFI_MEDIA_CHANGED;
  } else if ((Media->IoAlign > 2) && (((UINTN)Buffer & (Media->IoAlign - 1)) != 0)) {
    Status = EFI_INVALID_PARAMETER;
  } else {
    Status = NorFlashPlatformReadBlocks (Instance, Lba, BufferSizeInBytes, Buffer);
  }

  return Status;
}

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.WriteBlocks
//
EFI_STATUS
EFIAPI
NorFlashPlatformBlockIoWriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSizeInBytes,
  IN  VOID                    *Buffer
  )
{
  NOR_FLASH_INSTANCE  *Instance;
  EFI_STATUS          Status;

  Instance = INSTANCE_FROM_BLKIO_THIS(This);

  DEBUG ((DEBUG_RELEASE, "NorFlashBlockIoWriteBlocks(MediaId=0x%x, Lba=%ld, BufferSize=0x%x bytes (%d kB), BufferPtr @ 0x%08x)\n", MediaId, Lba, BufferSizeInBytes, Buffer));

  if( !This->Media->MediaPresent ) {
    Status = EFI_NO_MEDIA;
  } else if( This->Media->MediaId != MediaId ) {
    Status = EFI_MEDIA_CHANGED;
  } else if( This->Media->ReadOnly ) {
    Status = EFI_WRITE_PROTECTED;
  } else {
    Status = NorFlashPlatformWriteBlocks (Instance,Lba,BufferSizeInBytes,Buffer);
  }

  return Status;
}

//
// BlockIO Protocol function EFI_BLOCK_IO_PROTOCOL.FlushBlocks
//
EFI_STATUS
EFIAPI
NorFlashPlatformBlockIoFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
{
  // No Flush required for the NOR Flash driver
  // because cache operations are not permitted.

  DEBUG ((DEBUG_ERROR, "NorFlashBlockIoFlushBlocks: Function NOT IMPLEMENTED (not required).\n"));

  // Nothing to do so just return without error
  return EFI_SUCCESS;
}
