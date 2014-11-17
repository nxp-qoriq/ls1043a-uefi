/** @DdrCntrlRegs.c
  Ddr Library containing functions to calculate value for
  ddr controller registers

  Copyright (c) 2014, Freescale Ltd. All rights reserved.
  Author: Meenakshi Aggarwal <meenakshi.aggarwal@freescale.com>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include <Ddr.h>

UINT32
PicosToMclk (
  IN   UINT32 		Picos
  )
{
  UINTN Clks, ClksRem;
  UINT64 DataRate = GetDdrFreq();

  /* Short circuit for zero Picos */
  if (!Picos)
    return 0;

  /* First multiply the time by the data rate (32x32 => 64) */
  Clks = Picos * (UINTN)DataRate;
  /**
    Now divide by 5^12 and track the 32-bit remainder, then divide
    by 2*(2^12) using shifts (and updating the remainder).
  **/
  ClksRem = DoDiv(Clks, UL_5POW12);
  ClksRem += (Clks & (UL_2POW13-1)) * UL_5POW12;
  Clks >>= 13;

  /* If we had a remainder greater than the 1ps error, then round up */
  if (ClksRem > DataRate)
    Clks++;

  /* Clamp to the maximum representable value */
  if (Clks > ULL_8FS)
    Clks = ULL_8FS;
  
  return (UINT32) Clks;
}


UINT32
FslDdrGetVersion (
  VOID
  )
{
  struct CcsrDdr  *Ddr;
  UINT32 VerMajorMinorErrata;
  
  Ddr = (VOID *)CONFIG_SYS_FSL_DDR_ADDR;
  VerMajorMinorErrata = (MmioReadBe32((UINTN)&Ddr->IpRev1) & 0xFFFF) << 8;
  VerMajorMinorErrata |= (MmioReadBe32((UINTN)&Ddr->IpRev2) & 0xFF00) >> 8;
  
  return VerMajorMinorErrata;
}

VOID
SetDdrCdr1 (
  OUT  FslDdrCfgRegsT		*Ddr,
  IN   CONST MemctlOptionsT 	*Popts
  )
{
  Ddr->DdrCdr1 = Popts->DdrCdr1;
}


VOID
SetDdrCdr2 (
  OUT  FslDdrCfgRegsT 		*Ddr,
  IN   CONST MemctlOptionsT 	*Popts
  )
{
  Ddr->DdrCdr2 = Popts->DdrCdr2;
}


UINT32
CheckFslMemctlConfigRegs (
  IN   CONST FslDdrCfgRegsT 		*Ddr
  )
{
  UINT32 Res = 0;

  /**
    Check that DDR_SDRAM_CFG[RD_EN] and DDR_SDRAM_CFG[2T_EN] are
    not set at the same time.
   **/
  if (Ddr->DdrSdramCfg & 0x10000000
      && Ddr->DdrSdramCfg & 0x00008000) {
    Res++;
  }

  return Res;
}


/**
  compute the CAS write latency according to DDR3 spec
  CWL = 5 if tCK >= 2.5ns
        6 if 2.5ns > tCK >= 1.875ns
        7 if 1.875ns > tCK >= 1.5ns
        8 if 1.5ns > tCK >= 1.25ns
        9 if 1.25ns > tCK >= 1.07ns
        10 if 1.07ns > tCK >= 0.935ns
        11 if 0.935ns > tCK >= 0.833ns
        12 if 0.833ns > tCK >= 0.75ns
 **/
inline UINT32
ComputeCasWriteLatency (
  VOID
  )
{
  UINT32 Cwl;
  CONST UINT32 MclkPs = GetMemoryClkPeriodPs();
  
  if (MclkPs >= 2500)
  	Cwl = 5;
  else if (MclkPs >= 1875)
  	Cwl = 6;
  else if (MclkPs >= 1500)
  	Cwl = 7;
  else if (MclkPs >= 1250)
  	Cwl = 8;
  else if (MclkPs >= 1070)
	Cwl = 9;
  else if (MclkPs >= 935)
  	Cwl = 10;
  else if (MclkPs >= 833)
  	Cwl = 11;
  else if (MclkPs >= 750)
  	Cwl = 12;
  else {
  	Cwl = 12;
  }
  return Cwl;
}


/**
  -3E = 667 CL5, -25 = CL6 800, -25E = CL5 800
**/

inline INT32
AvoidOdtOverlap (
  IN   CONST DimmParamsT 		*DimmParams)
{
  if (DimmParams[0].NRanks == 4)
    return 1;

  return 0;
}


VOID
SetTimingCfg0 (
  OUT  FslDdrCfgRegsT 		*Ddr,
  IN   CONST MemctlOptionsT 	*Popts,
  IN   CONST DimmParamsT 		*DimmParams
  )
{
  UINT8 TrwtMclk = 0;   /** Read-to-write turnaround */
  UINT8 TwrtMclk = 0;   /** Write-to-read turnaround */
  /** 7.5 ns on -3E; 0 means WL - CL + BL/2 + 1 */
  UINT8 TrrtMclk = 0;   /** Read-to-read turnaround */
  UINT8 TwwtMclk = 0;   /** Write-to-write turnaround */
  
  /** Active powerdown exit timing (tXARD and tXARDS). */
  UINT8 ActPdExitMclk;
  /** Precharge powerdown exit timing (Txp). */
  UINT8 PrePdExitMclk;
  /** ODT powerdown exit timing (tAXPD). */
  UINT8 TaxpdMclk;
  /** Mode register set cycle time (tMRD). */
  UINT8 TmrdMclk;

  /**
    (tXARD and tXARDS). Empirical?
    The DDR3 spec has not tXARD,
    we use the Txp instead of it.
    Txp=Max(3nCK, 7.5ns) for DDR3.
    spec has not the tAXPD, we use
    tAXPD=1, need design to confirm.
   **/
  INT32 Txp = Max((GetMemoryClkPeriodPs() * 3), 7500); /** unit=ps */
  UINT32 DataRate = GetDdrFreq();
  TmrdMclk = 4;
  /** set the turnaround time */

  /**
    for single quad-rank DIMM and two dual-rank DIMMs
    to aVOID ODT overlap
   **/
  if (AvoidOdtOverlap(DimmParams)) {
    TwwtMclk = 2;
    TrrtMclk = 1;
  }
  /** for faster clock, need more time for data setup */
  TrwtMclk = (DataRate/1000000 > 1800) ? 2 : 1;

  if ((DataRate/1000000 > 1150) || (Popts->MemctlInterleaving))
    TwrtMclk = 1;

  if (Popts->DynamicPower == 0) {	/** powerdown is not used */
    ActPdExitMclk = 1;
    PrePdExitMclk = 1;
    TaxpdMclk = 1;
  } else {
    /** ActPdExitMclk = tXARD, see above */
    ActPdExitMclk = PicosToMclk(Txp);
    /** Mode register MR0[A12] is '1' - fast exit */
    PrePdExitMclk = ActPdExitMclk;
    TaxpdMclk = 1;
  }

  if (Popts->TrwtOverride)
    TrwtMclk = Popts->Trwt;

  Ddr->TimingCfg0 = (0
  	| ((TrwtMclk & 0x3) << 30)	/** RWT */
  	| ((TwrtMclk & 0x3) << 28)	/** WRT */
  	| ((TrrtMclk & 0x3) << 26)	/** RRT */
  	| ((TwwtMclk & 0x3) << 24)	/** WWT */
  	| ((ActPdExitMclk & 0xf) << 20)  /** ACT_PD_EXIT */
  	| ((PrePdExitMclk & 0xF) << 16)  /** PRE_PD_EXIT */
  	| ((TaxpdMclk & 0xf) << 8)	/** ODT_PD_EXIT */
  	| ((TmrdMclk & 0x1f) << 0)	/** MRS_CYC */
  );
}


