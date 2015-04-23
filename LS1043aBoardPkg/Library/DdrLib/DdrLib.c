/** @DdrLib.c
  Ddr Library containing functions to initialize ddr controller

  Copyright (c) 2014, Freescale Ltd. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include <Ddr.h>
#include <I2c.h>
#include <Library/BaseLib.h>

/**
  Function to dump DDRC registers

**/

VOID
DdrRegDump (
  VOID
  )
{

  UINT32 *Ddr;
  INT32 Count = 0x5D;

  Ddr = (UINT32 *)CONFIG_SYS_FSL_DDR_ADDR;

  while (Count) {
    DEBUG((EFI_D_ERROR, "0x%x : 0x%x\n", Ddr, MmioReadBe32((UINTN)Ddr)));
    Ddr++;
    Count--;
  }
  DEBUG((EFI_D_ERROR,"\n"));
}

/**
  These tables contain all valid speeds we want to override with board
  specific parameters. DatarateMhzHigh values need to be in ascending order
  for each NRanks group.
**/

const struct BoardSpecificParameters UDimm0[] = {
  /**
    memory controller 0
    num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl
    ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3
  **/
  {2,  2140, 0, 4,     4, 0x0, 0x0},
  {1,  2140, 0, 4,     4, 0x0, 0x0},
  {}
};

const struct BoardSpecificParameters RDimm0[] = {
  /**
    memory controller 0
    num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl
    ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3
  **/
  {4,  2140, 0, 5,     4, 0x0, 0x0},
  {2,  2140, 0, 5,     4, 0x0, 0x0},
  {1,  2140, 0, 4,     4, 0x0, 0x0},
  {}
};

const struct BoardSpecificParameters *UDimm[] = {
  UDimm0,
};

const struct BoardSpecificParameters *RDimm[] = {
  RDimm0,
};


UINT32
__Div64_32 (
  IN   UINT64		*N,
  IN   UINT32		Base
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


UINTN
GetDdrFreq (
  VOID
  )
{
  return 0x00 /*TODO*/;
}


UINTN
StepAssignAddresses (
  IN   FslDdrInfoT		*Pinfo,
  IN   UINT32			DbwCapAdj[]
  )
{
  INT32 i, j;
  UINTN TotalMem, CurrentMemBase, TotalCtlrMem;
  UINTN RankDensity, CtlrDensity = 0;

  CurrentMemBase = 0ull;
  TotalMem = 0;

  /**
    If a reduced data width is requested, but the SPD
    specifies a physically wider device, adjust the
    computed dimm capacities accordingly before
    assigning addresses.
  **/
  for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
    UINT32 Found = 0;

    switch (Pinfo->MemctlOpts[i].DataBusWidth) {
      case 2:
        /** 16-bit */
        for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
	   UINT32 Dw;
	   if (!Pinfo->DimmParams[i][j].NRanks)
	     continue;
	   Dw = Pinfo->DimmParams[i][j].PrimarySdramWidth;
	     if ((Dw == 72 || Dw == 64)) {
	      	DbwCapAdj[i] = 2;
	      	break;
	     } else if ((Dw == 40 || Dw == 32)) {
	      	DbwCapAdj[i] = 1;
	      	break;
	     }
	 }
	 break;
      case 1:
        /** 32-bit */
 	 for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
   	   UINT32 Dw;
   	   Dw = Pinfo->DimmParams[i][j].DataWidth;
   	   if (Pinfo->DimmParams[i][j].NRanks
   	     && (Dw == 72 || Dw == 64)) {
   	     /**
   	      FIXME: can't really do it
   	      like this because this just
   	      further reduces the memory
   	     **/
   	     Found = 1;
   	     break;
          }
 	 }
 	 if (Found) {
   	   DbwCapAdj[i] = 1;
   	 }
   	 break;
      case 0:
   	 /** 64-bit */
   	 break;
      default:
        return 1;
    }
  }

  CurrentMemBase = CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY;
  TotalMem = 0;
  if (Pinfo->MemctlOpts[0].MemctlInterleaving) {
    RankDensity = Pinfo->DimmParams[0][0].RankDensity >>
    			DbwCapAdj[0];
    switch (Pinfo->MemctlOpts[0].BaIntlvCtl &
    			FSL_DDR_CS0_CS1_CS2_CS3) {
      case FSL_DDR_CS0_CS1_CS2_CS3:
      	 CtlrDensity = 4 * RankDensity;
    	 break;
      case FSL_DDR_CS0_CS1:
      case FSL_DDR_CS0_CS1_AND_CS2_CS3:
        CtlrDensity = 2 * RankDensity;
      	 break;
      case FSL_DDR_CS2_CS3:
      default:
    	 CtlrDensity = RankDensity;
    	 break;
    }
    for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
      if (Pinfo->MemctlOpts[i].MemctlInterleaving) {
        switch (Pinfo->MemctlOpts[i].MemctlInterleavingMode) {
	   case FSL_DDR_256B_INTERLEAVING:
	   case FSL_DDR_CACHE_LINE_INTERLEAVING:
	   case FSL_DDR_PAGE_INTERLEAVING:
	   case FSL_DDR_BANK_INTERLEAVING:
	   case FSL_DDR_SUPERBANK_INTERLEAVING:
	     TotalCtlrMem = 2 * CtlrDensity;
	     break;
	   case FSL_DDR_3WAY_1KB_INTERLEAVING:
	   case FSL_DDR_3WAY_4KB_INTERLEAVING:
	   case FSL_DDR_3WAY_8KB_INTERLEAVING:
	   	TotalCtlrMem = 3 * CtlrDensity;
	   	break;
	   case FSL_DDR_4WAY_1KB_INTERLEAVING:
	   case FSL_DDR_4WAY_4KB_INTERLEAVING:
	   case FSL_DDR_4WAY_8KB_INTERLEAVING:
	   	TotalCtlrMem = 4 * CtlrDensity;
	   	break;
	 }
	 Pinfo->CommonTimingParams[i].BaseAddress =
				CurrentMemBase;
	 Pinfo->CommonTimingParams[i].TotalMem =
				TotalCtlrMem;
	 TotalMem = CurrentMemBase + TotalCtlrMem;
      } else {
	 /** when 3rd controller not interleaved */
	 CurrentMemBase = TotalMem;
	 TotalCtlrMem = 0;
	 Pinfo->CommonTimingParams[i].BaseAddress =
	 			CurrentMemBase;
	 for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
	   UINTN Cap =
		Pinfo->DimmParams[i][j].Capacity >> DbwCapAdj[i];
	   Pinfo->DimmParams[i][j].BaseAddress =
		CurrentMemBase;
	   CurrentMemBase += Cap;
	   TotalCtlrMem += Cap;
	 }
	 Pinfo->CommonTimingParams[i].TotalMem =
				TotalCtlrMem;
	 TotalMem += TotalCtlrMem;
      }
    }
  } else {
    /**
     Simple linear assignment if memory
     controllers are not interleaved.
    **/
    for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
      TotalCtlrMem = 0;
      Pinfo->CommonTimingParams[i].BaseAddress =
				CurrentMemBase;
      for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
        /** Compute DIMM base addresses. */
	 UINTN Cap =
		Pinfo->DimmParams[i][j].Capacity >> DbwCapAdj[i];
	 Pinfo->DimmParams[i][j].BaseAddress =
		CurrentMemBase;
	 CurrentMemBase += Cap;
	 TotalCtlrMem += Cap;
      }
      Pinfo->CommonTimingParams[i].TotalMem =
				TotalCtlrMem;
      TotalMem += TotalCtlrMem;
    }
  }

  return TotalMem;
}


VOID
CheckInterleavingOptions (
  IN   FslDdrInfoT		*Pinfo
  )
{
  INT32 i, j, k, CheckNRanks, IntlvInvalid = 0;
  UINT32 CheckIntlv, CheckNRowAddr, CheckNColAddr;
  UINTN CheckRankDensity;
  struct DimmParamsS *Dimm;
  /**
   Check if all controllers are Configured for memory
   controller interleaving. Identical dimms are recommended. At least
   the size, row and col address should be checked.
  **/
  j = 0;
  CheckNRanks = Pinfo->DimmParams[0][0].NRanks;
  CheckRankDensity = Pinfo->DimmParams[0][0].RankDensity;
  CheckNRowAddr =  Pinfo->DimmParams[0][0].NRowAddr;
  CheckNColAddr = Pinfo->DimmParams[0][0].NColAddr;
  CheckIntlv = Pinfo->MemctlOpts[0].MemctlInterleavingMode;
  for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
    Dimm = &Pinfo->DimmParams[i][0];
    if (!Pinfo->MemctlOpts[i].MemctlInterleaving) {
      continue;
    } else if (((CheckRankDensity != Dimm->RankDensity) ||
    	     (CheckNRanks != Dimm->NRanks) ||
    	     (CheckNRowAddr != Dimm->NRowAddr) ||
    	     (CheckNColAddr != Dimm->NColAddr) ||
    	     (CheckIntlv !=
    		Pinfo->MemctlOpts[i].MemctlInterleavingMode))){
      IntlvInvalid = 1;
      break;
    } else {
      j++;
    }
  }
  if (IntlvInvalid) {
    for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++)
      Pinfo->MemctlOpts[i].MemctlInterleaving = 0;
  } else {
    switch (CheckIntlv) {
      case FSL_DDR_256B_INTERLEAVING:
      case FSL_DDR_CACHE_LINE_INTERLEAVING:
      case FSL_DDR_PAGE_INTERLEAVING:
      case FSL_DDR_BANK_INTERLEAVING:
      case FSL_DDR_SUPERBANK_INTERLEAVING:
  	 if (3 == CONFIG_NUM_DDR_CONTROLLERS)
  	   k = 2;
  	 else
  	   k = CONFIG_NUM_DDR_CONTROLLERS;
  	 break;
      case FSL_DDR_3WAY_1KB_INTERLEAVING:
      case FSL_DDR_3WAY_4KB_INTERLEAVING:
      case FSL_DDR_3WAY_8KB_INTERLEAVING:
      case FSL_DDR_4WAY_1KB_INTERLEAVING:
      case FSL_DDR_4WAY_4KB_INTERLEAVING:
      case FSL_DDR_4WAY_8KB_INTERLEAVING:
      default:
  	 k = CONFIG_NUM_DDR_CONTROLLERS;
  	 break;
    }
    if (j && (j != k)) {
      for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++)
        Pinfo->MemctlOpts[i].MemctlInterleaving = 0;
    }
  }
}


