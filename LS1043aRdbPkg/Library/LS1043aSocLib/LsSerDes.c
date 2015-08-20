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

#include <Uefi.h>
#include <LS1043aRdb.h>
#include <LS1043aSocLib.h>
#include <Library/DebugLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/IoLib.h>
#include <Ls1043aSerDes.h>

#ifdef CONFIG_SYS_SRDS_1
static UINT16 SerDes1PrtclMap[SERDES_PRCTL_COUNT];
#endif

static struct SerDesConfig *SerDesConfigTbl[] = {
	SerDes1ConfigTbl
};

SrdsPrtcl
GetSerDesPrtcl
(
 IN INTN SerDes,
 IN INTN Cfg,
 IN INTN Lane
)
{
  struct SerDesConfig *Config;

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
  struct SerDesConfig *Config;

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
 IN SrdsPrtcl Device
)
{
  INTN Ret = 0;

#ifdef CONFIG_SYS_SRDS_1
  Ret |= SerDes1PrtclMap[Device];
#endif

  return !!Ret;
}

INTN
GetSerDesFirstLane
(
 IN UINT32 Sd,
 IN SrdsPrtcl Device
)
{
  struct CcsrGur *Gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
  UINT32 Cfg = MmioReadBe32((UINTN)&Gur->rcwsr[4]);
  INTN Cnt;

  switch (Sd) {
#ifdef CONFIG_SYS_SRDS_1
  case FSL_SRDS_1:
    Cfg &= LS1043_RCWSR4_SRDS1_PRTCL_MASK;
    Cfg >>= LS1043_RCWSR4_SRDS1_PRTCL_SHIFT;
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
 UINT32 SrdsPrtclMask,
 UINT32 SrdsPrtclShift,
 UINT16 SerDesPrtclMap[SERDES_PRCTL_COUNT]
)
{
  struct CcsrGur *Gur = (VOID *)(CONFIG_SYS_FSL_GUTS_ADDR);
  UINT32 SrdsProt;
  INTN Lane;
  UINT32 Flag = 0;

  SrdsProt = MmioReadBe32((UINTN)&Gur->rcwsr[4]) & SrdsPrtclMask;
  SrdsProt >>= SrdsPrtclShift;
  
  DEBUG((EFI_D_INFO, "Using SERDES%d Protocol: %d (0x%x)\n", Srds + 1, SrdsProt, SrdsProt));

  if (!CheckSerDesPrtclValid(Srds, SrdsProt)) {
    DEBUG((EFI_D_ERROR, "SERDES%d[PRTCL] = 0x%x is not valid\n", Srds + 1, SrdsProt));
    Flag++;
  }

  for (Lane = 0; Lane < SRDS_MAX_LANES; Lane++) {
    SrdsPrtcl LanePrtcl = GetSerDesPrtcl(Srds, SrdsProt, Lane);
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
