/** @LS1043aPrePiNor.c
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
		UINTN UefiNorBase,
		UINTN	UefiMemorySize
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

	DramInit();

	CopyImage((VOID*)UefiMemoryBase, (VOID*)UefiNorBase, UefiMemorySize);
	
	PrePiStart = (VOID (*)())((UINT64)PcdGet64(PcdFvBaseAddress));
  PrePiStart();
}
