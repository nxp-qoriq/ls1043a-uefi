/** SoCLib.h
*  Header defining the LS1043a SoC specific constants (Base addresses, sizes, flags)
*
*  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#ifndef __LS1043A_SOC_H__
#define __LS1043A_SOC_H__

#define CONFIG_SYS_FSL_NUM_CC_PLLS	2
#define HWA_CGA_M1_CLK_SEL		0xe0000000
#define HWA_CGA_M1_CLK_SHIFT		29
#define HWA_CGA_M2_CLK_SEL		0x00000007
#define HWA_CGA_M2_CLK_SHIFT		0

#define TP_ITYP_AV			0x00000001	/* Initiator available */
#define TP_ITYP_TYPE(x)			(((x) & 0x6) >> 1) /* Initiator Type */
#define TP_ITYP_TYPE_ARM		0x0
#define TP_ITYP_TYPE_PPC		0x1	/* PowerPC */
#define TP_ITYP_TYPE_OTHER		0x2	/* StarCore DSP */
#define TP_ITYP_TYPE_HA			0x3	/* HW Accelerator */
#define TP_ITYP_THDS(x)			(((x) & 0x18) >> 3)	/* # threads */
#define TP_ITYP_VER(x)			(((x) & 0xe0) >> 5)	/* Initiator Version */
#define TY_ITYP_VER_A53			0x2

#define TP_CLUSTER_EOC_MASK	0xc0000000	/* end of clusters mask */
#define TP_CLUSTER_INIT_MASK	0x0000003f	/* initiator mask */
#define TP_INIT_PER_CLUSTER	4

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000

#define CONFIG_MAX_CPUS				4
#define CONFIG_SYS_FMAN_V3
#define CONFIG_SYS_NUM_FMAN			1
#define CONFIG_SYS_NUM_FM1_DTSEC		7
#define CONFIG_SYS_NUM_FM1_10GEC		1

/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest UINTNeger. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 || (__x) > 0) ?	\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}							\
)

/*
 * HammingWeight32: returns the hamming weight (i.e. the number
 * of bits set) of a 32-bit word
 */
static inline UINTN HammingWeight32(UINTN w)
{
	UINTN Res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
	Res = (Res & 0x33333333) + ((Res >> 2) & 0x33333333);
	Res = (Res & 0x0F0F0F0F) + ((Res >> 4) & 0x0F0F0F0F);
	Res = (Res & 0x00FF00FF) + ((Res >> 8) & 0x00FF00FF);
	return (Res & 0x0000FFFF) + ((Res >> 16) & 0x0000FFFF);
}

static inline UINTN CpuMaskNext(UINTN Cpu, UINTN Mask)
{
	for (Cpu++; !((1 << Cpu) & Mask); Cpu++)
		;

	return Cpu;
}

#define ForEachCpu(iter, cpu, num_cpus, mask) \
	for (iter = 0, cpu = CpuMaskNext(-1, mask); \
		iter < num_cpus; \
		iter++, cpu = CpuMaskNext(cpu, mask)) \

struct SysInfo {
	UINTN FreqProcessor[CONFIG_MAX_CPUS];
	UINTN FreqSystemBus;
	UINTN FreqDdrBus;
	UINTN FreqLocalBus;
	UINTN FreqSdhc;
	UINTN FreqFman[CONFIG_SYS_NUM_FMAN];
	UINTN FreqQman;
};

typedef struct SocClocks {
	UINTN CpuClk;	/* CPU clock in Hz! */
	UINTN BusClk;
	UINTN MemClk;
	UINTN PciClk;
	UINTN SdhcClk;
} SocClockInfo;

enum PeriphClock {
	ARM_CLK = 0,
	BUS_CLK,
	UART_CLK,
	ESDHC_CLK,
	I2C_CLK,
	DSPI_CLK,
};

enum CsuCslxAccess {
	CSU_NS_SUP_R = 0x08,
	CSU_NS_SUP_W = 0x80,
	CSU_NS_SUP_RW = 0x88,
	CSU_NS_USER_R = 0x04,
	CSU_NS_USER_W = 0x40,
	CSU_NS_USER_RW = 0x44,
	CSU_S_SUP_R = 0x02,
	CSU_S_SUP_W = 0x20,
	CSU_S_SUP_RW = 0x22,
	CSU_S_USER_R = 0x01,
	CSU_S_USER_W = 0x10,
	CSU_S_USER_RW = 0x11,
	CSU_ALL_RW = 0xff,
};

