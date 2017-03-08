/** @file
  Main Header file for the RNG Library

  Copyright (c) 2017 NXP.
  All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __RNG_LIBRARY_H
#define __RNG_LIBRARY_H

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseMemoryLib/MemLibInternals.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Uefi.h>

#define MAX_RNG_BYTES           8   // max number of bytes in a returned rng
#define NUMBER_SUPPORTED_ALGO   1   // max number of rng algorithms supported by driver

#define GET_32BIT_VALUE         0
#define GET_64BIT_VALUE         1

#define SIP_PRNG_64             0xC200FF10
#define SIP_PRNG_32             0x8200FF10

#define SIZE_4_BYTE                 4
#define SIZE_8_BYTE                 8

EFI_STATUS
getRawRng(
    IN  UINTN    RNGLength,
    OUT UINT8   *RNGValue
  );

#endif
