/** @DdrCntrlRegs.c
  Ddr Library containing functions to calculate value for
  ddr controller registers

  Copyright (c) 2014, Freescale Ltd. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <LS1043aSocLib.h>
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
    DEBUG((EFI_D_INFO,"Error: DDR_SDRAM_CFG[RD_EN] And DDR_SDRAM_CFG[2T_EN] "
	" Should Not Be Set At The Same Time.\n"));
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
  UINT8 TaxpdMclk = 0;
  /** Mode register set cycle time (tMRD). */
  UINT8 TmrdMclk;
#if defined(CONFIG_SYS_FSL_DDR4) || defined(CONFIG_SYS_FSL_DDR3)
  CONST UINT32 MclkPs = GetMemoryClkPeriodPs();
#endif

#ifdef CONFIG_SYS_FSL_DDR4
  /* TXP=Max(4nCK, 6ns) */
  INT32 Txp = Max((INT32)MclkPs * 4, 6000); /* Unit=Ps */
  UINT32 DataRate = GetDdrFreq();

  /* for Faster Clock, Need More Time for Data Setup */
  TrwtMclk = (DataRate/1000000 > 1900) ? 3 : 2;
  TwrtMclk = 1;
  ActPdExitMclk = PicosToMclk(Txp);
  PrePdExitMclk = ActPdExitMclk;
  /*
   * MRS_CYC = Max(TMRD, TMOD)
   * TMRD = 8nCK, TMOD = Max(24nCK, 15ns)
   */
  TmrdMclk = Max(24U, PicosToMclk(15000));
#elif defined(CONFIG_SYS_FSL_DDR3)
  UINT32 DataRate = GetDdrFreq();
  INT32 Txp;
  UINT32 IpRev;
  INT32 OdtOverlap;

  /**
    (tXARD and tXARDS). Empirical?
    The DDR3 spec has not tXARD,
    we use the Txp instead of it.
    Txp=Max(3nCK, 7.5ns) for DDR3.
    spec has not the tAXPD, we use
    tAXPD=1, need design to confirm.
   **/
  Txp = Max(((INT32)MclkPs * 3), (MclkPs > 1540 ? 7500 : 6000));

  IpRev = FslDdrGetVersion();
  if (IpRev >= 0x40700) {
  /*
   * MRSCYC = max(tMRD, tMOD)
   * tMRD = 4nCK (8nCK for RDIMM)
   * tMOD = max(12nCK, 15ns)
   */
  TmrdMclk = Max((UINT32)12,
		PicosToMclk(15000));
  } else {
  /*
   * MRSCYC = tMRD
   * tMRD = 4nCK (8nCK for RDIMM)
   */
  if (Popts->RegisteredDimmEn)
    TmrdMclk = 8;
  else
    TmrdMclk = 4;
  }

  /** Set The Turnaround Time */

  /**
    for single quad-rank DIMM and two dual-rank DIMMs
    to aVOID ODT overlap
   **/
  OdtOverlap = AvoidOdtOverlap(DimmParams);
  switch (OdtOverlap) {
    case 2:
      TwwtMclk = 2;
      TrrtMclk = 1;
      break;
    case 1:
      TwwtMclk = 1;
      TrrtMclk = 0;
      break;
    default:
      break;
  }

  /** for Faster Clock, Need More Time for Data Setup */
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
#endif

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
  DEBUG((EFI_D_INFO, "FSLDDR: timing_cfg_0 = 0x%08x\n", Ddr->TimingCfg0));
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
#ifdef CONFIG_SYS_FSL_DDR4
  ExtRefrec = (PicosToMclk(CommonDimm->Trfc1Ps) - 8) >> 4;
#else
  ExtRefrec = (PicosToMclk(CommonDimm->TrfcPs) - 8) >> 4;
#endif
  /** Extwrrec Only Deals With 16 Clock And Above, Or 14 With OTF */
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
  DEBUG((EFI_D_INFO,"FSLDDR: TimingCfg3 = 0x%08x\n", Ddr->TimingCfg3));
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
#ifdef CONFIG_SYS_FSL_DDR4
	/* DDR4 Supports 10, 12, 14, 16, 18, 20, 24 */
  CONST UINT8 WrrecTable[] = {
		10, 10, 10, 10, 10,
		10, 10, 10, 10, 10,
		12, 12, 14, 14, 16,
		16, 18, 18, 20, 20,
		24, 24, 24, 24};
#else
  /** DDR_SDRAM_MODE Doesn'T Support 9,11,13,15 */
  CONST UINT8 WrrecTable[] = {
  	1, 2, 3, 4, 5, 6, 7, 8, 10, 10, 12, 12, 14, 14, 0, 0};
#endif
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
  if (FslDdrGetVersion() <= 0x40400)
    CaslatCtrl = 2 * CasLatency - 1;
  else
    CaslatCtrl = (CasLatency - 1) << 1;

#ifdef CONFIG_SYS_FSL_DDR4
  RefrecCtrl = PicosToMclk(CommonDimm->Trfc1Ps) - 8;
  WrrecMclk = PicosToMclk(CommonDimm->TwrPs);
  ActtoactMclk = Max(PicosToMclk(CommonDimm->TrrdPs), 4U);
  WrtordMclk = Max(2U, PicosToMclk(2500));
  if (!((WrrecMclk < 1) || (WrrecMclk > 24)))
    WrrecMclk = WrrecTable[WrrecMclk - 1];