enum CsuCslxInd {
	CSU_CSLX_PCIE2_IO = 0,
	CSU_CSLX_PCIE1_IO,
	CSU_CSLX_MG2TPR_IP,
	CSU_CSLX_IFC_MEM,
	CSU_CSLX_OCRAM,
	CSU_CSLX_GIC,
	CSU_CSLX_PCIE1,
	CSU_CSLX_OCRAM2,
	CSU_CSLX_QSPI_MEM,
	CSU_CSLX_PCIE2,
	CSU_CSLX_SATA,
	CSU_CSLX_USB1,
	CSU_CSLX_QM_BM_SWPORTAL,
	CSU_CSLX_PCIE3 = 16,
	CSU_CSLX_PCIE3_IO,
	CSU_CSLX_USB3 = 20,
	CSU_CSLX_USB2,
	CSU_CSLX_SERDES = 32,
	CSU_CSLX_QDMA,
	CSU_CSLX_LPUART2,
	CSU_CSLX_LPUART1,
	CSU_CSLX_LPUART4,
	CSU_CSLX_LPUART3,
	CSU_CSLX_LPUART6,
	CSU_CSLX_LPUART5,
	CSU_CSLX_DSPI1 = 41,
	CSU_CSLX_QSPI,
	CSU_CSLX_ESDHC,
	CSU_CSLX_IFC = 45,
	CSU_CSLX_I2C1,
	CSU_CSLX_I2C3 = 48,
	CSU_CSLX_I2C2,
	CSU_CSLX_DUART2 = 50,
	CSU_CSLX_DUART1,
	CSU_CSLX_WDT2,
	CSU_CSLX_WDT1,
	CSU_CSLX_EDMA,
	CSU_CSLX_SYS_CNT,
	CSU_CSLX_DMA_MUX2,
	CSU_CSLX_DMA_MUX1,
	CSU_CSLX_DDR,
	CSU_CSLX_QUICC,
	CSU_CSLX_DCFG_CCU_RCPM = 60,
	CSU_CSLX_SECURE_BOOTROM,
	CSU_CSLX_SFP,
	CSU_CSLX_TMU,
	CSU_CSLX_SECURE_MONITOR,
	CSU_CSLX_SCFG,
	CSU_CSLX_FM = 66,
	CSU_CSLX_SEC5_5,
	CSU_CSLX_BM,
	CSU_CSLX_QM,
	CSU_CSLX_GPIO2 = 70,
	CSU_CSLX_GPIO1,
	CSU_CSLX_GPIO4,
	CSU_CSLX_GPIO3,
	CSU_CSLX_PLATFORM_CONT,
	CSU_CSLX_CSU,
	CSU_CSLX_IIC4 = 77,
	CSU_CSLX_WDT4,
	CSU_CSLX_WDT3,
	CSU_CSLX_WDT5 = 81,
	CSU_CSLX_FTM2 = 86,
	CSU_CSLX_FTM1,
	CSU_CSLX_FTM4,
	CSU_CSLX_FTM3,
	CSU_CSLX_FTM6 = 90,
	CSU_CSLX_FTM5,
	CSU_CSLX_FTM8,
	CSU_CSLX_FTM7,
	CSU_CSLX_DSCR = 120,
};

struct CsuNsDev {
	UINTN Ind;
	UINT32 Val;
};