VOID
SetTimingCfg3 (
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts,
  IN   CONST CommonTimingParamsT 		*CommonDimm,
  IN   UINT32 				CasLatency,
  IN   UINT32 				AdditiveLatency
  )
{
  /** Extended precharge to activate interval (tRP) */
  UINT32 ExtPretoact = 0;
  /** Extended Activate to precharge interval (tRAS) */
  UINT32 ExtActtopre = 0;
  /** Extended activate to read/write interval (tRCD) */
  UINT32 ExtActtorw = 0;
  /** Extended refresh recovery time (tRFC) */
  UINT32 ExtRefrec;
  /** Extended MCAS latency from READ cmd */
  UINT32 ExtCaslat = 0;
  /** Extended additive latency */
  UINT32 ExtAddLat = 0;
  /** Extended last data to precharge interval (tWR) */
  UINT32 ExtWrrec = 0;
  /** Control Adjust */
  UINT32 CntlAdj = 0;

  ExtPretoact = PicosToMclk(CommonDimm->TrpPs) >> 4;
  ExtActtopre = PicosToMclk(CommonDimm->TrasPs) >> 4;
  ExtActtorw = PicosToMclk(CommonDimm->TrcdPs) >> 4;
  ExtCaslat = (2 * CasLatency - 1) >> 4;
  ExtAddLat = AdditiveLatency >> 4;
  ExtRefrec = (PicosToMclk(CommonDimm->TrfcPs) - 8) >> 4;
  /** Extwrrec only deals with 16 clock and above, or 14 with OTF */
  ExtWrrec = (PicosToMclk(CommonDimm->TwrPs) +
  	(Popts->OtfBurstChopEn ? 2 : 0)) >> 4;

  Ddr->TimingCfg3 = (0
  	| ((ExtPretoact & 0x1) << 28)
  	| ((ExtActtopre & 0x3) << 24)
  	| ((ExtActtorw & 0x1) << 22)
  	| ((ExtRefrec & 0x1F) << 16)
  	| ((ExtCaslat & 0x3) << 12)
  	| ((ExtAddLat & 0x1) << 10)
  	| ((ExtWrrec & 0x1) << 8)
  	| ((CntlAdj & 0x7) << 0)
  );
}


VOID
SetTimingCfg1(
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts,
  IN   CONST CommonTimingParamsT 		*CommonDimm,
  IN   UINT32 				CasLatency
  )
{
  /** Precharge-to-activate interval (tRP) */
  UINT8 PretoactMclk;
  /** Activate to precharge interval (tRAS) */
  UINT8 ActtopreMclk;
  /**  Activate to read/write interval (tRCD) */
  UINT8 ActtorwMclk;
  /** CASLAT */
  UINT8 CaslatCtrl;
  /**  Refresh recovery time (tRFC) ; trfc_low */
  UINT8 RefrecCtrl;
  /** Last data to precharge minimum interval (tWR) */
  UINT8 WrrecMclk;
  /** Activate-to-activate interval (tRRD) */
  UINT8 ActtoactMclk;
  /** Last write data pair to read command issue interval (tWTR) */
  UINT8 WrtordMclk;
  /** DDR_SDRAM_MODE doesn't support 9,11,13,15 */
  CONST UINT8 WrrecTable[] = {
  	1, 2, 3, 4, 5, 6, 7, 8, 10, 10, 12, 12, 14, 14, 0, 0};

  PretoactMclk = PicosToMclk(CommonDimm->TrpPs);
  ActtopreMclk = PicosToMclk(CommonDimm->TrasPs);
  ActtorwMclk = PicosToMclk(CommonDimm->TrcdPs);

  /**
    Translate CAS Latency to a Ddr controller field value:

         CAS Lat Ddr I   DDR II  Ctrl
         Clocks  SPD Bit SPD Bit Value
         ------- ------- ------- -----
         1.0     0               0001
         1.5     1               0010
         2.0     2       2       0011
         2.5     3               0100
         3.0     4       3       0101
         3.5     5               0110
         4.0             4       0111
         4.5                     1000
         5.0             5       1001
   **/
  /**
    if the CAS latency more than 8 cycle,
    we need set extend bit for it at
    TIMING_CFG_3[EXT_CASLAT]
   **/
  CaslatCtrl = 2 * CasLatency - 1;

  RefrecCtrl = PicosToMclk(CommonDimm->TrfcPs) - 8;
  WrrecMclk = PicosToMclk(CommonDimm->TwrPs);

  if (!((WrrecMclk < 1) || (WrrecMclk > 16)))
    WrrecMclk = WrrecTable[WrrecMclk - 1];

  if (Popts->OtfBurstChopEn)
    WrrecMclk += 2;

  ActtoactMclk = PicosToMclk(CommonDimm->TrrdPs);
  /**
    JEDEC has min requirement for tRRD
   **/
  if (ActtoactMclk < 4)
    ActtoactMclk = 4;
  WrtordMclk = PicosToMclk(CommonDimm->TwtrPs);
  /**
    JEDEC has some min requirements for tWTR
   **/
  if (WrtordMclk < 4)
    WrtordMclk = 4;
  if (Popts->OtfBurstChopEn)
    WrtordMclk += 2;

  Ddr->TimingCfg1 = (0
  	| ((PretoactMclk & 0x0F) << 28)
  	| ((ActtopreMclk & 0x0F) << 24)
  	| ((ActtorwMclk & 0xF) << 20)
  	| ((CaslatCtrl & 0xF) << 16)
  	| ((RefrecCtrl & 0xF) << 12)
  	| ((WrrecMclk & 0x0F) << 8)
  	| ((ActtoactMclk & 0x0F) << 4)
  	| ((WrtordMclk & 0x0F) << 0)
  );
}