#else
  RefrecCtrl = PicosToMclk(CommonDimm->TrfcPs) - 8;
  WrrecMclk = PicosToMclk(CommonDimm->TwrPs);
  ActtoactMclk = PicosToMclk(CommonDimm->TrrdPs);
  WrtordMclk = PicosToMclk(CommonDimm->TwtrPs);
  if (!((WrrecMclk < 1) || (WrrecMclk > 16)))
    WrrecMclk = WrrecTable[WrrecMclk - 1];
#endif


  if (Popts->OtfBurstChopEn)
    WrrecMclk += 2;

  /**
    JEDEC has min requirement for tRRD
   **/
#if defined(CONFIG_SYS_FSL_DDR3)
  if (ActtoactMclk < 4)
    ActtoactMclk = 4;
#endif
  /**
    JEDEC has some min requirements for tWTR
   **/
#if defined(CONFIG_SYS_FSL_DDR3)
  if (WrtordMclk < 4)
    WrtordMclk = 4;
#endif

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
  DEBUG((EFI_D_INFO,"FSLDDR: TimingCfg1 = 0x%08x\n", Ddr->TimingCfg1));
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

#ifdef CONFIG_SYS_FSL_DDR3
  CONST UINT32 MclkPs = GetMemoryClkPeriodPs();
#endif

  /** FIXME Add Check That This Must Be Less Than ActtorwMclk */
  AddLatMclk = AdditiveLatency;
  Cpo = Popts->CpoOverride;

  WrLat = ComputeCasWriteLatency();

#ifdef CONFIG_SYS_FSL_DDR4
  RdToPre = PicosToMclk(7500);
#else
  RdToPre = PicosToMclk(CommonDimm->TrtpPs);
#endif

  /**
    JEDEC has some min requirements for tRTP
  **/
  if (RdToPre < 4)
    RdToPre = 4;
  if (Popts->OtfBurstChopEn)
    RdToPre += 2; /** according to UM */

  WrDataDelay = Popts->WriteDataDelay;

#ifdef CONFIG_SYS_FSL_DDR4
	Cpo = 0;
	CkePls = Max(3U, PicosToMclk(5000));
#elif defined(CONFIG_SYS_FSL_DDR3)
	/*
	 * Cke Pulse = Max(3nCK, 7.5ns) for DDR3-800
	 *             Max(3nCK, 5.625ns) for DDR3-1066, 1333
	 *             Max(3nCK, 5ns) for DDR3-1600, 1866, 2133
	 */
	CkePls = Max(3U, PicosToMclk(MclkPs > 1870 ? 7500 :
				        (MclkPs > 1245 ? 5625 : 5000)));
#else
	CkePls = FSL_DDR_MIN_TCKE_PULSE_WIDTH_DDR;
#endif
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
  DEBUG((EFI_D_INFO,"FSLDDR: TimingCfg2 = 0x%08x\n", Ddr->TimingCfg2));
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
  IN   INT32 				I,
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
#ifdef CONFIG_SYS_FSL_DDR4
  UINT32 BgBitsCsN = 0; /* Num Of Bank Group Bits */
#else
  UINT32 NBanksPerSdramDevice;
#endif

  /** Compute CS_CONFIG only for existing ranks of each DIMM.  */
  switch (I) {
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
    CsNEn = 1;
    ApNEn = Popts->CsLocalOpts[I].AutoPrecharge;
    OdtRdCfg = Popts->CsLocalOpts[I].OdtRdCfg;
    OdtWrCfg = Popts->CsLocalOpts[I].OdtWrCfg;
#ifdef CONFIG_SYS_FSL_DDR4
    BaBitsCsN = DimmParams[DimmNumber].BankAddrBits;
    BgBitsCsN = DimmParams[DimmNumber].BankGroupBits;
#else
    NBanksPerSdramDevice
	= DimmParams[DimmNumber].NBanksPerSdramDevice;
    BaBitsCsN = __ILog2(NBanksPerSdramDevice) - 2;
#endif
    RowBitsCsN = DimmParams[DimmNumber].NRowAddr - 12;
    ColBitsCsN = DimmParams[DimmNumber].NColAddr - 8;
  }
  Ddr->Cs[I].Config = (0
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
#ifdef CONFIG_SYS_FSL_DDR4
	| ((BgBitsCsN & 0x3) << 4)
#endif
  	| ((ColBitsCsN & 0x7) << 0)
  );
  DEBUG((EFI_D_INFO, "FSLDDR: Cs[%d]Config = 0x%08x\n", I,Ddr->Cs[I].Config));
}


VOID
SetCsnConfig_2 (
  IN   INT32 				I,
  OUT  FslDdrCfgRegsT 		*Ddr
  )
{
  UINT32 PasrCfg = 0;	/** Partial array self refresh Config */

  Ddr->Cs[I].Config2 = ((PasrCfg & 7) << 24);
}

/* DDR SDRAM Register Control Word */
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
	DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramRcw1 = 0x%08x\n", Ddr->DdrSdramRcw1));
	DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramRcw2 = 0x%08x\n", Ddr->DdrSdramRcw2));
  }
}


/* DDR SDRAM Control Configuration (DDR_SDRAM_CFG) */
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
  DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramCfg = 0x%08x\n", Ddr->DdrSdramCfg));
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
  INT32 I;

#ifndef CONFIG_SYS_FSL_DDR4
  UINT32 DllRstDis = 1;	/* DLL Reset Disable */
  UINT32 DqsCfg = Popts->DqsConfig;