/* Device Configuration and Pin Control */
struct CcsrGur {
	UINT32     porsr1;         /* POR status 1 */
#define FSL_CHASSIS2_CCSR_PORSR1_RCW_MASK	0xFF800000
	UINT32     porsr2;         /* POR status 2 */
	UINT8      res_008[0x20-0x8];
	UINT32     gpporcr1;       /* General-purpose POR configuration */
	UINT32	gpporcr2;
	UINT32     dcfg_fusesr;    /* Fuse status register */
	UINT8      res_02c[0x70-0x2c];
	UINT32     devdisr;        /* Device disable control */
#define FSL_CHASSIS2_DEVDISR2_DTSEC1_1	0x80000000
#define FSL_CHASSIS2_DEVDISR2_DTSEC1_2	0x40000000
#define FSL_CHASSIS2_DEVDISR2_DTSEC1_3	0x20000000
#define FSL_CHASSIS2_DEVDISR2_DTSEC1_4	0x10000000
#define FSL_CHASSIS2_DEVDISR2_DTSEC1_5	0x08000000
#define FSL_CHASSIS2_DEVDISR2_DTSEC1_6	0x04000000
#define FSL_CHASSIS2_DEVDISR2_DTSEC1_9	0x00800000
#define FSL_CHASSIS2_DEVDISR2_DTSEC1_10	0x00400000
#define FSL_CHASSIS2_DEVDISR2_10GEC1_1	0x00800000
#define FSL_CHASSIS2_DEVDISR2_10GEC1_2	0x00400000
#define FSL_CHASSIS2_DEVDISR2_10GEC1_3	0x80000000
#define FSL_CHASSIS2_DEVDISR2_10GEC1_4	0x40000000
	UINT32     devdisr2;       /* Device disable control 2 */
	UINT32     devdisr3;       /* Device disable control 3 */
	UINT32     devdisr4;       /* Device disable control 4 */
	UINT32     devdisr5;       /* Device disable control 5 */
	UINT32     devdisr6;       /* Device disable control 6 */
	UINT32     devdisr7;       /* Device disable control 7 */
	UINT8      res_08c[0x94-0x8c];
	UINT32     coredisru;      /* uppper portion for support of 64 cores */
	UINT32     coredisrl;      /* lower portion for support of 64 cores */
	UINT8      res_09c[0xa0-0x9c];
	UINT32     pvr;            /* Processor version */
	UINT32     svr;            /* System version */
	UINT32     mvr;            /* Manufacturing version */
	UINT8	res_0ac[0xb0-0xac];
	UINT32	rstcr;		/* Reset control */
	UINT32	rstrqpblsr;	/* Reset request preboot loader status */
	UINT8	res_0b8[0xc0-0xb8];
	UINT32	rstrqmr1;	/* Reset request mask */
	UINT8	res_0c4[0xc8-0xc4];
	UINT32	rstrqsr1;	/* Reset request status */
	UINT8	res_0cc[0xd4-0xcc];
	UINT32	rstrqwdtmrl;	/* Reset request WDT mask */
	UINT8	res_0d8[0xdc-0xd8];
	UINT32	rstrqwdtsrl;	/* Reset request WDT status */
	UINT8	res_0e0[0xe4-0xe0];
	UINT32	brrl;		/* Boot release */
	UINT8      res_0e8[0x100-0xe8];
	UINT32     rcwsr[16];      /* Reset control word status */
#define FSL_CHASSIS2_RCWSR0_SYS_PLL_RAT_SHIFT	25
#define FSL_CHASSIS2_RCWSR0_SYS_PLL_RAT_MASK	0x1f
#define FSL_CHASSIS2_RCWSR0_MEM_PLL_RAT_SHIFT	16
#define FSL_CHASSIS2_RCWSR0_MEM_PLL_RAT_MASK	0x3f
#define FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK	0xffff0000
#define FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT	16
	UINT8      res_140[0x200-0x140];
	UINT32     scratchrw[4];  /* Scratch Read/Write */
	UINT8      res_210[0x300-0x210];
	UINT32     scratchw1r[4];  /* Scratch Read (Write once) */
	UINT8      res_310[0x400-0x310];
	UINT32	crstsr[12];
	UINT8	res_430[0x500-0x430];

	/* PCI Express n Logical I/O Device Number register */
	UINT32 dcfg_ccsr_pex1liodnr;
	UINT32 dcfg_ccsr_pex2liodnr;
	UINT32 dcfg_ccsr_pex3liodnr;
	UINT32 dcfg_ccsr_pex4liodnr;
	/* RIO n Logical I/O Device Number register */
	UINT32 dcfg_ccsr_rio1liodnr;
	UINT32 dcfg_ccsr_rio2liodnr;
	UINT32 dcfg_ccsr_rio3liodnr;
	UINT32 dcfg_ccsr_rio4liodnr;
	/* USB Logical I/O Device Number register */
	UINT32 dcfg_ccsr_usb1liodnr;
	UINT32 dcfg_ccsr_usb2liodnr;
	UINT32 dcfg_ccsr_usb3liodnr;
	UINT32 dcfg_ccsr_usb4liodnr;
	/* SD/MMC Logical I/O Device Number register */
	UINT32 dcfg_ccsr_sdmmc1liodnr;
	UINT32 dcfg_ccsr_sdmmc2liodnr;
	UINT32 dcfg_ccsr_sdmmc3liodnr;
	UINT32 dcfg_ccsr_sdmmc4liodnr;
	/* RIO Message Unit Logical I/O Device Number register */
	UINT32 dcfg_ccsr_riomaintliodnr;

	UINT8 res_544[0x550-0x544];
	UINT32 sataliodnr[4];
	UINT8 res_560[0x570-0x560];