VOID
SetTimingCfg2 (
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts,
  IN   CONST CommonTimingParamsT 		*CommonDimm,
  IN   UINT32 				CasLatency,
  IN   UINT32 				AdditiveLatency
  )
{
  /** Additive latency */
  UINT8 AddLatMclk;
  /** CAS-to-preamble override */
  UINT16 Cpo;
  /** Write latency */
  UINT8 WrLat;
  /**  Read to precharge (tRTP) */
  UINT8 RdToPre;
  /** Write command to write data strobe timing adjustment */
  UINT8 WrDataDelay;
  /** Minimum CKE pulse width (tCKE) */
  UINT8 CkePls;
  /** Window for four activates (tFAW) */
  UINT16 FourAct;

  /** FIXME add check that this must be less than ActtorwMclk */
  AddLatMclk = AdditiveLatency;
  Cpo = Popts->CpoOverride;

  WrLat = ComputeCasWriteLatency();

  RdToPre = PicosToMclk(CommonDimm->TrtpPs);
  /**
    JEDEC has some min requirements for tRTP
  **/
  if (RdToPre < 4)
    RdToPre = 4;
  if (Popts->OtfBurstChopEn)
    RdToPre += 2; /** according to UM */

  WrDataDelay = Popts->WriteDataDelay;
  CkePls = PicosToMclk(Popts->TckeClockPulseWidthPs);
  FourAct = PicosToMclk(Popts->TfawWindowFourActivatesPs);

  Ddr->TimingCfg2 = (0
  	| ((AddLatMclk & 0xf) << 28)
  	| ((Cpo & 0x1f) << 23)
  	| ((WrLat & 0xf) << 19)
  	| ((RdToPre & RD_TO_PRE_MASK) << RD_TO_PRE_SHIFT)
  	| ((WrDataDelay & WR_DATA_DELAY_MASK) << WR_DATA_DELAY_SHIFT)
  	| ((CkePls & 0x7) << 6)
  	| ((FourAct & 0x3f) << 0)
  );
}


VOID
SetDdrEor (
  OUT  FslDdrCfgRegsT 		*Ddr,
  IN   CONST MemctlOptionsT 	*Popts
  )
{
  if (Popts->AddrHash) {
    Ddr->DdrEor = 0x40000000;	/** address hash enable */
  }
}


VOID
SetCsnConfig (
  IN   INT32 				DimmNumber,
  IN   INT32 				i,
  OUT  FslDdrCfgRegsT 		*Ddr,
  IN   CONST MemctlOptionsT 	*Popts,
  IN   CONST DimmParamsT 		*DimmParams
  )
{
  UINT32 CsNEn = 0; /** Chip Select enable */
  UINT32 IntlvEn = 0; /** Memory controller interleave enable */
  UINT32 IntlvCtl = 0; /** Interleaving control */
  UINT32 ApNEn = 0; /** Chip select n auto-precharge enable */
  UINT32 OdtRdCfg = 0; /** ODT for reads Configuration */
  UINT32 OdtWrCfg = 0; /** ODT for writes Configuration */
  UINT32 BaBitsCsN = 0; /** Num of bank bits for SDRAM on CSn */
  UINT32 RowBitsCsN = 0; /** Num of row bits for SDRAM on CSn */
  UINT32 ColBitsCsN = 0; /** Num of ocl bits for SDRAM on CSn */
  INT32 GoConfig = 0;

  /** Compute CS_CONFIG only for existing ranks of each DIMM.  */
  switch (i) {
    case 0:
      if (DimmParams[DimmNumber].NRanks > 0) {
        GoConfig = 1;
  	 /** These fields only available in CS0_CONFIG */
  	 if (!Popts->MemctlInterleaving)
  	   break;
  	 switch (Popts->MemctlInterleavingMode) {
  	   case FSL_DDR_256B_INTERLEAVING:
  	   case FSL_DDR_CACHE_LINE_INTERLEAVING:
  	   case FSL_DDR_PAGE_INTERLEAVING:
	   case FSL_DDR_BANK_INTERLEAVING:
	   case FSL_DDR_SUPERBANK_INTERLEAVING:
	     IntlvEn = Popts->MemctlInterleaving;
	     IntlvCtl = Popts->MemctlInterleavingMode;
	     break;
	   default:
	     break;
	 }
      }
      break;
    case 1:
      if ((DimmNumber == 0 && DimmParams[0].NRanks > 1) || \
  	    (DimmNumber == 1 && DimmParams[1].NRanks > 0))
        GoConfig = 1;
      break;
    case 2:
      if ((DimmNumber == 0 && DimmParams[0].NRanks > 2) || \
  	   (DimmNumber >= 1 && DimmParams[DimmNumber].NRanks > 0))
        GoConfig = 1;
      break;
    case 3:
      if ((DimmNumber == 0 && DimmParams[0].NRanks > 3) || \
  	    (DimmNumber == 1 && DimmParams[1].NRanks > 1) || \
  	    (DimmNumber == 3 && DimmParams[3].NRanks > 0))
        GoConfig = 1;
      break;
    default:
      break;
  }

  if (GoConfig) {
    UINT32 NBanksPerSdramDevice;
    CsNEn = 1;
    ApNEn = Popts->CsLocalOpts[i].AutoPrecharge;
    OdtRdCfg = Popts->CsLocalOpts[i].OdtRdCfg;
    OdtWrCfg = Popts->CsLocalOpts[i].OdtWrCfg;
    NBanksPerSdramDevice
	= DimmParams[DimmNumber].NBanksPerSdramDevice;
    BaBitsCsN = __ILog2(NBanksPerSdramDevice) - 2;
    RowBitsCsN = DimmParams[DimmNumber].NRowAddr - 12;
    ColBitsCsN = DimmParams[DimmNumber].NColAddr - 8;
  }
  Ddr->Cs[i].Config = (0
  	| ((CsNEn & 0x1) << 31)
  	| ((IntlvEn & 0x3) << 29)
  	| ((IntlvCtl & 0xf) << 24)
  	| ((ApNEn & 0x1) << 23)

  	/** XXX: some implementation only have 1 bit starting at left */
  	| ((OdtRdCfg & 0x7) << 20)

  	/** XXX: Some implementation only have 1 bit starting at left */
  	| ((OdtWrCfg & 0x7) << 16)
  
  	| ((BaBitsCsN & 0x3) << 14)
  	| ((RowBitsCsN & 0x7) << 8)
  	| ((ColBitsCsN & 0x7) << 0)
  );
}


VOID
SetCsnConfig_2 (
  IN   INT32 				i,
  OUT  FslDdrCfgRegsT 		*Ddr
  )
{
  UINT32 PasrCfg = 0;	/** Partial array self refresh Config */

  Ddr->Cs[i].Config2 = ((PasrCfg & 7) << 24);
}