VOID
FslDdrBoardOptions (
  OUT   MemctlOptionsT	*Popts,
  IN    DimmParamsT 		*Pdimm,
  IN    UINT32 		CtrlNum
 )
{
  CONST struct BoardSpecificParameters *Pbsp, *PbspHighest = NULL;
  UINTN DdrFreq;

  if (CtrlNum > 3) {
    return;
  }
  if (!Pdimm->NRanks)
    return;

  /**
   we use identical timing for all slots. If needed, change the code
   to  Pbsp = RDimm[CtrlNum] or Pbsp = UDimms[CtrlNum];
  **/
  if (Popts->RegisteredDimmEn)
    Pbsp = RDimm[0];
  else
    Pbsp = UDimm[0];

  /**
   Get ClkAdjust, WrlvlStart, wrlvl_ctl, according to the board ddr
   freqency and n_banks specified in BoardSpecificParameters table.
  **/
  DdrFreq = GetDdrFreq() / 1000000;
  while (Pbsp->DatarateMhzHigh) {
    if (Pbsp->NRanks == Pdimm->NRanks &&
      (Pdimm->RankDensity >> 30) >= Pbsp->RankGb) {
      if (DdrFreq <= Pbsp->DatarateMhzHigh) {
        Popts->ClkAdjust = Pbsp->ClkAdjust;
    	 Popts->WrlvlStart = Pbsp->WrlvlStart;
    	 Popts->WrlvlCtl2 = Pbsp->WrlvlCtl2;
    	 Popts->WrlvlCtl3 = Pbsp->WrlvlCtl3;
    	 goto found;
      }
      PbspHighest = Pbsp;
    }
    Pbsp++;
  }

  if (PbspHighest) {
    Popts->ClkAdjust = PbspHighest->ClkAdjust;
    Popts->WrlvlStart = PbspHighest->WrlvlStart;
    Popts->WrlvlCtl2 = Pbsp->WrlvlCtl2;
    Popts->WrlvlCtl3 = Pbsp->WrlvlCtl3;
    } else {
  }

found:
  /**
    Factors to consider for half-strength driver enable:
   	- number of DIMMs installed
  **/
  Popts->HalfStrengthDriverEnable = 0;
  /**
    Write leveling override
  **/
  Popts->WrlvlOverride = 1;
  Popts->WrlvlSample = 0xf;

  /**
    Rtt and Rtt_WR override
  **/
  Popts->RttOverride = 0;

  /** Enable ZQ calibration */
  Popts->ZqEn = 1;

  /** DHC_EN =1, ODT = 75 Ohm */
  Popts->DdrCdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_75ohm);
  Popts->DdrCdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_75ohm);
}


/**
  Automatically select bank interleaving mode based on DIMMs
  in this order: cs0_cs1_cs2_cs3, cs0_cs1, null.
  This function only deal with one or two slots per controller.

  @param   Pdimm		Pointer to Dimm parameters

  @retval  Interleaving type : FSL_DDR_CS0_CS1_CS2_CS3,
				 : FSL_DDR_CS0_CS1
 **/
UINT32
AutoBankIntlv (
  IN   DimmParamsT	*Pdimm
  )
{
  if (Pdimm[0].NRanks == 4)
    return FSL_DDR_CS0_CS1_CS2_CS3;
  else if (Pdimm[0].NRanks == 2)
    return FSL_DDR_CS0_CS1;
  return 0;
}


EFI_STATUS
EFIAPI
HwConfigSubargCmpF (
  IN   CONST CHAR8 	*Opt,
  IN   CONST CHAR8 	*Subopt,
  IN   CONST CHAR8 	*Subarg,
  OUT  INT8 		*Buf
  )
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
HwConfigSubF (
  IN   CONST CHAR8 	*Opt,
  IN   CONST CHAR8 	*Subopt,
  IN   INT8 		*Buf
  )
{
  return EFI_UNSUPPORTED;
}

static CONST struct DynamicOdt DualDD[4] = {
  {	/** cs0 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_SAME_DIMM,
    DDR3_RTT_120_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs1 */
    FSL_DDR_ODT_OTHER_DIMM,
    FSL_DDR_ODT_OTHER_DIMM,
    DDR3_RTT_30_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs2 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_SAME_DIMM,
    DDR3_RTT_120_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs3 */
    FSL_DDR_ODT_OTHER_DIMM,
    FSL_DDR_ODT_OTHER_DIMM,
    DDR3_RTT_30_OHM,
    DDR3_RTT_OFF
  }
};

static CONST struct DynamicOdt DualDS[4] = {
  {	/** cs0 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_SAME_DIMM,
    DDR3_RTT_120_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs1 */
    FSL_DDR_ODT_OTHER_DIMM,
    FSL_DDR_ODT_OTHER_DIMM,
    DDR3_RTT_30_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs2 */
    FSL_DDR_ODT_OTHER_DIMM,
    FSL_DDR_ODT_ALL,
    DDR3_RTT_20_OHM,
    DDR3_RTT_120_OHM
  },
  {0, 0, 0, 0}
};


static CONST struct DynamicOdt DualSD[4] = {
  {	/** cs0 */
    FSL_DDR_ODT_OTHER_DIMM,
    FSL_DDR_ODT_ALL,
    DDR3_RTT_20_OHM,
    DDR3_RTT_120_OHM
  },
  {0, 0, 0, 0},
  {	/** cs2 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_SAME_DIMM,
    DDR3_RTT_120_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs3 */
    FSL_DDR_ODT_OTHER_DIMM,
    FSL_DDR_ODT_OTHER_DIMM,
    DDR3_RTT_20_OHM,
    DDR3_RTT_OFF
  }
};


static CONST struct DynamicOdt DualSS[4] = {
  {	/** cs0 */
    FSL_DDR_ODT_OTHER_DIMM,
    FSL_DDR_ODT_ALL,
    DDR3_RTT_30_OHM,
    DDR3_RTT_120_OHM
  },
  {0, 0, 0, 0},
  {	/** cs2 */
    FSL_DDR_ODT_OTHER_DIMM,
    FSL_DDR_ODT_ALL,
    DDR3_RTT_30_OHM,
    DDR3_RTT_120_OHM
  },
  {0, 0, 0, 0}
};

static CONST struct DynamicOdt DualD0[4] = {
  {	/** cs0 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_SAME_DIMM,
    DDR3_RTT_40_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs1 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_NEVER,
    DDR3_RTT_OFF,
    DDR3_RTT_OFF
  },
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

static CONST struct DynamicOdt Dual0D[4] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {	/** cs2 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_SAME_DIMM,
    DDR3_RTT_40_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs3 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_NEVER,
    DDR3_RTT_OFF,
    DDR3_RTT_OFF
  }
};

static CONST struct DynamicOdt DualS0[4] = {
  {	/** cs0 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_CS,
    DDR3_RTT_40_OHM,
    DDR3_RTT_OFF
  },
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

static CONST struct DynamicOdt Dual0S[4] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {	/** cs2 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_CS,
    DDR3_RTT_40_OHM,
    DDR3_RTT_OFF
  },
  {0, 0, 0, 0}
};

STATIC CONST struct DynamicOdt SingleQ[4] = {
  {	/** cs0 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_CS_AND_OTHER_DIMM,
    DDR3_RTT_20_OHM,
    DDR3_RTT_120_OHM
  },
  {	/** cs1 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_NEVER,
    DDR3_RTT_OFF,
    DDR3_RTT_120_OHM
  },
  {	/** cs2 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_CS_AND_OTHER_DIMM,
    DDR3_RTT_20_OHM,
    DDR3_RTT_120_OHM
  },
  {	/** cs3 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_NEVER,
    DDR3_RTT_OFF,
    DDR3_RTT_120_OHM
  }
};

static CONST struct DynamicOdt SingleD[4] = {
  {	/** cs0 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_ALL,
    DDR3_RTT_40_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs1 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_NEVER,
    DDR3_RTT_OFF,
    DDR3_RTT_OFF
  },
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};


static CONST struct DynamicOdt SingleS[4] = {
  {	/** cs0 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_ALL,
    DDR3_RTT_40_OHM,
    DDR3_RTT_OFF
  },
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
};


STATIC CONST struct DynamicOdt OdtUnknown[4] = {
  {	/** cs0 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_CS,
    DDR3_RTT_120_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs1 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_CS,
    DDR3_RTT_120_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs2 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_CS,
    DDR3_RTT_120_OHM,
    DDR3_RTT_OFF
  },
  {	/** cs3 */
    FSL_DDR_ODT_NEVER,
    FSL_DDR_ODT_CS,
    DDR3_RTT_120_OHM,
    DDR3_RTT_OFF
  }
};

UINT32 SpdI2cAddr[CONFIG_NUM_DDR_CONTROLLERS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
  [0][0] = I2C0_BASE_ADDRESS,
};


/**
  Dividing the initialization to two steps to deassert DDR reset signal
  to comply with JEDEC specs for RDIMMs.

  @param   Regs 		Pointer having to-be-set values for
				DDR controller registers
  @param   CtrlNum 		is the DDR controller number
  @param   Step		0 goes through the initialization in one pass
        			1 sets registers and returns before enabling controller
        			2 resumes from step 1 and continues to initialize

 **/
