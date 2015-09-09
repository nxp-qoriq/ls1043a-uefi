/** @file
*
*  Copyright (c) 2011-2014, ARM Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include "PrePi.h"

#include <Library/ArmGicLib.h>

#include <Ppi/ArmMpCoreInfo.h>


extern UINTN LS1043aGetBootLocPtr(VOID);
extern VOID LS1043aWaitForCoreRelease(UINTN);

VOID
PrimaryMain (
  IN  UINTN                     UefiMemoryBase,
  IN  UINTN                     StacksBase,
  IN  UINTN                     GlobalVariableBase,
  IN  UINT64                    StartTimeStamp
  )
{
  // Enable the GIC Distributor
  ArmGicEnableDistributor(PcdGet32(PcdGicDistributorBase));

  // In some cases, the secondary cores are waiting for an SGI from the next stage boot loader to resume their initialization
  if (!FixedPcdGet32(PcdSendSgiToBringUpSecondaryCores)) {
    // Sending SGI to all the Secondary CPU interfaces
    ArmGicSendSgiTo (PcdGet32(PcdGicDistributorBase), ARM_GIC_ICDSGIR_FILTER_EVERYONEELSE, 0x0E, PcdGet32 (PcdGicSgiIntId));
  }

  PrePiMain (UefiMemoryBase, StacksBase, GlobalVariableBase, StartTimeStamp);

  // We must never return
  ASSERT(FALSE);
}

VOID
SecondaryMain (
  IN  UINTN                     MpId
  )
{
  // The secondaries shouldn't reach here for LS1043A SoC
  ASSERT(FALSE);
}