VOID
SetDdrSdramRcw (
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts,
  IN   CONST CommonTimingParamsT 		*CommonDimm
  )
{
  if (CommonDimm->AllDimmsRegistered &&
      !CommonDimm->AllDimmsUnbuffered)	{
    if (Popts->RcwOverride) {
      Ddr->DdrSdramRcw1 = Popts->Rcw1;
      Ddr->DdrSdramRcw2 = Popts->Rcw2;
    } else {
      Ddr->DdrSdramRcw1 =
   		CommonDimm->Rcw[0] << 28 | \
   		CommonDimm->Rcw[1] << 24 | \
   		CommonDimm->Rcw[2] << 20 | \
   		CommonDimm->Rcw[3] << 16 | \
   		CommonDimm->Rcw[4] << 12 | \
   		CommonDimm->Rcw[5] << 8 | \
   		CommonDimm->Rcw[6] << 4 | \
   		CommonDimm->Rcw[7];
      Ddr->DdrSdramRcw2 =
   		CommonDimm->Rcw[8] << 28 | \
   		CommonDimm->Rcw[9] << 24 | \
   		CommonDimm->Rcw[10] << 20 | \
   		CommonDimm->Rcw[11] << 16 | \
   		CommonDimm->Rcw[12] << 12 | \
   		CommonDimm->Rcw[13] << 8 | \
   		CommonDimm->Rcw[14] << 4 | \
		CommonDimm->Rcw[15];
     }
  }
}


VOID
SetDdrSdramCfg (
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts,
  IN   CONST CommonTimingParamsT 	*CommonDimm
  )
{
  UINT32 MemEn;		/** Ddr SDRAM interface logic enable */
  UINT32 Sren;		/** Self refresh enable (during sleep) */
  UINT32 EccEn;		/** ECC enable. */
  UINT32 RdEn;		/** Registered DIMM enable */
  UINT32 SdramType;		/** Type of SDRAM */
  UINT32 DynPwr;		/** Dynamic power management Mode */
  UINT32 Dbw;			/** DRAM dta bus width */
  UINT32 EightBe = 0;	/** 8-beat burst enable, DDR2 is zero */
  UINT32 Ncap = 0;		/** Non-concurrent auto-precharge */
  UINT32 ThreetEn;		/** Enable 3T timing */
  UINT32 TwotEn;		/** Enable 2T timing */
  UINT32 BaIntlvCtl;		/** Bank (CS) interleaving control */
  UINT32 X32En = 0;		/** x32 enable */
  UINT32 Pchb8 = 0;		/** precharge bit 8 enable */
  UINT32 Hse;			/** Global half strength override */
  UINT32 MemHalt = 0;	/** memory controller halt */
  UINT32 Bi = 0;		/** Bypass initialization */

  MemEn = 1;
  Sren = Popts->SelfRefreshInSleep;
  if (CommonDimm->AllDimmsEccCapable) {
    /** Allow setting of ECC only if all DIMMs are ECC. */
    EccEn = Popts->EccMode;
  } else {
    EccEn = 0;
  }

  if (CommonDimm->AllDimmsRegistered &&
      !CommonDimm->AllDimmsUnbuffered)	{
    RdEn = 1;
    TwotEn = 0;
  } else {
    RdEn = 0;
    TwotEn = Popts->TwotEn;
  }

  SdramType = CONFIG_FSL_SDRAM_TYPE;

  DynPwr = Popts->DynamicPower;
  Dbw = Popts->DataBusWidth;
  /**
    8-beat burst enable Ddr-III case
    we must clear it when use the on-the-fly Mode,
    must set it when use the 32-bits bus Mode.
  **/
  if (SdramType == SDRAM_TYPE_DDR3) {
    if (Popts->BurstLength == DDR_BL8)
      EightBe = 1;
    if (Popts->BurstLength == DDR_OTF)
      EightBe = 0;
    if (Dbw == 0x1)
      EightBe = 1;
  }

  ThreetEn = Popts->ThreetEn;
  BaIntlvCtl = Popts->BaIntlvCtl;
  Hse = Popts->HalfStrengthDriverEnable;

  Ddr->DdrSdramCfg = (0
  		| ((MemEn & 0x1) << 31)
  		| ((Sren & 0x1) << 30)
  		| ((EccEn & 0x1) << 29)
  		| ((RdEn & 0x1) << 28)
  		| ((SdramType & 0x7) << 24)
  		| ((DynPwr & 0x1) << 21)
  		| ((Dbw & 0x3) << 19)
  		| ((EightBe & 0x1) << 18)
  		| ((Ncap & 0x1) << 17)
  		| ((ThreetEn & 0x1) << 16)
  		| ((TwotEn & 0x1) << 15)
  		| ((BaIntlvCtl & 0x7F) << 8)
  		| ((X32En & 0x1) << 5)
  		| ((Pchb8 & 0x1) << 4)
  		| ((Hse & 0x1) << 3)
  		| ((MemHalt & 0x1) << 1)
		| ((Bi & 0x1) << 0)
  );
}


VOID
SetDdrSdramCfg2 (
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts,
  IN   CONST UINT32 				UnqMrsEn
  )
{
  UINT32 FrcSr = 0;	/** Force self refresh */
  UINT32 SrIe = 0;	/** Self-refresh interrupt enable */
  UINT32 DllRstDis;	/** DLL reset disable */
  UINT32 DqsCfg;	/** DQS Configuration */
  UINT32 OdtCfg = 0;	/** ODT Configuration */
  UINT32 NumPr;	/** Number of posted refreshes */
  UINT32 Slow = 0;	/** Ddr will be run less than 1250 */
  UINT32 X4En = 0;	/** x4 DRAM enable */
  UINT32 ObcCfg;	/** On-The-Fly Burst Chop Cfg */
  UINT32 ApEn;	/** Address Parity Enable */
  UINT32 DInit;	/** DRAM data initialization */
  UINT32 RcwEn = 0;	/** Register Control Word Enable */
  UINT32 MdEn = 0;	/** Mirrored DIMM Enable */
  UINT32 QdEn = 0;	/** quad-rank DIMM Enable */
  INT32 i;

  DllRstDis = 1;	/** Make this Configurable */
  DqsCfg = Popts->DqsConfig;
  for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
    if (Popts->CsLocalOpts[i].OdtRdCfg
      	|| Popts->CsLocalOpts[i].OdtWrCfg) {
      OdtCfg = SDRAM_CFG2_ODT_ONLY_READ;
      break;
    }
  }

  NumPr = 1;	/** Make this Configurable */

  /**
    8572 manual says
        {TIMING_CFG_1[PRETOACT]
         + [DDR_SDRAM_CFG_2[NUM_PR]
           * ({EXT_REFREC || REFREC} + 8 + 2)]}
         << DDR_SDRAM_INTERVAL[REFINT]
  **/
  ObcCfg = Popts->OtfBurstChopEn;

  Slow = GetDdrFreq() < 1249000000;

  if (Popts->RegisteredDimmEn) {
    RcwEn = 1;
    ApEn = Popts->ApEn;
  } else {
    ApEn = 0;
  }

  X4En = Popts->X4En ? 1 : 0;
  /** Memory will be initialized via DMA, or not at all. */
  DInit = 0;

  MdEn = Popts->MirroredDimm;
  QdEn = Popts->QuadRankPresent ? 1 : 0;
  Ddr->DdrSdramCfg2 = (0
  	| ((FrcSr & 0x1) << 31)
  	| ((SrIe & 0x1) << 30)
  	| ((DllRstDis & 0x1) << 29)
  	| ((DqsCfg & 0x3) << 26)
  	| ((OdtCfg & 0x3) << 21)
  	| ((NumPr & 0xf) << 12)
  	| ((Slow & 1) << 11)
  	| (X4En << 10)
  	| (QdEn << 9)
  	| (UnqMrsEn << 8)
  	| ((ObcCfg & 0x1) << 6)
  	| ((ApEn & 0x1) << 5)
  	| ((DInit & 0x1) << 4)
  	| ((RcwEn & 0x1) << 2)
  	| ((MdEn & 0x1) << 0)
  );
}