VOID
FslDdrSetMemctlRegs (
  IN   CONST FslDdrCfgRegsT 	*Regs,
  IN   UINT32 			CtrlNum,
  IN   INT32 				Step
  )
{
  UINT8 i;
  UINT32 TempSdramCfg;
  UINT32 TotalGbSizePerController;
  UINT32 BusWidth, Timeout;
  struct CcsrDdr *Ddr;

  switch (CtrlNum) {
    case 0:
      Ddr = (VOID *)CONFIG_SYS_FSL_DDR_ADDR;
      break;
    default:
      return;
  }

  if (Regs->DdrEor)
    MmioWriteBe32((UINTN)&Ddr->Eor, Regs->DdrEor);

  for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
    if (i == 0) {
      MmioWriteBe32((UINTN)&Ddr->Cs0Bnds, 0x7F);
      MmioWriteBe32((UINTN)&Ddr->Cs0Config, Regs->Cs[i].Config);
      MmioWriteBe32((UINTN)&Ddr->Cs0Config2, Regs->Cs[i].Config2);
    } else if (i == 1) {
      MmioWriteBe32((UINTN)&Ddr->Cs1Bnds, 0x7F);
      MmioWriteBe32((UINTN)&Ddr->Cs1Config, Regs->Cs[i].Config);
      MmioWriteBe32((UINTN)&Ddr->Cs1Config2, Regs->Cs[i].Config2);
    } else if (i == 2) {
      MmioWriteBe32((UINTN)&Ddr->Cs2Bnds, Regs->Cs[i].Bnds);
      MmioWriteBe32((UINTN)&Ddr->Cs2Config, Regs->Cs[i].Config);
      MmioWriteBe32((UINTN)&Ddr->Cs2Config2, Regs->Cs[i].Config2);
    } else if (i == 3) {
      MmioWriteBe32((UINTN)&Ddr->Cs3Bnds, Regs->Cs[i].Bnds);
      MmioWriteBe32((UINTN)&Ddr->Cs3Config, Regs->Cs[i].Config);
      MmioWriteBe32((UINTN)&Ddr->Cs3Config2, Regs->Cs[i].Config2);
    }
  }

  MmioWriteBe32((UINTN)&Ddr->TimingCfg3, Regs->TimingCfg3);
  MmioWriteBe32((UINTN)&Ddr->TimingCfg0, Regs->TimingCfg0);
  MmioWriteBe32((UINTN)&Ddr->TimingCfg1, Regs->TimingCfg1);
  MmioWriteBe32((UINTN)&Ddr->TimingCfg2, Regs->TimingCfg2);
  MmioWriteBe32((UINTN)&Ddr->SdramCfg2, Regs->DdrSdramCfg2);
  MmioWriteBe32((UINTN)&Ddr->SdramMode, Regs->DdrSdramMode);
  MmioWriteBe32((UINTN)&Ddr->SdramMode2, Regs->DdrSdramMode2);
  MmioWriteBe32((UINTN)&Ddr->SdramMode3, Regs->DdrSdramMode3);
  MmioWriteBe32((UINTN)&Ddr->SdramMode4, Regs->DdrSdramMode4);
  MmioWriteBe32((UINTN)&Ddr->SdramMode5, Regs->DdrSdramMode5);
  MmioWriteBe32((UINTN)&Ddr->SdramMode6, Regs->DdrSdramMode6);
  MmioWriteBe32((UINTN)&Ddr->SdramMode7, Regs->DdrSdramMode7);
  MmioWriteBe32((UINTN)&Ddr->SdramMode8, Regs->DdrSdramMode8);
  MmioWriteBe32((UINTN)&Ddr->SdramMdCntl, Regs->DdrSdramMdCntl);
  MmioWriteBe32((UINTN)&Ddr->SdramInterval, Regs->DdrSdramInterval);
  MmioWriteBe32((UINTN)&Ddr->SdramDataInit, Regs->DdrDataInit);
  MmioWriteBe32((UINTN)&Ddr->SdramClkCntl, Regs->DdrSdramClkCntl);
  MmioWriteBe32((UINTN)&Ddr->InitAddr, Regs->DdrInitAddr);
  MmioWriteBe32((UINTN)&Ddr->InitExtAddr, Regs->DdrInitExtAddr);

  MmioWriteBe32((UINTN)&Ddr->TimingCfg4, Regs->TimingCfg4);
  MmioWriteBe32((UINTN)&Ddr->TimingCfg5, Regs->TimingCfg5);
  MmioWriteBe32((UINTN)&Ddr->DdrZqCntl, Regs->DdrZqCntl);
  MmioWriteBe32((UINTN)&Ddr->DdrWrlvlCntl, Regs->DdrWrlvlCntl);

  if (Regs->DdrWrlvlCntl2)
    MmioWriteBe32((UINTN)&Ddr->DdrWrlvlCntl2, Regs->DdrWrlvlCntl2);
  if (Regs->DdrWrlvlCntl3)
    MmioWriteBe32((UINTN)&Ddr->DdrWrlvlCntl3, Regs->DdrWrlvlCntl3);

  MmioWriteBe32((UINTN)&Ddr->DdrSrCntr, Regs->DdrSrCntr);
  MmioWriteBe32((UINTN)&Ddr->DdrSdramRcw1, Regs->DdrSdramRcw1);
  MmioWriteBe32((UINTN)&Ddr->DdrSdramRcw2, Regs->DdrSdramRcw2);
  MmioWriteBe32((UINTN)&Ddr->DdrCdr1, Regs->DdrCdr1);
  MmioWriteBe32((UINTN)&Ddr->DdrCdr2, Regs->DdrCdr2);
  MmioWriteBe32((UINTN)&Ddr->ErrDisable, Regs->ErrDisable);
  MmioWriteBe32((UINTN)&Ddr->ErrIntEn, Regs->ErrIntEn);
  for (i = 0; i < 32; i++) {
    if (Regs->Debug[i]) {
      MmioWriteBe32((UINTN)&Ddr->Debug[i], Regs->Debug[i]);
    }
  }

  /**
   For RDIMMs, JEDEC spec requires clocks to be stable before reset is
   deasserted. Clocks start when any chip select is enabled and clock
   control register is set. Because all DDR components are connected to
   one reset signal, this needs to be done in two steps. Step 1 is to
   get the clocks started. Step 2 resumes after reset signal is
   deasserted.
  **/

  if (Step == 1) {
    MicroSecondDelay(200);
    return;
  }

  /** Set, but do not enable the memory */
  TempSdramCfg = Regs->DdrSdramCfg;
  TempSdramCfg &= ~(SDRAM_CFG_MEM_EN);
  MmioWriteBe32((UINTN)&Ddr->SdramCfg, TempSdramCfg);

  /**
   500 painful micro-seconds must elapse between
   the DDR clock setup and the DDR Config enable.
   DDR2 need 200 us, and DDR3 need 500 us from spec,
   we choose the Max, that is 500 us for all of case.
   **/
  //MicroSecondDelay(500);
  asm volatile("dsb sy;isb");

  /** Let the controller go */
  TempSdramCfg = MmioReadBe32((UINTN)&Ddr->SdramCfg) & ~SDRAM_CFG_BI;
  MmioWriteBe32((UINTN)&Ddr->SdramCfg, TempSdramCfg | SDRAM_CFG_MEM_EN);
  asm volatile("dsb sy;isb");

  TotalGbSizePerController = 0;
  for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
    if (!(Regs->Cs[i].Config & 0x80000000))
      continue;
    TotalGbSizePerController += 1 << (
      ((Regs->Cs[i].Config >> 14) & 0x3) + 2 +
      ((Regs->Cs[i].Config >> 8) & 0x7) + 12 +
      ((Regs->Cs[i].Config >> 0) & 0x7) + 8 +
      3 - ((Regs->DdrSdramCfg >> 19) & 0x3) -
      26);	/** minus 26 (Count of 64M) */
  }
  if (Regs->Cs[0].Config & 0x20000000) {
    /** 2-way interleaving */
    TotalGbSizePerController <<= 1;
  }
  /**
    total memory / bus width = transactions needed
    transactions needed / data rate = seconds
    to add plenty of Buffer, double the time
    For example, 2GB on 666MT/s 64-bit bus takes about 402ms
    Let's wait for 800ms
   **/
  BusWidth = 3 - ((Ddr->SdramCfg & SDRAM_CFG_DBW_MASK)
  		>> SDRAM_CFG_DBW_SHIFT);
  Timeout = ((TotalGbSizePerController << (6 - BusWidth)) * 100 /
  	(GetDdrFreq() >> 20)) << 1;
  TotalGbSizePerController >>= 4;	/** shift down to gb size */

  /** Poll DDR_SDRAM_CFG_2[D_INIT] bit until auto-data init is done. */
  while ((MmioReadBe32((UINTN)&Ddr->SdramCfg2) & SDRAM_CFG2_D_INIT) &&
    (Timeout >= 0)) {
    MicroSecondDelay(10000);	/** throttle polling rate */
    Timeout--;
  }
}

INT32
BoardNeedMemReset (
  VOID
  )
{
  return EFI_SUCCESS;
}


VOID
GetSpd (
  OUT  GenericSpdEepromT		*Spd,
  IN   UINT64				I2cAddress
  )
{
  VOID *Address = (VOID *)I2cAddress;
  UINT32 Ret = I2cRead(Address, EEPROM0_ADDRESS, 0x0, 0x01, (UINT8 *)Spd,
               sizeof(GenericSpdEepromT));

  if (Ret != EFI_SUCCESS) {
    InternalMemZeroMem(Spd, sizeof(GenericSpdEepromT));
  }
}


VOID
FslDdrGetSpd (
  OUT  GenericSpdEepromT 	*CtrlDimmsSpd,
  IN   UINT32 		CtrlNum
  )
{
  UINT32 i;
  UINT64 I2cAddress = 0;

  if (CtrlNum >= CONFIG_NUM_DDR_CONTROLLERS) {
    return;
  }

  for (i = 0; i < CONFIG_DIMM_SLOTS_PER_CTLR; i++) {
    I2cAddress = SpdI2cAddr[CtrlNum][i];
    GetSpd(&(CtrlDimmsSpd[i]), I2cAddress);
  }
}

/**
  CRC16 compute for DDR3 SPD
  Copied from DDR3 SPD spec.
 **/
