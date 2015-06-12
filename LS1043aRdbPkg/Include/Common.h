/** @Common.h
  Header defining the General Purpose Utilities

  Copyright (c) 2014, Freescale Ltd. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __COMMON_H__
#define __COMMON_H__



#define BUS_CLK      		300000000

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

#define GET_CLOCK  		((BUS_CLK)  / 2)

#define RoundUp(X, Y)       ((((X) + ((Y) - 1)) / (Y)) * (Y))

#define ARCH_DMA_MINALIGN   64

/* Interface types: */
#define IF_TYPE_UNKNOWN     0
#define IF_TYPE_IDE         1
#define IF_TYPE_SCSI        2
#define IF_TYPE_ATAPI       3
#define IF_TYPE_USB         4
#define IF_TYPE_DOC         5
#define IF_TYPE_MMC         6
#define IF_TYPE_SD          7
#define IF_TYPE_SATA        8
#define IF_TYPE_HOST        9
#define IF_TYPE_MAX         10     /* Max number of IF_TYPE_* supported */


#define offsetof(TYPE, MEMBER) ((UINTN) &((TYPE *)0)->MEMBER)

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