VOID
SetDdrSdramMode2 (
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts,
  IN   CONST CommonTimingParamsT 		*CommonDimm,
  IN   CONST UINT32 				UnqMrsEn
  )
{
  UINT16 ESdmode2 = 0;	/** Extended SDRAM Mode 2 */
  UINT16 ESdmode3 = 0;	/** Extended SDRAM Mode 3 */

  INT32 i;
  UINT32 RttWr = 0;	/** Rtt_WR - dynamic ODT off */
  UINT32 Srt = 0;	/** self-refresh temerature, normal range */
  UINT32 Asr = 0;	/** auto self-refresh disable */
  UINT32 Cwl = ComputeCasWriteLatency() - 5;
  UINT32 Pasr = 0;	/** partial array self refresh disable */

  if (Popts->RttOverride)
    RttWr = Popts->RttWrOverrideValue;
  else
    RttWr = Popts->CsLocalOpts[0].OdtRttWr;

  if (CommonDimm->ExtendedOpSrt)
    Srt = CommonDimm->ExtendedOpSrt;

  ESdmode2 = (0
  	| ((RttWr & 0x3) << 9)
  	| ((Srt & 0x1) << 7)
  	| ((Asr & 0x1) << 6)
  	| ((Cwl & 0x7) << 3)
	| ((Pasr & 0x7) << 0));

  Ddr->DdrSdramMode2 = (0
	| ((ESdmode2 & 0xFFFF) << 16)
	| ((ESdmode3 & 0xFFFF) << 0)
  );

  if (UnqMrsEn) {	/** unique Mode registers are supported */
    for (i = 1; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
      if (Popts->RttOverride)
        RttWr = Popts->RttWrOverrideValue;
      else
        RttWr = Popts->CsLocalOpts[i].OdtRttWr;

      ESdmode2 &= 0xF9FF;	/** clear bit 10, 9 */
      ESdmode2 |= (RttWr & 0x3) << 9;
      switch (i) {
	 case 1:
	   Ddr->DdrSdramMode4 = (0
			| ((ESdmode2 & 0xFFFF) << 16)
			| ((ESdmode3 & 0xFFFF) << 0)
	   );
	   break;
	 case 2:
	   Ddr->DdrSdramMode6 = (0
	     		| ((ESdmode2 & 0xFFFF) << 16)
			| ((ESdmode3 & 0xFFFF) << 0)
	   );
	   break;
	 case 3:
	   Ddr->DdrSdramMode8 = (0
			| ((ESdmode2 & 0xFFFF) << 16)
			| ((ESdmode3 & 0xFFFF) << 0)
	   );
	   break;
      }
    }
  }
}


VOID
set_DdrSdramInterval (
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts,
  IN   CONST CommonTimingParamsT	*CommonDimm
  )
{
  UINT32 Refint;	/** Refresh interval */
  UINT32 Bstopre;	/** Precharge interval */

  Refint = PicosToMclk(CommonDimm->RefreshRatePs);

  Bstopre = Popts->Bstopre;

  /** Refint field used 0x3FFF in earlier controllers */
  Ddr->DdrSdramInterval = (0
  			   | ((Refint & 0xFFFF) << 16)
  			   | ((Bstopre & 0x3FFF) << 0)
  );
}