INT32
Crc16 (
  IN   UINT8		*Ptr,
  IN   INT32		Count
  )
{
  INT32 Crc, i;

  Crc = 0;
  while (--Count >= 0) {
    Crc = Crc ^ (INT32)*Ptr++ << 8;
    for (i = 0; i < 8; ++i)
      if (Crc & 0x8000)
        Crc = Crc << 1 ^ 0x1021;
      else
        Crc = Crc << 1;
  }
  return Crc & 0xffff;
}


UINT32
Ddr3SpdCheck (
  IN   CONST Ddr3SpdEepromT		*Spd
  )
{
  UINT8 *P = (UINT8 *)Spd;
  INT32 Csum16;
  INT32 Len;
  UINT8 CrcLsb;	/** byte 126 */
  UINT8 CrcMsb;	/** byte 127 */

  /**
    SPD byte0[7] - CRC coverage
    0 = CRC covers bytes 0~125
    1 = CRC covers bytes 0~116
  **/

  Len = !(Spd->InfoSizeCrc & 0x80) ? 126 : 117;
  Csum16 = Crc16(P, Len);

  CrcLsb = (UINT8) (Csum16 & 0xff);
  CrcMsb = (UINT8) (Csum16 >> 8);

  if (Spd->Crc[0] == CrcLsb && Spd->Crc[1] == CrcMsb) {
    return 0;
  } else {
    return 1;
  }
}


/**
  Calculate the Density of each Physical Rank.
  Returned size is in bytes.

  each rank size =
  sdram Capacity(bit) / 8 * primary bus width / sdram width

  where: sdram Capacity  = Spd byte4[3:0]
         primary bus width = Spd byte8[2:0]
         sdram width = Spd byte7[2:0]

  SPD byte4 - sdram density and banks
 	bit[3:0]	size(bit)	size(byte)
 	0000		256Mb		32MB
 	0001		512Mb		64MB
 	0010		1Gb		128MB
 	0011		2Gb		256MB
 	0100		4Gb		512MB
 	0101		8Gb		1GB
 	0110		16Gb		2GB

  SPD byte8 - module memory bus width
  	bit[2:0]	primary bus width
 	000		8bits
  	001		16bits
  	010		32bits
  	011		64bits

  SPD byte7 - module organiztion
  	bit[2:0]	sdram device width
  	000		4bits
  	001		8bits
  	010		16bits
  	011		32bits

  @param   Spd		Pointer to SPD settings

 **/
UINTN
ComputeRanksize (
  IN   CONST Ddr3SpdEepromT		*Spd
  )
{
  UINTN Bsize;

  INT32 NbitSdramCapBsize = 0;
  INT32 NbitPrimaryBusWidth = 0;
  INT32 NbitSdramWidth = 0;

  if ((Spd->DensityBanks & 0xf) < 7)
  	NbitSdramCapBsize = (Spd->DensityBanks & 0xf) + 28;
  if ((Spd->BusWidth & 0x7) < 4)
  	NbitPrimaryBusWidth = (Spd->BusWidth & 0x7) + 3;
  if ((Spd->Organization & 0x7) < 4)
  	NbitSdramWidth = (Spd->Organization & 0x7) + 2;

  Bsize = 1ULL << (NbitSdramCapBsize - 3
      + NbitPrimaryBusWidth - NbitSdramWidth);

  return Bsize;
}


/**
  DdrComputeDimmParameters for DDR3 SPD

  Compute DIMM parameters based upon the SPD information in Spd.
  Writes the results to the DimmParamsT structure pointed by Pdimm.

  @param   Spd		Pointer to SPD parameters
  @param   DimmNumber	Dimm number

  @retval  Dimm parameters in Pdimm pointer

 **/
UINT32
DdrComputeDimmParameters (
  IN   CONST Ddr3SpdEepromT		*Spd,
  OUT  DimmParamsT			*Pdimm,
  IN   UINT32				DimmNumber
  )
{
  UINT32 Retval;
  UINT32 MtbPs;
  INT32 Ftb10thPs;
  INT32 i;

  if (Spd->MemType) {
    if (Spd->MemType != SPD_MEMTYPE_DDR3) {
      return 1;
    }
  } else {
    InternalMemZeroMem(Pdimm, sizeof(DimmParamsT));
    return 1;
  }

  Retval = Ddr3SpdCheck(Spd);
  if (Retval) {
    return 2;
  }

  /**
    The part name in ASCII in the SPD EEPROM is not null terminated.
    Guarantee null termination here by presetting all bytes to 0
    and copying the part name in ASCII from the SPD onto it
   **/
  InternalMemZeroMem(Pdimm->Mpart, sizeof(Pdimm->Mpart));
  if ((Spd->InfoSizeCrc & 0xF) > 1)
    InternalMemCopyMem(Pdimm->Mpart, Spd->Mpart, sizeof(Pdimm->Mpart) - 1);

  /** DIMM Organization parameters */
  Pdimm->NRanks = ((Spd->Organization >> 3) & 0x7) + 1;
  Pdimm->RankDensity = ComputeRanksize(Spd);
  Pdimm->Capacity = Pdimm->NRanks * Pdimm->RankDensity;
  Pdimm->PrimarySdramWidth = 1 << (3 + (Spd->BusWidth & 0x7));
  if ((Spd->BusWidth >> 3) & 0x3)
    Pdimm->EcSdramWidth = 8;
  else
    Pdimm->EcSdramWidth = 0;
  Pdimm->DataWidth = Pdimm->PrimarySdramWidth
  		  + Pdimm->EcSdramWidth;
  Pdimm->DeviceWidth = 1 << ((Spd->Organization & 0x7) + 2);

  /** These are the types defined by the JEDEC DDR3 SPD spec */
  Pdimm->MirroredDimm = 0;
  Pdimm->RegisteredDimm = 0;
  switch (Spd->ModuleType & DDR3_SPD_MODULETYPE_MASK) {
    case DDR3_SPD_MODULETYPE_RDIMM:
    case DDR3_SPD_MODULETYPE_MINI_RDIMM:
    case DDR3_SPD_MODULETYPE_72B_SO_RDIMM:
      /** Registered/Buffered DIMMs */
      Pdimm->RegisteredDimm = 1;
      for (i = 0; i < 16; i += 2) {
  	 UINT8 Rcw = Spd->ModSection.Registered.Rcw[i/2];
  	 Pdimm->Rcw[i]   = (Rcw >> 0) & 0x0F;
  	 Pdimm->Rcw[i+1] = (Rcw >> 4) & 0x0F;
      }
      break;
    case DDR3_SPD_MODULETYPE_UDIMM:
    case DDR3_SPD_MODULETYPE_SO_DIMM:
    case DDR3_SPD_MODULETYPE_MICRO_DIMM:
    case DDR3_SPD_MODULETYPE_MINI_UDIMM:
    case DDR3_SPD_MODULETYPE_MINI_CDIMM:
    case DDR3_SPD_MODULETYPE_72B_SO_UDIMM:
    case DDR3_SPD_MODULETYPE_72B_SO_CDIMM:
    case DDR3_SPD_MODULETYPE_LRDIMM:
    case DDR3_SPD_MODULETYPE_16B_SO_DIMM:
    case DDR3_SPD_MODULETYPE_32B_SO_DIMM:
      /** UnBuffered DIMMs */
      if (Spd->ModSection.Unbuffered.AddrMapping & 0x1)
        Pdimm->MirroredDimm = 1;
      break;
    default:
      return 1;
  }

  /** SDRAM device parameters */
  Pdimm->NRowAddr = ((Spd->Addressing >> 3) & 0x7) + 12;
  Pdimm->NColAddr = (Spd->Addressing & 0x7) + 9;
  Pdimm->NBanksPerSdramDevice = 8 << ((Spd->DensityBanks >> 4) & 0x7);

  /**
    The SPD spec has not the ECC bit,
    We consider the DIMM as ECC capability
    when the extension bus exist
   **/
  if (Pdimm->EcSdramWidth)
    Pdimm->EdcConfig = 0x02;
  else
    Pdimm->EdcConfig = 0x00;

  /**
    The SPD spec has not the burst length byte
    but DDR3 spec has nature BL8 and BC4,
    BL8 -bit3, BC4 -bit2
   **/
  Pdimm->BurstLengthsBitmask = 0x0c;
  Pdimm->RowDensity = __ILog2(Pdimm->RankDensity);

  /** MTB - medium timebase
    The unit in the SPD spec is ns,
    We convert it to ps.
    eg: MTB = 0.125ns (125ps)
   **/
  MtbPs = (Spd->MtbDividend * 1000) /Spd->MtbDivisor;
  Pdimm->MtbPs = MtbPs;

  /**
    FTB - fine timebase
    use 1/10th of ps as our unit to aVOID floating poINT32
    eg, 10 for 1ps, 25 for 2.5ps, 50 for 5ps
  **/
  Ftb10thPs =
  	((Spd->FtbDiv & 0xf0) >> 4) * 10 / (Spd->FtbDiv & 0x0f);
  Pdimm->Ftb10thPs = Ftb10thPs;
  /**
    sdram minimum cycle time
    we assume the MTB is 0.125ns
    eg:
    TckMin=15 MTB (1.875ns) ->DDR3-1066
           =12 MTB (1.5ns) ->DDR3-1333
           =10 MTB (1.25ns) ->DDR3-1600
   **/
  Pdimm->TckminXPs = Spd->TckMin * MtbPs +
  	(Spd->FineTckMin * Ftb10thPs) / 10;

  /**
    CAS latency supported
    bit4 - CL4
    bit5 - CL5
    bit18 - CL18
   **/
  Pdimm->CaslatX  = ((Spd->CaslatMsb << 8) | Spd->CaslatLsb) << 4;

  /**
    Min CAS latency time
    eg: TaaMin =
    DDR3-800D	100 MTB (12.5ns)
    DDR3-1066F	105 MTB (13.125ns)
    DDR3-1333H	108 MTB (13.5ns)
    DDR3-1600H	90 MTB (11.25ns)
   **/
  Pdimm->TaaPs = Spd->TaaMin * MtbPs +
  	(Spd->FineTaaMin * Ftb10thPs) / 10;

  /**
    Min write recovery time
    eg:
    TwrMin = 120 MTB (15ns) -> all speed grades.
   **/
  Pdimm->TwrPs = Spd->TwrMin * MtbPs;

  /**
    Min RAS to CAS delay time
    eg: TrcdMin =
    DDR3-800	100 MTB (12.5ns)
    DDR3-1066F	105 MTB (13.125ns)
    DDR3-1333H	108 MTB (13.5ns)
    DDR3-1600H	90 MTB (11.25)
   **/
  Pdimm->TrcdPs = Spd->TrcdMin * MtbPs +
  	(Spd->FineTrcdMin * Ftb10thPs) / 10;

  /**
    Min row active to row active delay time
    eg: TrrdMin =
    DDR3-800(1KB page)	80 MTB (10ns)
    DDR3-1333(1KB page)	48 MTB (6ns)
   **/
  Pdimm->TrrdPs = Spd->TrrdMin * MtbPs;

  /**
    Min row precharge delay time
    eg: TrpMin =
    DDR3-800D	100 MTB (12.5ns)
    DDR3-1066F	105 MTB (13.125ns)
    DDR3-1333H	108 MTB (13.5ns)
    DDR3-1600H	90 MTB (11.25ns)
   **/
  Pdimm->TrpPs = Spd->TrpMin * MtbPs +
	(Spd->FineTrpMin * Ftb10thPs) / 10;

  /**
    Min active to precharge delay time
    eg: tRAS_min =
    DDR3-800D	300 MTB (37.5ns)
    DDR3-1066F	300 MTB (37.5ns)
    DDR3-1333H	288 MTB (36ns)
    DDR3-1600H	280 MTB (35ns)
   **/
  Pdimm->TrasPs = (((Spd->TrasTrcExt & 0xf) << 8) | Spd->TrasMinLsb)
  		* MtbPs;
  /**
    Min active to actice/refresh delay time
    eg: tRC_min =
    DDR3-800D	400 MTB (50ns)
    DDR3-1066F	405 MTB (50.625ns)
    DDR3-1333H	396 MTB (49.5ns)
    DDR3-1600H	370 MTB (46.25ns)
   **/
  Pdimm->TrcPs = (((Spd->TrasTrcExt & 0xf0) << 4) | Spd->TrcMinLsb)
  		* MtbPs + (Spd->FinetrcMin * Ftb10thPs) / 10;
  /**
    Min refresh recovery delay time
    eg: tRFC_min =
    512Mb	720 MTB (90ns)
    1Gb		880 MTB (110ns)
    2Gb		1280 MTB (160ns)
   **/
  Pdimm->TrfcPs = ((Spd->TrfcMinMsb << 8) | Spd->TrfcMinLsb)
  		* MtbPs;
  /**
    Min INT32ernal write to read command delay time
    eg: TwtrMin = 40 MTB (7.5ns) - all speed bins.
    tWRT is at least 4 mclk independent of operating freq.
   **/
  Pdimm->TwtrPs = Spd->TwtrMin * MtbPs;

  /**
    Min INT32ernal read to precharge command delay time
    eg: TrtpMin = 40 MTB (7.5ns) - all speed bins.
    tRTP is at least 4 mclk independent of operating freq.
   **/
  Pdimm->TrtpPs = Spd->TrtpMin * MtbPs;

  /**
    Average periodic refresh INT32erval
    tREFI = 7.8 us at normal temperature range
          = 3.9 us at ext temperature range
   **/
  Pdimm->RefreshRatePs = 7800000;
  if ((Spd->ThermRefOpt & 0x1) && !(Spd->ThermRefOpt & 0x2)) {
    Pdimm->RefreshRatePs = 3900000;
    Pdimm->ExtendedOpSrt = 1;
  }

  /**
    Min four active window delay time
    eg: TfawMin =
    DDR3-800(1KB page)	320 MTB (40ns)
    DDR3-1066(1KB page)	300 MTB (37.5ns)
    DDR3-1333(1KB page)	240 MTB (30ns)
    DDR3-1600(1KB page)	240 MTB (30ns)
   **/
  Pdimm->TfawPs = (((Spd->TfawMsb & 0xf) << 8) | Spd->TfawMin)
  		* MtbPs;

  return 0;
}


