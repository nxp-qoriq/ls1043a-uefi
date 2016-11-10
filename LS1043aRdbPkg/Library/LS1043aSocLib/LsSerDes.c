/** LsSerDes.c
 Provides the basic interfaces for SerDes Module

 Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

 This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution. The full text of the license may be found 
 http://opensource.org/licenses/bsd-license.php

 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/ 

#include <I2c.h>
#include <Ddr.h>
#include <Uefi.h>
#include <LS1043aRdb.h>
#include <LS1043aSocLib.h>
#include <Ls1043aSerDes.h>

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Ppi/ArmMpCoreInfo.h>


#ifdef CONFIG_SYS_SRDS_1
STATIC UINT16 SerDes1PrtclMap[SERDES_PRCTL_COUNT];
#endif

STATIC CONST SerDesConfig SerDes1ConfigTbl[] = {
       /* SerDes 1 */
       {0x1555, {XFI_FM1_MAC9, PCIE1, PCIE2, PCIE3} },
       {0x2555, {SGMII_2500_FM1_DTSEC9, PCIE1, PCIE2, PCIE3} },
       {0x4555, {QSGMII_FM1_A, PCIE1, PCIE2, PCIE3} },
       {0x4558, {QSGMII_FM1_A, PCIE1, PCIE2, SATA} },
       {0x1355, {XFI_FM1_MAC9, SGMII_FM1_DTSEC2, PCIE2, PCIE3} },
       {0x2355, {SGMII_2500_FM1_DTSEC9, SGMII_FM1_DTSEC2, PCIE2, PCIE3} },
       {0x3335, {SGMII_FM1_DTSEC9, SGMII_FM1_DTSEC2, SGMII_FM1_DTSEC5, PCIE3} },
       {0x3355, {SGMII_FM1_DTSEC9, SGMII_FM1_DTSEC2, PCIE2, PCIE3} },
       {0x3358, {SGMII_FM1_DTSEC9, SGMII_FM1_DTSEC2, PCIE2, SATA} },
       {0x3555, {SGMII_FM1_DTSEC9, PCIE1, PCIE2, PCIE3} },
       {0x3558, {SGMII_FM1_DTSEC9, PCIE1, PCIE2, SATA} },
       {0x7000, {PCIE1, PCIE1, PCIE1, PCIE1} },
       {0x9998, {PCIE1, PCIE2, PCIE3, SATA} },
       {0x6058, {PCIE1, PCIE1, PCIE2, SATA} },
       {0x1455, {XFI_FM1_MAC9, QSGMII_FM1_A, PCIE2, PCIE3} },
       {0x2455, {SGMII_2500_FM1_DTSEC9, QSGMII_FM1_A, PCIE2, PCIE3} },
       {0x2255, {SGMII_2500_FM1_DTSEC9, SGMII_2500_FM1_DTSEC2, PCIE2, PCIE3} },
       {0x3333, {SGMII_FM1_DTSEC9, SGMII_FM1_DTSEC2, SGMII_FM1_DTSEC5, SGMII_FM1_DTSEC6} },
       {}
};

STATIC CONST SerDesConfig *SerDesConfigTbl[] = {
	SerDes1ConfigTbl
};