VOID
SetDdrSdramMode (
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts,
  IN   CONST CommonTimingParamsT 		*CommonDimm,
  IN   UINT32 				CasLatency,
  IN   UINT32 				AdditiveLatency,
  IN   CONST UINT32 				UnqMrsEn
  )
{
  UINT16 ESdmode;		/** Extended SDRAM Mode */
  UINT16 Sdmode;		/** SDRAM Mode */

  /** Mode Register - MR1 */
  UINT32 Qoff = 0;		/** Output buffer enable 0=yes, 1=no */
  UINT32 TdqsEn = 0;	       /** TDQS Enable: 0=no, 1=yes */
  UINT32 Rtt;
  UINT32 WrlvlEn = 0;	/** Write level enable: 0=no, 1=yes */
  UINT32 Al = 0;		/** Posted CAS# additive latency (AL) */
  UINT32 Dic = 0;		/** Output driver impedance, 40ohm */
  UINT32 DllEn = 0;	       /** DLL Enable  0=Enable (Normal),
					       1=Disable (Test/Debug) */

  /** Mode Register - MR0 */
  UINT32 DllOn;	/** DLL control for precharge PD, 0=off, 1=on */
  UINT32 Wr = 0;	/** Write Recovery */
  UINT32 DllRst;	/** DLL Reset */
  UINT32 Mode;	/** Normal=0 or Test=1 */
  UINT32 Caslat = 4; /** CAS# latency, default set as 6 cycles */
  /** BT: Burst Type (0=Nibble Sequential, 1=Interleaved) */
  UINT32 Bt;
  UINT32 Bl;	/** BL: Burst Length */

  UINT32 WrMclk;
  /**
    DDR_SDRAM_MODE doesn't support 9,11,13,15
    Please refer JEDEC Standard No. 79-3E for Mode Register MR0
    for this table
  **/
  CONST UINT8 WrTable[] = {1, 2, 3, 4, 5, 5, 6, 6, 7, 7, 0, 0};

  CONST UINT32 MclkPs = GetMemoryClkPeriodPs();
  INT32 i;

  if (Popts->RttOverride)
    Rtt = Popts->RttOverrideValue;
  else
    Rtt = Popts->CsLocalOpts[0].OdtRttNorm;

  if (AdditiveLatency == (CasLatency - 1))
    Al = 1;
  if (AdditiveLatency == (CasLatency - 2))
    Al = 2;

  if (Popts->QuadRankPresent)
    Dic = 1;	/** output driver impedance 240/7 ohm */

  /**
    The ESdmode value will also be used for writing
    MR1 during write leveling for DDR3, although the
    bits specifically related to the write leveling
    scheme will be handled automatically by the Ddr
    controller. so we set the WrlvlEn = 0 here.
  **/
  ESdmode = (0
  	| ((Qoff & 0x1) << 12)
  	| ((TdqsEn & 0x1) << 11)
  	| ((Rtt & 0x4) << 7)   /** Rtt field is split */
  	| ((WrlvlEn & 0x1) << 7)
  	| ((Rtt & 0x2) << 5)   /** Rtt field is split */
  	| ((Dic & 0x2) << 4)   /** DIC field is split */
  	| ((Al & 0x3) << 3)
  	| ((Rtt & 0x1) << 2)   /** Rtt field is split */
  	| ((Dic & 0x1) << 1)   /** DIC field is split */
  	| ((DllEn & 0x1) << 0)
  );

  /**
    DLL control for precharge PD
    0=Slow exit DLL off (TxpDLL)
    1=fast exit DLL on (Txp)
  **/
  DllOn = 1;

  WrMclk = (CommonDimm->TwrPs + MclkPs - 1) / MclkPs;
  if (WrMclk <= 16) {
    Wr = WrTable[WrMclk - 5];
  }

  DllRst = 0;	/** dll no reset */
  Mode = 0;	/** normal Mode */

  /** look up table to get the cas latency bits */
  if (CasLatency >= 5 && CasLatency <= 16) {
    UINT8 CasLatency_table[] = {
		0x2,	/** 5 clocks */
		0x4,	/** 6 clocks */
		0x6,	/** 7 clocks */
		0x8,	/** 8 clocks */
		0xa,	/** 9 clocks */
		0xc,	/** 10 clocks */
		0xe,	/** 11 clocks */
		0x1,	/** 12 clocks */
		0x3,	/** 13 clocks */
		0x5,	/** 14 clocks */
		0x7,	/** 15 clocks */
		0x9,	/** 16 clocks */
    };
    Caslat = CasLatency_table[CasLatency - 5];
  }

  Bt = 0;	/** Nibble sequential */

  switch (Popts->BurstLength) {
    case DDR_BL8:
      Bl = 0;
      break;
    case DDR_OTF:
      Bl = 1;
      break;
    case DDR_BC4:
      Bl = 2;
      break;
    default:
      Bl = 1;
      break;
  }

  Sdmode = (0
  	  | ((DllOn & 0x1) << 12)
  	  | ((Wr & 0x7) << 9)
  	  | ((DllRst & 0x1) << 8)
	  | ((Mode & 0x1) << 7)
	  | (((Caslat >> 1) & 0x7) << 4)
	  | ((Bt & 0x1) << 3)
	  | ((Caslat & 1) << 2)
	  | ((Bl & 0x3) << 0)
  );

  Ddr->DdrSdramMode = (0
  		       | ((ESdmode & 0xFFFF) << 16)
  		       | ((Sdmode & 0xFFFF) << 0)
  );

  if (UnqMrsEn) {	/** unique Mode registers are supported */
    for (i = 1; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
      if (Popts->RttOverride)
        Rtt = Popts->RttOverrideValue;
      else
  	 Rtt = Popts->CsLocalOpts[i].OdtRttNorm;

      ESdmode &= 0xFDBB;	/** clear bit 9,6,2 */
      ESdmode |= (0
		| ((Rtt & 0x4) << 7)   /** Rtt field is split */
		| ((Rtt & 0x2) << 5)   /** Rtt field is split */
		| ((Rtt & 0x1) << 2)   /** Rtt field is split */
      );
      switch (i) {
	 case 1:
	   Ddr->DdrSdramMode3 = (0
	  	       | ((ESdmode & 0xFFFF) << 16)
		       | ((Sdmode & 0xFFFF) << 0)
	   );
	   break;
	 case 2:
	   Ddr->DdrSdramMode5 = (0
		       | ((ESdmode & 0xFFFF) << 16)
		       | ((Sdmode & 0xFFFF) << 0)
	   );
	   break;
	 case 3:
	   Ddr->DdrSdramMode7 = (0
		       | ((ESdmode & 0xFFFF) << 16)
		       | ((Sdmode & 0xFFFF) << 0)
	   );
	   break;
      }
    }
  }
}


VOID
SetDdrDataInit (
  OUT  FslDdrCfgRegsT 		*Ddr
  )
{
  UINT32 InitValue;	/** Initialization value */

  InitValue = 0xDEADBEEF;
  Ddr->DdrDataInit = InitValue;
}


VOID
SetDdrSdramClkCntl (
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts
  )
{
  UINT32 ClkAdjust;	/** Clock adjust */

  ClkAdjust = Popts->ClkAdjust;
  Ddr->DdrSdramClkCntl = (ClkAdjust & 0xF) << 23;
}


VOID
SetDdrInitAddr (
  OUT  FslDdrCfgRegsT 			*Ddr
  )
{
  UINT32 InitAddr = 0;	/** Initialization address */

  Ddr->DdrInitAddr = InitAddr;
}


VOID
SetDdrInitExtAddr (
  OUT  FslDdrCfgRegsT 		*Ddr
  )
{
  UINT32 Uia = 0;	       /** Use initialization address */
  UINT32 InitExtaddr = 0;	/** Initialization address */

  Ddr->DdrInitExtAddr = (0
  		 | ((Uia & 0x1) << 31)
  		 | (InitExtaddr & 0xF)
  );
}


VOID
SetTimingCfg4 (
  OUT  FslDdrCfgRegsT 		*Ddr,
  IN   CONST MemctlOptionsT 	*Popts
  )
{
  UINT32 Rwt = 0; /** Read-to-write turnaround for same CS */
  UINT32 Wrt = 0; /** Write-to-read turnaround for same CS */
  UINT32 Rrt = 0; /** Read-to-read turnaround for same CS */
  UINT32 Wwt = 0; /** Write-to-write turnaround for same CS */
  UINT32 DllLock = 0; /** Ddr SDRAM DLL Lock Time */

  if (Popts->BurstLength == DDR_BL8) {
    /** We set BL/2 for fixed BL8 */
    Rrt = 0;	/** BL/2 clocks */
    Wwt = 0;	/** BL/2 clocks */
  } else {
    /** We need to set BL/2 + 2 to BC4 and OTF */
    Rrt = 2;	/** BL/2 + 2 clocks */
    Wwt = 2;	/** BL/2 + 2 clocks */
  }
  DllLock = 1;	/** tDLLK = 512 clocks from spec */
  Ddr->TimingCfg4 = (0
  	     | ((Rwt & 0xf) << 28)
  	     | ((Wrt & 0xf) << 24)
  	     | ((Rrt & 0xf) << 20)
  	     | ((Wwt & 0xf) << 16)
  	     | (DllLock & 0x3)
  );
}