#ifdef CONFIG_SYS_DDR_RAW_TIMING
DimmParamsT DdrRawTiming = {
  .NRanks = 2,
  .RankDensity = 1073741824u,
  .Capacity = 2147483648,
  .PrimarySdramWidth = 64,
  .EcSdramWidth = 0,
  .RegisteredDimm = 0,
  .MirroredDimm = 0,
  .NRowAddr = 14,
  .NColAddr = 10,
  .NBanksPerSdramDevice = 8,
  .EdcConfig = 0,
  .BurstLengthsBitmask = 0x0c,
  .TckminXPs = 937,
  .CaslatX = 0x6FC << 4,  /** 14,13,11,10,9,8,7,6 */
  
  .TaaPs = 13090,
  .TwrPs = 15000,
  .TrcdPs = 13090,
  .TrrdPs = 5000,
  .TrpPs = 13090,
  .TrasPs = 33000,
  .TrcPs = 46090,
  .TrfcPs = 160000,
  .TwtrPs = 7500,
  .TrtpPs = 7500,
  .RefreshRatePs = 7800000,
  .TfawPs = 25000,
};


INT32
FslDdrGetDimmParams (
  OUT  DimmParamsT		*Pdimm,
  IN   UINT32 		ControllerNumber,
  IN   UINT32 		DimmNumber
  )
{
  CONST UINT8 DimmModel[] = "Fixed DDR on board";

  if (((ControllerNumber == 0) && (DimmNumber == 0)) ||
      ((ControllerNumber == 1) && (DimmNumber == 0))) {
    InternalMemCopyMem(Pdimm, &DdrRawTiming, sizeof(DimmParamsT));
    InternalMemZeroMem(Pdimm->Mpart, sizeof(sizeof(Pdimm->Mpart)));
    InternalMemCopyMem(Pdimm->Mpart, DimmModel, sizeof(DimmModel) - 1);
  }

  return 0;
}
#endif

/**
  Round up MclkPs to nearest 1 ps in memory controller code
  if the error is 0.5ps or more.
 
  If an imprecise data rate is too high due to rounding error
  propagation, compute a suitably rounded MclkPs to compute
  a working memory controller Configuration.

  @retval  memory clock in picosecond

**/
UINT32
GetMemoryClkPeriodPs (
  VOID
  )
{
  UINT32 DataRate = 0x0;/*0*/
  UINT32 Result;

  /** Round to nearest 10ps, being careful about 64-bit multiply/divide */
  UINTN Rem, MclkPs = ULL_2E12;

  /** Now perform the big divide, the result fits in 32-bits */
  Rem = DoDiv(MclkPs, DataRate);
  Result = (Rem >= (DataRate >> 1)) ? MclkPs + 1 : MclkPs;

  return Result;
}



UINT32
ComputeCasLatencyDdr3 (
  IN   CONST DimmParamsT 		*DimmParams,
  OUT  CommonTimingParamsT 		*Outpdimm,
  IN   UINT32 			NumberOfDimms
  )
{
  UINT32 i;
  UINT32 TaaminPs = 0;
  UINT32 TckminXPs = 0;
  UINT32 CommonCaslat;
  UINT32 CaslatActual;
  UINT32 Retry = 16;
  UINT32 Tmp;
  CONST UINT32 MclkPs = GetMemoryClkPeriodPs();

  /** compute the common CAS latency supported between slots */
  Tmp = DimmParams[0].CaslatX;
  for (i = 1; i < NumberOfDimms; i++) {
    if (DimmParams[i].NRanks)
      Tmp &= DimmParams[i].CaslatX;
    }
  CommonCaslat = Tmp;

  /** compute the Max tAAmin tCKmin between slots */
  for (i = 0; i < NumberOfDimms; i++) {
    TaaminPs = Max(TaaminPs, DimmParams[i].TaaPs);
    TckminXPs = Max(TckminXPs, DimmParams[i].TckminXPs);
  }
  /** determine the acutal cas latency */
  CaslatActual = (TaaminPs + MclkPs - 1) / MclkPs;
  /** check if the dimms support the CAS latency */
  while (!(CommonCaslat & (1 << CaslatActual)) && Retry > 0) {
    CaslatActual++;
    Retry--;
  }
  /**
    once the caculation of CaslatActual is completed
    we must verify that this CAS latency value does not
    exceed tAAmax, which is 20 ns for all DDR3 speed grades
  **/
  Outpdimm->LowestCommonSPDCaslat = CaslatActual;

  return 0;
}

UINT32
MclkToPicos (
  IN   UINT32   	Mclk
  )
{
  return GetMemoryClkPeriodPs() * Mclk;
}


/**
  Determine the worst-case DIMM timing parameters from the set of DIMMs
  whose parameters have been computed into the array pointed to
  by DimmParams.

  @param   DimmParams	Pointer to Dimm parameters
  @param   NumberOfDimms	no. of dimms

  @retval  Worst case dimm parameters in pointer Outpdimm

 **/
