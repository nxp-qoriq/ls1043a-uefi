/** Common.c
*
*  Copyright (c) 2015, Freescale Ltd. All rights reserved.
*  Author: Bhupesh Sharma <bhupesh.sharma@freescale.com>
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

#include <LS1043aRdb.h>

UINT32
__Div64_32 (
  IN   UINT64        *N,
  IN   UINT32        Base
  )
{
  UINT64 Rem = *N;
  UINT64 b = Base;
  UINT64 Res, d = 1;
  UINT32 High = Rem >> 32;

  /** Reduce the thing a bit first */
  Res = 0;
  if (High >= Base) {
    High /= Base;
    Res = (UINT64) High << 32;
    Rem -= (UINT64) (High*Base) << 32;
  }

  while ((UINTN)b > 0 && b < Rem) {
    b = b+b;
    d = d+d;
  }

  do {
    if (Rem >= b) {
      Rem -= b;
      Res += d;
    }
    b >>= 1;
    d >>= 1;
  } while (d);

  *N = Res;
  return Rem;
}

