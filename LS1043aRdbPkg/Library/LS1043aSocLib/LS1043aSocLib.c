/** @SoCLib.c
  SoC specific Library for LS1043A SoC, containing functions to initialize various SoC components

  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <ArmTrustzone.h>
#include <Base.h>
#include <CpldLib.h>
#include <PiPei.h>
#include <Library/BaseMemoryLib/MemLibInternals.h>
#include <Library/BaseLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/Dpaa1BoardSpecificLib.h>
#include <Library/FrameManager.h>
#include <Library/FslIfc.h>
#include <Library/IoLib.h>
#include <Library/PrePiLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <libfdt.h>
#include <LS1043aRdb.h>
#include <LS1043aSocLib.h>
#include <Uefi.h>

/* Global Clock Information pointer */
STATIC SocClockInfo gClkInfo;

STATIC CPU_TYPE CpuTypeList[] = {
	CPU_TYPE_ENTRY(LS1043A, LS1043A, 4),
};

STATIC struct CsuNsDev NonSecureDevices[] =
{
	 {SEC_UNIT_CSLX_PCIE2_IO, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_PCIE1_IO, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_MG2TPR_IP, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_IFC_MEM, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_OCRAM, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_GIC, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_PCIE1, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_OCRAM2, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_QSPI_MEM, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_PCIE2, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_SATA, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_USB1, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_QM_BM_SWPORTAL, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_PCIE3, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_PCIE3_IO, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_USB3, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_USB2, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_SERDES, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_QDMA, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_LPUART2, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_LPUART1, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_LPUART4, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_LPUART3, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_LPUART6, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_LPUART5, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_DSPI1, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_QSPI, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_ESDHC, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_IFC, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_I2C1, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_I2C3, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_I2C2, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_DUART2, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_DUART1, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_WDT2, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_WDT1, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_EDMA, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_SYS_CNT, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_DMA_MUX2, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_DMA_MUX1, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_DDR, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_QUICC, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_DCFG_CCU_RCPM, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_SECURE_BOOTROM, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_SFP, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_TMU, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_SECURE_MONITOR, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_SCFG, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_FM, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_SEC5_5, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_BM, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_QM, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_GPIO2, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_GPIO1, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_GPIO4, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_GPIO3, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_PLATFORM_CONT, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_SEC_UNIT, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_IIC4, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_WDT4, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_WDT3, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_WDT5, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_FTM2, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_FTM1, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_FTM4, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_FTM3, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_FTM6, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_FTM5, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_FTM8, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_FTM7, SEC_UNIT_ALL_RW},
	 {SEC_UNIT_CSLX_DSCR, SEC_UNIT_ALL_RW},
};

CHAR8 *StringToMHz (
  CHAR8 *Buf,
  unsigned long Hz
  )
{
	long l, m, n;

	n = DIV_ROUND_CLOSEST(Hz, 1000) / 1000L;
	l = AsciiSPrint (Buf, sizeof(Buf), "%ld", n);

	Hz -= n * 1000000L;
	m = DIV_ROUND_CLOSEST(Hz, 1000L);
	if (m != 0)
		AsciiSPrint (Buf + l, sizeof(Buf), ".%03ld", m);
	return (Buf);
}

VOID
CciConfigureSnoopDvm (
  struct CcsrCci400 *CciBase
  )
{
	// Enable snoop requests and DVM message requests for
	// Slave insterface S4 (A53 core cluster)
	MmioWrite32((UINTN)&CciBase->slave[4].snoop_ctrl,
			CCI400_DVM_MESSAGE_REQ_EN | CCI400_SNOOP_REQ_EN);
}

VOID
CciConfigureQos (
  struct CcsrCci400 *CciBase
  )
{
	// FIXME: Empty for now. Populate if required later.
	return;
}

