/** @SoCLib.c
  SoC specific Library for LS1043A SoC, containing functions to initialize various SoC components

  Copyright (c) 2015, Freescale Ltd. All rights reserved.

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

static struct CsuNsDev ns_dev[] =
{
	 {CSU_CSLX_PCIE2_IO, CSU_ALL_RW},
	 {CSU_CSLX_PCIE1_IO, CSU_ALL_RW},
	 {CSU_CSLX_MG2TPR_IP, CSU_ALL_RW},
	 {CSU_CSLX_IFC_MEM, CSU_ALL_RW},
	 {CSU_CSLX_OCRAM, CSU_ALL_RW},
	 {CSU_CSLX_GIC, CSU_ALL_RW},
	 {CSU_CSLX_PCIE1, CSU_ALL_RW},
	 {CSU_CSLX_OCRAM2, CSU_ALL_RW},
	 {CSU_CSLX_QSPI_MEM, CSU_ALL_RW},
	 {CSU_CSLX_PCIE2, CSU_ALL_RW},
	 {CSU_CSLX_SATA, CSU_ALL_RW},
	 {CSU_CSLX_USB1, CSU_ALL_RW},
	 {CSU_CSLX_QM_BM_SWPORTAL, CSU_ALL_RW},
	 {CSU_CSLX_PCIE3, CSU_ALL_RW},
	 {CSU_CSLX_PCIE3_IO, CSU_ALL_RW},
	 {CSU_CSLX_USB3, CSU_ALL_RW},
	 {CSU_CSLX_USB2, CSU_ALL_RW},
	 {CSU_CSLX_SERDES, CSU_ALL_RW},
	 {CSU_CSLX_QDMA, CSU_ALL_RW},
	 {CSU_CSLX_LPUART2, CSU_ALL_RW},
	 {CSU_CSLX_LPUART1, CSU_ALL_RW},
	 {CSU_CSLX_LPUART4, CSU_ALL_RW},
	 {CSU_CSLX_LPUART3, CSU_ALL_RW},
	 {CSU_CSLX_LPUART6, CSU_ALL_RW},
	 {CSU_CSLX_LPUART5, CSU_ALL_RW},
	 {CSU_CSLX_DSPI1, CSU_ALL_RW},
	 {CSU_CSLX_QSPI, CSU_ALL_RW},
	 {CSU_CSLX_ESDHC, CSU_ALL_RW},
	 {CSU_CSLX_IFC, CSU_ALL_RW},
	 {CSU_CSLX_I2C1, CSU_ALL_RW},
	 {CSU_CSLX_I2C3, CSU_ALL_RW},
	 {CSU_CSLX_I2C2, CSU_ALL_RW},
	 {CSU_CSLX_DUART2, CSU_ALL_RW},
	 {CSU_CSLX_DUART1, CSU_ALL_RW},
	 {CSU_CSLX_WDT2, CSU_ALL_RW},
	 {CSU_CSLX_WDT1, CSU_ALL_RW},
	 {CSU_CSLX_EDMA, CSU_ALL_RW},
	 {CSU_CSLX_SYS_CNT, CSU_ALL_RW},
	 {CSU_CSLX_DMA_MUX2, CSU_ALL_RW},
	 {CSU_CSLX_DMA_MUX1, CSU_ALL_RW},
	 {CSU_CSLX_DDR, CSU_ALL_RW},
	 {CSU_CSLX_QUICC, CSU_ALL_RW},
	 {CSU_CSLX_DCFG_CCU_RCPM, CSU_ALL_RW},
	 {CSU_CSLX_SECURE_BOOTROM, CSU_ALL_RW},
	 {CSU_CSLX_SFP, CSU_ALL_RW},
	 {CSU_CSLX_TMU, CSU_ALL_RW},
	 {CSU_CSLX_SECURE_MONITOR, CSU_ALL_RW},
	 {CSU_CSLX_SCFG, CSU_ALL_RW},
	 {CSU_CSLX_FM, CSU_ALL_RW},
	 {CSU_CSLX_SEC5_5, CSU_ALL_RW},
	 {CSU_CSLX_BM, CSU_ALL_RW},
	 {CSU_CSLX_QM, CSU_ALL_RW},
	 {CSU_CSLX_GPIO2, CSU_ALL_RW},
	 {CSU_CSLX_GPIO1, CSU_ALL_RW},
	 {CSU_CSLX_GPIO4, CSU_ALL_RW},
	 {CSU_CSLX_GPIO3, CSU_ALL_RW},
	 {CSU_CSLX_PLATFORM_CONT, CSU_ALL_RW},
	 {CSU_CSLX_CSU, CSU_ALL_RW},
	 {CSU_CSLX_IIC4, CSU_ALL_RW},
	 {CSU_CSLX_WDT4, CSU_ALL_RW},
	 {CSU_CSLX_WDT3, CSU_ALL_RW},
	 {CSU_CSLX_WDT5, CSU_ALL_RW},
	 {CSU_CSLX_FTM2, CSU_ALL_RW},
	 {CSU_CSLX_FTM1, CSU_ALL_RW},
	 {CSU_CSLX_FTM4, CSU_ALL_RW},
	 {CSU_CSLX_FTM3, CSU_ALL_RW},
	 {CSU_CSLX_FTM6, CSU_ALL_RW},
	 {CSU_CSLX_FTM5, CSU_ALL_RW},
	 {CSU_CSLX_FTM8, CSU_ALL_RW},
	 {CSU_CSLX_FTM7, CSU_ALL_RW},
	 {CSU_CSLX_DSCR, CSU_ALL_RW},
};

char *strmhz (char *buf, unsigned long hz)
{
	long l, n;
	long m;

	n = DIV_ROUND_CLOSEST(hz, 1000) / 1000L;
	l = AsciiSPrint (buf, sizeof(buf), "%ld", n);

	hz -= n * 1000000L;
	m = DIV_ROUND_CLOSEST(hz, 1000L);
	if (m != 0)
		AsciiSPrint (buf + l, sizeof(buf), ".%03ld", m);
	return (buf);
}

VOID
CciConfigureSnoopDvm (
  struct ccsr_cci400 *cci
  )
{
	// Enable snoop requests and DVM message requests for
	// Slave insterface S4 (A53 core cluster)
	MmioWrite32((UINTN)&cci->slave[4].snoop_ctrl, CCI400_DVM_MESSAGE_REQ_EN
			| CCI400_SNOOP_REQ_EN);

}

VOID
CciConfigureQos (
  struct ccsr_cci400 *cci
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
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)CONFIG_SYS_CCI400_ADDR;

	/* Set CCI-400 control override register to enable barrier transaction */
	MmioWrite32((UINTN)&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

	CciConfigureSnoopDvm(cci);
	CciConfigureQos(cci);
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
  OUT struct CsuNsDev *ns_dev,
  IN UINT32 num
  )
{
	UINT32 *base = (UINT32 *)CONFIG_SYS_FSL_CSU_ADDR;
	UINT32 *reg;
	UINT32 val;
	UINT32 i;

	for (i = 0; i < num; i++) {
		reg = base + ns_dev[i].ind / 2;
		val = MmioReadBe32((UINTN)reg);
		if (ns_dev[i].ind % 2 == 0) {
			val &= 0x0000ffff;
			val |= ns_dev[i].val << 16;
		} else {
			val &= 0xffff0000;
			val |= ns_dev[i].val;
		}
		MmioWriteBe32((UINTN)reg, val);
	}
}