UINT32
ComputeLowestCommonDimmParameters (
  IN   CONST DimmParamsT 		*DimmParams,
  OUT  CommonTimingParamsT		*Outpdimm,
  IN   CONST UINT32 			NumberOfDimms
  )
{
  UINT32 i, j;

  UINT32 TckminXPs = 0;
  UINT32 TckmaxPs = 0xFFFFFFFF;
  UINT32 TckmaxMaxPs = 0;
  UINT32 TrcdPs = 0;
  UINT32 TrpPs = 0;
  UINT32 TrasPs = 0;
  UINT32 TwrPs = 0;
  UINT32 TwtrPs = 0;
  UINT32 TrfcPs = 0;
  UINT32 TrrdPs = 0;
  UINT32 TrcPs = 0;
  UINT32 RefreshRatePs = 0;
  UINT32 ExtendedOpSrt = 1;
  UINT32 TisPs = 0;
  UINT32 TihPs = 0;
  UINT32 TdsPs = 0;
  UINT32 TdhPs = 0;
  UINT32 TrtpPs = 0;
  UINT32 TdqsqMaxPs = 0;
  UINT32 TqhsPs = 0;

  UINT32 Temp1, Temp2;
  UINT32 AdditiveLatency = 0;

  Temp1 = 0;
  for (i = 0; i < NumberOfDimms; i++) {
    /**
     If there are no ranks on this DIMM,
     it probably doesn't exist, so skip it.
    **/
    if (DimmParams[i].NRanks == 0) {
      Temp1++;
      continue;
    }
    if (DimmParams[i].NRanks == 4 && i != 0) {
      Temp1++;
      continue;
    }

    /**
     check if quad-rank DIMM is plugged if
     CONFIG_CHIP_SELECT_QUAD_CAPABLE is not defined
     Only the board with proper design is capable
    **/
    if (DimmParams[i].NRanks == 4 && \
 	  CONFIG_CHIP_SELECTS_PER_CTRL/CONFIG_DIMM_SLOTS_PER_CTLR < 4) {
      Temp1++;
      continue;
    }
    /**
      Find minimum TckmaxPs to find fastest slow speed,
      i.e., this is the slowest the whole system can go.
    **/
    TckmaxPs = Min(TckmaxPs, DimmParams[i].TckmaxPs);

    /**
      Either find maximum value to determine slowest
      speed, delay, time, period, etc
    **/
    TckminXPs = Max(TckminXPs, DimmParams[i].TckminXPs);
    TckmaxMaxPs = Max(TckmaxMaxPs, DimmParams[i].TckmaxPs);
    TrcdPs = Max(TrcdPs, DimmParams[i].TrcdPs);
    TrpPs = Max(TrpPs, DimmParams[i].TrpPs);
    TrasPs = Max(TrasPs, DimmParams[i].TrasPs);
    TwrPs = Max(TwrPs, DimmParams[i].TwrPs);
    TwtrPs = Max(TwtrPs, DimmParams[i].TwtrPs);
    TrfcPs = Max(TrfcPs, DimmParams[i].TrfcPs);
    TrrdPs = Max(TrrdPs, DimmParams[i].TrrdPs);
    TrcPs = Max(TrcPs, DimmParams[i].TrcPs);
    TisPs = Max(TisPs, DimmParams[i].TisPs);
    TihPs = Max(TihPs, DimmParams[i].TihPs);
    TdsPs = Max(TdsPs, DimmParams[i].TdsPs);
    TdhPs = Max(TdhPs, DimmParams[i].TdhPs);
    TrtpPs = Max(TrtpPs, DimmParams[i].TrtpPs);
    TqhsPs = Max(TqhsPs, DimmParams[i].TqhsPs);
    RefreshRatePs = Max(RefreshRatePs,
    		      DimmParams[i].RefreshRatePs);
    /** ExtendedOpSrt is either 0 or 1, 0 having priority */
    ExtendedOpSrt = Min(ExtendedOpSrt,
	    		  DimmParams[i].ExtendedOpSrt);

    /**
      Find maximum TdqsqMaxPs to find slowest.

      FIXME: is finding the slowest value the correct
      strategy for this parameter?
    **/
    TdqsqMaxPs = Max(TdqsqMaxPs, DimmParams[i].TdqsqMaxPs);
  }

  Outpdimm->NdimmsPresent = NumberOfDimms - Temp1;

  if (Temp1 == NumberOfDimms) {
    return 0;
  }

  Outpdimm->TckminXPs = TckminXPs;
  Outpdimm->TckmaxPs = TckmaxPs;
  Outpdimm->TckmaxMaxPs = TckmaxMaxPs;
  Outpdimm->TrcdPs = TrcdPs;
  Outpdimm->TrpPs = TrpPs;
  Outpdimm->TrasPs = TrasPs;
  Outpdimm->TwrPs = TwrPs;
  Outpdimm->TwtrPs = TwtrPs;
  Outpdimm->TrfcPs = TrfcPs;
  Outpdimm->TrrdPs = TrrdPs;
  Outpdimm->TrcPs = TrcPs;
  Outpdimm->RefreshRatePs = RefreshRatePs;
  Outpdimm->ExtendedOpSrt = ExtendedOpSrt;
  Outpdimm->TisPs = TisPs;
  Outpdimm->TihPs = TihPs;
  Outpdimm->TdsPs = TdsPs;
  Outpdimm->TdhPs = TdhPs;
  Outpdimm->TrtpPs = TrtpPs;
  Outpdimm->TdqsqMaxPs = TdqsqMaxPs;
  Outpdimm->TqhsPs = TqhsPs;

  /** Determine common burst length for all DIMMs. */
  Temp1 = 0xff;
  for (i = 0; i < NumberOfDimms; i++) {
    if (DimmParams[i].NRanks) {
      Temp1 &= DimmParams[i].BurstLengthsBitmask;
    }
  }
  Outpdimm->AllDimmsBurstLengthsBitmask = Temp1;

  /** Determine if all DIMMs Registered Buffered. */
  Temp1 = Temp2 = 0;
  for (i = 0; i < NumberOfDimms; i++) {
    if (DimmParams[i].NRanks) {
      if (DimmParams[i].RegisteredDimm) {
        Temp1 = 1;
      } else {
        Temp2 = 1;
      }
    }
  }

  Outpdimm->AllDimmsRegistered = 0;
  Outpdimm->AllDimmsUnbuffered = 0;
  if (Temp1 && !Temp2) {
    Outpdimm->AllDimmsRegistered= 1;
  } else if (!Temp1 && Temp2) {
    Outpdimm->AllDimmsUnbuffered = 1;
  }

  Temp1 = 0;
  if (Outpdimm->AllDimmsRegistered)
    for (j = 0; j < 16; j++) {
      Outpdimm->Rcw[j] = DimmParams[0].Rcw[j];
      for (i = 1; i < NumberOfDimms; i++) {
  	 if (!DimmParams[i].NRanks)
  	   continue;
  	 if (DimmParams[i].Rcw[j] != DimmParams[0].Rcw[j]) {
  	   Temp1 = 1;
  	   break;
  	 }
      }
    }

  if (ComputeCasLatencyDdr3(DimmParams, Outpdimm, NumberOfDimms))
    return 1;

  /** Determine if all DIMMs ECC capable. */
  Temp1 = 1;
  for (i = 0; i < NumberOfDimms; i++) {
    if (DimmParams[i].NRanks &&
     	!(DimmParams[i].EdcConfig & EDC_ECC)) {
      Temp1 = 0;
      break;
    }
  }
  Outpdimm->AllDimmsEccCapable = Temp1;

  /**
   Compute additive latency.

   For DDR1, additive latency should be 0.

   For DDR2, with ODT enabled, use "a value" less than ACTTORW,
     which comes from Trcd, and also note that:
         add_lat + caslat must be >= 4

   For DDR3, we use the AL=0

   When to use additive latency for DDR2:

   I. Because you are using CL=3 and need to do ODT on writes and
      want functionality.
      1. Are you going to use ODT? (Does your board not have
        additional termination circuitry for DQ, DQS, DQS_,
        DM, RDQS, RDQS_ for x4/x8 Configs?)
      2. If so, is your lowest supported CL going to be 3?
      3. If so, then you must set AL=1 because

         WL >= 3 for ODT on writes
         RL = AL + CL
         WL = RL - 1
         ->
         WL = AL + CL - 1
         AL + CL - 1 >= 3
         AL + CL >= 4
         QED

         RL >= 3 for ODT on reads
         RL = AL + CL

         Since CL aren't usually less than 2, AL=0 is a minimum,
         so the WL-derived AL should be the  -- FIXME?

   II. Because you are using auto-precharge globally and want to
       use additive latency (posted CAS) to get more bandwidth.
       1. Are you going to use auto-precharge mode globally?

          Use addtivie latency and compute AL to be 1 cycle less than
          tRCD, i.e. the READ or WRITE command is in the cycle
          immediately following the ACTIVATE command..

   III. Because you feel like it or want to do some sort of
	  degraded-performance experiment.
	 1.  Do you just want to use additive latency because you feel
	     like it?

	Validation:  AL is less than tRCD, and within the other
	read-to-precharge CONSTraINT32s.
  **/

  AdditiveLatency = 0;

  /**
    The system will not use the global auto-precharge mode.
    However, it uses the page mode, so we set AL=0
  **/
  AdditiveLatency = 0;

  /**
    Validate additive latency
    FIXME: move to somewhere else to validate

    AL <= tRCD(Min)
  **/
  if (MclkToPicos(AdditiveLatency) > TrcdPs) {
    return 1;
  }

  /**
    RL = CL + AL;  RL >= 3 for ODT_RD_CFG to be enabled
    WL = RL - 1;  WL >= 3 for ODT_WL_CFG to be enabled
    ADD_LAT (the register) must be set to a value less
    than ACTTORW if WL = 1, then AL must be set to 1
    RD_TO_PRE (the register) must be set to a minimum
    tRTP + AL if AL is nonzero
  **/

  /**
    Additive latency will be applied only if the memctl option to
    use it.
  **/
  Outpdimm->AdditiveLatency = AdditiveLatency;

  return 0;
}