SERDES_LANE_PROTOCOL
GetSerDesPrtcl
(
 IN INTN SerDes,
 IN INTN Cfg,
 IN INTN Lane
)
{
  CONST SerDesConfig *Config;

  if (SerDes >= ARRAY_SIZE(SerDesConfigTbl))
    return 0;

  Config = SerDesConfigTbl[SerDes];
  while (Config->Protocol) {
    if (Config->Protocol == Cfg) {
      return Config->SrdsLane[Lane];
    }
    Config++;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
CheckSerDesPrtclValid
(
 IN INTN SerDes,
 IN UINT32 Prtcl
)
{
  INTN Cnt;
  CONST SerDesConfig *Config;

  if (SerDes >= ARRAY_SIZE(SerDesConfigTbl))
    return 0;

  Config = SerDesConfigTbl[SerDes];
  while (Config->Protocol) {
    if (Config->Protocol == Prtcl) {
      DEBUG((EFI_D_INFO, "Protocol: %x Matched with the one in Table\n", Prtcl));
      break;
    }
    Config++;
  }

  if (!Config->Protocol)
    return 0;

  for (Cnt = 0; Cnt < SRDS_MAX_LANES; Cnt++) {
    if (Config->SrdsLane[Cnt] != NONE)
      return 1;
  }

  return 0;
}


EFI_STATUS
IsSerDesConfigured
(
 IN SERDES_LANE_PROTOCOL Device
)
{
  INTN Ret = 0;

#ifdef CONFIG_SYS_SRDS_1
  Ret |= SerDes1PrtclMap[Device];
#endif

  return !!Ret;
}

UINT32
GetSerDesProtFromRCWSR (
  IN VOID
  )
{
  struct CcsrGur *Gur = (VOID *)(GUTS_ADDR);
  UINT32 SrdsProt = MmioReadBe32((UINTN)&Gur->rcwsr[4])
                    & LS1043_RCWSR4_SRDS1_PRTCL_MASK;
  SrdsProt >>= LS1043_RCWSR4_SRDS1_PRTCL_SHIFT;
  return SrdsProt;
}

INTN
GetSerDesFirstLane
(
 IN UINT32 Sd,
 IN SERDES_LANE_PROTOCOL Device
)
{
  UINT32 Cfg = 0;
  INTN Cnt;

  switch (Sd) {
#ifdef CONFIG_SYS_SRDS_1
  case FSL_SRDS_1:
    Cfg = GetSerDesProtFromRCWSR();
    break;
#endif
  default:
    DEBUG((EFI_D_INFO, "Invalid SerDes%d, Only one SerDes is there.\n", Sd));
    break;
  }

  /* Is serdes enabled at all? */
  if (Cfg == 0)
    return EFI_DEVICE_ERROR;

  for (Cnt = 0; Cnt < SRDS_MAX_LANES; Cnt++) {
    if (GetSerDesPrtcl(Sd, Cfg, Cnt) == Device)
      return Cnt;
  }

  return EFI_DEVICE_ERROR;
}

VOID
LSSerDesInit
(
 UINT32 Srds,
 UINT32 SrdsAddr,
 UINT32 SERDES_LANE_PROTOCOLMask,
 UINT32 SERDES_LANE_PROTOCOLShift,
 UINT16 SerDesPrtclMap[SERDES_PRCTL_COUNT]
)
{
  INTN Lane;
  UINT32 Flag = 0;

  UINT32 SrdsProt = GetSerDesProtFromRCWSR();
  
  DEBUG((EFI_D_INFO, "Using SERDES%d Protocol: %d (0x%x)\n", Srds + 1, SrdsProt, SrdsProt));

  if (!CheckSerDesPrtclValid(Srds, SrdsProt)) {
    DEBUG((EFI_D_ERROR, "SERDES%d[PRTCL] = 0x%x is not valid\n", Srds + 1, SrdsProt));
    Flag++;
  }

  for (Lane = 0; Lane < SRDS_MAX_LANES; Lane++) {
    SERDES_LANE_PROTOCOL LanePrtcl = GetSerDesPrtcl(Srds, SrdsProt, Lane);
    if (LanePrtcl >= SERDES_PRCTL_COUNT) {
      DEBUG((EFI_D_ERROR, "Unknown SerDes lane protocol %d\n", LanePrtcl));
      Flag++;
    } else {
      SerDesPrtclMap[LanePrtcl] = 1;
    }
  }

  if (Flag)
    DEBUG((EFI_D_ERROR, "Could not configure SerDes module!!\n"));
  else
    DEBUG((EFI_D_INFO, "Successfully configured SerDes module!!\n"));
}

VOID 
SerDesInit
(
 VOID
)
{
  DEBUG((EFI_D_INFO, "Initializing SerDes....\n"));
#ifdef CONFIG_SYS_SRDS_1
  LSSerDesInit(FSL_SRDS_1,
	      CONFIG_SYS_SERDES_ADDR,
	      LS1043_RCWSR4_SRDS1_PRTCL_MASK,
	      LS1043_RCWSR4_SRDS1_PRTCL_SHIFT,
	      SerDes1PrtclMap);
#endif
}

/**
   Probes lanes in all SerDes instances enabled and calls
   the given callback for each lane.

   @param[in] SerDesLaneProbeCallback Driver-specific callback to be invoked
                                      for every lane protocol configured in
                                      each SerDes instance, according to
                                      the SerDes instance's RCW configuration.

   @param[in] Arg                     Callback argument or NULL
 **/
VOID
SerDesProbeLanes(
  IN SERDES_LANE_PROBE_CALLBACK *SerDesLaneProbeCallback,
  IN VOID *Arg
  )
{
  CONST SerDesConfig *Config = SerDesConfigTbl[0];
  UINT32 SrdsProt = GetSerDesProtFromRCWSR();
  UINT16 Lane;

  DEBUG((EFI_D_INFO,"SrdsProt = 0x%x \n", SrdsProt));
  while (Config->Protocol) {
    if (Config->Protocol == SrdsProt) {
      for (Lane = 0; Lane < SRDS_MAX_LANES; Lane++) {
        UINT16 LaneProtocol = Config->SrdsLane[Lane];
        DEBUG((EFI_D_INFO, "LaneProtocol[%d] = 0x%x\n", Lane, LaneProtocol));
        ASSERT(LaneProtocol < SERDES_PRCTL_COUNT);
        ASSERT(LaneProtocol != NONE);
        if(SerDesLaneProbeCallback) {
          SerDesLaneProbeCallback(LaneProtocol, Arg);
        }
      }
      break;
    }
    Config++;
  }
}
