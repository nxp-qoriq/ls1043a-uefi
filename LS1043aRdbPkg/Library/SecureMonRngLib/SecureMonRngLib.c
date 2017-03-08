/** SecureMonRngLib.c
  RNG Library to provide get random number from SEC.

  Copyright (c) 2017 NXP.
  All rights reserved.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmSmcLib.h>
#include <Library/SecureMonRngLib.h>

EFI_STATUS
getRawRng(
    IN  UINTN    RNGLength,
    OUT UINT8   *RNGValue
  )
{
  UINT8        I             = 0;

  ARM_SMC_ARGS ArmSmcArgs;

  // Send command to get RNG value
  if (SIZE_4_BYTE == RNGLength) {
    ArmSmcArgs.Arg0 = SIP_PRNG_32;
    ArmSmcArgs.Arg1 = GET_32BIT_VALUE;
  } else if (SIZE_8_BYTE == RNGLength) {
    ArmSmcArgs.Arg0 = SIP_PRNG_64;
    ArmSmcArgs.Arg1 = GET_64BIT_VALUE;
  }

  ArmCallSmc (&ArmSmcArgs);

  if (ArmSmcArgs.Arg0) {
    DEBUG((EFI_D_ERROR, "SIP_PRNG_64 failed : 0x%x \n", ArmSmcArgs.Arg0));
    return EFI_DEVICE_ERROR;
  }

  DEBUG((EFI_D_INFO, "Random value from PPA %p \n", ArmSmcArgs.Arg1));

  CopyMem(RNGValue,(UINT8 *)&ArmSmcArgs.Arg1, RNGLength);

  for (I = 0; I < RNGLength; I++) {
    DEBUG((EFI_D_INFO, "RNGValue[%d] 0x%x \n", I, *(RNGValue+I)));
  }

  return EFI_SUCCESS;
}