VOID
CsuInit (
  VOID
  )
{
	EnableDevicesNsAccess(ns_dev, ARRAY_SIZE(ns_dev));
}

VOID
GetSysInfo (
  OUT struct SysInfo *PtrSysInfo
  )
{
	struct ccsr_gur  *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct ccsr_clk *clk = (void *)(CONFIG_SYS_FSL_CLK_ADDR);
	UINTN cpu;
	UINT32 rcw_tmp;
	const UINT8 core_cplx_pll[8] = {
		[0] = 0,	/* CC1 PPL / 1 */
		[1] = 0,	/* CC1 PPL / 2 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 1,	/* CC2 PPL / 2 */
	};

	const UINT8 core_cplx_pll_div[8] = {
		[0] = 1,	/* CC1 PPL / 1 */
		[1] = 2,	/* CC1 PPL / 2 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 2,	/* CC2 PPL / 2 */
	};

	UINTN i;
	UINTN freq_c_pll[CONFIG_SYS_FSL_NUM_CC_PLLS];
	UINTN ratio[CONFIG_SYS_FSL_NUM_CC_PLLS];
	UINTN sysclk = CONFIG_SYS_CLK_FREQ;

	PtrSysInfo->FreqSystemBus = sysclk;
	PtrSysInfo->FreqDdrBus = sysclk;

	PtrSysInfo->FreqSystemBus *= (MmioReadBe32((UINTN)&gur->rcwsr[0]) >>
			FSL_CHASSIS2_RCWSR0_SYS_PLL_RAT_SHIFT) &
			FSL_CHASSIS2_RCWSR0_SYS_PLL_RAT_MASK;
	PtrSysInfo->FreqDdrBus *= (MmioReadBe32((UINTN)&gur->rcwsr[0]) >>
			FSL_CHASSIS2_RCWSR0_MEM_PLL_RAT_SHIFT) &
			FSL_CHASSIS2_RCWSR0_MEM_PLL_RAT_MASK;

	for (i = 0; i < CONFIG_SYS_FSL_NUM_CC_PLLS; i++) {
		ratio[i] = (MmioReadBe32((UINTN)&clk->pllcgsr[i].pllcngsr) >> 1) & 0xff;
		if (ratio[i] > 4)
			freq_c_pll[i] = sysclk * ratio[i];
		else
			freq_c_pll[i] = PtrSysInfo->FreqSystemBus * ratio[i];
	}

	for (cpu = 0; cpu < CONFIG_MAX_CPUS; cpu++) {
		UINT32 c_pll_sel = (MmioReadBe32((UINTN)&clk->clkcsr[cpu].clkcncsr) >> 27)
				& 0xf;
		UINT32 cplx_pll = core_cplx_pll[c_pll_sel];

		PtrSysInfo->FreqProcessor[cpu] =
			freq_c_pll[cplx_pll] / core_cplx_pll_div[c_pll_sel];
	}

	rcw_tmp = MmioReadBe32((UINTN)&gur->rcwsr[7]);
	switch ((rcw_tmp & HWA_CGA_M1_CLK_SEL) >> HWA_CGA_M1_CLK_SHIFT) {
	case 2:
		PtrSysInfo->FreqFman[0] = freq_c_pll[0] / 2;
		break;
	case 3:
		PtrSysInfo->FreqFman[0] = freq_c_pll[1] / 3;
		break;
	case 6:
		PtrSysInfo->FreqFman[0] = freq_c_pll[0] / 2;
		break;
	case 7:
		PtrSysInfo->FreqFman[0] = freq_c_pll[1] / 3;
		break;
	default:
		DEBUG((EFI_D_ERROR, "Error: Unknown FMan1 clock select!\n"));
		break;
	}
	rcw_tmp = MmioReadBe32((UINTN)&gur->rcwsr[15]);
	rcw_tmp = (rcw_tmp & HWA_CGA_M2_CLK_SEL) >> HWA_CGA_M2_CLK_SHIFT;
	PtrSysInfo->FreqSdhc = freq_c_pll[1] / rcw_tmp;
	PtrSysInfo->FreqQman = 0 /* FIXME */;
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

UINTN
GetBusFreq (
  VOID
  )
{
	return gClkInfo.BusClk;
}

UINTN
GetDdrFreq (
  VOID
  )
{
	return gClkInfo.MemClk;
}

INTN
GetSdhcFreq (
  VOID
  )
{
	return gClkInfo.SdhcClk;
}

INTN
GetSerialClock (
  VOID
  )
{
	return gClkInfo.BusClk;
}

UINTN
GetPeripheralClock (
  IN enum PeriphClock Clk
  )
{
	switch (Clk) {
	case I2C_CLK:
		return GetBusFreq();
	case ESDHC_CLK:
		return GetSdhcFreq();
	case DSPI_CLK:
		return GetBusFreq();
	case UART_CLK:
		return GetBusFreq();
	default:
		DEBUG((EFI_D_ERROR, "Unsupported clock\n"));
	}
	return 0;
}

INTN
TimerInit (
  VOID
  )
{
	UINT32 *cntcr = (UINT32 *)CONFIG_SYS_FSL_TIMER_ADDR;
  
	if (PcdGetBool(PcdCounterFrequencyReal)) {
		UINTN cntfrq = PcdGet32(PcdCounterFrequency);

		/* Update with accurate clock frequency */
		asm volatile("msr cntfrq_el0, %0" : : "r" (cntfrq) : "memory");
	}

	/*  Enable clock for timer. This is a global setting. */
	MmioWrite32((UINTN)cntcr, 0x1);

	return 0;
}

static inline
UINT32
InitiatorType (
  IN UINT32 cluster,
  IN UINTN init_id
  )
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	UINT32 idx = (cluster >> (init_id * 8)) & TP_CLUSTER_INIT_MASK;
	UINT32 type = MmioReadBe32((UINTN)&gur->tp_ityp[idx]);

	if (type & TP_ITYP_AV)
		return type;

	return 0;
}

