#------------------------------------------------------------------------------
#
# Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
# Copyright (c) 2011 - 2014, ARM Limited. All rights reserved.
# Copyright (c) 2016, Linaro Limited. All rights reserved.
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#------------------------------------------------------------------------------

#include <AsmMacroIoLibV8.h>

.set MPIDR_U_BIT,    (30)
.set MPIDR_U_MASK,   (1 << MPIDR_U_BIT)

// DAIF bit definitions for writing through msr daifclr/sr daifset
.set DAIF_WR_FIQ_BIT,   (1 << 0)
.set DAIF_WR_IRQ_BIT,   (1 << 1)
.set DAIF_WR_ABORT_BIT, (1 << 2)
.set DAIF_WR_DEBUG_BIT, (1 << 3)
.set DAIF_WR_INT_BITS,  (DAIF_WR_FIQ_BIT | DAIF_WR_IRQ_BIT)
.set DAIF_WR_ALL,       (DAIF_WR_DEBUG_BIT | DAIF_WR_ABORT_BIT | DAIF_WR_INT_BITS)


ASM_FUNC(ArmIsMpCore)
  mrs   x0, mpidr_el1         // Read EL1 Mutliprocessor Affinty Reg (MPIDR)
  and   x0, x0, #MPIDR_U_MASK // U Bit clear, the processor is part of a multiprocessor system
  lsr   x0, x0, #MPIDR_U_BIT
  eor   x0, x0, #1
  ret


ASM_FUNC(ArmEnableAsynchronousAbort)
  msr   daifclr, #DAIF_WR_ABORT_BIT
  isb
  ret


ASM_FUNC(ArmDisableAsynchronousAbort)
  msr   daifset, #DAIF_WR_ABORT_BIT
  isb
  ret


ASM_FUNC(ArmEnableIrq)
  msr   daifclr, #DAIF_WR_IRQ_BIT
  isb
  ret


ASM_FUNC(ArmDisableIrq)
  msr   daifset, #DAIF_WR_IRQ_BIT
  isb
  ret


ASM_FUNC(ArmEnableFiq)
  msr   daifclr, #DAIF_WR_FIQ_BIT
  isb
  ret


ASM_FUNC(ArmDisableFiq)
  msr   daifset, #DAIF_WR_FIQ_BIT
  isb
  ret


ASM_FUNC(ArmEnableInterrupts)
  msr   daifclr, #DAIF_WR_INT_BITS
  isb
  ret


ASM_FUNC(ArmDisableInterrupts)
  msr   daifset, #DAIF_WR_INT_BITS
  isb
  ret


ASM_FUNC(ArmDisableAllExceptions)
  msr   daifset, #DAIF_WR_ALL
  isb
  ret


// UINT32
// ReadCCSIDR (
//   IN UINT32 CSSELR
//   )
ASM_FUNC(ReadCCSIDR)
  msr   csselr_el1, x0        // Write Cache Size Selection Register (CSSELR)
  isb
  mrs   x0, ccsidr_el1        // Read current Cache Size ID Register (CCSIDR)
  ret


// UINT32
// ReadCLIDR (
//   IN UINT32 CSSELR
//   )
ASM_FUNC(ReadCLIDR)
  mrs   x0, clidr_el1         // Read Cache Level ID Register
  ret

ASM_FUNC(GicV2InitSecure)
	EL1_OR_EL2_OR_EL3(x0)
	1:
	ret
	2:
	MOV64 (x0, FixedPcdGet64 (PcdGicDistributorBase))
	MOV64 (x1, FixedPcdGet64 (PcdGicInterruptInterfaceBase))
	mov w9, #0x3    // EnableGrp0 | EnableGrp1
	str w9, [x0, 0x0000]	// GICD_CTLR: Secure GICD_CTLR
	ldr w9, [x0, 0x0004]	// GICD_TYPER
	and w10, w9, #0x1f      // ITLinesNumber
	cbz w10, 3f		// No SPIs
	add x11, x0, (0x0080 + 4)   // GICD_IGROUPRn
	mov w9, #~0  // Config SPIs as Grp1
	4:
	str w9, [x11], #0x4
	sub w10, w10, #0x1
	cbnz w10, 4b
	3:
	/*
	** Initialize SGIs and PPIs	
	** x0: Distributor Base	
	** x1: Cpu Interface Base	
	**/
	mov w9, #~0	/*Config SGIs and PPIs as Grp1 */
	str w9, [x0, 0x0080]       /*GICD_IGROUPR0 */
	mov w9, #0x1              /*Enable SGI 0 */
	str w9, [x0, 0x0100]

	/* Initialize Cpu Interface */	
	mov w9, #0x1e7	/*Disable IRQ/FIQ Bypass & */
									/*Enable Ack Group1 Interrupt & */
									/*EnableGrp0 & EnableGrp1 */

	str w9, [x1, 0x0000]	/* Secure GICC_CTLR */	
	mov w9, #0x1 << 7			/* Non-Secure access to GICC_PMR */
	str w9, [x1,0x0004]	

	ret

ASM_FUNCTION_REMOVE_IF_UNREFERENCED