	UINT32 dcfg_ccsr_misc1liodnr;
	UINT32 dcfg_ccsr_misc2liodnr;
	UINT32 dcfg_ccsr_misc3liodnr;
	UINT32 dcfg_ccsr_misc4liodnr;
	UINT32 dcfg_ccsr_dma1liodnr;
	UINT32 dcfg_ccsr_dma2liodnr;
	UINT32 dcfg_ccsr_dma3liodnr;
	UINT32 dcfg_ccsr_dma4liodnr;
	UINT32 dcfg_ccsr_spare1liodnr;
	UINT32 dcfg_ccsr_spare2liodnr;
	UINT32 dcfg_ccsr_spare3liodnr;
	UINT32 dcfg_ccsr_spare4liodnr;
	UINT8	res_5a0[0x600-0x5a0];
	UINT32 dcfg_ccsr_pblsr;

	UINT32	pamubypenr;
	UINT32	dmacr1;

	UINT8	res_60c[0x610-0x60c];
	UINT32 dcfg_ccsr_gensr1;
	UINT32 dcfg_ccsr_gensr2;
	UINT32 dcfg_ccsr_gensr3;
	UINT32 dcfg_ccsr_gensr4;
	UINT32 dcfg_ccsr_gencr1;
	UINT32 dcfg_ccsr_gencr2;
	UINT32 dcfg_ccsr_gencr3;
	UINT32 dcfg_ccsr_gencr4;
	UINT32 dcfg_ccsr_gencr5;
	UINT32 dcfg_ccsr_gencr6;
	UINT32 dcfg_ccsr_gencr7;
	UINT8 res_63c[0x658-0x63c];
	UINT32 dcfg_ccsr_cgensr1;
	UINT32 dcfg_ccsr_cgensr0;
	UINT8 res_660[0x678-0x660];
	UINT32 dcfg_ccsr_cgencr1;

	UINT32 dcfg_ccsr_cgencr0;
	UINT8 res_680[0x700-0x680];
	UINT32 dcfg_ccsr_sriopstecr;
	UINT32 dcfg_ccsr_dcsrcr;

	UINT8 res_708[0x740-0x708]; /* add more registers when needed */
	UINT32 tp_ityp[64]; /* Topology Initiator Type Register */
	struct {
		UINT32 upper;
		UINT32 lower;
	} tp_cluster[16];
	UINT8 res_8c0[0xa00-0x8c0]; /* add more registers when needed */
	UINT32 dcfg_ccsr_qmbm_warmrst;
	UINT8 res_a04[0xa20-0xa04]; /* add more registers when needed */
	UINT32 dcfg_ccsr_reserved0;
	UINT32 dcfg_ccsr_reserved1;
};

/* Supplemental Configuration Unit */
struct CcsrScfg {
	UINT8 res_000[0x100-0x000];
	UINT32 usb2_icid;
	UINT32 usb3_icid;
	UINT8 res_108[0x114-0x108];
	UINT32 dma_icid;
	UINT32 sata_icid;
	UINT32 usb1_icid;
	UINT32 qe_icid;
	UINT32 sdhc_icid;
	UINT32 edma_icid;
	UINT32 etr_icid;
	UINT32 core0_sft_rst;
	UINT32 core1_sft_rst;
	UINT32 core2_sft_rst;
	UINT32 core3_sft_rst;
	UINT8 res_140[0x158-0x140];
	UINT32 altcbar;
	UINT32 qspi_cfg;
	UINT8 res_160[0x180-0x160];
	UINT32 dmamcr;
	UINT8 res_184[0x18c-0x184];
	UINT32 debug_icid;
	UINT8 res_190[0x1a4-0x190];
	UINT32 snpcnfgcr;
	UINT8 res_1a8[0x1ac-0x1a8];
	UINT32 intpcr;
	UINT8 res_1b0[0x204-0x1b0];
	UINT32 coresrencr;
	UINT8 res_208[0x220-0x208];
	UINT32 rvbar0_0;
	UINT32 rvbar0_1;
	UINT32 rvbar1_0;
	UINT32 rvbar1_1;
	UINT32 rvbar2_0;
	UINT32 rvbar2_1;
	UINT32 rvbar3_0;
	UINT32 rvbar3_1;
	UINT32 lpmcsr;
	UINT8 res_244[0x400-0x244];
	UINT32 qspidqscr;
	UINT32 ecgtxcmcr;
	UINT32 sdhciovselcr;
	UINT32 rcwpmuxcr0;
	UINT32 usbdrvvbus_selcr;
	UINT32 usbpwrfault_selcr;
	UINT32 usb_refclk_selcr1;
	UINT32 usb_refclk_selcr2;
	UINT32 usb_refclk_selcr3;
	UINT8 res_424[0x600-0x424];
	UINT32 scratchrw[4];
	UINT8 res_610[0x680-0x610];
	UINT32 corebcr;
	UINT8 res_684[0x1000-0x684];
	UINT32 pex1msiir;
	UINT32 pex1msir;
	UINT8 res_1008[0x2000-0x1008];
	UINT32 pex2;
	UINT32 pex2msir;
	UINT8 res_2008[0x3000-0x2008];
	UINT32 pex3msiir;
	UINT32 pex3msir;
};

