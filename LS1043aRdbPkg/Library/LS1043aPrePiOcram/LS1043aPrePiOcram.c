#include <Library/ArmLib.h>
#include <Library/PcdLib.h>
#include <Library/FslIfcNand.h>
#include <LS1043aRdb.h>
#include <Ddr.h>

UINTN mGlobalVariableBase = 0;

VOID InitDdr(VOID)
{

}

EFI_STATUS UefiFdNandToDdr(
	UINTN	Lba,
	UINTN FdSize,
	UINTN	DdrDestAddress
)
{
	EFI_STATUS Status;
	//Init Nand Flash
	Status = FslIfcNandFlashInit(NULL);
	if (Status!=EFI_SUCCESS)
		return Status;

	//Test code: Copy from DDR to NAND
	
	Status = FslIfcNandFlashWriteBlocks(NULL, 0, Lba, FdSize, (VOID*)(DdrDestAddress + FdSize));
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
/*
	MemoryTable[0].PhysicalBase = PcdGet64(PcdSystemMemoryBase);
	MemoryTable[0].VirtualBase	= PcdGet64(PcdSystemMemoryBase);
	MemoryTable[0].Length       = PcdGet64(PcdSystemMemorySize);
	MemoryTable[0].Attributes		= ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

	MemoryTable[1].PhysicalBase =	ROMCP_BASE1_ADDR;
  MemoryTable[1].VirtualBase	= ROMCP_BASE1_ADDR;
  MemoryTable[1].Length       = ROMCP_SIZE1 + ROMCP_SIZE2;
	MemoryTable[1].Attributes		= ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

	MemoryTable[2].PhysicalBase =	CCSR_REG_ADDR;
	MemoryTable[2].VirtualBase	=	CCSR_REG_ADDR;
	MemoryTable[2].Length       =	CCSR_REG_SIZE;
	MemoryTable[2].Attributes		=	ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

	MemoryTable[3].PhysicalBase = OCRAM1_BASE_ADDR;
	MemoryTable[3].VirtualBase	= OCRAM1_BASE_ADDR;
	MemoryTable[3].Length       = OCRAM1_SIZE + OCRAM2_SIZE;
  MemoryTable[3].Attributes		= ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

	MemoryTable[4].PhysicalBase = IFC_MEM1_BASE_ADDR;
	MemoryTable[4].VirtualBase	= IFC_MEM1_BASE_ADDR;
	MemoryTable[4].Length       = IFC_MEM1_SIZE;
  MemoryTable[4].Attributes		= ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;
*/
	//ArmConfigureMmu(MemoryTable, NULL, NULL); 

	DramInit();
	
	UefiFdNandToDdr(FixedPcdGet32(PcdFdNandLba), FixedPcdGet32(PcdFdSize), UefiMemoryBase);

	//ASSERT(Status==EFI_SUCCESS);

	PrePiStart = (VOID (*)())UefiMemoryBase;
  PrePiStart();

	//ASSERT(0);
}
