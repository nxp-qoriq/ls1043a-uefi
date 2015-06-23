/** @file IfcNorLib.c

 Copyright (c) 2014, Freescale Ltd. All rights reserved.

 This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php

 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

 **/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/NorFlashPlatformLib.h>
#include <Library/FslIfc.h>

/*
  Tune IFC NOR timings.
*/
VOID
IfcNorSetMemctlRegs (
  VOID
  )
{
  MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->cspr_cs[IFC_CS0].cspr_ext, IFC_NOR_CSPR_EXT);

  MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->ftim_cs[IFC_CS0].ftim[IFC_FTIM0], IFC_NOR_FTIM0);
  MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->ftim_cs[IFC_CS0].ftim[IFC_FTIM1], IFC_NOR_FTIM1);
  MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->ftim_cs[IFC_CS0].ftim[IFC_FTIM2], IFC_NOR_FTIM2);
  MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->ftim_cs[IFC_CS0].ftim[IFC_FTIM3], IFC_NOR_FTIM3);

  MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->cspr_cs[IFC_CS0].cspr, IFC_NOR_CSPR0);
  MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->amask_cs[IFC_CS0].amask, IFC_NOR_AMASK0);
  MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->csor_cs[IFC_CS0].csor, IFC_NOR_CSOR0);
}

/*
  Init IFC NOR.
*/
VOID
IfcNorInit (
  VOID
  )
{
  // Tune IFC NOR timings.
  IfcNorSetMemctlRegs ();
}