VOID
SetTimingCfg5 (
  OUT  FslDdrCfgRegsT 		*Ddr,
  IN   UINT32 			CasLatency
  )
{
  UINT32 RodtOn = 0;		/** Read to ODT on */
  UINT32 RodtOff = 0;	/** Read to ODT off */
  UINT32 WodtOn = 0;		/** Write to ODT on */
  UINT32 WodtOff = 0;	/** Write to ODT off */

  /** rodtOn = TimingCfg1[Caslat] - TimingCfg2[wrlat] + 1 */
  RodtOn = CasLatency - ((Ddr->TimingCfg2 & 0x00780000) >> 19) + 1;
  RodtOff = 4;	/**  4 clocks */
  WodtOn = 1;		/**  1 clocks */
  WodtOff = 4;	/**  4 clocks */

  Ddr->TimingCfg5 = (0
            | ((RodtOn & 0x1f) << 24)
            | ((RodtOff & 0x7) << 20)
            | ((WodtOn & 0x1f) << 12)
            | ((WodtOff & 0x7) << 8)
  );
}


VOID
SetDdrZqCntl (
  OUT  FslDdrCfgRegsT 	*Ddr,
  IN   UINT32 		ZqEn
  )
{
  UINT32 Zqinit = 0;/** POR ZQ Calibration Time (tZQinit) */
  /** Normal Operation Full Calibration Time (tZQoper) */
  UINT32 Zqoper = 0;
  /** Normal Operation Short Calibration Time (tZQCS) */
  UINT32 Zqcs = 0;

  if (ZqEn) {
    Zqinit = 9;	/** 512 clocks */
    Zqoper = 8;	/** 256 clocks */
    Zqcs = 6;		/** 64 clocks */
  }

  Ddr->DdrZqCntl = (0
  	   | ((ZqEn & 0x1) << 31)
  	   | ((Zqinit & 0xF) << 24)
  	   | ((Zqoper & 0xF) << 16)
  	   | ((Zqcs & 0xF) << 8)
  );
}


VOID
SetDdrWrlvlCntl (
  OUT  FslDdrCfgRegsT 		*Ddr,
  IN   UINT32 			WrlvlEn,
  IN   CONST MemctlOptionsT 	*Popts
  )
{
  /**
    First DQS pulse rising edge after margining Mode
    is programmed (tWL_MRD)
  **/
  UINT32 WrlvlMrd = 0;
  /** ODT delay after margining Mode is programmed (tWL_ODTEN) */
  UINT32 WrlvlOdten = 0;
  /** DQS/DQS_ delay after margining Mode is programmed (tWL_DQSEN) */
  UINT32 WrlvlDqsen = 0;
  /** WRLVL_SMPL: Write leveling sample time */
  UINT32 WrlvlSmpl = 0;
  /** WRLVL_WLR: Write leveling repeition time */
  UINT32 WrlvlWlr = 0;
  /** WRLVL_START: Write leveling start time */
  UINT32 WrlvlStart = 0;

  /** suggest enable write leveling for DDR3 due to fly-by topology */
  if (WrlvlEn) {
    /** tWL_MRD min = 40 nCK, we set it 64 */
    WrlvlMrd = 0x6;
    /** tWL_ODTEN 128 */
    WrlvlOdten = 0x7;
    /* tWL_DQSEN min = 25 nCK, we set it 32 */
    WrlvlDqsen = 0x5;
    /**
      Write leveling sample time at least need 6 clocks
      higher than tWLO to allow enough time for progagation
      delay and sampling the prime data bits.
    **/
    WrlvlSmpl = 0xf;
    /**
      Write leveling repetition time
      at least tWLO + 6 clocks clocks
      we set it 64
    **/
    WrlvlWlr = 0x6;
    /**
      Write leveling start time
      The value use for the DQS_ADJUST for the first sample
      when write leveling is enabled. It probably needs to be
      overriden per platform.
    **/
    WrlvlStart = 0x8;
    /**
      Override the write leveling sample and start time
      according to specific board
    **/
    if (Popts->WrlvlOverride) {
      WrlvlSmpl = Popts->WrlvlSample;
      WrlvlStart = Popts->WrlvlStart;
    }
  }

  Ddr->DdrWrlvlCntl = (0
         | ((WrlvlEn & 0x1) << 31)
         | ((WrlvlMrd & 0x7) << 24)
         | ((WrlvlOdten & 0x7) << 20)
         | ((WrlvlDqsen & 0x7) << 16)
         | ((WrlvlSmpl & 0xf) << 12)
         | ((WrlvlWlr & 0x7) << 8)
         | ((WrlvlStart & 0x1F) << 0)
  );
}


VOID
SetDdrSrCntr (
  OUT  FslDdrCfgRegsT *Ddr,
  IN   UINT32 SrIt
  )
{
  /** Self Refresh Idle Threshold */
  Ddr->DdrSrCntr = (SrIt & 0xF) << 16;
}


