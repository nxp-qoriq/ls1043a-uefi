/** @Common.h
  Header defining the General Purpose Utilities

  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __COMMON_H__
#define __COMMON_H__


/**
  Utility to return minimum among X and Y
**/
#define Min(X, Y)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		(__x < __y) ? __x : __y; })

/**
  Utility to return maximum among X and Y
**/
#define Max(X, Y)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		(__x > __y) ? __x : __y; })

/**
  Utility to perform division (N must be 64bit)
**/
# define DoDiv(N,Base) ({				\
	UINT32 __Base = (Base);			\
	UINT32 __Rem;					\
	(VOID)(((typeof((N)) *)0) == ((UINT64 *)0));	\
	if (((N) >> 32) == 0) {			\
		__Rem = (UINT32)(N) % __Base;		\
		(N) = (UINT32)(N) / __Base;		\
	} else						\
		__Rem = __Div64_32(&(N), __Base);	\
	__Rem;						\
 })


#define RoundUp(X, Y)       ((((X) + ((Y) - 1)) / (Y)) * (Y))

#define ARCH_DMA_MINALIGN   64

#define offsetof(TYPE, MEMBER) ((UINTN) &((TYPE *)0)->MEMBER)

/**
 * Upper32Bits - return bits 32-63 of a number
 * @N: the number we're accessing
 *
 * A basic shift-right of a 64- or 32-bit quantity.  Use this to suppress
 * the "right shift count >= width of type" warning when that quantity is
 * 32-bits.
 */
#define Upper32Bits(N) ((UINT32)(((N) >> 16) >> 16))

/**
 * Lower32Bits - return bits 0-31 of a number
 * @N: the number we're accessing
 */
#define Lower32Bits(N) ((UINT32)(N))

typedef UINTN PhysAddrT;
typedef UINTN PhysSizeT;

static inline PhysAddrT VirtToPhys(VOID * VAddr)
{
       return (PhysAddrT)(VAddr);
}

/*
 * The ALLOC_CACHE_ALIGN_BUFFER macro is used to allocate a buffer on the
 * stack that meets the minimum architecture alignment requirements for DMA.
 * Such a buffer is useful for DMA operations where flushing and invalidating
 * the cache before and after a read and/or write operation is required for
 * correct operations.
 *
 * When called the macro creates an array on the stack that is sized such
 * that:
 *
 * 1) The beginning of the array can be advanced enough to be aligned.
 *
 * 2) The size of the aligned portion of the array is a multiple of the minimum
 *    architecture alignment required for DMA.
 *
 * 3) The aligned portion contains enough space for the original number of
 *    elements requested.
 *
 * The macro then creates a pointer to the aligned portion of this array and
 * assigns to the pointer the address of the first element in the aligned
 * portion of the array.
 *
 * Calling the macro as:
 *
 *     ALLOC_CACHE_ALIGN_BUFFER(uint32_t, buffer, 1024);
 *
 * Will result in something similar to saying:
 *
 *     uint32_t    buffer[1024];
 *
 * The following differences exist:
 *
 * 1) The resulting buffer is guaranteed to be aligned to the value of
 *    ARCH_DMA_MINALIGN.
 *
 * 2) The buffer variable created by the macro is a pointer to the specified
 *    type, and NOT an array of the specified type.  This can be very important
 *    if you want the address of the buffer, which you probably do, to pass it
 *    to the DMA hardware.  The value of &buffer is different in the two cases.
 *    In the macro case it will be the address of the pointer, not the address
 *    of the space reserved for the buffer.  However, in the second case it
 *    would be the address of the buffer.  So if you are replacing hard coded
 *    stack buffers with this macro you need to make sure you remove the & from
 *    the locations where you are taking the address of the buffer.
 *
 * Note that the size parameter is the number of array elements to allocate,
 * not the number of bytes.
 *
 * This macro can not be used outside of function scope, or for the creation
 * of a function scoped static buffer.  It can not be used to create a cache
 * line aligned global buffer.
 */

#define ALIGN(x,a)          __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)       (((x)+(mask))&~(mask))

#define PAD_COUNT(S, Pad) (((S) - 1) / (Pad) + 1)
#define PAD_SIZE(S, Pad) (PAD_COUNT(S, Pad) * Pad)

#define ROUND(a,b)          (((a) + (b) - 1) & ~((b) - 1))

#define ARCH_DMA_MINALIGN   64

#define ALLOC_ALIGN_BUFFER_PAD(Type, Name, Size, Align, Pad)		\
	INT8 __##Name[ROUND(PAD_SIZE((Size) * sizeof(Type), Pad), Align)  \
		      + (Align - 1)];					\
									\
	Type *Name = (Type *) ALIGN((UINTN)__##Name, Align)
#define ALLOC_ALIGN_BUFFER(Type, Name, Size, Align)		\
	ALLOC_ALIGN_BUFFER_PAD(Type, Name, Size, Align, 1)
#define ALLOC_CACHE_ALIGN_BUFFER_PAD(Type, Name, Size, Pad)		\
	ALLOC_ALIGN_BUFFER_PAD(Type, Name, size, ARCH_DMA_MINALIGN, Pad)
#define ALLOC_CACHE_ALIGN_BUFFER(Type, Name, Size)			\
	ALLOC_ALIGN_BUFFER(Type, Name, Size, ARCH_DMA_MINALIGN)

/**
  Funtion to divide N to base Base

  @param   N         Dividend
  @param   Base      Base

**/
UINT32
__Div64_32 (
  IN   UINT64        *N,
  IN   UINT32        Base
  );

VOID
PrintSize (
  IN  UINT64 Size,
  IN  CONST INT8 *S
  );

#endif
