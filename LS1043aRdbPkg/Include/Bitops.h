/** @Bitops.h
  Header defining the general bitwise operations

  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __BITOPS_H__
#define __BITOPS_H__

/*
 * Returns the bit mask for a bit index from 0 to 31
 */
#define BIT(_BitIndex)         (0x1u << (_BitIndex))

/**
  Find last (most-significant) bit set

  @param   X		the word to search

  Note Fls(0) = 0, Fls(1) = 1, Fls(0x80000000) = 32.

**/
static inline INT32 GenericFls(INT32 X)
{
	INT32 R = 32;

	if (!X)
		return 0;
	if (!(X & 0xffff0000u)) {
		X <<= 16;
		R -= 16;
	}
	if (!(X & 0xff000000u)) {
		X <<= 8;
		R -= 8;
	}
	if (!(X & 0xf0000000u)) {
		X <<= 4;
		R -= 4;
	}
	if (!(X & 0xc0000000u)) {
		X <<= 2;
		R -= 2;
	}
	if (!(X & 0x80000000u)) {
		X <<= 1;
		R -= 1;
	}
	return R;
}


/**
  Return position of base 2 of X

**/
static inline int __ILog2(UINT32 X)
{
	return GenericFls(X) - 1;
}

#define LOG2(X) (((X & 0xaaaaaaaa) ? 1 : 0) + ((X & 0xcccccccc) ? 2 : 0) + \
               ((X & 0xf0f0f0f0) ? 4 : 0) + ((X & 0xff00ff00) ? 8 : 0) + \
               ((X & 0xffff0000) ? 16 : 0))

#endif