UINT32
PopulateMemctlOptions (
  IN   INT32 		AllDimmsRegistered,
  OUT  MemctlOptionsT 	*Popts,
  IN   DimmParamsT 	*Pdimm,
  IN   UINT32 		CtrlNum
  )
{
  UINT32 i;
  INT8 *Buf = NULL;
  CONST struct DynamicOdt *pdodt = OdtUnknown;
  UINTN DdrFreq;

  /**
    TODO Extract hwConfig from environment since we have not properly setup
    the environment but need it for ddr Config params
  **/

  /** Chip select options. */
  if (CONFIG_DIMM_SLOTS_PER_CTLR == 1) {
    switch (Pdimm[0].NRanks) {
      case 1:
        pdodt = SingleS;
  	 break;
      case 2:
        pdodt = SingleD;
  	 break;
      case 4:
        pdodt = SingleQ;
  	 break;
    }
  } else if (CONFIG_DIMM_SLOTS_PER_CTLR == 2) {
    switch (Pdimm[0].NRanks) {
      case 2:
	 switch (Pdimm[1].NRanks) {
	   case 2:
	     pdodt = DualDD;
	     break;
	   case 1:
	     pdodt = DualDS;
	     break;
	   case 0:
	     pdodt = DualD0;
	     break;
	 }
	 break;
      case 1:
        switch (Pdimm[1].NRanks) {
	   case 2:
	     pdodt = DualSD;
	     break;
	   case 1:
	     pdodt = DualSS;
	     break;
	   case 0:
	     pdodt = DualS0;
	     break;
	 }
	 break;
      case 0:
	 switch (Pdimm[1].NRanks) {
	   case 2:
	     pdodt = Dual0D;
	     break;
	   case 1:
	     pdodt = Dual0S;
	     break;
	 }
	 break;
    }
  }

  /** Pick chip-select local options. */
  for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
    Popts->CsLocalOpts[i].OdtRdCfg = pdodt[i].OdtRdCfg;
    Popts->CsLocalOpts[i].OdtWrCfg = pdodt[i].OdtWrCfg;
    Popts->CsLocalOpts[i].OdtRttNorm = pdodt[i].OdtRttNorm;
    Popts->CsLocalOpts[i].OdtRttWr = pdodt[i].OdtRttWr;
    Popts->CsLocalOpts[i].AutoPrecharge = 0;
  }

  /** Pick interleaving mode. */

  /**
    0 = no interleaving
    1 = interleaving between 2 controllers
  **/
  Popts->MemctlInterleaving = 0;

  /**
    0 = cacheline
    1 = page
    2 = (logical) bank
    3 = superbank (only if CS interleaving is enabled)
  **/
  Popts->MemctlInterleavingMode = 0;

  /**
    0: cacheline: bit 30 of the 36-bit physical addr selects the memctl
    1: page:      bit to the left of the column bits selects the memctl
    2: bank:      bit to the left of the bank bits selects the memctl
    3: superbank: bit to the left of the chip select selects the memctl

   NOTE: ba_intlv (rank interleaving) is independent of memory
   controller interleaving; it is only within a memory controller.
   Must use superbank interleaving if rank interleaving is used and
   memory controller interleaving is enabled.
  **/

  /*
   0 = no
   0x40 = CS0,CS1
   0x20 = CS2,CS3
   0x60 = CS0,CS1 + CS2,CS3
   0x04 = CS0,CS1,CS2,CS3
  **/
  Popts->BaIntlvCtl = 0;

  /** Memory Organization Parameters */
  Popts->RegisteredDimmEn = AllDimmsRegistered;

  /** Operational Mode Paramters */

  /** Pick ECC modes */
  Popts->EccMode = 0;
  Popts->EccInitUsingMemctl = 1; /* 0 = use DMA, 1 = use memctl */

  /**
    Choose DQS Config
    0 for DDR1
    1 for DDR2 and DDR3
  **/
  Popts->DqsConfig = 1;

  /** Choose self-refresh during sleep. */
  Popts->SelfRefreshInSleep = 1;

  /** Choose dynamic power management mode. */
  Popts->DynamicPower = 0;

  /**
    check first Dimm for primary sdram width
    presuming all dimms are similar
    0 = 64-bit, 1 = 32-bit, 2 = 16-bit
  **/
  if (Pdimm[0].NRanks != 0) {
    if (Pdimm[0].PrimarySdramWidth == 64)
      Popts->DataBusWidth = 0;
    else if (Pdimm[0].PrimarySdramWidth == 32)
      Popts->DataBusWidth = 1;
    else if (Pdimm[0].PrimarySdramWidth == 16)
      Popts->DataBusWidth = 2;
  }

  Popts->X4En = (Pdimm[0].DeviceWidth == 4) ? 1 : 0;

  /** Choose burst length. */
  if ((Popts->DataBusWidth == 1) || (Popts->DataBusWidth == 2)) {
    /** 32-bit or 16-bit bus */
    Popts->OtfBurstChopEn = 0;
    Popts->BurstLength = DDR_BL8;
  } else {
    Popts->OtfBurstChopEn = 1;	/** on-the-fly burst chop */
    Popts->BurstLength = DDR_OTF;	/** on-the-fly BC4 and BL8 */
  }

  /** Choose ddr controller address mirror mode */
  Popts->MirroredDimm = Pdimm[0].MirroredDimm;


  /** Pick a caslat override. */
  Popts->CasLatencyOverride = 0;
  Popts->CasLatencyOverrideValue = 3;

  /** Decide whether to use the computed derated latency */
  Popts->UseDeratedCaslat = 0;

  /** Choose an additive latency. */
  Popts->AdditiveLatencyOverride = 0;
  Popts->AdditiveLatencyOverrideValue = 3;

  /**
    2T_EN setting

    Factors to consider for 2T_EN:
   	- number of DIMMs installed
   	- number of components, number of active ranks
   	- how much time you want to spend playing around
  **/
  Popts->TwotEn = 0;
  Popts->ThreetEn = 0;

  /** for RDIMM, address parity enable */
  Popts->ApEn = 1;

  /**
    BSTTOPRE precharge INT32erval

    Set this to 0 for global auto precharge

    FIXME: Should this be Configured in picoseconds?
    Why it should be in ps:  better understanding of this
    relative to actual DRAM timing parameters such as tRAS.
    e.g. tRAS(Min) = 40 ns
  **/
  Popts->Bstopre = 0x100;

  /** Minimum CKE pulse width -- tCKE(MIN) */
  Popts->TckeClockPulseWidthPs
  	= MclkToPicos(FSL_DDR_MIN_TCKE_PULSE_WIDTH_DDR);

  /**
    Window for four activates -- tFAW

    FIXME: UM: applies only to DDR2/DDR3 with eight logical banks only
    FIXME: varies depending upon number of column addresses or data
    FIXME: width, was considering looking at Pdimm->PrimarySdramWidth
  **/
  Popts->TfawWindowFourActivatesPs = Pdimm[0].TfawPs;
  Popts->ZqEn = 0;
  Popts->WrlvlEn = 0;
  /**
    due to ddr3 Dimm is fly-by topology
    we suggest to enable write leveling to
    meet the tQDSS under different loading.
  **/
  Popts->WrlvlEn = 1;
  Popts->ZqEn = 1;
  Popts->WrlvlOverride = 0;

  /**
    Check interleaving Configuration from environment.

    If memory controller interleaving is enabled, then the data
    bus widths must be programmed identically for all memory controllers.

    Attempt to set all controllers to the same chip select
    interleaving mode. It will do a best effort to get the
    requested ranks interleaved together such that the result
    should be a subset of the requested Configuration.

    if CONFIG_SYS_FSL_DDR_INTLV_256B is defined, mandatory interleaving
    with 256 Byte is enabled.
  **/

  if ((HwConfigSubF("fsl_ddr", "bank_intlv", Buf)) &&
	(CONFIG_CHIP_SELECTS_PER_CTRL > 1)) {
    /**
      test null first. if CONFIG_HWCONFIG is not defined,
      HwConfigSubargCmpF returns non-zero
    **/
    if (HwConfigSubargCmpF("fsl_ddr", "bank_intlv",
  			    "null", Buf))
      Popts->BaIntlvCtl = 0;
    else if (HwConfigSubargCmpF("fsl_ddr", "bank_intlv",
  				 "cs0_cs1", Buf))
      Popts->BaIntlvCtl = FSL_DDR_CS0_CS1;
    else if (HwConfigSubargCmpF("fsl_ddr", "bank_intlv",
  				 "cs2_cs3", Buf))
      Popts->BaIntlvCtl = FSL_DDR_CS2_CS3;
    else if (HwConfigSubargCmpF("fsl_ddr", "bank_intlv",
  				 "cs0_cs1_and_cs2_cs3", Buf))
      Popts->BaIntlvCtl = FSL_DDR_CS0_CS1_AND_CS2_CS3;
    else if (HwConfigSubargCmpF("fsl_ddr", "bank_intlv",
					 "cs0_cs1_cs2_cs3", Buf))
      Popts->BaIntlvCtl = FSL_DDR_CS0_CS1_CS2_CS3;
    else if (HwConfigSubargCmpF("fsl_ddr", "bank_intlv",
					"auto", Buf))
      Popts->BaIntlvCtl = AutoBankIntlv(Pdimm);

    Popts->BaIntlvCtl = FSL_DDR_CS0_CS1; /*FIXME*/

    switch (Popts->BaIntlvCtl & FSL_DDR_CS0_CS1_CS2_CS3) {
      case FSL_DDR_CS0_CS1_CS2_CS3:
	 if (Pdimm[0].NRanks < 4) {
	   Popts->BaIntlvCtl = 0;
	 }
	 if ((Pdimm[0].NRanks < 2) && (Pdimm[1].NRanks < 2)) {
	   Popts->BaIntlvCtl = 0;
	 }
	 if (Pdimm[0].Capacity != Pdimm[1].Capacity) {
	   Popts->BaIntlvCtl = 0;
	 }
	 break;
      case FSL_DDR_CS0_CS1:
	 if (Pdimm[0].NRanks < 2) {
	   Popts->BaIntlvCtl = 0;
	 }
	 break;
      case FSL_DDR_CS2_CS3:
	 if (Pdimm[0].NRanks < 4) {
	   Popts->BaIntlvCtl = 0;
	 }
	 break;
      case FSL_DDR_CS0_CS1_AND_CS2_CS3:
	 if (Pdimm[0].NRanks < 4) {
	   Popts->BaIntlvCtl = 0;
	 }
	 break;
      default:
	 Popts->BaIntlvCtl = 0;
	 break;
    }
  }

  if (HwConfigSubF("fsl_ddr", "AddrHash", Buf)) {
    if (HwConfigSubargCmpF("fsl_ddr", "AddrHash", "null", Buf))
      Popts->AddrHash = 0;
    else if (HwConfigSubargCmpF("fsl_ddr", "AddrHash",
  				       "true", Buf))
      Popts->AddrHash = 1;
  }

  if (Pdimm[0].NRanks == 4)
    Popts->QuadRankPresent = 1;

  DdrFreq = GetDdrFreq() / 1000000;
  if (Popts->RegisteredDimmEn) {
    Popts->RcwOverride = 1;
    Popts->Rcw1 = 0x000a5a00;
    if (DdrFreq <= 800)
      Popts->Rcw2 = 0x00000000;
    else if (DdrFreq <= 1066)
      Popts->Rcw2 = 0x00100000;
    else if (DdrFreq <= 1333)
      Popts->Rcw2 = 0x00200000;
    else
      Popts->Rcw2 = 0x00300000;
  }

  FslDdrBoardOptions(Popts, Pdimm, CtrlNum);

  return 0;
}
 