#endif

  for (I = 0; I < CONFIG_CHIP_SELECTS_PER_CTRL; I++) {
    if (Popts->CsLocalOpts[I].OdtRdCfg
      	|| Popts->CsLocalOpts[I].OdtWrCfg) {
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
#ifndef CONFIG_SYS_FSL_DDR4
  	| ((DllRstDis & 0x1) << 29)
  	| ((DqsCfg & 0x3) << 26)
#endif
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
  DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramCfg2 = 0x%08x\n", Ddr->DdrSdramCfg2));
}


#ifdef CONFIG_SYS_FSL_DDR4
/* DDR SDRAM Mode Configuration 2 (DDR_SDRAM_MODE_2) */
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

  INT32 I;
  UINT32 WrCrc = 0;  /** Disable */
  UINT32 RttWr = 0;	/** Rtt_WR - Dynamic ODT Off */
  UINT32 Srt = 0;	/** Self-Refresh Temerature, Normal Range */
  UINT32 Cwl = ComputeCasWriteLatency() - 9;
  UINT32 Mpr = 0;	/* Serial */
  UINT32 WcLat;
  CONST UINT32 MclkPs = GetMemoryClkPeriodPs();

  if (Popts->RttOverride)
    RttWr = Popts->RttWrOverrideValue;
  else
    RttWr = Popts->CsLocalOpts[0].OdtRttWr;

  if (CommonDimm->ExtendedOpSrt)
    Srt = CommonDimm->ExtendedOpSrt;

  ESdmode2 = (0
  	| ((WrCrc & 0x1) << 12)
  	| ((RttWr & 0x3) << 9)
  	| ((Srt & 0x3) << 6)
  	| ((Cwl & 0x7) << 3));

  if (MclkPs >= 1250)
    WcLat = 0;
  else if (MclkPs >= 833)
    WcLat = 1;
  else
    WcLat = 2;

  ESdmode3 = (0
	| ((Mpr & 0x3) << 11)
	| ((WcLat & 0x3) << 9));

  Ddr->DdrSdramMode2 = (0
	| ((ESdmode2 & 0xFFFF) << 16)
	| ((ESdmode3 & 0xFFFF) << 0)
  );

  if (UnqMrsEn) {	/** Unique Mode Registers Are Supported */
    for (I = 1; I < CONFIG_CHIP_SELECTS_PER_CTRL; I++) {
      if (Popts->RttOverride)
        RttWr = Popts->RttWrOverrideValue;
      else
        RttWr = Popts->CsLocalOpts[I].OdtRttWr;

      ESdmode2 &= 0xF9FF;	/** Clear Bit 10, 9 */
      ESdmode2 |= (RttWr & 0x3) << 9;
      switch (I) {
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
#elif defined(CONFIG_SYS_FSL_DDR3)
/* DDR SDRAM Mode Configuration 2 (DDR_SDRAM_MODE_2) */
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

  INT32 I;
  UINT32 RttWr = 0;	/** Rtt_WR - Dynamic ODT Off */
  UINT32 Srt = 0;	/** Self-Refresh Temerature, Normal Range */
  UINT32 Asr = 0;	/** Auto Self-Refresh Disable */
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
    for (I = 1; I < CONFIG_CHIP_SELECTS_PER_CTRL; I++) {
      if (Popts->RttOverride)
        RttWr = Popts->RttWrOverrideValue;
      else
        RttWr = Popts->CsLocalOpts[I].OdtRttWr;

      ESdmode2 &= 0xF9FF;	/** clear bit 10, 9 */
      ESdmode2 |= (RttWr & 0x3) << 9;
      switch (I) {
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
#endif

#ifdef CONFIG_SYS_FSL_DDR4
/* DDR SDRAM Mode Configuration 9 (DDR_SDRAM_MODE_9) */
VOID SetDdrSdramMode9(
  OUT  FslDdrCfgRegsT *Ddr,
  IN   CONST MemctlOptionsT *Popts,
  IN   CONST CommonTimingParamsT *CommonDimm,
  IN   CONST UINT32 UnqMrsEn
  )
{
  INT32 I;
  UINT16 Esdmode4 = 0;	/* Extended SDRAM Mode 4 */
  UINT16 Esdmode5;	/* Extended SDRAM Mode 5 */

  Esdmode5 = 0x00000500;		/* Data Mask Enabled */

  Ddr->DdrSdramMode9 = (0
	| ((Esdmode4 & 0xffff) << 16)
	| ((Esdmode5 & 0xffff) << 0)
	);

  /* Only Mode9 Use 0x500, Others Use 0x400 */
  Esdmode5 = 0x00000400;		/* Data Mask Enabled */

  DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramMode9) = 0x%08x\n", Ddr->DdrSdramMode9));
  if (UnqMrsEn) {	/* Unique Mode Registers Are Supported */
    for (I = 1; I < CONFIG_CHIP_SELECTS_PER_CTRL; I++) {
      switch (I) {
	 case 1:
	   Ddr->DdrSdramMode11 = (0
	   		| ((Esdmode4 & 0xFFFF) << 16)
			| ((Esdmode5 & 0xFFFF) << 0)
		 );
	   break;
	 case 2:
	   Ddr->DdrSdramMode13 = (0
			| ((Esdmode4 & 0xFFFF) << 16)
			| ((Esdmode5 & 0xFFFF) << 0)
		 );
	   break;
	 case 3:
	   Ddr->DdrSdramMode15 = (0
			| ((Esdmode4 & 0xFFFF) << 16)
			| ((Esdmode5 & 0xFFFF) << 0)
	 	 );
	   break;
      }
    }
    DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramMode11 = 0x%08x\n",
		      Ddr->DdrSdramMode11));
    DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramMode13 = 0x%08x\n",
		      Ddr->DdrSdramMode13));
    DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramMode15 = 0x%08x\n",
		      Ddr->DdrSdramMode15));
  }
}