/* Clocking */
struct CcsrClk {
	struct {
		UINT32 clkcncsr;	/* core cluster n clock control status */
		UINT8 res_004[0x0c];
		UINT32 clkcghwacsr; /* Clock generator n hardware accelerator */
		UINT8 res_014[0x0c];
	} clkcsr[4];
	UINT8	res_040[0x780]; /* 0x100 */
	struct {
		UINT32 pllcngsr;
		UINT8 res_804[0x1c];
	} pllcgsr[2];
	UINT8	res_840[0x1c0];
	UINT32	clkpcsr;	/* 0xa00 Platform clock domain control/status */
	UINT8	res_a04[0x1fc];
	UINT32	pllpgsr;	/* 0xc00 Platform PLL General Status */
	UINT8	res_c04[0x1c];
	UINT32	plldgsr;	/* 0xc20 DDR PLL General Status */
	UINT8	res_c24[0x3dc];
};

#define CCI400_CTRLORD_TERM_BARRIER	0x00000008
#define CCI400_CTRLORD_EN_BARRIER	0
#define CCI400_SHAORD_NON_SHAREABLE	0x00000002
#define CCI400_DVM_MESSAGE_REQ_EN	0x00000002
#define CCI400_SNOOP_REQ_EN		0x00000001

/* CCI-400 registers */
struct CcsrCci400 {
	UINT32 ctrl_ord;			/* Control Override */
	UINT32 spec_ctrl;			/* Speculation Control */
	UINT32 secure_access;		/* Secure Access */
	UINT32 status;			/* Status */
	UINT32 impr_err;			/* Imprecise Error */
	UINT8 res_14[0x100 - 0x14];
	UINT32 pmcr;			/* Performance Monitor Control */
	UINT8 res_104[0xfd0 - 0x104];
	UINT32 pid[8];			/* Peripheral ID */
	UINT32 cid[4];			/* Component ID */
	struct {
		UINT32 snoop_ctrl;		/* Snoop Control */
		UINT32 sha_ord;		/* Shareable Override */
		UINT8 res_1008[0x1100 - 0x1008];
		UINT32 rc_qos_ord;		/* read channel QoS Value Override */
		UINT32 wc_qos_ord;		/* read channel QoS Value Override */
		UINT8 res_1108[0x110c - 0x1108];
		UINT32 qos_ctrl;		/* QoS Control */
		UINT32 max_ot;		/* Max OT */
		UINT8 res_1114[0x1130 - 0x1114];
		UINT32 target_lat;		/* Target Latency */
		UINT32 latency_regu;	/* Latency Regulation */
		UINT32 qos_range;		/* QoS Range */
		UINT8 res_113c[0x2000 - 0x113c];
	} slave[5];			/* Slave Interface */
	UINT8 res_6000[0x9004 - 0x6000];
	UINT32 cycle_counter;		/* Cycle counter */
	UINT32 count_ctrl;			/* Count Control */
	UINT32 overflow_status;		/* Overflow Flag Status */
	UINT8 res_9010[0xa000 - 0x9010];
	struct {
		UINT32 event_select;	/* Event Select */
		UINT32 event_count;	/* Event Count */
		UINT32 counter_ctrl;	/* Counter Control */
		UINT32 overflow_status;	/* Overflow Flag Status */
		UINT8 res_a010[0xb000 - 0xa010];
	} pcounter[4];			/* Performance Counter */
	UINT8 res_e004[0x10000 - 0xe004];
};

VOID EnableDevicesNsAccess(struct CsuNsDev *NonSecureDevices, UINT32 Num);

VOID GetSysInfo(struct SysInfo *PtrSysInfo);

VOID SocInit(VOID);

VOID SerDesInit(VOID);

VOID FdtCpuSetup(VOID *Blob);

#endif /* __LS1043A_SOC_H__ */
