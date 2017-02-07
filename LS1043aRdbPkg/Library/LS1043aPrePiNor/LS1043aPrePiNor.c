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


#include <Ddr.h>
#include <Library/ArmLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <LS1043aRdb.h>
#include <LS1043aSocLib.h>
#include <Library/SerialPortLib.h>

UINTN mGlobalVariableBase = 0;

VOID CopyImage(UINT8* Dest, UINT8* Src, UINTN Size)
{
  UINTN Count;

  for(Count = 0; Count < Size; Count++)
    Dest[Count] = Src[Count];
}

STATIC VOID ErratumA008550Pre (VOID)
{
  struct CcsrCci400 *Base = (struct CcsrCci400 *)CONFIG_SYS_CCI400_ADDR;
  struct CcsrDdr *Ddr = (VOID *)CONFIG_SYS_FSL_DDR_ADDR;

  /* disables barrier transactions to DDRC from CCI400 */
  MmioWrite32((UINTN)&Base->ctrl_ord, CCI400_CTRLORD_TERM_BARRIER);

  /* disable re-ordering in DDR Controller */
  MmioWriteBe32((UINTN)&Ddr->Eor,
	DDR_EOR_READ_REOD_DIS | DDR_EOR_WRITE_REOD_DIS);
}

STATIC VOID ErratumA008550Post(VOID)
{
  struct CcsrCci400 *Base = (struct CcsrCci400 *)CONFIG_SYS_CCI400_ADDR;
  struct CcsrDdr *Ddr = (VOID *)CONFIG_SYS_FSL_DDR_ADDR;
  UINT32 Temp = 0;

  /* enable barrier transactions to DDRC from CCI400 */
  MmioWrite32((UINTN)&Base->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

  /* enable re-ordering in DDR Controller */
  Temp = MmioReadBe32((UINTN)&Ddr->Eor);
  Temp &= ~(DDR_EOR_READ_REOD_DIS | DDR_EOR_WRITE_REOD_DIS);
  MmioWriteBe32((UINTN)&Ddr->Eor, Temp);
}

VOID CEntryPoint(
  UINTN	UefiMemoryBase,
  UINTN 	UefiNorBase,
  UINTN	UefiMemorySize
  )
{ 
  VOID	(*PrePiStart)(VOID);

  if (PcdGetBool(PcdDdrErratumA008550))
    ErratumA008550Pre();

  // Data Cache enabled on Primary core when MMU is enabled.
  ArmDisableDataCache ();
  // Invalidate Data cache
  //ArmInvalidateDataCache ();
  // Invalidate instruction cache
  ArmInvalidateInstructionCache ();
  // Enable Instruction Caches on all cores.
  ArmEnableInstructionCache ();

  SerialPortInitialize ();

  TimerInit();
  DramInit();

  if (PcdGetBool(PcdDdrErratumA008550))
    ErratumA008550Post();

  CopyImage((VOID*)UefiMemoryBase, (VOID*)UefiNorBase, UefiMemorySize);

  PrePiStart = (VOID (*)())((UINT64)PcdGet64(PcdFvBaseAddress));
  PrePiStart();
}