/* DDR SDRAM Mode Configuration 10 (DDR_SDRAM_MODE_10) */
VOID SetDdrSdramMode10 (
  OUT FslDdrCfgRegsT *Ddr,
  IN  CONST MemctlOptionsT *Popts,
  IN  CONST CommonTimingParamsT *CommonDimm,
  IN  CONST UINT32 UnqMrsEn
  )
{
  INT32 I;
  UINT16 Esdmode6 = 0;	/* Extended SDRAM Mode 6 */
  UINT16 Esdmode7 = 0;	/* Extended SDRAM Mode 7 */
  UINT32 TccdlMin = PicosToMclk(CommonDimm->TccdlPs);

  Esdmode6 = ((TccdlMin - 4) & 0x7) << 10;

  Ddr->DdrSdramMode10 = (0
		 | ((Esdmode6 & 0xffff) << 16)
		 | ((Esdmode7 & 0xffff) << 0)
	);
  DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramMode10) = 0x%08x\n", Ddr->DdrSdramMode10));
  if (UnqMrsEn) {	/* Unique Mode Registers Are Supported */
    for (I = 1; I < CONFIG_CHIP_SELECTS_PER_CTRL; I++) {
      switch (I) {
	 case 1:
	   Ddr->DdrSdramMode12 = (0
			| ((Esdmode6 & 0xFFFF) << 16)
			| ((Esdmode7 & 0xFFFF) << 0)
		 );
	   break;
	 case 2:
	   Ddr->DdrSdramMode14 = (0
			| ((Esdmode6 & 0xFFFF) << 16)
			| ((Esdmode7 & 0xFFFF) << 0)
		 );
	   break;
	 case 3:
	   Ddr->DdrSdramMode16 = (0
			| ((Esdmode6 & 0xFFFF) << 16)
			| ((Esdmode7 & 0xFFFF) << 0)
		 );
	   break;
	 }
      }
  DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramMode12 = 0x%08x\n",
		      Ddr->DdrSdramMode12));
  DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramMode14 = 0x%08x\n",
		      Ddr->DdrSdramMode14));
  DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramMode16 = 0x%08x\n",
		      Ddr->DdrSdramMode16));
  }
}
#endif

/* DDR SDRAM Interval Configuration (DDR_SDRAM_INTERVAL) */
VOID
SetDdrSdramInterval (
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
  DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramInterval = 0x%08x\n", Ddr->DdrSdramInterval));
}


