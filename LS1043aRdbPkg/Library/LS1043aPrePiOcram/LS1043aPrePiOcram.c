#include <Library/ArmLib.h>
#include <Library/PcdLib.h>
#include <Library/FslIfc.h>
#include <Library/FslIfcNand.h>
#include <Library/IoLib.h>
#include <Library/SerialPortLib.h>
#include <CpldLib.h>
#include <LS1043aRdb.h>
#include <LS1043aSocLib.h>
#include <Ddr.h>

UINTN mGlobalVariableBase = 0;

EFI_STATUS UefiFdNandToDdr(
	UINTN	Lba,
	UINTN FdSize,
	UINTN	DdrDestAddress
)
{
	EFI_STATUS Status;
	
	//Init Nand Flash
	FslIfcNandInit();

	Status = FslIfcNandFlashInit(NULL);
	if (Status!=EFI_SUCCESS)
		return Status;

//Copy from NAnd to DDR
	return FslIfcNandFlashReadBlocks(NULL, 0, Lba, FdSize, (VOID*)DdrDestAddress);  	
}

VOID CEntryPoint(
		UINTN	UefiMemoryBase
		)
{ 
	//ARM_MEMORY_REGION_DESCRIPTOR  MemoryTable[5];
	VOID	(*PrePiStart)(VOID);

	// Data Cache enabled on Primary core when MMU is enabled.
  ArmDisableDataCache ();
  // Invalidate Data cache
  ArmInvalidateDataCache ();
  // Invalidate instruction cache
  ArmInvalidateInstructionCache ();
  // Enable Instruction Caches on all cores.
  ArmEnableInstructionCache ();

	DramInit();

	SerialPortInitialize();

	DEBUG((EFI_D_RELEASE, "\nUEFI primary boot firmware (built at %a on %a)\n", __TIME__, __DATE__));
	
	if(PcdGet32(PcdBootMode) == NAND_BOOT) {
		DEBUG((EFI_D_INFO, "Loading secondary firmware from NAND.....\n"));
		UefiFdNandToDdr(FixedPcdGet32(PcdFdNandLba), FixedPcdGet32(PcdFdSize), UefiMemoryBase);
	} else {
		DEBUG((EFI_D_ERROR, "Unsupported boot mode\n"));
		ASSERT(0);
	}

	DEBUG((EFI_D_INFO, "Starting secondary boot firmware....\n"));
	PrePiStart = (VOID (*)())UefiMemoryBase;
  PrePiStart();

	ASSERT(0);
}
