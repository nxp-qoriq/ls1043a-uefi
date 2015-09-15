/** @LS1043aPrePiNand.c
#
#  Driver for installing BlockIo protocol over IFC NAND
#
#  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
**/

#include <Library/ArmLib.h>
#include <Library/PcdLib.h>
#include <Library/FslIfc.h>
#include <Library/IfcNand.h>
#include <Library/IoLib.h>
#include <Library/SerialPortLib.h>
#include <LS1043aRdb.h>

EFI_STATUS UefiFdNandToDdr(
	UINTN	Lba,
	UINTN FdSize,
	UINTN	DdrDestAddress
)
{
	EFI_STATUS Status;
	
	//Init Nand Flash
	IfcNandInit();

	Status = IfcNandFlashInit(NULL);
	if (Status!=EFI_SUCCESS)
		return Status;

//Copy from NAnd to DDR
	return IfcNandFlashReadBlocks(NULL, 0, Lba, FdSize, (VOID*)DdrDestAddress);  	
}

VOID
LoadImageToDdr(
  IN   UINTN  UefiMemoryBase
  )
{ 
  //ARM_MEMORY_REGION_DESCRIPTOR  MemoryTable[5];
  VOID	(*PrePiStart)(VOID);

  DEBUG((EFI_D_RELEASE, "Loading secondary firmware from NAND.....\n"));
  UefiFdNandToDdr(FixedPcdGet32(PcdFdNandLba), FixedPcdGet32(PcdFdSize), UefiMemoryBase);

  DEBUG((EFI_D_RELEASE, "Starting secondary boot firmware....\n"));
  PrePiStart = (VOID (*)())UefiMemoryBase;
  PrePiStart();

  ASSERT(0);
}