VOID
Cci400Init (
  VOID
  )
{
	struct CcsrCci400 *Base = (struct CcsrCci400 *)CONFIG_SYS_CCI400_ADDR;

	/* Set CCI-400 control override register to enable barrier transaction */
	MmioWrite32((UINTN)&Base->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

	CciConfigureSnoopDvm(Base);
	CciConfigureQos(Base);
}

/**
  Initialize the Secure peripherals and memory regions

  If Trustzone is supported by your platform then this function makes the required initialization
  of the secure peripherals and memory regions.

**/
VOID
Tzc380Init (
  VOID
  )
{
  // Setup TZ Address Space Controller.
  // Assumption: We have 2GB DDR mounted on the DIMMs.
  //
  // Since, we need ONE secure DDR region which will be used for keeping
  // the PPA (EL3 platform security fw) code and the rest of the regions
  // would be non-secure regions which can be accessed via NS software as
  // well - so we create one TZASC region of 2GB and divided it into
  // 8 equal su-regions. Now, we keep the 1st sub-regions for housing
  // the PPA and use the rest of the sub-regions to allow NS accesses.

  // Note: Your OS Kernel must be aware of the secure regions before to
  // enable this region
  TZASCSetRegion(CONFIG_SYS_TZASC380_ADDR, 1, TZASC_REGION_ENABLED, CONFIG_DRAM1_BASE_ADDR, 0,
		TZASC_REGION_SIZE_2GB, TZASC_REGION_SECURITY_SRW, 0x7F);
}

VOID
EnableDevicesNsAccess (
  OUT struct CsuNsDev *NonSecureDevices,
  IN UINT32 Num
  )
{
	UINT32 *Base = (UINT32 *)CONFIG_SYS_FSL_CSU_ADDR;
	UINT32 *Reg;
	UINT32 Val;
	UINT32 Count;

	for (Count = 0; Count < Num; Count++) {
		Reg = Base + NonSecureDevices[Count].Ind / 2;
		Val = MmioReadBe32((UINTN)Reg);
		if (NonSecureDevices[Count].Ind % 2 == 0) {
			Val &= 0x0000ffff;
			Val |= NonSecureDevices[Count].Val << 16;
		} else {
			Val &= 0xffff0000;
			Val |= NonSecureDevices[Count].Val;
		}
		MmioWriteBe32((UINTN)Reg, Val);
	}
}

VOID
CsuInit (
  VOID
  )
{
	EnableDevicesNsAccess(NonSecureDevices, ARRAY_SIZE(NonSecureDevices));
}

VOID
GetSysInfo (
  OUT struct SysInfo *PtrSysInfo
  )
{
	struct CcsrGur *GurBase = (VOID *)(GUTS_ADDR);
	struct CcsrClk *ClkBase = (VOID *)(CONFIG_SYS_FSL_CLK_ADDR);
	UINTN CpuIndex;
	UINT32 TempRcw;
	CONST UINT8 CoreCplxPll[8] = {
		[0] = 0,	/* CC1 PPL / 1 */
		[1] = 0,	/* CC1 PPL / 2 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 1,	/* CC2 PPL / 2 */
	};

	CONST UINT8 CoreCplxPllDivisor[8] = {
		[0] = 1,	/* CC1 PPL / 1 */
		[1] = 2,	/* CC1 PPL / 2 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 2,	/* CC2 PPL / 2 */
	};

	UINTN PllCount;
	UINTN FreqCPll[CONFIG_SYS_FSL_NUM_CC_PLLS];
	UINTN PllRatio[CONFIG_SYS_FSL_NUM_CC_PLLS];
	UINTN SysClk = CONFIG_SYS_CLK_FREQ;

	PtrSysInfo->FreqSystemBus = SysClk;
	PtrSysInfo->FreqDdrBus = SysClk;

	PtrSysInfo->FreqSystemBus *= (MmioReadBe32((UINTN)&GurBase->rcwsr[0]) >>
			FSL_CHASSIS2_RCWSR0_SYS_PLL_RAT_SHIFT) &
			FSL_CHASSIS2_RCWSR0_SYS_PLL_RAT_MASK;
	PtrSysInfo->FreqDdrBus *= (MmioReadBe32((UINTN)&GurBase->rcwsr[0]) >>
			FSL_CHASSIS2_RCWSR0_MEM_PLL_RAT_SHIFT) &
			FSL_CHASSIS2_RCWSR0_MEM_PLL_RAT_MASK;

	for (PllCount = 0; PllCount < CONFIG_SYS_FSL_NUM_CC_PLLS; PllCount++) {
		PllRatio[PllCount] = (MmioReadBe32((UINTN)&ClkBase->pllcgsr[PllCount].pllcngsr) >> 1) & 0xff;
		if (PllRatio[PllCount] > 4)
			FreqCPll[PllCount] = SysClk * PllRatio[PllCount];
		else
			FreqCPll[PllCount] = PtrSysInfo->FreqSystemBus * PllRatio[PllCount];
	}

	for (CpuIndex = 0; CpuIndex < CONFIG_MAX_CPUS; CpuIndex++) {
		UINT32 c_pll_sel = (MmioReadBe32((UINTN)&ClkBase->clkcsr[CpuIndex].clkcncsr) >> 27)
				& 0xf;
		UINT32 cplx_pll = CoreCplxPll[c_pll_sel];

		PtrSysInfo->FreqProcessor[CpuIndex] =
			FreqCPll[cplx_pll] / CoreCplxPllDivisor[c_pll_sel];
	}

	TempRcw = MmioReadBe32((UINTN)&GurBase->rcwsr[7]);
	switch ((TempRcw & HWA_CGA_M1_CLK_SEL) >> HWA_CGA_M1_CLK_SHIFT) {
	case 2:
		PtrSysInfo->FreqFman[0] = FreqCPll[0] / 2;
		break;
	case 3:
		PtrSysInfo->FreqFman[0] = FreqCPll[0] / 3;
		break;
	case 4:
		PtrSysInfo->FreqFman[0] = FreqCPll[0] / 4;
		break;
	case 5:
		PtrSysInfo->FreqFman[0] = PtrSysInfo->FreqSystemBus;
		break;
	case 6:
		PtrSysInfo->FreqFman[0] = FreqCPll[1] / 2;
		break;
	case 7:
		PtrSysInfo->FreqFman[0] = FreqCPll[1] / 3;
		break;
	default:
		DEBUG((EFI_D_WARN, "Error: Unknown FMan1 clock select!\n"));
		break;
	}
	PtrSysInfo->FreqSdhc = PtrSysInfo->FreqSystemBus;
	PtrSysInfo->FreqQman = PtrSysInfo->FreqSystemBus;
}

VOID
ClockInit (
  VOID
  )
{
	struct SysInfo SocSysInfo;

	GetSysInfo(&SocSysInfo);
	gClkInfo.CpuClk = SocSysInfo.FreqProcessor[0];
	gClkInfo.BusClk = SocSysInfo.FreqSystemBus;
	gClkInfo.MemClk = SocSysInfo.FreqDdrBus;
	gClkInfo.SdhcClk = SocSysInfo.FreqSdhc;
}

INTN
TimerInit (
  VOID
  )
{
	UINT32 *TimerBase = (UINT32 *)CONFIG_SYS_FSL_TIMER_ADDR;

	if (PcdGetBool(PcdCounterFrequencyReal)) {
		UINTN cntfrq = PcdGet32(PcdCounterFrequency);

		/* Update with accurate clock frequency */
		asm volatile("msr cntfrq_el0, %0" : : "r" (cntfrq) : "memory");
	}

	/* Enable clock for timer. This is a global setting. */
	MmioWrite32((UINTN)TimerBase, 0x1);

	return 0;
}

STATIC inline
UINT32
InitiatorType (
  IN UINT32 Cluster,
  IN UINTN InitId
  )
{
	struct CcsrGur *GurBase = (VOID *)(GUTS_ADDR);
	UINT32 Idx = (Cluster >> (InitId * 8)) & TP_CLUSTER_INIT_MASK;
	UINT32 Type = MmioReadBe32((UINTN)&GurBase->tp_ityp[Idx]);

	if (Type & TP_ITYP_AV)
		return Type;

	return 0;
}

UINT32
CpuMask (
  VOID
  )
{
	struct CcsrGur *GurBase = (VOID *)(GUTS_ADDR);
	UINTN ClusterIndex = 0, Count = 0;
	UINT32 Cluster, Type, Mask = 0;

	do {
		UINTN InitiatorIndex;
		Cluster = MmioReadBe32((UINTN)&GurBase->tp_cluster[ClusterIndex].lower);
		for (InitiatorIndex = 0; InitiatorIndex < TP_INIT_PER_CLUSTER; InitiatorIndex++) {
			Type = InitiatorType(Cluster, InitiatorIndex);
			if (Type) {
				if (TP_ITYP_TYPE(Type) == TP_ITYP_TYPE_ARM)
					Mask |= 1 << Count;
				Count++;
			}
		}
		ClusterIndex++;
	} while ((Cluster & TP_CLUSTER_EOC_MASK) == 0x0);

	return Mask;
}

/*
 * Return the number of cores on this SOC.
 */
UINTN
CpuNumCores (
  VOID
  )
{
	return HammingWeight32(CpuMask());
}

UINT32
QoriqCoreToType (
  IN UINTN Core
  )
{
	struct CcsrGur *GurBase = (VOID *)(GUTS_ADDR);
	UINTN ClusterIndex = 0, Count = 0;
	UINT32 Cluster, Type;

	do {
		UINTN InitiatorIndex;
		Cluster = MmioReadBe32((UINTN)&GurBase->tp_cluster[ClusterIndex].lower);
		for (InitiatorIndex = 0; InitiatorIndex < TP_INIT_PER_CLUSTER; InitiatorIndex++) {
			Type = InitiatorType(Cluster, InitiatorIndex);
			if (Type) {
				if (Count == Core)
					return Type;
				Count++;
			}
		}
		ClusterIndex++;
	} while ((Cluster & TP_CLUSTER_EOC_MASK) == 0x0);

	return -1;      /* cannot identify the cluster */
}

VOID
PrintCpuInfo (
  VOID
  )
{
	struct SysInfo SysInfo;
	UINTN CoreIndex, Core;
	UINT32 Type;
	CHAR8 Buf[32];
	CHAR8 Buffer[100];
	UINTN CharCount;

	GetSysInfo(&SysInfo);
	CharCount = AsciiSPrint (Buffer, sizeof (Buffer), "Clock Configuration:");
	SerialPortWrite ((UINT8 *) Buffer, CharCount);

	ForEachCpu(CoreIndex, Core, CpuNumCores(), CpuMask()) {
		if (!(CoreIndex % 3))
			DEBUG((EFI_D_RELEASE, "\n       "));
		Type = TP_ITYP_VER(QoriqCoreToType(Core));
		CharCount = AsciiSPrint (Buffer, sizeof (Buffer), "CPU%d(%a):%-4a MHz  ", Core,
		       Type == TY_ITYP_VER_A53 ? "A53" : "Unknown Core",
		       StringToMHz(Buf, SysInfo.FreqProcessor[Core]));
		SerialPortWrite ((UINT8 *) Buffer, CharCount);
	}

	CharCount = AsciiSPrint (Buffer, sizeof (Buffer), "\n       Bus:      %-4a MHz  ",
	 		StringToMHz(Buf, SysInfo.FreqSystemBus));
	SerialPortWrite ((UINT8 *) Buffer, CharCount);
	CharCount = AsciiSPrint (Buffer, sizeof (Buffer), "DDR:      %-4a MHz", StringToMHz(Buf, SysInfo.FreqDdrBus));
	SerialPortWrite ((UINT8 *) Buffer, CharCount);
	CharCount = AsciiSPrint (Buffer, sizeof (Buffer), "\n       FMAN:      %-4a MHz  ",
				StringToMHz(Buf, SysInfo.FreqFman[0]));
	SerialPortWrite ((UINT8 *) Buffer, CharCount);
	CharCount = AsciiSPrint (Buffer, sizeof (Buffer), "\n");
	SerialPortWrite ((UINT8 *) Buffer, CharCount);
}

VOID
PrintSocPersonality (
  VOID
  )
{
}

VOID
IfcInit (
  VOID
  )
{
	/* NOR Init */
	IfcNorInit();

	/* CPLD Init */
	CpldInit();

	/* NAND Init */
	IfcNandInit();
}

VOID
PrintBoardPersonality (
  VOID
  )
{
	STATIC CONST CHAR8 *Freq[3] = {"100.00MHZ", "156.25MHZ"};
	UINT8 RcwSrc1, RcwSrc2;
	UINT32 RcwSrc;
	UINT32 sd1refclk_sel;

	CHAR8 Buf[16];
	struct CcsrGur *GurBase = (VOID *)(GUTS_ADDR);
	UINTN Count, Svr, Ver;

	Buf[0] = L'\0';
	Svr = MmioReadBe32((UINTN)&GurBase->svr);
	Ver = SVR_SOC_VER(Svr);

	for (Count = 0; Count < ARRAY_SIZE(CpuTypeList); Count++)
		if ((CpuTypeList[Count].soc_ver & SVR_WO_E) == Ver) {
			AsciiStrCpy(Buf, (CONST CHAR8 *)CpuTypeList[Count].name);

			if (IS_E_PROCESSOR(Svr))
				AsciiStrCat(Buf, (CONST CHAR8 *)"E");
			break;
		}

	if (Count == ARRAY_SIZE(CpuTypeList)) {
		AsciiStrCpy(Buf, (CONST CHAR8 *)"unknown");
	}

	DEBUG((EFI_D_RELEASE, "SoC: %a; Rev %d.%d\n",
			Buf, SVR_MAJOR(Svr), SVR_MINOR(Svr)));
	DEBUG((EFI_D_RELEASE, "Board: LS1043A-RDB\n"));

	RcwSrc1 = CPLD_READ(RcwSource1);
	RcwSrc2 = CPLD_READ(RcwSource1);
	CpldRevBit(&RcwSrc1);
	RcwSrc = RcwSrc1;
	RcwSrc = (RcwSrc << 1) | RcwSrc2;

	if (RcwSrc == 0x25)
		DEBUG((EFI_D_INFO, "vBank %d\n", CPLD_READ(Vbank)));
	else if (RcwSrc == 0x106)
		DEBUG((EFI_D_INFO, "NAND\n"));
	else
		DEBUG((EFI_D_INFO, "Invalid setting of SW4\n"));

	DEBUG((EFI_D_INFO, "CPLD:  V%x.%x\nPCBA:  V%x.0\n", CPLD_READ(CpldVersionMajor),
		CPLD_READ(CpldVersionMinor), CPLD_READ(PcbaVersion)));

	DEBUG((EFI_D_INFO, "SERDES Reference Clocks:\n"));
	sd1refclk_sel = CPLD_READ(Sd1RefClkSel);
	DEBUG((EFI_D_INFO, "SD1_CLK1 = %a, SD1_CLK2 = %a\n", Freq[sd1refclk_sel], Freq[0]));
}

VOID
PrintRCW (
  VOID
  )
{
	struct CcsrGur *Base = (VOID *)(GUTS_ADDR);
	UINTN Count;
	CHAR8 Buffer[100];
	UINTN CharCount;

	/*
	 * Display the RCW, so that no one gets confused as to what RCW
	 * we're actually using for this boot.
	 */

	CharCount = AsciiSPrint (Buffer, sizeof (Buffer),
				 "Reset Configuration Word (RCW):");
	SerialPortWrite ((UINT8 *) Buffer, CharCount);
	for (Count = 0; Count < ARRAY_SIZE(Base->rcwsr); Count++) {
		UINT32 Rcw = MmioReadBe32((UINTN)&Base->rcwsr[Count]);

		if ((Count % 4) == 0) {
			CharCount = AsciiSPrint (Buffer, sizeof (Buffer),
						 "\n       %08x:", Count * 4);
		 	SerialPortWrite ((UINT8 *) Buffer, CharCount);
		}

		CharCount = AsciiSPrint (Buffer, sizeof (Buffer), " %08x", Rcw);
		SerialPortWrite ((UINT8 *) Buffer, CharCount);
	}
	CharCount = AsciiSPrint (Buffer, sizeof (Buffer), "\n");
	SerialPortWrite ((UINT8 *) Buffer, CharCount);
}

VOID
SmmuInit (
  VOID
  )
{
	UINT32 Value;

	/* set pagesize as 64K and ssmu-500 in bypass mode */
	Value = (MmioRead32((UINTN)SMMU_REG_SACR) | SACR_PAGESIZE_MASK);
	MmioWrite32((UINTN)SMMU_REG_SACR, Value);

	Value = (MmioRead32((UINTN)SMMU_REG_SCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	MmioWrite32((UINTN)SMMU_REG_SCR0, Value);

	Value = (MmioRead32((UINTN)SMMU_REG_NSCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	MmioWrite32((UINTN)SMMU_REG_NSCR0, Value);
}

/**
  Function to initialize SoC specific constructs
  // CSU
  // TZC-380
  // CCI-400
  // ClockInit
  // TimerInit
  // CPU Info
  // SoC Personality
  // Board Personality
  // RCW prints
  // SerDes support
 **/
VOID
SocInit (
  VOID
  )
{
  
  CHAR8 Buffer[100];
  UINTN CharCount;
  
  // LS1043A SoC has a CSU (Central Security Unit)
  if (PcdGetBool(PcdCsuInitialize))
	  CsuInit();

  if (PcdGetBool(PcdTzc380Initialize))
	  Tzc380Init();
  
  if (PcdGetBool(PcdCci400Initialize))
	  Cci400Init();
  
  if (PcdGetBool(PcdClockInitialize))
	  ClockInit();

  SmmuInit();
  TimerInit();

  // Initialize the Serial Port
  SerialPortInitialize ();
  CharCount = AsciiSPrint (Buffer, sizeof (Buffer), "\nUEFI firmware (version %s built at %a on %a)\n\r",
    (CHAR16*)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__);
  SerialPortWrite ((UINT8 *) Buffer, CharCount);
  
  PrintCpuInfo();
  PrintRCW();

  PrintSocPersonality();
 
  IfcInit();

  PrintBoardPersonality();

  return;
}

/* fdt fixup for LS1043A */
#define ___swab32(x) \
	((UINT32)( \
		(((UINT32)(x) & (UINT32)0x000000ffUL) << 24) | \
		(((UINT32)(x) & (UINT32)0x0000ff00UL) <<  8) | \
		(((UINT32)(x) & (UINT32)0x00ff0000UL) >>  8) | \
		(((UINT32)(x) & (UINT32)0xff000000UL) >> 24) ))

/* Max address size we deal with */
#define OF_MAX_ADDR_CELLS	4
#define OF_BAD_ADDR	((UINTN)-1)
#define OF_CHECK_COUNTS(na, ns)	((na) > 0 && (na) <= OF_MAX_ADDR_CELLS && \
			(ns) > 0)
/* Errors in device tree content */
#define FDT_ERR_BADNCELLS	14
/* FDT_ERR_BADNCELLS: Device tree has a #address-cells, #size-cells
 * or similar property with a bad format or value */

/**
 * FDT_MAX_NCELLS - maximum value for #address-cells and #size-cells
 *
 * This is the maximum value for #address-cells, #size-cells and
 * similar properties that will be processed by libfdt.  IEE1275
 * requires that OF implementations handle values up to 4.
 * Implementations may support larger values, but in practice higher
 * values aren't used.
 */
#define FDT_MAX_NCELLS		4
int fdt_address_cells(const void *fdt, int nodeoffset)
{
	const fdt32_t *ac;
	int val;
	int len;

	ac = fdt_getprop(fdt, nodeoffset, "#address-cells", &len);
	if (!ac)
		return 2;

	if (len != sizeof(*ac))
		return -FDT_ERR_BADNCELLS;

	val = fdt32_to_cpu(*ac);
	if ((val <= 0) || (val > FDT_MAX_NCELLS))
			return -FDT_ERR_BADNCELLS;

	return val;
}

/* Helper to read a big number; size is in cells (not bytes) */
static inline UINTN of_read_number(const fdt32_t *cell, int size)
{
	UINTN r = 0;
	while (size--)
		r = (r << 32) | fdt32_to_cpu(*(cell++));
	return r;
}

/* Callbacks for bus specific translators */
struct of_bus {
	const char	*name;
	const char	*addresses;
	void		(*count_cells)(void *blob, int parentoffset,
				int *addrc, int *sizec);
	UINTN		(*map)(fdt32_t *addr, const fdt32_t *range,
				int na, int ns, int pna);
	int		(*translate)(fdt32_t *addr, UINTN offset, int na);
};

/* Default translator (generic bus) */
void of_bus_default_count_cells(void *blob, int parentoffset,
					int *addrc, int *sizec)
{
	const fdt32_t *prop;

	if (addrc)
		*addrc = fdt_address_cells(blob, parentoffset);

	if (sizec) {
		prop = fdt_getprop(blob, parentoffset, "#size-cells", NULL);
		if (prop)
			*sizec = ___swab32(*prop);
		else
			*sizec = 1;
	}
}

static UINTN of_bus_default_map(fdt32_t *addr, const fdt32_t *range,
		int na, int ns, int pna)
{
	UINTN cp, s, da;

	cp = of_read_number(range, na);
	s  = of_read_number(range + na + pna, ns);
	da = of_read_number(addr, na);

	if (da < cp || da >= (cp + s))
		return OF_BAD_ADDR;
	return da - cp;
}

static int of_bus_default_translate(fdt32_t *addr, UINTN offset, int na)
{
	UINTN a = of_read_number(addr, na);
	memset(addr, 0, na * 4);
	a += offset;
	if (na > 1)
		addr[na - 2] = cpu_to_fdt32(a >> 32);
	addr[na - 1] = cpu_to_fdt32(a & 0xffffffffu);

	return 0;
}

/* Array of bus specific translators */
static struct of_bus of_busses[] = {
	/* Default */
	{
		.name = "default",
		.addresses = "reg",
		.count_cells = of_bus_default_count_cells,
		.map = of_bus_default_map,
		.translate = of_bus_default_translate,
	},
};

static int of_translate_one(void * blob, int parent, struct of_bus *bus,
			    struct of_bus *pbus, fdt32_t *addr,
			    int na, int ns, int pna, const char *rprop)
{
	const fdt32_t *ranges;
	int rlen;
	int rone;
	UINTN offset = OF_BAD_ADDR;

	/* Normally, an absence of a "ranges" property means we are
	 * crossing a non-translatable boundary, and thus the addresses
	 * below the current not cannot be converted to CPU physical ones.
	 * Unfortunately, while this is very clear in the spec, it's not
	 * what Apple understood, and they do have things like /uni-n or
	 * /ht nodes with no "ranges" property and a lot of perfectly
	 * useable mapped devices below them. Thus we treat the absence of
	 * "ranges" as equivalent to an empty "ranges" property which means
	 * a 1:1 translation at that level. It's up to the caller not to try
	 * to translate addresses that aren't supposed to be translated in
	 * the first place. --BenH.
	 */
	ranges = fdt_getprop(blob, parent, rprop, &rlen);
	if (ranges == NULL || rlen == 0) {
		offset = of_read_number(addr, na);
		memset(addr, 0, pna * 4);
		goto finish;
	}


	/* Now walk through the ranges */
	rlen /= 4;
	rone = na + pna + ns;
	for (; rlen >= rone; rlen -= rone, ranges += rone) {
		offset = bus->map(addr, ranges, na, ns, pna);
		if (offset != OF_BAD_ADDR)
			break;
	}
	if (offset == OF_BAD_ADDR) {
		return 1;
	}
	memcpy(addr, ranges + na, 4 * pna);

 finish:

	/* Translate it into parent bus space */
	return pbus->translate(addr, offset, pna);
}

/*
 * Translate an address from the device-tree into a CPU physical address,
 * this walks up the tree and applies the various bus mappings on the
 * way.
 *
 * Note: We consider that crossing any level with #size-cells == 0 to mean
 * that translation is impossible (that is we are not dealing with a value
 * that can be mapped to a cpu physical address). This is not really specified
 * that way, but this is traditionally the way IBM at least do things
 */
static UINT64 __of_translate_address(void *blob, int node_offset, const fdt32_t *in_addr,
				  const char *rprop)
{
	int parent;
	struct of_bus *bus, *pbus;
	fdt32_t addr[OF_MAX_ADDR_CELLS];
	int na, ns, pna, pns;
	UINTN result = OF_BAD_ADDR;

	/* Get parent & match bus type */
	parent = fdt_parent_offset(blob, node_offset);
	if (parent < 0)
		goto bail;
	bus = &of_busses[0];

	/* Cound address cells & copy address locally */
	bus->count_cells(blob, parent, &na, &ns);
	if (!OF_CHECK_COUNTS(na, ns)) {
		DEBUG((EFI_D_ERROR,"%s: Bad cell count for %s\n", __FUNCTION__,
		       fdt_get_name(blob, node_offset, NULL)));
		goto bail;
	}
	memcpy(addr, in_addr, na * 4);

	/* Translate */
	for (;;) {
		/* Switch to parent bus */
		node_offset = parent;
		parent = fdt_parent_offset(blob, node_offset);

		/* If root, we have finished */
		if (parent < 0) {
			result = of_read_number(addr, na);
			break;
		}

		/* Get new parent bus and counts */
		pbus = &of_busses[0];
		pbus->count_cells(blob, parent, &pna, &pns);
		if (!OF_CHECK_COUNTS(pna, pns)) {
			DEBUG((EFI_D_ERROR,"%s: Bad cell count for %s\n", __FUNCTION__,
				fdt_get_name(blob, node_offset, NULL)));
			break;
		}

		/* Apply bus translation */
		if (of_translate_one(blob, node_offset, bus, pbus,
					addr, na, ns, pna, rprop))
			break;

		/* Complete the move up one level */
		na = pna;
		ns = pns;
		bus = pbus;
	}
 bail:

	return result;
}

UINTN fdt_translate_address(void *blob, int node_offset, const fdt32_t *in_addr)
{
	return __of_translate_address(blob, node_offset, in_addr, "ranges");
}

/**
 * fdt_node_offset_by_compat_reg: Find a node that matches compatiable and
 * who's reg property matches a physical cpu address
 *
 * @blob: ptr to device tree
 * @compat: compatiable string to match
 * @compat_off: property name
 *
 */
int fdt_node_offset_by_compat_reg(void *blob, const char *compat,
					UINT32 compat_off)
{
	int len, off = fdt_node_offset_by_compatible(blob, -1, compat);
	while (off != -FDT_ERR_NOTFOUND) {
		const fdt32_t *reg = fdt_getprop(blob, off, "reg", &len);
		if (reg) {
			if (compat_off == fdt_translate_address(blob, off, reg))
				return off;
		}
		off = fdt_node_offset_by_compatible(blob, off, compat);
	}

	return -FDT_ERR_NOTFOUND;
}

/**
 * fdt_alloc_phandle: Return next free phandle value
 *
 * @blob: ptr to device tree
 */
int fdt_alloc_phandle(void *blob)
{
	int offset;
	uint32_t phandle = 0;

	for (offset = fdt_next_node(blob, -1, NULL); offset >= 0;
	     offset = fdt_next_node(blob, offset, NULL)) {
		phandle = Max(phandle, fdt_get_phandle(blob, offset));
	}

	return phandle + 1;
}

/*
 * fdt_set_phandle: Create a phandle property for the given node
 *
 * @fdt: ptr to device tree
 * @nodeoffset: node to update
 * @phandle: phandle value to set (must be unique)
 */
int fdt_set_phandle(void *fdt, int nodeoffset, uint32_t phandle)
{
	int ret;

	ret = fdt_setprop_cell(fdt, nodeoffset, "phandle", phandle);
	if (ret < 0)
		return ret;

	/*
	 * For now, also set the deprecated "linux,phandle" property, so that we
	 * don't break older kernels.
	 */
	ret = fdt_setprop_cell(fdt, nodeoffset, "linux,phandle", phandle);

	return ret;
}

/*
 * fdt_create_phandle: Create a phandle property for the given node
 *
 * @fdt: ptr to device tree
 * @nodeoffset: node to update
 */
unsigned int fdt_create_phandle(void *fdt, int nodeoffset)
{
	/* see if there is a phandle already */
	int phandle = fdt_get_phandle(fdt, nodeoffset);

	/* if we got 0, means no phandle so create one */
	if (phandle == 0) {
		int ret;

		phandle = fdt_alloc_phandle(fdt);
		ret = fdt_set_phandle(fdt, nodeoffset, phandle);
		if (ret < 0) {
			DEBUG((EFI_D_ERROR,"Can't set phandle %u: %s\n", phandle,
			       fdt_strerror(ret)));
			return 0;
		}
	}

	return phandle;
}

/**
 * fdt_find_and_setprop: Find a node and set it's property
 *
 * @fdt: ptr to device tree
 * @node: path of node
 * @prop: property name
 * @val: ptr to new value
 * @len: length of new property value
 * @create: flag to create the property if it doesn't exist
 *
 * Convenience function to directly set a property given the path to the node.
 */
int fdt_find_and_setprop(void *fdt, const char *node, const char *prop,
                      const void *val, int len, int create)
{
       int nodeoff = fdt_path_offset(fdt, node);

       if (nodeoff < 0)
              return nodeoff;

       if ((!create) && (fdt_get_property(fdt, nodeoff, prop, NULL) == NULL))
              return 0; /* create flag not set; so exit quietly */

       return fdt_setprop(fdt, nodeoff, prop, val, len);
}

void do_fixup_by_path(void *fdt, const char *path, const char *prop,
                    const void *val, int len, int create)
{
       int rc = fdt_find_and_setprop(fdt, path, prop, val, len, create);
       if (rc)
              DEBUG((EFI_D_ERROR, "Unable to update property %s:%s, err=%s\n",
                     path, prop, fdt_strerror(rc)));
}

VOID
FixupByCompatibleField (
  VOID *Fdt,
  CONST CHAR8 *Compat,
  CONST CHAR8 *Prop,
  CONST VOID *Val,
  INTN Len,
  INTN Create
  )
{
	INTN Offset = -1;
	Offset = fdt_node_offset_by_compatible(Fdt, -1, Compat);
	while (Offset != -FDT_ERR_NOTFOUND) {
		if (Create || (fdt_get_property(Fdt, Offset, Prop, NULL) != NULL))
			fdt_setprop(Fdt, Offset, Prop, Val, Len);
		Offset = fdt_node_offset_by_compatible(Fdt, Offset, Compat);
	}
}

VOID
FixupByCompatibleField32 (
  VOID *Fdt,
  CONST CHAR8 *Compat,
  CONST CHAR8 *Prop,
  UINT32 Val,
  INTN Create
  )
{
	fdt32_t Tmp = cpu_to_fdt32(Val);
	FixupByCompatibleField(Fdt, Compat, Prop, &Tmp, 4, Create);
}

void fdt_del_node_and_alias(void *blob, const char *alias)
{
       int off = fdt_path_offset(blob, alias);

       if (off < 0)
              return;

       fdt_del_node(blob, off);

       off = fdt_path_offset(blob, "/aliases");
       fdt_delprop(blob, off, alias);
}

#define FMAN1_OFFSET		0xa00000
#define CCSRBAR_PHYS		0x1000000
#define ADDRESS		FMAN1_OFFSET + CCSRBAR_PHYS
STATIC FDT_PORT gFdtPort[] = {
    {FM1_DTSEC_1, PHY_INTERFACE_NONE , ADDRESS + offsetof(FMAN_CCSR, memac[FM1_DTSEC_1-1])},
    {FM1_DTSEC_2, PHY_INTERFACE_NONE , ADDRESS + offsetof(FMAN_CCSR, memac[FM1_DTSEC_2-1])},
    {FM1_DTSEC_3, PHY_INTERFACE_NONE , ADDRESS + offsetof(FMAN_CCSR, memac[FM1_DTSEC_3-1])},
    {FM1_DTSEC_4, PHY_INTERFACE_NONE , ADDRESS + offsetof(FMAN_CCSR, memac[FM1_DTSEC_4-1])},
    {FM1_DTSEC_5, PHY_INTERFACE_NONE , ADDRESS + offsetof(FMAN_CCSR, memac[FM1_DTSEC_5-1])},
    {FM1_DTSEC_6, PHY_INTERFACE_NONE , ADDRESS + offsetof(FMAN_CCSR, memac[FM1_DTSEC_6-1])},
    {FM1_DTSEC_9, PHY_INTERFACE_NONE , ADDRESS + offsetof(FMAN_CCSR, memac[FM1_DTSEC_9-1])}
};

STATIC
VOID
GetPhy (
  IN SERDES_LANE_PROTOCOL LaneProtocol,
  IN VOID  *Arg
  )
{
  FMAN_MEMAC_ID MemacId = (FMAN_MEMAC_ID)INVALID_FMAN_MEMAC_ID;
  PHY_INTERFACE_TYPE PhyInterfaceType = (PHY_INTERFACE_TYPE)PHY_INTERFACE_NONE;

  GetMemacIdAndPhyType(LaneProtocol, &MemacId, &PhyInterfaceType);

  if (MemacId != INVALID_FMAN_MEMAC_ID) {
    if (MemacId <= FM1_DTSEC_6)
      gFdtPort[MemacId-1].PhyInterfaceType = PhyInterfaceType;
    else if (MemacId == FM1_DTSEC_9)
      gFdtPort[FM1_DTSEC_9 - 3].PhyInterfaceType = PhyInterfaceType;
    else if (MemacId == FM1_DTSEC_M) {
      gFdtPort[FM1_DTSEC_1 - 1].PhyInterfaceType = PhyInterfaceType;
      gFdtPort[FM1_DTSEC_2 - 1].PhyInterfaceType = PhyInterfaceType;
      gFdtPort[FM1_DTSEC_5 - 1].PhyInterfaceType = PhyInterfaceType;
      gFdtPort[FM1_DTSEC_6 - 1].PhyInterfaceType = PhyInterfaceType;
    }
  }
}

STATIC CHAR8 *gPhyStrings[] = {
  [PHY_INTERFACE_XFI] = "xfi",
  [PHY_INTERFACE_SGMII] = "sgmii",
  [PHY_INTERFACE_SGMII_2500] = "Sgmii_2500",
  [PHY_INTERFACE_QSGMII] = "qsgmii",
  [PHY_INTERFACE_NONE] = "",
};

STATIC
EFI_STATUS
FixupPort (
  IN VOID  *Blob,
  IN VOID  *Prop,
  IN UINT32  Index
  )
{
       UINT32 Offset = 0;

       Offset = fdt_node_offset_by_compat_reg(Blob, Prop, gFdtPort[Index].CompatAddress);
       if (Offset == -FDT_ERR_NOTFOUND)
              return -FDT_ERR_NOTFOUND;

       if (IsMemacEnabled(gFdtPort[Index].Id)) {
		return fdt_setprop_string(Blob, Offset, "phy-connection-type",
			gPhyStrings[gFdtPort[Index].PhyInterfaceType]);
       }

	return EFI_SUCCESS;
}

STATIC
VOID
FdtFixupFmanMac (
  IN  VOID *Blob
  )
{
	UINT32 I, J = 0, Prop;
	CONST CHAR8 *Path;
	UINT32 SocUniqueId = GetSocUniqueId();
	EFI_MAC_ADDRESS  MacAddr;
	INT32 Offset;

	if (fdt_path_offset(Blob, "/aliases") < 0)
		return;

	/* Cycle through all aliases */
	for (Prop = 0; ; Prop++) {
		const char *Name;

		/* FDT might have been changed, recompute the offset */
		Offset = fdt_first_property_offset(Blob,
				fdt_path_offset(Blob, "/aliases"));
		/* Select property number 'prop' */
		for (I = 0; I < Prop; I++)
			Offset = fdt_next_property_offset(Blob, Offset);

		if (Offset < 0)
			break;

		Path = fdt_getprop_by_offset(Blob, Offset, &Name, NULL);
		if (!AsciiStrnCmp(Name, "ethernet", AsciiStrLen("ethernet"))) {
			DEBUG((EFI_D_INFO, "Name %a \n", Name));
			GenerateMacAddress(SocUniqueId, gFdtPort[J].Id, &MacAddr);
			J++;

			do_fixup_by_path(Blob, Path, "mac-address",
					&MacAddr, 6, 0);
			do_fixup_by_path(Blob, Path, "local-mac-address",
					&MacAddr, 6, 1);
		}
	}
	DEBUG((EFI_D_INFO, "Fman MAC Address fixup done!!!!\n"));
}

STATIC
VOID
FdtFixupFmanEthernet (
  IN  VOID *Blob
  )
{
	UINT32 I = 0;
	EFI_STATUS Status;

	SerDesProbeLanes(GetPhy, NULL);

	for (I = 0; I < ARRAY_SIZE(gFdtPort); I++) {
		Status = FixupPort(Blob, "fsl,fman-memac", I);
		if (Status != EFI_SUCCESS)
			DEBUG((EFI_D_ERROR, "Failed to update memac %d \n",
					gFdtPort[I].Id));
	}
	DEBUG((EFI_D_INFO, "Fman Phy fixup done!!!!\n"));
}

/**
 * FdtFixupFmanFirmware - insert the Fman firmware into the device tree
 */
STATIC
VOID
FdtFixupFmanFirmware (
  IN  VOID *Blob
  )
{
       INT32 Ret, FmanNode, FirmwareNode = -1;
       UINT32 PHandle;
       UINT32 Length;
       UINT32 Crc;
	CONST FMAN_FIRMWARE *FwAddr = NULL;
	CONST FMAN_FW_HEADER *FwHdr = NULL;

       /* The first Fman we find will contain the actual firmware. */
       FmanNode = fdt_node_offset_by_compatible(Blob, -1, "fsl,fman");
       if (FmanNode < 0)
              /* Exit silently if there are no Fman devices */
              return;

       /* If we already have a firmware node, then also exit silently. */
       if (fdt_node_offset_by_compatible(Blob, -1, "fsl,fman-firmware") > 0)
              return;

	FwAddr = (VOID*)(UINTN)PcdGet32(PcdFManFwNorAddr);
	if (!FwAddr)
		return;

	FwHdr = &FwAddr->Header;

       Length = fdt32_to_cpu(FwHdr->Length);

       if ((FwHdr->Magic[0] != 'Q') || (FwHdr->Magic[1] != 'E') ||
           (FwHdr->Magic[2] != 'F')) {
              DEBUG((EFI_D_ERROR, "No Fman firmware at %p\n", FwAddr));
              return;
       }

       if (Length > FMAN_FW_LENGTH) {
              DEBUG((EFI_D_ERROR, "Fman firmware is too large (size=%u)\n",
                     Length));
              return;
       }

       Length -= sizeof(Crc);      /* Subtract the size of the CRC */
       Crc = fdt32_to_cpu(*(UINT32 *)((VOID *)FwAddr + Length));
       if (Crc != ComputeCrc32((VOID *)FwAddr, Length)) {
              DEBUG((EFI_D_ERROR, "Fman firmware has invalid CRC\n"));
              return;
       }

       /* Create the firmware node. */
       FirmwareNode = fdt_add_subnode(Blob, FmanNode, "fman-firmware");
       if (FirmwareNode < 0) {
              CHAR8 String[64];
              fdt_get_path(Blob, FmanNode, String, sizeof(String));
              DEBUG((EFI_D_ERROR, "Failed to add firmware node to %s: %s\n",
			String, fdt_strerror(FirmwareNode)));
              return;
       }
       Ret = fdt_setprop_string(Blob, FirmwareNode, "compatible",
                                   "fsl,fman-firmware");
       if (Ret < 0) {
              CHAR8 String[64];
              fdt_get_path(Blob, FirmwareNode, String, sizeof(String));
              DEBUG((EFI_D_ERROR, "Failed to add compatible property %s: %s\n",
			String, fdt_strerror(Ret)));
              return;
       }
       PHandle = fdt_create_phandle(Blob, FirmwareNode);
       if (!PHandle) {
              CHAR8 String[64];
              fdt_get_path(Blob, FirmwareNode, String, sizeof(String));
              DEBUG((EFI_D_ERROR, "Failed to add phandle property %s: %s\n",
			String, fdt_strerror(Ret)));
              return;
       }
       Ret = fdt_setprop(Blob, FirmwareNode, "fsl,firmware", FwAddr, Length);
       if (Ret < 0) {
              CHAR8 String[64];
              fdt_get_path(Blob, FirmwareNode, String, sizeof(String));
              DEBUG((EFI_D_ERROR, "Failed to add firmware property %s: %s\n",
			String, fdt_strerror(Ret)));
              return;
       }

       /* Find other Fman nodes and point them to firmware node. */
       while ((FmanNode = fdt_node_offset_by_compatible(Blob, FmanNode,
              "fsl,fman")) > 0) {
              Ret = fdt_setprop_cell(Blob, FmanNode, "fsl,firmware-phandle",
                                  PHandle);
              if (Ret < 0) {
			CHAR8 String[64];
			fdt_get_path(Blob, FmanNode, String, sizeof(String));
			DEBUG((EFI_D_ERROR, "Failed to add pointer property %s: %s\n",
							String, fdt_strerror(Ret)));
			return;
              }
       }

  DEBUG((EFI_D_INFO, "FDT Firmware fixup done!!!!\n"));
}


#define BMAN_IP_REV_1 0xBF8
#define BMAN_IP_REV_2 0xBFC
STATIC
VOID
FdtFixupBmanPortals (
  VOID *Blob
  )
{
	UINTN Offset, Error, CompatibleLength;
	UINTN Maj, Min, IpCfg;
	CHAR8 Compatible[64];

	UINT32 BmanRev1 = MmioReadBe32(BMAN_ADDR + BMAN_IP_REV_1);
	UINT32 BmanRev2 = MmioReadBe32(BMAN_ADDR + BMAN_IP_REV_2);

	Maj = (BmanRev1 >> 8) & 0xff;
	Min = BmanRev1 & 0xff;

	IpCfg = BmanRev2 & 0xff;

	CompatibleLength = AsciiSPrint(Compatible, sizeof(Compatible),
				       "fsl,bman-portal-%u.%u.%u",
				       Maj, Min, IpCfg) + 1;
	CompatibleLength += AsciiSPrint(Compatible + CompatibleLength,
					sizeof(Compatible), "fsl,bman-portal") + 1;

	Offset = fdt_node_offset_by_compatible(Blob, -1, "fsl,bman-portal");
	while (Offset != -FDT_ERR_NOTFOUND) {
		Error = fdt_setprop(Blob, Offset, "compatible", Compatible,
				  CompatibleLength);
		if (Error < 0) {
			DEBUG((EFI_D_ERROR, "Unable to create props for %a: %s\n",
				fdt_get_name(Blob, Offset, NULL),
				fdt_strerror(Error)));
			return;
		}

		Offset = fdt_node_offset_by_compatible
				(Blob, Offset,"fsl,bman-portal");
	}
  DEBUG((EFI_D_INFO, "BMAN Portal fixup done!!!!\n"));
}

#define QMAN_IP_REV_1 0xBF8
#define QMAN_IP_REV_2 0xBFC
STATIC
VOID
FdtFixupQmanPortals (
  VOID *Blob
  )
{
	INTN Off, Err, CompatLength;
	UINTN Maj, Min, IpCfg;
	CHAR8 Compatible[64];

	UINT32 QmanRev1 = MmioReadBe32(QMAN_ADDR + QMAN_IP_REV_1);
	UINT32 QmanRev2 = MmioReadBe32(QMAN_ADDR + QMAN_IP_REV_2);

	Maj = (QmanRev1 >> 8) & 0xff;
	Min = QmanRev1 & 0xff;
	IpCfg = QmanRev2 & 0xff;

	CompatLength = AsciiSPrint(Compatible, sizeof(Compatible),
				   "fsl,qman-portal-%u.%u.%u",
				   Maj, Min, IpCfg) + 1;
	CompatLength += AsciiSPrint(Compatible + CompatLength,
				    sizeof(Compatible), "fsl,qman-portal") + 1;

	Off = fdt_node_offset_by_compatible(Blob, -1, "fsl,qman-portal");
	while (Off != -FDT_ERR_NOTFOUND) {
		Err = fdt_setprop(Blob, Off, "compatible", Compatible,
				  CompatLength);
		if (Err < 0) {
			DEBUG((EFI_D_ERROR, "ERROR: unable to create props for %a: %a\n",
				fdt_get_name(Blob, Off, NULL), fdt_strerror(Err)));
			return;
		}

		Off = fdt_node_offset_by_compatible(Blob, Off, "fsl,qman-portal");
	}

	DEBUG((EFI_D_INFO, "QMAN Portal fixup done!!!!\n"));
}

STATIC
VOID
FdtFixupSdhc (
  VOID *Blob,
  UINTN SdhcClk
  )
{

	CONST CHAR8 *Compat = "fsl,esdhc";

	FixupByCompatibleField32(Blob, Compat, "clock-frequency",
			       SdhcClk, 1);

	FixupByCompatibleField(Blob, Compat, "status", "okay",
			   AsciiStrSize ("okay"), 1);
}

#define SEC_ADDR			0x1700000
#define MCFG_OFFSET			0x4
#define VIDMS_OFFSET			0xFF8
#define CCBVID_OFFSET		0xFE4

#define MCFGR_PS_SHIFT		16
#define MCFGR_AWCACHE_SHIFT		8
#define MCFGR_AWCACHE_MASK		(0xf << MCFGR_AWCACHE_SHIFT)

#define SECVID_MS_IPID_MASK		0xffff0000
#define SECVID_MS_IPID_SHIFT	16
#define SECVID_MS_MAJ_REV_MASK	0x0000ff00
#define SECVID_MS_MAJ_REV_SHIFT	8
#define CCBVID_ERA_MASK		0xff000000
#define CCBVID_ERA_SHIFT		24
STATIC
VOID
FixupCaam (VOID)
{
	UINTN SecMcfgAddr = SEC_ADDR + MCFG_OFFSET;
	UINT32 Mcr = MmioReadBe32(SecMcfgAddr);

	Mcr = (Mcr & ~MCFGR_AWCACHE_MASK) | (0x2 << MCFGR_AWCACHE_SHIFT);
	Mcr |= (1 << MCFGR_PS_SHIFT);

	MmioWriteBe32(SecMcfgAddr, Mcr);
}

STATIC
UINT8
GetEra (VOID)
{
	STATIC CONST struct {
		UINT16 Id;
		UINT8 Rev;
		UINT8 Era;
	} Eras[] = {
		{0x0A10, 1, 1},
		{0x0A10, 2, 2},
		{0x0A12, 1, 3},
		{0x0A14, 1, 3},
		{0x0A14, 2, 4},
		{0x0A16, 1, 4},
		{0x0A10, 3, 4},
		{0x0A11, 1, 4},
		{0x0A18, 1, 4},
		{0x0A11, 2, 5},
		{0x0A12, 2, 5},
		{0x0A13, 1, 5},
		{0x0A1C, 1, 5}
	};

	UINT32 SecVidMs = MmioReadBe32((UINTN)(SEC_ADDR + VIDMS_OFFSET));
	UINT32 CcbVid = MmioReadBe32((UINTN)(SEC_ADDR + CCBVID_OFFSET));
	UINT16 Id = (SecVidMs & SECVID_MS_IPID_MASK) >>
                     SECVID_MS_IPID_SHIFT;
	UINT8 Rev = (SecVidMs & SECVID_MS_MAJ_REV_MASK) >>
                     SECVID_MS_MAJ_REV_SHIFT;
	UINT8 Era = (CcbVid & CCBVID_ERA_MASK) >> CCBVID_ERA_SHIFT;

	UINT32 I;

	if (Era)
		return Era;

	for (I = 0; I < ARRAY_SIZE(Eras); I++)
		if (Eras[I].Id == Id && Eras[I].Rev == Rev)
			return Eras[I].Era;

	return 0;
}

STATIC
INT32
FdtFixupCryptoEra (
  IN  VOID  *Blob,
  IN  UINT32 Era
  )
{
	INT32 Error;
	INT32 Node;

	Node = fdt_path_offset(Blob, "crypto");
	if (Node < 0) {
		DEBUG((EFI_D_INFO, "WARNING: Missing crypto node\n"));
		return EFI_SUCCESS;
	}

	Error = fdt_setprop_u32(Blob, Node, "fsl,sec-era", Era);
	if (Error < 0) {
		DEBUG((EFI_D_ERROR, "could not set fsl,sec-era property: %s\n",
			fdt_strerror(Error)));
		return Error;
	}

	return EFI_SUCCESS;
}

STATIC
VOID
FdtFixupCryptoNode (
  IN  VOID   *Blob,
  IN  UINT32 SecRev
  )
{
	UINT8 Era;

	if (!SecRev) {
		fdt_del_node_and_alias(Blob, "crypto");
		return;
	}

	/* Add SEC ERA information in compatible */
	Era = GetEra();
	if (Era) {
		if (!FdtFixupCryptoEra(Blob, Era))
			DEBUG((EFI_D_INFO, "Crypto Node fixup done!!!!\n"));
	} else {
		DEBUG((EFI_D_INFO, "WARN: Unable to get ERA for CAAM rev: %d\n",
		SecRev));
	}
}

STATIC
VOID
FdtFixupGic (
  IN  VOID   *Blob
  )
{
	INT32  Off, Error;
	UINT64 Reg[8];
	UINT32 Value;

	struct CcsrGur *GurBase = (VOID *)(GUTS_ADDR);
	struct CcsrScfg *Scfg = (VOID *)SCFG_BASE_ADDR;

	Value = MmioReadBe32((UINTN)&GurBase->svr) & MASK_UPPER_8;

	if (Value == REV1_1) {
		Value = MmioReadBe32((UINTN)&Scfg->gic_align) &
						(0x01 << GIC_ADDR_BIT);
		if (!Value)
			return;
	}

	Off = fdt_subnode_offset(Blob, 0, "interrupt-controller@1400000");
	if (Off < 0) {
		DEBUG((EFI_D_ERROR, "WARNING: Fdt can't find node %s: %s\n",
			"interrupt-controller@1400000", fdt_strerror(Off)));
		return;
	}

	Reg[0] = cpu_to_fdt64(GICD_BASE_4K);
	Reg[1] = cpu_to_fdt64(GICD_SIZE_4K);
	Reg[2] = cpu_to_fdt64(GICC_BASE_4K);
	Reg[3] = cpu_to_fdt64(GICC_SIZE_4K);
	Reg[4] = cpu_to_fdt64(GICH_BASE_4K);
	Reg[5] = cpu_to_fdt64(GICH_SIZE_4K);
	Reg[6] = cpu_to_fdt64(GICV_BASE_4K);
	Reg[7] = cpu_to_fdt64(GICV_SIZE_4K);

	Error = fdt_setprop(Blob, Off, "reg", Reg, sizeof(Reg));
	if (Error < 0) {
		DEBUG((EFI_D_ERROR,"WARNING: fdt_setprop can't set %s "
				"from node %s: %s\n",
				"reg", "interrupt-controller@1400000",
				fdt_strerror(Error)));
		return;
	}

	DEBUG((EFI_D_INFO, "GIC fixup done!!!!\n"));
	return;
}

VOID FdtCpuSetup(VOID *Blob, UINTN BlobSize)
{
	struct SysInfo SocSysInfo;
	struct CcsrGur *GurBase = (VOID *)(GUTS_ADDR);
	UINTN Svr;

	fdt_open_into(Blob, (VOID *)Blob, BlobSize);

	GetSysInfo(&SocSysInfo);

	Svr = MmioReadBe32((UINTN)&GurBase->svr);

	if (!IS_E_PROCESSOR(Svr)) {
              FdtFixupCryptoNode(Blob, 0);
       } else {
		UINTN SecVidMs;
              SecVidMs = SEC_ADDR + VIDMS_OFFSET;
              FdtFixupCryptoNode(Blob, MmioReadBe32(SecVidMs));
       }

	FixupByCompatibleField32(Blob, "fsl,ns16550",
			       "clock-frequency", SocSysInfo.FreqSystemBus, 1);

	FdtFixupSdhc(Blob, SocSysInfo.FreqSdhc);

	/* DPAA 1.x fixups */
	FdtFixupBmanPortals(Blob);
	FdtFixupQmanPortals(Blob);
	FixupByCompatibleField32(Blob, "fsl,qman",
			"clock-frequency", SocSysInfo.FreqQman, 1);
	FdtFixupFmanFirmware(Blob);
	FdtFixupFmanEthernet(Blob);
	FdtFixupFmanMac(Blob);
	FdtFixupGic(Blob);
	FixupCaam();
}
