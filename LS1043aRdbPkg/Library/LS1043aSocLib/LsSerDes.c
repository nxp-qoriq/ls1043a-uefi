/** @file
 Provides the basic interfaces for SerDes Module

 Copyright (c) 2015, Freescale Ltd. All rights reserved.

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

static struct SerDesConfig *SerDesCfgTbl[] = {
	SerDes1CfgTbl
};

SrdsPrtcl
SerDesGetPrtcl
(
 IN INTN SerDes,
 IN INTN Cfg,
 IN INTN Lane
)
{
  struct SerDesConfig *Ptr;

  if (SerDes >= ARRAY_SIZE(SerDesCfgTbl))
    return 0;

  Ptr = SerDesCfgTbl[SerDes];
  while (Ptr->Protocol) {
    if (Ptr->Protocol == Cfg) {
      return Ptr->Lanes[Lane];
    }
    Ptr++;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
IsSerDesPrtclValid
(
 IN INTN SerDes,
 IN UINT32 Prtcl
)
{
  INTN Cnt;
  struct SerDesConfig *Ptr;

  if (SerDes >= ARRAY_SIZE(SerDesCfgTbl))
    return 0;

  Ptr = SerDesCfgTbl[SerDes];
  while (Ptr->Protocol) {
    if (Ptr->Protocol == Prtcl) {
      DEBUG((EFI_D_INFO, "Protocol: %x Matched with the one in Table\n", Prtcl));
      break;
    }
    Ptr++;
  }

  if (!Ptr->Protocol)
    return 0;

  for (Cnt = 0; Cnt < SRDS_MAX_LANES; Cnt++) {
    if (Ptr->Lanes[Cnt] != NONE)
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
SerDesGetFirstLane
(
 IN UINT32 Sd,
 IN SrdsPrtcl Device
)
{
  struct ccsr_gur *Gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
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
    DEBUG((EFI_D_INFO, "Invalid SerDes%d\n", Sd));
    break;
  }

  /* Is serdes enabled at all? */
  if (Cfg == 0)
    return EFI_DEVICE_ERROR;

  for (Cnt = 0; Cnt < SRDS_MAX_LANES; Cnt++) {
    if (SerDesGetPrtcl(Sd, Cfg, Cnt) == Device)
      return Cnt;
  }

  return EFI_DEVICE_ERROR;
}

VOID
LSSerDesInit
(
 UINT32 Sd,
 UINT32 SdAddr,
 UINT32 SdPrtclMask,
 UINT32 SdPrtclShift,
 UINT16 SerDesPrtclMap[SERDES_PRCTL_COUNT]
)
{
  struct ccsr_gur *Gur = (VOID *)(CONFIG_SYS_FSL_GUTS_ADDR);
  UINT32 Cfg;
  INTN Lane;
  UINT32 Flag = 0;

  Cfg = MmioReadBe32((UINTN)&Gur->rcwsr[4]) & SdPrtclMask;
  Cfg >>= SdPrtclShift;
  
  DEBUG((EFI_D_INFO, "Using SERDES%d Protocol: %d (0x%x)\n", Sd + 1, Cfg, Cfg));

  if (!IsSerDesPrtclValid(Sd, Cfg)) {
    DEBUG((EFI_D_ERROR, "SERDES%d[PRTCL] = 0x%x is not valid\n", Sd + 1, Cfg));
    Flag++;
  }

  for (Lane = 0; Lane < SRDS_MAX_LANES; Lane++) {
    SrdsPrtcl LanePrtcl = SerDesGetPrtcl(Sd, Cfg, Lane);
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
  
  DEBUG((EFI_D_INFO, "SerDesInit:\n"));
#ifdef CONFIG_SYS_SRDS_1
  LSSerDesInit(FSL_SRDS_1,
	      CONFIG_SYS_SERDES_ADDR,
	      LS1043_RCWSR4_SRDS1_PRTCL_MASK,
	      LS1043_RCWSR4_SRDS1_PRTCL_SHIFT,
	      SerDes1PrtclMap);
#endif
}