/**
  Bind the main DDR setup driver's generic data
  to this specific DDR technology.

  @param  Spd			structure containing SPD settings.
  @param  Pdimm		structure containing dimm parameters
  @param  DimmNumber		dimm number

**/
INT32
ComputeDimmParameters(
  IN	CONST GenericSpdEepromT 	*Spd,
  OUT	DimmParamsT 			*Pdimm,
  IN	UINT32 			DimmNumber
  )
{
  return DdrComputeDimmParameters(Spd, Pdimm, DimmNumber);
};

UINTN
FslDdrCompute (
  IN   FslDdrInfoT 		*Pinfo,
  IN   UINT32 		StartStep,
  IN   UINT32 		SizeOnly
  )
{
  UINT32 i, j;
  UINTN TotalMem = 0;
  INT32 AssertReset;

  FslDdrCfgRegsT *DdrReg = Pinfo->FslDdrConfigReg;
  CommonTimingParamsT *TimingParams = Pinfo->CommonTimingParams;
  AssertReset = BoardNeedMemReset();

  /** data bus width Capacity adjust shift amount */
  UINT32 DbwCapacityAdjust[CONFIG_NUM_DDR_CONTROLLERS];

  for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
  	DbwCapacityAdjust[i] = 0;
  }

  switch (StartStep) {
    case STEP_GET_SPD:
#if defined(CONFIG_DDR_SPD) || defined(CONFIG_SPD_EEPROM)
      /** STEP 1:  Gather all DIMM SPD data */
      for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
        FslDdrGetSpd(Pinfo->SpdInstalledDimms[i], i);
      }
    case STEP_COMPUTE_DIMM_PARMS:
      /** STEP 2:  Compute DIMM parameters from SPD data */
         
      for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
        for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
	   UINT32 Retval;
	   GenericSpdEepromT *Spd =
	  		&(Pinfo->SpdInstalledDimms[i][j]);
	   DimmParamsT *Pdimm =
	   		&(Pinfo->DimmParams[i][j]);

	   Retval = ComputeDimmParameters(Spd, Pdimm, i);
#ifdef  CONFIG_SYS_DDR_RAW_TIMING
	   if (Retval) {
	     FslDdrGetDimmParams(Pdimm, i, j);
	   }
#else    
	   if (Retval == 2) {
	     return 0;
	   }
#endif   
	 }
      }

#elif defined(CONFIG_SYS_DDR_RAW_TIMING)
    case STEP_COMPUTE_DIMM_PARMS:
      for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
        for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
	   DimmParamsT *Pdimm =
	  		&(Pinfo->DimmParams[i][j]);
	   FslDdrGetDimmParams(Pdimm, i, j);
	 }
      }
#endif
    case STEP_COMPUTE_COMMON_PARMS:
      /**
  	 STEP 3: Compute a common set of timing parameters
  	 suitable for all of the DIMMs on each memory controller
      **/
      for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
        ComputeLowestCommonDimmParameters(
  			Pinfo->DimmParams[i],
  			&TimingParams[i],
  			CONFIG_DIMM_SLOTS_PER_CTLR);
      }

    case STEP_GATHER_OPTS:
      /** STEP 4:  Gather Configuration requirements from user */
      for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
	 /**
	   This "reloads" the memory controller options
	   to defaults.  If the user "edits" an option,
	   next_step poINT32s to the step after this,
	   which is currently STEP_ASSIGN_ADDRESSES.
	 **/
	 PopulateMemctlOptions(
			TimingParams[i].AllDimmsRegistered,
			&Pinfo->MemctlOpts[i],
			Pinfo->DimmParams[i], i);
	 /**
	   For RDIMMs, JEDEC spec requires clocks to be stable
	   before reset signal is deasserted. For the boards
	   using fixed parameters, this function should be
	   be called from board init file.
	 **/
	 if (TimingParams[i].AllDimmsRegistered)
	   AssertReset = 1;
      }
      if (AssertReset) {
	 /** board_assert_mem_reset();*/
      }

    case STEP_ASSIGN_ADDRESSES:
      /** STEP 5:  Assign addresses to chip selects */
      CheckInterleavingOptions(Pinfo);
      TotalMem = StepAssignAddresses(Pinfo, DbwCapacityAdjust);

    case STEP_COMPUTE_REGS:
      /** STEP 6:  compute controller register values */
      for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
        if (TimingParams[i].NdimmsPresent == 0) {
	   InternalMemZeroMem(&DdrReg[i], sizeof(FslDdrCfgRegsT));
          continue;
	 }

	 ComputeFslMemctlConfigRegs(
	 	&Pinfo->MemctlOpts[i],
		&DdrReg[i], &TimingParams[i],
		Pinfo->DimmParams[i],
		DbwCapacityAdjust[i],
		SizeOnly);
      }
    default:
      break;
  }

  /**
    Compute the amount of memory available just by
    looking for the highest valid CSn_BNDS value.
    This allows us to also experiment with using
    only CS0 when using dual-rank DIMMs.
  **/
  UINT32 MaxEnd = 0;

  for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
    for (j = 0; j < CONFIG_CHIP_SELECTS_PER_CTRL; j++) {
      FslDdrCfgRegsT *Reg = &DdrReg[i];
      if (Reg->Cs[j].Config & 0x80000000) {
  	 UINT32 End;
  	 /**
  	   0xfffffff is a special value we put
  	   for unused Bnds
  	 **/
  	 if (Reg->Cs[j].Bnds == 0xffffffff)
  	   continue;
  	 End = Reg->Cs[j].Bnds & 0xffff;
  	 if (End > MaxEnd) {
  	   MaxEnd = End;
  	 }
      }
    }
  }

  TotalMem = 1 + (((UINTN)MaxEnd << 24ULL) |
	 0xFFFFFFULL) - CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY;

  return TotalMem;
}

VOID
FslDdrSetIntl3r (
  IN   CONST UINT32		GranuleSize
  )
{
  return;
}


/**
  FslDdrSdram() -- this is the main function to be called by
  DramInit().

  It returns amount of memory Configured in bytes.
**/
UINT32
FslDdrSdram (
  VOID
  )
{
  UINT32 i;
  UINTN TotalMemory;
  FslDdrInfoT Info;
  INT32 DeassertReset;

  /** Reset info structure. */
  InternalMemZeroMem(&Info, sizeof(FslDdrInfoT));

  /** Compute it once normally. */
  /** TODO Fill data in interactive mode of DDR*/
  /** TODO Fill data of DDR from SPD */
  TotalMemory = FslDdrCompute(&Info, STEP_GET_SPD, 0);
  /** 
    TODO setup 3-way interleaving before enabling DDRC
    will give TotalMemory
  **/
  if (Info.MemctlOpts[0].MemctlInterleaving) {
    switch (Info.MemctlOpts[0].MemctlInterleavingMode) {
      case FSL_DDR_3WAY_1KB_INTERLEAVING:
      case FSL_DDR_3WAY_4KB_INTERLEAVING:
      case FSL_DDR_3WAY_8KB_INTERLEAVING:
        FslDdrSetIntl3r(
  	      Info.MemctlOpts[0].MemctlInterleavingMode);
        break;
      default:
       break;
    }
  }

  /**
    Program Configuration registers.
    JEDEC specs requires clocks to be stable before deasserting reset
    for RDIMMs. Clocks start after chip select is enabled and clock
    control register is set. During step 1, all controllers have their
    registers set but not enabled. Step 2 proceeds after deasserting
    reset through board FPGA or GPIO.
    For non-Registered DIMMs, initialization can go through but it is
    also OK to follow the same flow.
  **/

  /* TODO mem reset board */
  DeassertReset = BoardNeedMemReset();
  for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
    if (Info.CommonTimingParams[i].AllDimmsRegistered)
      DeassertReset = 1;
  }

  for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
    if (Info.CommonTimingParams[i].NdimmsPresent == 0) {
      continue;
    }
    /**
      The following call with step = 1 returns before enabling
      the controller. It has to finish with step = 2 later.
    **/
    FslDdrSetMemctlRegs(&(Info.FslDdrConfigReg[i]), i, 0);
  }

  if (DeassertReset) {
    /** Use board FPGA or GPIO to deassert reset signal */
    //board_deassert_mem_reset();	/*TODO*/
    for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
      /** Call with step = 2 to continue initialization */
      FslDdrSetMemctlRegs(&(Info.FslDdrConfigReg[i]), i, 2);
    }
  }
  
#if !defined(CONFIG_PHYS_64BIT)
  /** Check for 4G or more.  Bad. */
  if (TotalMemory >= (1ull << 32)) {
    TotalMemory = CONFIG_MAX_MEM_MAPPED;
  }
#endif

  return TotalMemory;
}

VOID
BoardAddRamInfo (
  VOID
  )
{
  struct CcsrDdr *ddr =
         (VOID *)(CONFIG_SYS_FSL_DDR_ADDR);
  INT32 CasLat;

  /** Calculate CAS latency based on timing cfg values */
  CasLat = ((MmioReadBe32((UINTN)&ddr->TimingCfg1) >> 16) & 0xf) + 1;
  if ((MmioReadBe32((UINTN)&ddr->TimingCfg3) >> 12) & 1)
    CasLat += (8 << 1);
}


/**
  Function to initialize DDR
 **/
VOID
DramInit (
  VOID
  )
{
  UINT32 RamSize = 0;
  RamSize = FslDdrSdram();

  if (RamSize > 0) {
    //BoardAddRamInfo();
  }

  return;
}

