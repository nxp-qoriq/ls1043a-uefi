/** Ls1043aSerDes.h
 The Header file of SerDes Module 

 Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

 This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution. The full text of the license may be found 
 http://opensource.org/licenses/bsd-license.php

 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/ 

#ifndef __LS1043A_SERDES_H
#define __LS1043A_SERDES_H

#define	SRDS_MAX_LANES	4

#define LS1043_RCWSR4_SRDS1_PRTCL_MASK  0xFFFF0000
#define LS1043_RCWSR4_SRDS1_PRTCL_SHIFT 16

typedef enum {
	NONE = 0,
	PCIE1,
	PCIE2,
	PCIE3,
	SATA1,
	XFI1,
	XFI2,
	SGMII1,
	SGMII2,
	SGMII3,
	SGMII4,
	SGMII5,
	SGMII6,
	SGMII7,
	SGMII8,
	SGMII9,
	QSGMII1,
	QSGMII2,
	SERDES_PRCTL_COUNT
} SrdsPrtcl;

enum Srds {
	FSL_SRDS_1  = 0,
	FSL_SRDS_2  = 1
};

struct SerDesConfig {
	UINT16 Protocol;
	UINT8 SrdsLane[SRDS_MAX_LANES];
};

struct SerDesConfig SerDes1ConfigTbl[] = {
	/* SerDes 1 */
	{0x1555, {XFI1, PCIE1, PCIE2, PCIE3 } },
	{0x1560, {XFI1, PCIE1, PCIE3, PCIE3 } },
	{0x1460, {XFI1, QSGMII1, PCIE3, PCIE3 } },
	{0x1360, {XFI1, SGMII2, PCIE3, PCIE3 } },
	{0x2555, {SGMII9, PCIE1, PCIE2, PCIE3 } },
	{0x4555, {QSGMII1, PCIE1, PCIE2, PCIE3 } },
	{0x4558, {QSGMII1, PCIE1, PCIE2, SATA1 } },
	{0x1355, {XFI1, SGMII2, PCIE2, PCIE3 } },
	{0x1335, {XFI1, SGMII2, SGMII5, PCIE3 } },
	{0x1333, {XFI1, SGMII2, SGMII5, SGMII6 } },
	{0x2355, {SGMII9, SGMII2, PCIE2, PCIE3 } },
	{0x2260, {SGMII9, SGMII2, PCIE3, PCIE3 } },
	{0x2235, {SGMII9, SGMII2, SGMII5, PCIE3 } },
	{0x2233, {SGMII9, SGMII2, SGMII5, SGMII6 } },
	{0x3335, {SGMII9, SGMII2, SGMII5, PCIE3 } },
	{0x3355, {SGMII9, SGMII2, PCIE2, PCIE3 } },
	{0x3358, {SGMII9, SGMII2, PCIE2, SATA1 } },
	{0x3360, {SGMII9, SGMII2, PCIE3, PCIE3 } },
	{0x3560, {SGMII9, PCIE1, PCIE3, PCIE3 } },
	{0x3555, {SGMII9, PCIE1, PCIE2, PCIE3 } },
	{0x7000, {PCIE1, PCIE1, PCIE1, PCIE1 } },
	{0x9998, {PCIE1, PCIE2, PCIE3, SATA1 } },
	{0x6058, {PCIE1, PCIE1, PCIE2, SATA1 } },
	{0x1455, {XFI1, QSGMII1, PCIE2, PCIE3 } },
	{0x2455, {SGMII9, QSGMII1, PCIE2, PCIE3 } },
	{0x2255, {SGMII9, SGMII2, PCIE2, PCIE3 } },
	{0x3333, {SGMII9, SGMII2, SGMII5, SGMII6 } },
	{0x3338, {SGMII9, SGMII2, SGMII5, SATA1 } },
	{}
};

#endif /* __LS1043A_SERDES_H */
