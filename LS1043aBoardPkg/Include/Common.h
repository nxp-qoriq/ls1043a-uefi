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

#endif