UINT32
CpuMask (
  VOID
  )
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	UINTN i = 0, count = 0;
	UINT32 cluster, type, mask = 0;

	do {
		int j;
		cluster = MmioReadBe32((UINTN)&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = InitiatorType(cluster, j);
			if (type) {
				if (TP_ITYP_TYPE(type) == TP_ITYP_TYPE_ARM)
					mask |= 1 << count;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC_MASK) == 0x0);

	return mask;
}

/*
 * Return the number of cores on this SOC.
 */
UINTN CpuNumCores(void)
{
	return hweight32(CpuMask());
}


UINT32
QoriqCoreToType (
  IN UINTN core
  )
{
	struct ccsr_gur  *gur =
		(void  *)(CONFIG_SYS_FSL_GUTS_ADDR);
	UINTN i = 0, count = 0;
	UINT32 cluster, type;

	do {
		UINTN j;
		cluster = MmioReadBe32((UINTN)&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = InitiatorType(cluster, j);
			if (type) {
				if (count == core)
					return type;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC_MASK) == 0x0);

	return -1;      /* cannot identify the cluster */
}

VOID
PrintCpuInfo (
  VOID
  )
{
	struct SysInfo sysinfo;
	CHAR8 buf[32];
	UINTN i, core;
	UINT32 type;
  	CHAR8  Buffer[100];
	UINTN  CharCount;

	GetSysInfo(&sysinfo);
	CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"Clock Configuration:");
  	SerialPortWrite ((UINT8 *) Buffer, CharCount);
	
	ForEachCpu(i, core, CpuNumCores(), CpuMask()) {
		if (!(i % 3))
			DEBUG((EFI_D_INFO, "\n       "));
		type = TP_ITYP_VER(QoriqCoreToType(core));
		CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"CPU%d(%a):%-4a MHz  ", core,
		       type == TY_ITYP_VER_A53 ? "A53" : "Unknown Core",
		       strmhz(buf, sysinfo.FreqProcessor[core]));
  		SerialPortWrite ((UINT8 *) Buffer, CharCount);
	}
	
	CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"\n       Bus:      %-4a MHz  ",
	       		strmhz(buf, sysinfo.FreqSystemBus));
  	SerialPortWrite ((UINT8 *) Buffer, CharCount);
	CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"DDR:      %-4a MHz", strmhz(buf, sysinfo.FreqDdrBus));
  	SerialPortWrite ((UINT8 *) Buffer, CharCount);
	CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"\n");
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
	FslIfcNandInit();
}