#ifdef CONFIG_SYS_FSL_DDR4
/* DDR SDRAM Mode Configuration Set (DDR_SDRAM_MODE) */
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
  INT32 I = 0;
  UINT16 ESdmode;		/** Extended SDRAM Mode */
  UINT16 Sdmode;		/** SDRAM Mode */

  /** Mode Register - MR1 */
  UINT32 Qoff = 0;		/** Output Buffer Enable 0=Yes, 1=No */
  UINT32 TdqsEn = 0;	       /** TDQS Enable: 0=No, 1=Yes */
  UINT32 Rtt;
  UINT32 WrlvlEn = 0;	/** Write Level Enable: 0=No, 1=Yes */
  UINT32 Al = 0;		/** Posted CAS# Additive Latency (AL) */
  UINT32 Dic = 0;		/** Output Driver Impedance, 40ohm */
  UINT32 DllEn = 0;	       /** DLL Enable  0=Enable (Normal),
					       1=Disable (Test/Debug) */

  /** Mode Register - MR0 */
  UINT32 Wr = 0;	/** Write Recovery */
  UINT32 DllRst;	/** DLL Reset */
  UINT32 Mode;	/** Normal=0 Or Test=1 */
  UINT32 Caslat = 4; /** CAS# Latency, default Set As 6 Cycles */
  /** BT: Burst Type (0=Nibble Sequential, 1=Interleaved) */
  UINT32 Bt;
  UINT32 Bl;	/** BL: Burst Length */

  UINT32 WrMclk;
  /* DDR4 Support WR 10, 12, 14, 16, 18, 20, 24 */
  CONST UINT8 WrTable[] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6};

  /* DDR4 Support CAS 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 24 */
  CONST UINT8 CasLatencyTable[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 8,
		9, 9, 10, 10, 11, 11};

  if (Popts->RttOverride)
    Rtt = Popts->RttOverrideValue;
  else
    Rtt = Popts->CsLocalOpts[0].OdtRttNorm;

  if (AdditiveLatency == (CasLatency - 1))
    Al = 1;
  if (AdditiveLatency == (CasLatency - 2))
    Al = 2;

  if (Popts->QuadRankPresent)
    Dic = 1;	/** Output Driver Impedance 240/7 Ohm */

  /**
    The ESdmode Value Will Also Be Used for Writing
    MR1 During Write Leveling for DDR3, Although The
    Bits Specifically Related To The Write Leveling
    Scheme Will Be Handled Automatically By The Ddr
    Controller. So We Set The WrlvlEn = 0 Here.
  **/
  ESdmode = (0
  	| ((Qoff & 0x1) << 12)
  	| ((TdqsEn & 0x1) << 11)
  	| ((Rtt & 0x7) << 8)   /** Rtt Field Is Split */
  	| ((WrlvlEn & 0x1) << 7)
  	| ((Al & 0x3) << 3)
  	| ((Dic & 0x3) << 1)   /** DIC Field Is Split */
  	| ((DllEn & 0x1) << 0)
  );

  /**
    DLL Control for Precharge PD
    0=Slow Exit DLL Off (TxpDLL)
    1=Fast Exit DLL On (Txp)
  **/
  WrMclk = PicosToMclk(CommonDimm->TwrPs);
  if (WrMclk <= 24) {
    Wr = WrTable[WrMclk - 10];
  } else {
    DEBUG((EFI_D_INFO,"Error: Unsupported Write Recovery "
		"for Mode Register WrMclk = %d\n", WrMclk));
  }

  DllRst = 0;	/** Dll No Reset */
  Mode = 0;	/** Normal Mode */

  /** Look Up Table To Get The Cas Latency Bits */
  if (CasLatency >= 9 && CasLatency <= 24)
    Caslat = CasLatencyTable[CasLatency - 9];
  else
    DEBUG((EFI_D_INFO,"R: Unsupported Cas Latency for Mode Register\n"));

  Bt = 0;	/** Nibble Sequential */

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
      DEBUG((EFI_D_INFO,"Error: Invalid Burst Length Of %d Specified. ",
	       Popts->BurstLength));
      DEBUG((EFI_D_INFO,"Defaulting To On-The-Fly BC4 Or BL8 Beats.\n"));
      Bl = 1;
      break;
  }

  Sdmode = (0
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

  DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramMode = 0x%08x\n", Ddr->DdrSdramMode));

  if (UnqMrsEn) {	/** Unique Mode Registers Are Supported */
    for (I = 1; I < CONFIG_CHIP_SELECTS_PER_CTRL; I++) {
      if (Popts->RttOverride)
        Rtt = Popts->RttOverrideValue;
      else
  	 Rtt = Popts->CsLocalOpts[I].OdtRttNorm;

      ESdmode &= 0xF8FF;	/** Clear Bit 10,9,8 for Rtt */
      ESdmode |= ((Rtt & 0x7) << 8);
      switch (I) {
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
#elif defined(CONFIG_SYS_FSL_DDR3)
/* DDR SDRAM Mode Configuration Set (DDR_SDRAM_MODE) */
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
  INT32 I;
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

  WrMclk = PicosToMclk(CommonDimm->TwrPs);
  if (WrMclk <= 16) {
    Wr = WrTable[WrMclk - 5];
  }

  DllRst = 0;	/** dll no reset */
  Mode = 0;	/** normal Mode */

  /** look up table to get the cas latency bits */
  if (CasLatency >= 5 && CasLatency <= 16) {
    UINT8 CasLatencyTable[] = {
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
    Caslat = CasLatencyTable[CasLatency - 5];
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
    for (I = 1; I < CONFIG_CHIP_SELECTS_PER_CTRL; I++) {
      if (Popts->RttOverride)
        Rtt = Popts->RttOverrideValue;
      else
  	 Rtt = Popts->CsLocalOpts[I].OdtRttNorm;

      ESdmode &= 0xFDBB;	/** clear bit 9,6,2 */
      ESdmode |= (0
		| ((Rtt & 0x4) << 7)   /** Rtt field is split */
		| ((Rtt & 0x2) << 5)   /** Rtt field is split */
		| ((Rtt & 0x1) << 2)   /** Rtt field is split */
      );
      switch (I) {
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
#endif

/* DDR SDRAM Data Initialization (DDR_DATA_INIT) */
VOID
SetDdrDataInit (
  OUT  FslDdrCfgRegsT 		*Ddr
  )
{
  UINT32 InitValue;	/** Initialization Value */

  InitValue = 0xDEADBEEF;
  Ddr->DdrDataInit = InitValue;
}

/*
 * DDR SDRAM Clock Control (DDR_SDRAM_CLK_CNTL)
 * The Old Controller On The 8540/60 Doesn'T Have This Register.
 * Hope It's OK To Set It (To 0) Anyway.
 */
VOID
SetDdrSdramClkCntl (
  OUT  FslDdrCfgRegsT 			*Ddr,
  IN   CONST MemctlOptionsT 		*Popts
  )
{
  UINT32 ClkAdjust;	/** Clock adjust */

  ClkAdjust = Popts->ClkAdjust;
  Ddr->DdrSdramClkCntl = (ClkAdjust & 0xF) << 23;
  DEBUG((EFI_D_INFO,"FSLDDR: ClkCntl = 0x%08x\n", Ddr->DdrSdramClkCntl));
}


VOID
SetDdrInitAddr (
  OUT  FslDdrCfgRegsT 			*Ddr
  )
{
  UINT32 InitAddr = 0;	/** Initialization address */

  Ddr->DdrInitAddr = InitAddr;
}

/* DDR Initialization Address (DDR_INIT_EXT_ADDR) */
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

/* DDR SDRAM Timing Configuration 4 (TIMING_CFG_4) */
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
#ifdef CONFIG_SYS_FSL_DDR4
  DllLock = 2;	/* TDLLK = 1024 Clocks */
#elif defined(CONFIG_SYS_FSL_DDR3)
  DllLock = 1;	/* TDLLK = 512 Clocks From Spec */
#endif
  Ddr->TimingCfg4 = (0
  	     | ((Rwt & 0xf) << 28)
  	     | ((Wrt & 0xf) << 24)
  	     | ((Rrt & 0xf) << 20)
  	     | ((Wwt & 0xf) << 16)
  	     | (DllLock & 0x3)
  );
  DEBUG((EFI_D_INFO,"FSLDDR: TimingCfg4 = 0x%08x\n", Ddr->TimingCfg4));
}

/* DDR SDRAM Timing Configuration 5 (TIMING_CFG_5) */
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

  UINT32 WrLat = ((Ddr->TimingCfg2 & 0x00780000) >> 19) +
		    ((Ddr->TimingCfg2 & 0x00040000) >> 14);
  /** RodtOn = TimingCfg1[Caslat] - TimingCfg2[Wrlat] + 1 */
  if (CasLatency >= WrLat)
    RodtOn = CasLatency - WrLat + 1;
  RodtOff = 4;	/**  4 Clocks */
  WodtOn = 1;		/**  1 Clocks */
  WodtOff = 4;	/**  4 Clocks */

  Ddr->TimingCfg5 = (0
            | ((RodtOn & 0x1f) << 24)
            | ((RodtOff & 0x7) << 20)
            | ((WodtOn & 0x1f) << 12)
            | ((WodtOff & 0x7) << 8)
  );
  DEBUG((EFI_D_INFO,"FSLDDR: TimingCfg5 = 0x%08x\n", Ddr->TimingCfg5));
}

#ifdef CONFIG_SYS_FSL_DDR4
static VOID
SetTimingCfg6 (
  OUT  FslDdrCfgRegsT *Ddr
  )
{
  UINT32 HsCaslat = 0;
  UINT32 HsWrlat = 0;
  UINT32 HsWrrec = 0;
  UINT32 HsClkadj = 0;
  UINT32 HsWrlvlStart = 0;

  Ddr->TimingCfg6 = (0
		| ((HsCaslat & 0x1f) << 24)
		| ((HsWrlat & 0x1f) << 19)
		| ((HsWrrec & 0x1f) << 12)
		| ((HsClkadj & 0x1f) << 6)
		| ((HsWrlvlStart & 0x1f) << 0)
	);
  DEBUG((EFI_D_INFO,"FSLDDR: TimingCfg6 = 0x%08x\n", Ddr->TimingCfg6));
}

static VOID
SetTimingCfg7 (
  OUT  FslDdrCfgRegsT *Ddr,
  IN  CONST CommonTimingParamsT *CommonDimm
  )
{
  UINT32 Txpr, Tcksre, Tcksrx;
  UINT32 CkeRst, Cksre, Cksrx, ParLat, CsToCmd;

  Txpr = Max(5U, PicosToMclk(CommonDimm->Trfc1Ps + 10000));
  Tcksre = Max(5U, PicosToMclk(10000));
  Tcksrx = Max(5U, PicosToMclk(10000));
  ParLat = 0;
  CsToCmd = 0;

  if (Txpr <= 200)
    CkeRst = 0;
  else if (Txpr <= 256)
    CkeRst = 1;
  else if (Txpr <= 512)
    CkeRst = 2;
  else
    CkeRst = 3;

  if (Tcksre <= 19)
    Cksre = Tcksre - 5;
  else
    Cksre = 15;

  if (Tcksrx <= 19)
    Cksrx = Tcksrx - 5;
  else
    Cksrx = 15;

  Ddr->TimingCfg7 = (0
	     | ((CkeRst & 0x3) << 28)
	     | ((Cksre & 0xf) << 24)
	     | ((Cksrx & 0xf) << 20)
	     | ((ParLat & 0xf) << 16)
	     | ((CsToCmd & 0xf) << 4)
       );
  DEBUG((EFI_D_INFO,"FSLDDR: TimingCfg7 = 0x%08x\n", Ddr->TimingCfg7));
}

static VOID
SetTimingCfg8 (
  OUT  FslDdrCfgRegsT *Ddr,
  IN   CONST MemctlOptionsT *Popts,
  IN   CONST CommonTimingParamsT *CommonDimm,
  IN   UINT32 CasLatency
  )
{
  UINT32 RwtBg, WrtBg, RrtBg, WwtBg;
  UINT32 ActtoactBg, WrtordBg, PreAllRec;
  UINT32 Tccdl = PicosToMclk(CommonDimm->TccdlPs);
  UINT32 WrLat = ((Ddr->TimingCfg2 & 0x00780000) >> 19) +
			      ((Ddr->TimingCfg2 & 0x00040000) >> 14);

  RwtBg = CasLatency + 2 + 4 - WrLat;
  if (RwtBg < Tccdl)
    RwtBg = Tccdl - RwtBg;
  else
    RwtBg = 0;

  WrtBg = WrLat + 4 + 1 - CasLatency;
  if (WrtBg < Tccdl)
    WrtBg = Tccdl - WrtBg;
  else
    WrtBg = 0;

  if (Popts->BurstLength == DDR_BL8) {
    RrtBg = Tccdl - 4;
    WwtBg = Tccdl - 4;
  } else {
    RrtBg = Tccdl - 2;
    WwtBg = Tccdl - 2;
  }

  ActtoactBg = PicosToMclk(CommonDimm->TrrdlPs);
  WrtordBg = Max(4U, PicosToMclk(7500));
  if (Popts->OtfBurstChopEn)
    WrtordBg += 2;

  PreAllRec = 0;

  Ddr->TimingCfg8 = (0
	     | ((RwtBg & 0xf) << 28)
	     | ((WrtBg & 0xf) << 24)
	     | ((RrtBg & 0xf) << 20)
	     | ((WwtBg & 0xf) << 16)
	     | ((ActtoactBg & 0xf) << 12)
	     | ((WrtordBg & 0xf) << 8)
	     | ((PreAllRec & 0x1f) << 0)
       );

  DEBUG((EFI_D_INFO,"FSLDDR: TimingCfg8 = 0x%08x\n", Ddr->TimingCfg8));
}

static VOID SetTimingCfg9 (
  OUT  FslDdrCfgRegsT *Ddr
  )
{
  Ddr->TimingCfg9 = 0;
  DEBUG((EFI_D_INFO,"FSLDDR: TimingCfg9 = 0x%08x\n", Ddr->TimingCfg9));
}

/* This Function Needs To Be Called After SetDdrSdramCfg() Is Called */
static VOID SetDdrDqMapping (
  OUT  FslDdrCfgRegsT *Ddr,
  IN   CONST DimmParamsT *DimmParams
  )
{
#ifndef INTPSR_DDR4_WORKAROUND
  UINT32 AccEccEn = (Ddr->DdrSdramCfg >> 2) & 0x1;

  Ddr->DqMap0 = ((DimmParams->DqMapping[0] & 0x3F) << 26) |
			((DimmParams->DqMapping[1] & 0x3F) << 20) |
			((DimmParams->DqMapping[2] & 0x3F) << 14) |
			((DimmParams->DqMapping[3] & 0x3F) << 8) |
			((DimmParams->DqMapping[4] & 0x3F) << 2);

  Ddr->DqMap1 = ((DimmParams->DqMapping[5] & 0x3F) << 26) |
			((DimmParams->DqMapping[6] & 0x3F) << 20) |
			((DimmParams->DqMapping[7] & 0x3F) << 14) |
			((DimmParams->DqMapping[10] & 0x3F) << 8) |
			((DimmParams->DqMapping[11] & 0x3F) << 2);

  Ddr->DqMap2 = ((DimmParams->DqMapping[12] & 0x3F) << 26) |
			((DimmParams->DqMapping[13] & 0x3F) << 20) |
			((DimmParams->DqMapping[14] & 0x3F) << 14) |
			((DimmParams->DqMapping[15] & 0x3F) << 8) |
			((DimmParams->DqMapping[16] & 0x3F) << 2);

	/* DqMap for ECC[4:7] Is Set To 0 if Accumulated ECC Is Enabled */
  Ddr->DqMap3 = ((DimmParams->DqMapping[17] & 0x3F) << 26) |
			((DimmParams->DqMapping[8] & 0x3F) << 20) |
			(AccEccEn ? 0 :
			 (DimmParams->DqMapping[9] & 0x3F) << 14) |
			DimmParams->DqMappingOrs;
#endif
	DEBUG((EFI_D_INFO,"FSLDDR: DqMap0 = 0x%08x\n", Ddr->DqMap0));
	DEBUG((EFI_D_INFO,"FSLDDR: DqMap1 = 0x%08x\n", Ddr->DqMap1));
	DEBUG((EFI_D_INFO,"FSLDDR: DqMap2 = 0x%08x\n", Ddr->DqMap2));
	DEBUG((EFI_D_INFO,"FSLDDR: DqMap3 = 0x%08x\n", Ddr->DqMap3));
}

static VOID SetDdrSdramCfg3 (
  OUT  FslDdrCfgRegsT *Ddr,
  IN   CONST MemctlOptionsT *Popts
  )
{
  INT32 RdPre;

  RdPre = Popts->QuadRankPresent ? 1 : 0;

  Ddr->DdrSdramCfg3 = (RdPre & 0x1) << 16;

  DEBUG((EFI_D_INFO,"FSLDDR: DdrSdramCfg3 = 0x%08x\n", Ddr->DdrSdramCfg3));
}
#endif	/* CONFIG_SYS_FSL_DDR4 */

/* DDR ZQ Calibration Control (DDR_ZQ_CNTL) */
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
#ifdef CONFIG_SYS_FSL_DDR4
  UINT32 ZqcsInit = 0;
#endif

  if (ZqEn) {
#ifdef CONFIG_SYS_FSL_DDR4
    Zqinit = 10;	/* 1024 Clocks */
    Zqoper = 9;	/* 512 Clocks */
    Zqcs = 7;		/* 128 Clocks */
    ZqcsInit = 5;	/* 1024 Refresh Sequences */
#else
    Zqinit = 9;	/** 512 Clocks */
    Zqoper = 8;	/** 256 Clocks */
    Zqcs = 6;		/** 64 Clocks */
#endif
  }

  Ddr->DdrZqCntl = (0
  	   | ((ZqEn & 0x1) << 31)
  	   | ((Zqinit & 0xF) << 24)
  	   | ((Zqoper & 0xF) << 16)
  	   | ((Zqcs & 0xF) << 8)
#ifdef CONFIG_SYS_FSL_DDR4
          | ((ZqcsInit & 0xF) << 0)
#endif
  );
  DEBUG((EFI_D_INFO,"FSLDDR: ZqCntl = 0x%08x\n", Ddr->DdrZqCntl));
}

/* DDR Write Leveling Control (DDR_WRLVL_CNTL) */
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
  DEBUG((EFI_D_INFO,"FSLDDR: WrlvlCntl = 0x%08x\n", Ddr->DdrWrlvlCntl));
  Ddr->DdrWrlvlCntl2 = Popts->WrlvlCtl2;
  DEBUG((EFI_D_INFO,"FSLDDR: WrlvlCntl2 = 0x%08x\n", Ddr->DdrWrlvlCntl2));
  Ddr->DdrWrlvlCntl3 = Popts->WrlvlCtl3;
  DEBUG((EFI_D_INFO,"FSLDDR: WrlvlCntl3 = 0x%08x\n", Ddr->DdrWrlvlCntl3));
}

/* DDR Self Refresh Counter (DDR_SR_CNTR) */
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
  IN   CONST MemctlOptionsT 	*Popts,
  OUT  FslDdrCfgRegsT 		*Ddr,
  IN   CONST CommonTimingParamsT 	*CommonDimm,
  IN   CONST DimmParamsT 		*DimmParams,
  IN   UINT32 			DbwCapAdj,
  IN   UINT32 			SizeOnly
  )
{
  UINT32 I;
  UINT32 CasLatency;
  UINT32 AdditiveLatency;
  UINT32 SrIt;
  UINT32 ZqEn;
  UINT32 WrlvlEn;
  UINT32 IpRev = 0;
  UINT32 UnqMrsEn = 0;
  INT32 CsEn = 1;

  InternalMemZeroMem(Ddr, sizeof(FslDdrCfgRegsT));

  if (CommonDimm == NULL) {
    DEBUG((EFI_D_INFO,"Error: Subset DIMM Params struct Null Pointer\n"));
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
  for (I = 0; I < CONFIG_CHIP_SELECTS_PER_CTRL; I++) {
    UINT64 Ea, Sa;
    UINT32 CsPerDimm
	= CONFIG_CHIP_SELECTS_PER_CTRL / CONFIG_DIMM_SLOTS_PER_CTLR;
    UINT32 DimmNumber
	= I / CsPerDimm;
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
	   if (I > 1)
	     CsEn = 0;
	     break;
	 case FSL_DDR_CS2_CS3:
	 default:
	   if (I > 0)
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
	   if ((I >= 2) && (DimmNumber == 0)) {
	     Sa = DimmParams[DimmNumber].BaseAddress +
	   		      2 * RankDensity;
	     Ea = Sa + 2 * RankDensity - 1;
	   } else {
	     Sa = DimmParams[DimmNumber].BaseAddress;
	     Ea = Sa + 2 * RankDensity - 1;
	   }
	   break;
	 case FSL_DDR_CS0_CS1:
	   if (DimmParams[DimmNumber].NRanks > (I % CsPerDimm)) {
	     Sa = DimmParams[DimmNumber].BaseAddress;
	     Ea = Sa + RankDensity - 1;
	     if (I != 1)
	       Sa += (I % CsPerDimm) * RankDensity;
		Ea += (I % CsPerDimm) * RankDensity;
	     } else {
	       Sa = 0;
		Ea = 0;
	     }
	     if (I == 0)
	       Ea += RankDensity;
	     break;
	 case FSL_DDR_CS2_CS3:
	   if (DimmParams[DimmNumber].NRanks > (I % CsPerDimm)) {
	     Sa = DimmParams[DimmNumber].BaseAddress;
	     Ea = Sa + RankDensity - 1;
	     if (I != 3)
	       Sa += (I % CsPerDimm) * RankDensity;
	     Ea += (I % CsPerDimm) * RankDensity;
	   } else {
	     Sa = 0;
	     Ea = 0;
	   }
	   if (I == 2)
	     Ea += (RankDensity >> DbwCapAdj);
	   break;
	 default:  /** No bank(chip-select) interleaving */
	   Sa = DimmParams[DimmNumber].BaseAddress; /** DOUBT base address shows 0 in uboot code*/
	   Ea = Sa + RankDensity - 1;
	   if (DimmParams[DimmNumber].NRanks > (I % CsPerDimm)) {
	     Sa += (I % CsPerDimm) * RankDensity;
	     Ea += (I % CsPerDimm) * RankDensity;
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
      Ddr->Cs[I].Bnds = (0
		| ((Sa & 0xffff) << 16)     /* starting address */
		| ((Ea & 0xffff) << 0)	/* ending address */
      );
    } else {
      /** setting Bnds to 0xffffffff for inactive CS */
      Ddr->Cs[I].Bnds = 0xffffffff;
    }

    SetCsnConfig(DimmNumber, I, Ddr, Popts, DimmParams);
    SetCsnConfig_2(I, Ddr);
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

  if ((IpRev > 0x40700) && (Popts->CswlOverride != 0))
    Ddr->Debug[18] = Popts->CswlOverride;

  SetDdrSdramCfg2(Ddr, Popts, UnqMrsEn);
  SetDdrSdramMode(Ddr, Popts, CommonDimm,
			CasLatency, AdditiveLatency, UnqMrsEn);
  SetDdrSdramMode2(Ddr, Popts, CommonDimm, UnqMrsEn);
#ifdef CONFIG_SYS_FSL_DDR4
  SetDdrSdramMode9(Ddr, Popts, CommonDimm, UnqMrsEn);
  SetDdrSdramMode10(Ddr, Popts, CommonDimm, UnqMrsEn);
#endif
  SetDdrSdramInterval(Ddr, Popts, CommonDimm);
  SetDdrDataInit(Ddr);
  SetDdrSdramClkCntl(Ddr, Popts);
  SetDdrInitAddr(Ddr);
  SetDdrInitExtAddr(Ddr);
  SetTimingCfg4(Ddr, Popts);
  SetTimingCfg5(Ddr, CasLatency);
#ifdef CONFIG_SYS_FSL_DDR4
  SetDdrSdramCfg3(Ddr, Popts);
  SetTimingCfg6(Ddr);
  SetTimingCfg7(Ddr, CommonDimm);
  SetTimingCfg8(Ddr, Popts, CommonDimm, CasLatency);
  SetTimingCfg9(Ddr);
  SetDdrDqMapping(Ddr, DimmParams);
#endif

  SetDdrZqCntl(Ddr, ZqEn);
  SetDdrWrlvlCntl(Ddr, WrlvlEn, Popts);

  SetDdrSrCntr(Ddr, SrIt);

  SetDdrSdramRcw(Ddr, Popts, CommonDimm);

#ifdef INTPSR_DDR4_WORKAROUND
  Ddr->Debug[1] = 0x10;	/* Force Slow */
#endif

#ifdef   CONFIG_SYS_FSL_DDR_EMU
  /* disble Ddr training for emulator */
  Ddr->Debug[2] = 0x00000400;
  Ddr->Debug[4] = 0xff800000;
  Ddr->Debug[5] = 0x08000800;
  Ddr->Debug[6] = 0x08000800;
  Ddr->Debug[7] = 0x08000800;
  Ddr->Debug[8] = 0x08000800;
#endif
  return CheckFslMemctlConfigRegs(Ddr);
}


