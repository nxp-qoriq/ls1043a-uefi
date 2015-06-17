//
//  Copyright (c) 2011-2013, ARM Limited. All rights reserved.
//
//  This program and the accompanying materials
//  are licensed and made available under the terms and conditions of the BSD License
//  which accompanies this distribution.  The full text of the license may be found at
//  http://opensource.org/licenses/bsd-license.php
//
//  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
//  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
//
//

#include <AutoGen.h>
#include <AsmMacroIoLibV8.h>
#include <Library/PcdLib.h>
#inculde <Ddr.h>

.text
.align 3

GCC_ASM_EXPORT(_ModuleEntryPoint)

StartupAddr:        .8byte ASM_PFX(CEntryPoint)
DramInitAddr:				.8byte ASM_PFX(DramInit)

ASM_PFX(_ModuleEntryPoint):
  LoadConstantToReg (FixedPcdGet32(PcdOcramStackBase), x0)
	mov sp, x0
	LoadConstantToReg (FixedPcdGet64(PcdFdBaseAddress), x0)
	LoadConstantToReg (FixedPcdGet32(PcdFdNorBaseAddress), x1)
	LoadConstantToReg (FixedPcdGet32(PcdFdSize), x2)
	sub x5, x0, x1
	ldr x4, StartupAddr
	sub x4, x4, x5
	ldr x3, DramInitAddr
	sub x3, x3, x5
	blr x4
  