VOID
PrintBoardPersonality (
  VOID
  )
{
	static const char *freq[3] = {"100.00MHZ", "156.25MHZ"};
#ifndef CONFIG_SD_BOOT
	UINT8 cfg_rcw_src1, cfg_rcw_src2;
	UINT32 cfg_rcw_src;
#endif
	UINT32 sd1refclk_sel;

	DEBUG((EFI_D_INFO, "Board: LS1043ARDB, boot from "));

#ifdef CONFIG_SD_BOOT
	DEBUG((EFI_D_INFO, "SD\n"));
#else
	cfg_rcw_src1 = CPLD_READ(cfg_rcw_src1);
	cfg_rcw_src2 = CPLD_READ(cfg_rcw_src2);
	CpldRevBit(&cfg_rcw_src1);
	cfg_rcw_src = cfg_rcw_src1;
	cfg_rcw_src = (cfg_rcw_src << 1) | cfg_rcw_src2;

	if (cfg_rcw_src == 0x25)
		DEBUG((EFI_D_INFO, "vBank %d\n", CPLD_READ(vbank)));
	else if (cfg_rcw_src == 0x106)
		DEBUG((EFI_D_INFO, "NAND\n"));
	else
		DEBUG((EFI_D_INFO, "Invalid setting of SW4\n"));
#endif

	DEBUG((EFI_D_INFO, "CPLD:  V%x.%x\nPCBA:  V%x.0\n", CPLD_READ(cpld_ver),
		CPLD_READ(cpld_ver_sub), CPLD_READ(pcba_ver)));

	DEBUG((EFI_D_INFO, "SERDES Reference Clocks:\n"));
	sd1refclk_sel = CPLD_READ(sd1refclk_sel);
	DEBUG((EFI_D_INFO, "SD1_CLK1 = %a, SD1_CLK2 = %a\n", freq[sd1refclk_sel], freq[0]));
}

