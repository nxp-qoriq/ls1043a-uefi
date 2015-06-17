#include <Library/ArmLib.h>
#include <Library/PcdLib.h>
#include <LS1043aRdb.h>
#include <Ddr.h>

UINTN mGlobalVariableBase = 0;

VOID CopyImage(UINT8* Dest, UINT8* Src, UINTN Size)
{
	UINTN Count;
	for(Count = 0; Count < Size; Count++) {
		Dest[Count] = Src[Count];
	}
}

VOID CEntryPoint(
		UINTN	UefiMemoryBase,
		UINTN	UefiNorBase,
		UINTN	UefiMemorySize,
		UINTN	DramInitAddr
		)
{ 
	VOID	(*PrePiStart)(VOID);

	// Data Cache enabled on Primary core when MMU is enabled.
  ArmDisableDataCache ();
  // Invalidate Data cache
  //ArmInvalidateDataCache ();
  // Invalidate instruction cache
  ArmInvalidateInstructionCache ();
  // Enable Instruction Caches on all cores.
  ArmEnableInstructionCache ();

	((VOID (*)(UINTN))DramInitAddr)(UefiMemoryBase - UefiNorBase);

	CopyImage((VOID*)UefiMemoryBase, (VOID*)UefiNorBase, UefiMemorySize);
	
	PrePiStart = (VOID (*)())((UINT64)PcdGet32(PcdFvBaseAddress));
  PrePiStart();
}
