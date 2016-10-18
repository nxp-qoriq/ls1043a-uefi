/** @file
#
#  Driver for installing BlockIo protocol over IFC NAND
#
#  Copyright (c) 2014, Freescale Ltd. All rights reserved.
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

extern EFI_STATUS PpaInit(UINT64);
extern VOID El2SwitchSetup(VOID);
extern VOID InitMmu(VOID);

EFI_STATUS
PpaInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
	EFI_STATUS Status;

//El2SwitchSetup();
//GicV2InitSecure();
	Status = PpaInit((UINT64)0x80000000);
	InitMmu();
	return Status;
}