VOID
PrintRCW (
  VOID
  )
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	UINTN i;
	CHAR8  Buffer[100];
	UINTN  CharCount;

	/*
	 * Display the RCW, so that no one gets confused as to what RCW
	 * we're actually using for this boot.
	 */

	CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"Reset Configuration Word (RCW):");
  	SerialPortWrite ((UINT8 *) Buffer, CharCount);
	for (i = 0; i < ARRAY_SIZE(gur->rcwsr); i++) {
		UINT32 rcw = MmioReadBe32((UINTN)&gur->rcwsr[i]);

		if ((i % 4) == 0) {
			CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"\n       %08x:", i * 4);
		  	SerialPortWrite ((UINT8 *) Buffer, CharCount);
		}
		CharCount = AsciiSPrint (Buffer,sizeof (Buffer)," %08x", rcw);
		SerialPortWrite ((UINT8 *) Buffer, CharCount);
	}
	CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"\n");
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
  CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"\nUEFI firmware (version %s built at %a on %a)\n\r",
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

VOID do_fixup_by_compat(VOID *fdt, CONST char *compat,
			CONST char *prop, CONST VOID *val, INTN len, INTN create)
{
	INTN off = -1;
	off = fdt_node_offset_by_compatible(fdt, -1, compat);
	while (off != -FDT_ERR_NOTFOUND) {
		if (create || (fdt_get_property(fdt, off, prop, NULL) != NULL))
			fdt_setprop(fdt, off, prop, val, len);
		off = fdt_node_offset_by_compatible(fdt, off, compat);
	}
}

VOID do_fixup_by_compat_u32(VOID *fdt, CONST char *compat,
			    CONST char *prop, UINT32 val, INTN create)
{
	fdt32_t tmp = cpu_to_fdt32(val);
	do_fixup_by_compat(fdt, compat, prop, &tmp, 4, create);
}

