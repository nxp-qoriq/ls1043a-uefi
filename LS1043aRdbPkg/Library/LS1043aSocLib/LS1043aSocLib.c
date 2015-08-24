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

#include <Base.h>
#include <PiPei.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib/MemLibInternals.h>

#include <Library/PrePiLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <Library/FslIfc.h>

#include <ArmTrustzone.h>

#include <LS1043aRdb.h>
#include <LS1043aSocLib.h>
#include <CpldLib.h>

#include <libfdt.h>

/* Global Clock Information pointer */
static SocClockInfo gClkInfo;

static struct CsuNsDev NonSecureDevices[] =
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

char *StringToMHz (
  char *Buf,
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
	struct CcsrGur *GurBase = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct CcsrClk *ClkBase = (void *)(CONFIG_SYS_FSL_CLK_ADDR);
	UINTN CpuIndex;
	UINT32 TempRcw;
	const UINT8 CoreCplxPll[8] = {
		[0] = 0,	/* CC1 PPL / 1 */
		[1] = 0,	/* CC1 PPL / 2 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 1,	/* CC2 PPL / 2 */
	};

	const UINT8 CoreCplxPllDivisor[8] = {
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
		PtrSysInfo->FreqFman[0] = FreqCPll[1] / 3;
		break;
	case 6:
		PtrSysInfo->FreqFman[0] = FreqCPll[0] / 2;
		break;
	case 7:
		PtrSysInfo->FreqFman[0] = FreqCPll[1] / 3;
		break;
	default:
		DEBUG((EFI_D_WARN, "Error: Unknown FMan1 clock select!\n"));
		break;
	}
	TempRcw = MmioReadBe32((UINTN)&GurBase->rcwsr[15]);
	TempRcw = (TempRcw & HWA_CGA_M2_CLK_SEL) >> HWA_CGA_M2_CLK_SHIFT;
	PtrSysInfo->FreqSdhc = FreqCPll[1] / TempRcw;
	PtrSysInfo->FreqQman = PtrSysInfo->FreqSystemBus / 2;
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

static inline
UINT32
InitiatorType (
  IN UINT32 Cluster,
  IN UINTN InitId
  )
{
	struct CcsrGur *GurBase = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
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
	struct CcsrGur *GurBase = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
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
	struct CcsrGur *GurBase = (VOID *)(CONFIG_SYS_FSL_GUTS_ADDR);
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
			DEBUG((EFI_D_INFO, "\n       "));
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
	static const char *Freq[3] = {"100.00MHZ", "156.25MHZ"};
	UINT8 RcwSrc1, RcwSrc2;
	UINT32 RcwSrc;
	UINT32 sd1refclk_sel;

	DEBUG((EFI_D_INFO, "Board: LS1043ARDB, boot from "));

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
	struct CcsrGur *Base = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
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

  SerDesInit();
  return;
}

/* fdt fixup for LS1043A */

VOID
FixupByCompatibleField (
  VOID *Fdt,
  CONST char *Compat,
  CONST char *Prop,
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
  CONST char *Compat,
  CONST char *Prop,
  UINT32 Val,
  INTN Create
  )
{
	fdt32_t Tmp = cpu_to_fdt32(Val);
	FixupByCompatibleField(Fdt, Compat, Prop, &Tmp, 4, Create);
}

#define BMAN_IP_REV_1 0xBF8
#define BMAN_IP_REV_2 0xBFC
VOID
FdtFixupBmanPortals (
  VOID *Blob
  )
{
	UINTN Off, Err;
	UINTN Maj, Min;
	UINTN IpCfg;

	UINT32 BmanRev1 = MmioReadBe32(CONFIG_SYS_FSL_BMAN_ADDR + BMAN_IP_REV_1);
	UINT32 BmanRev2 = MmioReadBe32(CONFIG_SYS_FSL_BMAN_ADDR + BMAN_IP_REV_2);
	char Compatible[64];
	INTN CompatibleLength;

	Maj = (BmanRev1 >> 8) & 0xff;
	Min = BmanRev1 & 0xff;

	IpCfg = BmanRev2 & 0xff;

	CompatibleLength = AsciiSPrint(Compatible, sizeof(Compatible),
				       "fsl,bman-portal-%u.%u.%u",
				       Maj, Min, IpCfg) + 1;
	CompatibleLength += AsciiSPrint(Compatible + CompatibleLength,
					sizeof(Compatible), "fsl,bman-portal")
					+ 1;

	Off = fdt_node_offset_by_compatible(Blob, -1, "fsl,bman-portal");
	while (Off != -FDT_ERR_NOTFOUND) {
		Err = fdt_setprop(Blob, Off, "compatible", Compatible,
				  CompatibleLength);
		if (Err < 0) {
			DEBUG((EFI_D_ERROR, "ERROR: unable to create props for %a: %s\n",
				fdt_get_name(Blob, Off, NULL), fdt_strerror(Err)));
			return;
		}

		Off = fdt_node_offset_by_compatible(Blob, Off, "fsl,bman-portal");
	}
}

#define QMAN_IP_REV_1 0xBF8
#define QMAN_IP_REV_2 0xBFC
VOID
FdtFixupQmanPortals (
  VOID *Blob
  )
{
	INTN Off, Err;
	UINTN Maj, Min;
	UINTN IpCfg;
	UINT32 QmanRev1 = MmioReadBe32(CONFIG_SYS_FSL_QMAN_ADDR + QMAN_IP_REV_1);
	UINT32 QmanRev2 = MmioReadBe32(CONFIG_SYS_FSL_QMAN_ADDR + QMAN_IP_REV_2);
	char Compatible[64];
	INTN CompatLength;

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
}

VOID
FdtFixupSdhc (
  VOID *Blob,
  UINTN SdhcClk
  )
{
	const char *Compatible = "fsl,esdhc";

	FixupByCompatibleField32(Blob, Compatible, "clock-frequency", SdhcClk, 1);

	FixupByCompatibleField(Blob, Compatible, "status", "okay", 4 + 1, 1);
}

VOID FdtCpuSetup(VOID *blob)
{
	struct SysInfo SocSysInfo;
	GetSysInfo(&SocSysInfo);

	FixupByCompatibleField32(blob, "fsl,ns16550",
			       "clock-frequency", SocSysInfo.FreqSystemBus, 1);

	FdtFixupSdhc(blob, SocSysInfo.FreqSdhc);

	FdtFixupBmanPortals(blob);
	FdtFixupQmanPortals(blob);

	FixupByCompatibleField32(blob, "fsl,qman",
			"clock-frequency", SocSysInfo.FreqQman, 1);
}
