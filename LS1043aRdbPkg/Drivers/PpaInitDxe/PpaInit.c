/** @file
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

#include <Chipset/AArch64.h>
#include <Library/ArmPlatformLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/BdsLinuxFit.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib/MemLibInternals.h>
#include <Library/FslIfc.h>
#include <Library/IfcNand.h>

extern EFI_STATUS PpaInit(UINT64);
extern VOID InitMmu(ARM_MEMORY_REGION_DESCRIPTOR*);


EFI_STATUS GetPpaFromNand(
		UINT32								PpaLba, 
		EFI_PHYSICAL_ADDRESS *FitImage)
{
	EFI_STATUS Status;
	UINTN PpaMaxSize = 128 * 1024;
	Status = IfcNandFlashInit(NULL);
	if (Status!=EFI_SUCCESS)
		return Status;

	*FitImage = (EFI_PHYSICAL_ADDRESS)AllocateRuntimePages(EFI_SIZE_TO_PAGES(PpaMaxSize));
	if(*FitImage == 0x0)
		return EFI_OUT_OF_RESOURCES;

//Copy from Nand to DDR
	return IfcNandFlashReadBlocks(NULL, 0, PpaLba, PpaMaxSize, (VOID*)*FitImage);  	
}


/**
 * Copying PPA firmware to DDR
 */
VOID
CopyPpaImage (
  const char *title,
  UINTN image_addr,
  UINTN image_size,
  UINTN PpaRamAddr)
{
	DEBUG((EFI_D_INFO, "%a copied to address 0x%x\n", title, PpaRamAddr));
  	InternalMemCopyMem((void *)PpaRamAddr, (void *)image_addr, image_size);
}

UINTN
GetPpaImagefromFlash (
  VOID
  )
{
	EFI_STATUS Status;
	EFI_PHYSICAL_ADDRESS FitImage;
	EFI_PHYSICAL_ADDRESS PpaImage;
	INT32 CfgNodeOffset;
	INT32 NodeOffset;
	INT32 PpaImageSize;
	UINTN PpaRamAddr;

	// Assuming that the PPA FW is present on NOR flash
	// FIXME: Add support for other flash devices.
	if(PcdGet32(PcdBootMode) == NAND_BOOT) {
		Status = GetPpaFromNand(PcdGet32(PcdPpaNandLba), &FitImage);
		ASSERT(Status == EFI_SUCCESS);
	}
	else
		FitImage = PcdGet64 (PcdPpaNorBaseAddr);
	
	// PPA will be placed on DDR at this address:
	// Top of DDR - PcdPpaDdrOffsetAddr
	PpaRamAddr = PcdGet64 (PcdSystemMemoryBase) + PcdGet64 (PcdSystemMemorySize)
			- PcdGet64 (PcdPpaDdrOffsetAddr);

	Status = FitCheckHeader(FitImage);
	if (EFI_ERROR (Status)) {
		DEBUG((EFI_D_ERROR, "Bad FIT image header (0x%x).\n", Status));
		goto EXIT_FREE_FIT;
	}

	Status = FitGetConfNode(FitImage, (void *)(PcdGetPtr(PcdPpaFitConfiguration/*PcdPpaFitConfiguration*/)), &CfgNodeOffset);
	if (EFI_ERROR (Status)) {
		DEBUG((EFI_D_ERROR, "Did not find configuration node in FIT header (0x%x).\n", Status));
		goto EXIT_FREE_FIT;
	}

	Status = FitGetNodeFromConf(FitImage, CfgNodeOffset, FIT_FIRMWARE_IMAGE, &NodeOffset);
	if (EFI_ERROR (Status)) {
		DEBUG((EFI_D_ERROR, "Did not find PPA node in FIT header (0x%x).\n", Status));
		goto EXIT_FREE_FIT;
	}

	Status = FitGetNodeData(FitImage, NodeOffset, (VOID*)&PpaImage, &PpaImageSize);
	if (EFI_ERROR (Status)) {
		DEBUG((EFI_D_ERROR, "Did not find PPA f/w in FIT image (0x%x).\n", Status));
		goto EXIT_FREE_FIT;
	}

	CopyPpaImage ("PPA Firmware", PpaImage, PpaImageSize, PpaRamAddr);

	return PpaRamAddr;

EXIT_FREE_FIT:
	// Flow should never reach here
	ASSERT (Status == EFI_SUCCESS);
	
	return 0;
}

EFI_STATUS
PpaInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
	EFI_STATUS Status;
	UINTN PpaRamAddr;
  ARM_MEMORY_REGION_DESCRIPTOR *MemoryTable;

	PpaRamAddr = GetPpaImagefromFlash();

	Status = PpaInit(PpaRamAddr);
  ArmPlatformGetVirtualMemoryMap (&MemoryTable);
	InitMmu(MemoryTable);
	return Status;
}
