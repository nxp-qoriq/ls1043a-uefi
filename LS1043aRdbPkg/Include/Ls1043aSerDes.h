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

typedef enum _SERDES_LANE_PROTOCOL {
	NONE = 0,
	PCIE1,
	PCIE2,
	PCIE3,
       SATA,
       SGMII_FM1_DTSEC1,
       SGMII_FM1_DTSEC2,
       SGMII_FM1_DTSEC3,
       SGMII_FM1_DTSEC4,
       SGMII_FM1_DTSEC5,
       SGMII_FM1_DTSEC6,
       SGMII_FM1_DTSEC9,
       QSGMII_FM1_A,        /* A indicates MACs 1,2,5,6 */
       XFI_FM1_MAC9,
       SGMII_2500_FM1_DTSEC2,
       SGMII_2500_FM1_DTSEC9,
       SERDES_PRCTL_COUNT
} SERDES_LANE_PROTOCOL;

enum Srds {
	FSL_SRDS_1  = 0,
	FSL_SRDS_2  = 1
};

typedef struct _SerDesConfig {
	UINT16 Protocol;
	UINT8 SrdsLane[SRDS_MAX_LANES];
} SerDesConfig;

typedef VOID
SERDES_LANE_PROBE_CALLBACK(
  IN SERDES_LANE_PROTOCOL LaneProtocol,
  IN VOID *Arg
);

VOID
SerDesProbeLanes(
  IN SERDES_LANE_PROBE_CALLBACK *SerDesLaneProbeCallback,
  IN VOID *Arg
);

#endif /* __LS1043A_SERDES_H */