#define BMAN_IP_REV_1 0xBF8
#define BMAN_IP_REV_2 0xBFC
VOID fdt_fixup_bportals(VOID *blob)
{
	UINTN off, err;
	UINTN maj, min;
	UINTN ip_cfg;

	UINT32 rev_1 = MmioReadBe32(CONFIG_SYS_FSL_BMAN_ADDR + BMAN_IP_REV_1);
	UINT32 rev_2 = MmioReadBe32(CONFIG_SYS_FSL_BMAN_ADDR + BMAN_IP_REV_2);
	char compat[64];
	INTN compat_len;

	maj = (rev_1 >> 8) & 0xff;
	min = rev_1 & 0xff;

	ip_cfg = rev_2 & 0xff;

	compat_len = AsciiSPrint(compat, sizeof(compat), "fsl,bman-portal-%u.%u.%u",
				 maj, min, ip_cfg) + 1;
	compat_len += AsciiSPrint(compat + compat_len,  sizeof(compat), "fsl,bman-portal") + 1;

	off = fdt_node_offset_by_compatible(blob, -1, "fsl,bman-portal");
	while (off != -FDT_ERR_NOTFOUND) {
		err = fdt_setprop(blob, off, "compatible", compat, compat_len);
		if (err < 0) {
			DEBUG((EFI_D_ERROR, "ERROR: unable to create props for %a: %s\n",
				fdt_get_name(blob, off, NULL), fdt_strerror(err)));
			return;
		}

		off = fdt_node_offset_by_compatible(blob, off, "fsl,bman-portal");
	}
}

#define QMAN_IP_REV_1 0xBF8
#define QMAN_IP_REV_2 0xBFC
VOID fdt_fixup_qportals(VOID *blob)
{
	INTN off, err;
	UINTN maj, min;
	UINTN ip_cfg;
	UINT32 rev_1 = MmioReadBe32(CONFIG_SYS_FSL_QMAN_ADDR + QMAN_IP_REV_1);
	UINT32 rev_2 = MmioReadBe32(CONFIG_SYS_FSL_QMAN_ADDR + QMAN_IP_REV_2);
	char compat[64];
	INTN compat_len;

	maj = (rev_1 >> 8) & 0xff;
	min = rev_1 & 0xff;
	ip_cfg = rev_2 & 0xff;

	compat_len = AsciiSPrint(compat, sizeof(compat), "fsl,qman-portal-%u.%u.%u",
					maj, min, ip_cfg) + 1;
	compat_len += AsciiSPrint(compat + compat_len,  sizeof(compat), "fsl,qman-portal") + 1;

	off = fdt_node_offset_by_compatible(blob, -1, "fsl,qman-portal");
	while (off != -FDT_ERR_NOTFOUND) {
		err = fdt_setprop(blob, off, "compatible", compat, compat_len);
		if (err < 0) {
			DEBUG((EFI_D_ERROR, "ERROR: unable to create props for %a: %a\n",
				fdt_get_name(blob, off, NULL), fdt_strerror(err)));
			return;
		}

		off = fdt_node_offset_by_compatible(blob, off, "fsl,qman-portal");
	}
}

VOID fdt_fixup_esdhc(VOID *blob, UINTN SdhcClk)
{
	const char *compat = "fsl,esdhc";

	do_fixup_by_compat_u32(blob, compat, "clock-frequency", SdhcClk, 1);

	do_fixup_by_compat(blob, compat, "status", "okay", 4 + 1, 1);
}

VOID fdt_cpu_setup(VOID *blob)
{
  	struct SysInfo SocSysInfo;
	GetSysInfo(&SocSysInfo);

	do_fixup_by_compat_u32(blob, "fsl,ns16550",
			       "clock-frequency", SocSysInfo.FreqSystemBus, 1);

	fdt_fixup_esdhc(blob, SocSysInfo.FreqSdhc);

	fdt_fixup_bportals(blob);
	fdt_fixup_qportals(blob);

	do_fixup_by_compat_u32(blob, "fsl,qman",
			"clock-frequency", SocSysInfo.FreqQman, 1);
}