UINT32
ComputeFslMemctlConfigRegs (
  IN   CONST MemctlOptionsT 		*Popts,
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST CommonTimingParamsT 	*CommonDimm,
  IN   CONST DimmParamsT 		*DimmParams,
  IN   UINT32 				DbwCapAdj,
  IN   UINT32 				SizeOnly
  )
{
  UINT32 i;
  UINT32 CasLatency;
  UINT32 AdditiveLatency;
  UINT32 SrIt;
  UINT32 ZqEn;
  UINT32 WrlvlEn;
  UINT32 IpRev = 0;
  UINT32 UnqMrsEn = 0;
  INT32 CsEn = 1;

  InternalMemCopyMem(Ddr, 0, sizeof(FslDdrCfgRegsT));

  if (CommonDimm == NULL) {
  	return 1;
  }

  /**
    Process overrides first.

    FIXME: somehow add dereated Caslat to this
  **/
  CasLatency = (Popts->CasLatencyOverride)
  	? Popts->CasLatencyOverrideValue
	: CommonDimm->LowestCommonSPDCaslat;

  AdditiveLatency = (Popts->AdditiveLatencyOverride)
  	? Popts->AdditiveLatencyOverrideValue
  	: CommonDimm->AdditiveLatency;

  SrIt = (Popts->AutoSelfRefreshEn)
  	? Popts->SrIt
  	: 0;
  /** ZQ calibration */
  ZqEn = (Popts->ZqEn) ? 1 : 0;
  /** write leveling */
  WrlvlEn = (Popts->WrlvlEn) ? 1 : 0;

  /** Chip Select Memory Bounds (CSn_BNDS) */
  for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
    UINT64 Ea, Sa;
    UINT32 CsPerDimm
	= CONFIG_CHIP_SELECTS_PER_CTRL / CONFIG_DIMM_SLOTS_PER_CTLR;
    UINT32 DimmNumber
	= i / CsPerDimm;
    UINT64 RankDensity
	= DimmParams[DimmNumber].RankDensity >> DbwCapAdj;

    if (DimmParams[DimmNumber].NRanks == 0) {
      continue;
    }
    if (Popts->MemctlInterleaving) {
      switch (Popts->BaIntlvCtl & FSL_DDR_CS0_CS1_CS2_CS3) {
	 case FSL_DDR_CS0_CS1_CS2_CS3:
	   break;
	 case FSL_DDR_CS0_CS1:
	 case FSL_DDR_CS0_CS1_AND_CS2_CS3:
	   if (i > 1)
	     CsEn = 0;
	     break;
	 case FSL_DDR_CS2_CS3:
	 default:
	   if (i > 0)
	     CsEn = 0;
	     break;
      }
      Sa = CommonDimm->BaseAddress;
      Ea = Sa + CommonDimm->TotalMem - 1;
    } else if (!Popts->MemctlInterleaving) {
      /**
	 If memory interleaving between controllers is NOT
	 enabled, the starting address for each memory
	 controller is distinct.  However, because rank
	 interleaving is enabled, the starting and ending
	 addresses of the total memory on that memory
	 controller needs to be programmed into its
	 respective CS0_BNDS.
      **/
      switch (Popts->BaIntlvCtl & FSL_DDR_CS0_CS1_CS2_CS3) {
	 case FSL_DDR_CS0_CS1_CS2_CS3:
	   Sa = CommonDimm->BaseAddress;
	   Ea = Sa + CommonDimm->TotalMem - 1;
	   break;
	 case FSL_DDR_CS0_CS1_AND_CS2_CS3:
	   if ((i >= 2) && (DimmNumber == 0)) {
	     Sa = DimmParams[DimmNumber].BaseAddress +
	   		      2 * RankDensity;
	     Ea = Sa + 2 * RankDensity - 1;
	   } else {
	     Sa = DimmParams[DimmNumber].BaseAddress;
	     Ea = Sa + 2 * RankDensity - 1;
	   }
	   break;
	 case FSL_DDR_CS0_CS1:
	   if (DimmParams[DimmNumber].NRanks > (i % CsPerDimm)) {
	     Sa = DimmParams[DimmNumber].BaseAddress;
	     Ea = Sa + RankDensity - 1;
	     if (i != 1)
	       Sa += (i % CsPerDimm) * RankDensity;
		Ea += (i % CsPerDimm) * RankDensity;
	     } else {
	       Sa = 0;
		Ea = 0;
	     }
	     if (i == 0)
	       Ea += RankDensity;
	     break;
	 case FSL_DDR_CS2_CS3:
	   if (DimmParams[DimmNumber].NRanks > (i % CsPerDimm)) {
	     Sa = DimmParams[DimmNumber].BaseAddress;
	     Ea = Sa + RankDensity - 1;
	     if (i != 3)
	       Sa += (i % CsPerDimm) * RankDensity;
	     Ea += (i % CsPerDimm) * RankDensity;
	   } else {
	     Sa = 0;
	     Ea = 0;
	   }
	   if (i == 2)
	     Ea += (RankDensity >> DbwCapAdj);
	   break;
	 default:  /** No bank(chip-select) interleaving */
	   Sa = DimmParams[DimmNumber].BaseAddress; /** DOUBT base address shows 0 in uboot code*/
	   Ea = Sa + RankDensity - 1;
	   if (DimmParams[DimmNumber].NRanks > (i % CsPerDimm)) {
	     Sa += (i % CsPerDimm) * RankDensity;
	     Ea += (i % CsPerDimm) * RankDensity;
	   } else {
	     Sa = 0;
	     Ea = 0;
	   }
	   break;
      }
    }

    Sa >>= 24;
    Ea >>= 24;

    if (CsEn) {
      Ddr->Cs[i].Bnds = (0
		| ((Sa & 0xffff) << 16)     /* starting address */
		| ((Ea & 0xffff) << 0)	/* ending address */
      );
    } else {
      /** setting Bnds to 0xffffffff for inactive CS */
      Ddr->Cs[i].Bnds = 0xffffffff;
    }

    SetCsnConfig(DimmNumber, i, Ddr, Popts, DimmParams);
    SetCsnConfig_2(i, Ddr);
  }

  /**
    In the case we only need to compute the Ddr sdram size, we only need
    to set csn registers, so return from here.
  **/
  if (SizeOnly)
    return 0;

  SetDdrEor(Ddr, Popts);

  SetTimingCfg0(Ddr, Popts, DimmParams);

  SetTimingCfg3(Ddr, Popts, CommonDimm, CasLatency,
		 AdditiveLatency);
  SetTimingCfg1(Ddr, Popts, CommonDimm, CasLatency);
  SetTimingCfg2(Ddr, Popts, CommonDimm,
  			CasLatency, AdditiveLatency);

  SetDdrCdr1(Ddr, Popts);
  SetDdrCdr2(Ddr, Popts);
  SetDdrSdramCfg(Ddr, Popts, CommonDimm);
  IpRev = FslDdrGetVersion();
  if (IpRev > 0x40400)
    UnqMrsEn = 1;

  SetDdrSdramCfg2(Ddr, Popts, UnqMrsEn);
  SetDdrSdramMode(Ddr, Popts, CommonDimm,
			CasLatency, AdditiveLatency, UnqMrsEn);
  SetDdrSdramMode2(Ddr, Popts, CommonDimm, UnqMrsEn);
  set_DdrSdramInterval(Ddr, Popts, CommonDimm);
  SetDdrDataInit(Ddr);
  SetDdrSdramClkCntl(Ddr, Popts);
  SetDdrInitAddr(Ddr);
  SetDdrInitExtAddr(Ddr);
  SetTimingCfg4(Ddr, Popts);
  SetTimingCfg5(Ddr, CasLatency);

  SetDdrZqCntl(Ddr, ZqEn);
  SetDdrWrlvlCntl(Ddr, WrlvlEn, Popts);

  SetDdrSrCntr(Ddr, SrIt);

  SetDdrSdramRcw(Ddr, Popts, CommonDimm);

  return CheckFslMemctlConfigRegs(Ddr);
}


