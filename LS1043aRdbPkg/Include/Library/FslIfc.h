/** @file FslIfc.h

  Copyright (c) 2014, Freescale Ltd. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __FLASH_H__
#define __FLASH_H__

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/Cpu.h>

#define NOIBR
#define NAND_BANK 1
#define NOR_BANK 0

#define FSL_IFC_BANK_COUNT	7
#define PAGE_SIZE(x)		((x) & 0x01)
#define PAGE_SIZE_2K_VAL	(0x01UL)

#define SPARE_AREA_SIZE(x)	(((x) >> 2) & 0x01)
#define SPARE_AREA_SIZE_64B_VAL	(0x1UL)

#define BLOCK_SIZE(x)		(((x) >> 4) & 0x01)
#define BLOCK_SIZE_128K_VAL	(0x01UL)

#define ORGANIZATION(x)		(((x) >> 6) & 0x01)
#define ORGANIZATION_X8		(0x0UL)
#define ORGANIZATION_X16	(0x1UL)

#define PAGE_SIZE_512B		(512)
#define PAGE_SIZE_2K		(2048)
#define PAGE_SIZE_4K		(4096)
#define SPARE_AREA_SIZE_16B	(16)
#define SPARE_AREA_SIZE_64B	(64)

#define BLOCK_SIZE_16K		(16*1024)
#define BLOCK_SIZE_128K		(128*1024)

#define BLOCK_COUNT		(2048)
#define LAST_BLOCK		(BLOCK_COUNT - 1)

#define ECC_POSITION		2

//List of commands.
#define NAND_CMD_RESET		0xFF
#define NAND_CMD_READID		0x90

#define NAND_CMD_STATUS		0x70

#define NAND_CMD_READ0		0x00
#define NAND_CMD_READSTART	0x30

#define NAND_CMD_ERASE1		0x60
#define NAND_CMD_ERASE2 	0xD0

#define NAND_CMD_SEQIN		0x80
#define NAND_CMD_PAGEPROG	0x10

//Nand status register bit definition
#define NAND_SUCCESS		(0x0UL << 0)
#define NAND_FAILURE		BIT0

#define NAND_BUSY		(0x0UL << 6)
#define NAND_READY		BIT6

#define NAND_RESET_STATUS	(0x60UL << 0)

#define MAX_RETRY_COUNT		1500

#define IFC_REG_BASE	 	0x1530000
#define IFC_NAND_BUF_BASE	0x70000000
#define IFC_NOR_BUF_BASE	0x60000000

#define IFC_CSPR_REG_LEN	148
#define IFC_AMASK_REG_LEN	144
#define IFC_CSOR_REG_LEN	144
#define IFC_FTIM_REG_LEN	576

#define IFC_NAND_SEQ_STRT_FIR_STRT	0x80000000

#define IFC_CSPR_USED_LEN	sizeof(FSL_IFC_CSPR) * \
					FSL_IFC_BANK_COUNT
#define IFC_AMASK_USED_LEN	sizeof(FSL_IFC_AMASK) * \
					FSL_IFC_BANK_COUNT
#define IFC_CSOR_USED_LEN	sizeof(FSL_IFC_CSOR) * \
					FSL_IFC_BANK_COUNT
#define IFC_FTIM_USED_LEN	sizeof(FSL_IFC_FTIM) * \
					FSL_IFC_BANK_COUNT

/*
 * NAND Event and Error Status Register (NAND_EVTER_STAT)
 */
/* Operation Complete */
#define IFC_NAND_EVTER_STAT_OPC		0x80000000
/* Flash Timeout Error */
#define IFC_NAND_EVTER_STAT_FTOER	0x08000000
/* Write Protect Error */
#define IFC_NAND_EVTER_STAT_WPER	0x04000000
/* ECC Error */
#define IFC_NAND_EVTER_STAT_ECCER	0x02000000

/*
 * NAND Flash Byte Count Register (NAND_BC)
 */
/* Byte Count for read/Write */
#define IFC_NAND_BC			0x000001FF

/*
 * NAND Flash Instruction Registers (NAND_FIR0/NAND_FIR1/NAND_FIR2)
 */
/* NAND Machine specific opcodes OP0-OP14*/
#define IFC_NAND_FIR0_OP0		0xFC000000
#define IFC_NAND_FIR0_OP0_SHIFT		26
#define IFC_NAND_FIR0_OP1		0x03F00000
#define IFC_NAND_FIR0_OP1_SHIFT		20
#define IFC_NAND_FIR0_OP2		0x000FC000
#define IFC_NAND_FIR0_OP2_SHIFT		14
#define IFC_NAND_FIR0_OP3		0x00003F00
#define IFC_NAND_FIR0_OP3_SHIFT		8
#define IFC_NAND_FIR0_OP4		0x000000FC
#define IFC_NAND_FIR0_OP4_SHIFT		2
#define IFC_NAND_FIR1_OP5		0xFC000000
#define IFC_NAND_FIR1_OP5_SHIFT		26
#define IFC_NAND_FIR1_OP6		0x03F00000
#define IFC_NAND_FIR1_OP6_SHIFT		20
#define IFC_NAND_FIR1_OP7		0x000FC000
#define IFC_NAND_FIR1_OP7_SHIFT		14
#define IFC_NAND_FIR1_OP8		0x00003F00
#define IFC_NAND_FIR1_OP8_SHIFT		8
#define IFC_NAND_FIR1_OP9		0x000000FC
#define IFC_NAND_FIR1_OP9_SHIFT		2
#define IFC_NAND_FIR2_OP10		0xFC000000
#define IFC_NAND_FIR2_OP10_SHIFT	26
#define IFC_NAND_FIR2_OP11		0x03F00000
#define IFC_NAND_FIR2_OP11_SHIFT	20
#define IFC_NAND_FIR2_OP12		0x000FC000
#define IFC_NAND_FIR2_OP12_SHIFT	14
#define IFC_NAND_FIR2_OP13		0x00003F00
#define IFC_NAND_FIR2_OP13_SHIFT	8
#define IFC_NAND_FIR2_OP14		0x000000FC
#define IFC_NAND_FIR2_OP14_SHIFT	2

/*
 * NAND Flash Command Registers (NAND_FCR0/NAND_FCR1)
 */
/* General purpose FCM flash command bytes CMD0-CMD7 */
#define IFC_NAND_FCR0_CMD0		0xFF000000
#define IFC_NAND_FCR0_CMD0_SHIFT	24
#define IFC_NAND_FCR0_CMD1		0x00FF0000
#define IFC_NAND_FCR0_CMD1_SHIFT	16
#define IFC_NAND_FCR0_CMD2		0x0000FF00
#define IFC_NAND_FCR0_CMD2_SHIFT	8
#define IFC_NAND_FCR0_CMD3		0x000000FF
#define IFC_NAND_FCR0_CMD3_SHIFT	0
#define IFC_NAND_FCR1_CMD4		0xFF000000
#define IFC_NAND_FCR1_CMD4_SHIFT	24
#define IFC_NAND_FCR1_CMD5		0x00FF0000
#define IFC_NAND_FCR1_CMD5_SHIFT	16
#define IFC_NAND_FCR1_CMD6		0x0000FF00
#define IFC_NAND_FCR1_CMD6_SHIFT	8
#define IFC_NAND_FCR1_CMD7		0x000000FF
#define IFC_NAND_FCR1_CMD7_SHIFT	0

/*
 * Flash ROW and COL Address Register (ROWn, COLn)
 */
/* Main/spare region locator */
#define IFC_NAND_COL_MS			0x80000000
/* Column Address */
#define IFC_NAND_COL_CA_MASK		0x00000FFF

#define NAND_STATUS_WP			0x80

/*
 * NAND Event and Error Enable Register (NAND_EVTER_EN)
 */
/* Operation complete event enable */
#define IFC_NAND_EVTER_EN_OPC_EN	0x80000000
/* Page read complete event enable */
#define IFC_NAND_EVTER_EN_PGRDCMPL_EN	0x20000000
/* Flash Timeout error enable */
#define IFC_NAND_EVTER_EN_FTOER_EN	0x08000000
/* Write Protect error enable */
#define IFC_NAND_EVTER_EN_WPER_EN	0x04000000
/* ECC error logging enable */
#define IFC_NAND_EVTER_EN_ECCER_EN	0x02000000

/*
 * CSPR - Chip Select Property Register
 */
#define CSPR_BA			0xFFFF0000
#define CSPR_BA_SHIFT 		16
#define CSPR_PORT_SIZE		0x00000180
#define CSPR_PORT_SIZE_SHIFT	7
/* Port Size 8 bit */
#define CSPR_PORT_SIZE_8	0x00000080
/* Port Size 16 bit */
#define CSPR_PORT_SIZE_16	0x00000100
/* Port Size 32 bit */
#define CSPR_PORT_SIZE_32	0x00000180
/* Write Protect */
#define CSPR_WP			0x00000040
#define CSPR_WP_SHIFT		6
/* Machine Select */
#define CSPR_MSEL		0x00000006
#define CSPR_MSEL_SHIFT		1
/* NOR */
#define CSPR_MSEL_NOR		0x00000000
/* NAND */
#define CSPR_MSEL_NAND		0x00000002
/* GPCM */
#define CSPR_MSEL_GPCM		0x00000004
/* Bank Valid */
#define CSPR_V			0x00000001
#define CSPR_V_SHIFT		0

/*
 * Chip Select Option Register - NOR Flash Mode
 */
/* Enable Address shift Mode */
#define CSOR_NOR_ADM_SHFT_MODE_EN	0x80000000
/* Page Read Enable from NOR device */
#define CSOR_NOR_PGRD_EN		0x10000000
/* AVD Toggle Enable during Burst Program */
#define CSOR_NOR_AVD_TGL_PGM_EN		0x01000000
/* Address Data Multiplexing Shift */
#define CSOR_NOR_ADM_MASK		0x0003E000
#define CSOR_NOR_ADM_SHIFT_SHIFT	13
#define CSOR_NOR_ADM_SHIFT(n)	((n) << CSOR_NOR_ADM_SHIFT_SHIFT)
/* Type of the NOR device hooked */
#define CSOR_NOR_NOR_MODE_AYSNC_NOR	0x00000000
#define CSOR_NOR_NOR_MODE_AVD_NOR	0x00000020
/* Time for Read Enable High to Output High Impedance */
#define CSOR_NOR_TRHZ_MASK		0x0000001C
#define CSOR_NOR_TRHZ_SHIFT		2
#define CSOR_NOR_TRHZ_20		0x00000000
#define CSOR_NOR_TRHZ_40		0x00000004
#define CSOR_NOR_TRHZ_60		0x00000008
#define CSOR_NOR_TRHZ_80		0x0000000C
#define CSOR_NOR_TRHZ_100		0x00000010
/* Buffer control disable */
#define CSOR_NOR_BCTLD			0x00000001

/*
 * Chip Select Option Register IFC_NAND Machine
 */
/* Enable ECC Encoder */
#define CSOR_NAND_ECC_ENC_EN	0x80000000
#define CSOR_NAND_ECC_MODE_MASK	0x30000000
/* 4 bit correction per 520 Byte sector */
#define CSOR_NAND_ECC_MODE_4	0x00000000
/* 8 bit correction per 528 Byte sector */
#define CSOR_NAND_ECC_MODE_8	0x10000000
/* Enable ECC Decoder */
#define CSOR_NAND_ECC_DEC_EN	0x04000000
/* Row Address Length */
#define CSOR_NAND_RAL_MASK	0x01800000
#define CSOR_NAND_RAL_SHIFT	20
#define CSOR_NAND_RAL_1		0x00000000
#define CSOR_NAND_RAL_2		0x00800000
#define CSOR_NAND_RAL_3		0x01000000
#define CSOR_NAND_RAL_4		0x01800000
/* Page Size 512b, 2k, 4k */
#define CSOR_NAND_PGS_MASK	0x00180000
#define CSOR_NAND_PGS_SHIFT	16
#define CSOR_NAND_PGS_512	0x00000000
#define CSOR_NAND_PGS_2K	0x00080000
#define CSOR_NAND_PGS_4K	0x00100000
#define CSOR_NAND_PGS_8K	0x00180000
/* Spare region Size */
#define CSOR_NAND_SPRZ_MASK		0x0000E000
#define CSOR_NAND_SPRZ_SHIFT		13
#define CSOR_NAND_SPRZ_16		0x00000000
#define CSOR_NAND_SPRZ_64		0x00002000
#define CSOR_NAND_SPRZ_128		0x00004000
#define CSOR_NAND_SPRZ_210		0x00006000
#define CSOR_NAND_SPRZ_218		0x00008000
#define CSOR_NAND_SPRZ_224		0x0000A000
#define CSOR_NAND_SPRZ_CSOR_EXT 0x0000C000
/* Pages Per Block */
#define CSOR_NAND_PB_MASK		0x00000700
#define CSOR_NAND_PB_SHIFT		8
#define CSOR_NAND_PB(n)			(n-5) << CSOR_NAND_PB_SHIFT
/* Time for Read Enable High to Output High Impedance */
#define CSOR_NAND_TRHZ_MASK		0x0000001C
#define CSOR_NAND_TRHZ_SHIFT		2
#define CSOR_NAND_TRHZ_20		0x00000000
#define CSOR_NAND_TRHZ_40		0x00000004
#define CSOR_NAND_TRHZ_60		0x00000008
#define CSOR_NAND_TRHZ_80		0x0000000C
#define CSOR_NAND_TRHZ_100		0x00000010
/* Buffer control disable */
#define CSOR_NAND_BCTLD			0x00000001

#define IFC_NAND_BUF_MASK		0xffff0000
#define IFC_NOR_BUF_MASK		0xffff0000

#define IFC_NAND_CSPR		((IFC_NAND_BUF_BASE & IFC_NAND_BUF_MASK)\
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_NAND \
				| CSPR_V)
#define IFC_NAND_CSPR_EXT	0x0
#define IFC_NAND_AMASK		0xFFFF0000
#define IFC_NAND_CSOR    	(CSOR_NAND_ECC_ENC_EN /* ECC on encode */ \
                                | CSOR_NAND_ECC_DEC_EN /* ECC on decode */ \
                                | CSOR_NAND_ECC_MODE_4 /* 4-bit ECC */ \
                                | CSOR_NAND_RAL_3       /* RAL = 3 Bytes */ \
                                | CSOR_NAND_PGS_2K      /* Page Size = 2K */ \
                                | CSOR_NAND_SPRZ_64     /* Spare size = 64 */ \
                                | CSOR_NAND_PB(6))     /* 2^6 Pages Per Block */

/*
 * FTIM0 - NOR Flash Mode
 */
#define FTIM0_NOR			0xF03F3F3F
#define FTIM0_NOR_TACSE_SHIFT	28
#define FTIM0_NOR_TACSE(n)	((n) << FTIM0_NOR_TACSE_SHIFT)
#define FTIM0_NOR_TEADC_SHIFT	16
#define FTIM0_NOR_TEADC(n)	((n) << FTIM0_NOR_TEADC_SHIFT)
#define FTIM0_NOR_TAVDS_SHIFT	8
#define FTIM0_NOR_TAVDS(n)	((n) << FTIM0_NOR_TAVDS_SHIFT)
#define FTIM0_NOR_TEAHC_SHIFT	0
#define FTIM0_NOR_TEAHC(n)	((n) << FTIM0_NOR_TEAHC_SHIFT)
/*
 * FTIM1 - NOR Flash Mode
 */
#define FTIM1_NOR			0xFF003F3F
#define FTIM1_NOR_TACO_SHIFT	24
#define FTIM1_NOR_TACO(n)	((n) << FTIM1_NOR_TACO_SHIFT)
#define FTIM1_NOR_TRAD_NOR_SHIFT	8
#define FTIM1_NOR_TRAD_NOR(n)	((n) << FTIM1_NOR_TRAD_NOR_SHIFT)
#define FTIM1_NOR_TSEQRAD_NOR_SHIFT	0
#define FTIM1_NOR_TSEQRAD_NOR(n)	((n) << FTIM1_NOR_TSEQRAD_NOR_SHIFT)
/*
 * FTIM2 - NOR Flash Mode
 */
#define FTIM2_NOR			0x0F3CFCFF
#define FTIM2_NOR_TCS_SHIFT		24
#define FTIM2_NOR_TCS(n)	((n) << FTIM2_NOR_TCS_SHIFT)
#define FTIM2_NOR_TCH_SHIFT		18
#define FTIM2_NOR_TCH(n)	((n) << FTIM2_NOR_TCH_SHIFT)
#define FTIM2_NOR_TWPH_SHIFT	10
#define FTIM2_NOR_TWPH(n)	((n) << FTIM2_NOR_TWPH_SHIFT)
#define FTIM2_NOR_TWP_SHIFT		0
#define FTIM2_NOR_TWP(n)	((n) << FTIM2_NOR_TWP_SHIFT)

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline int generic_fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

static inline int __ilog2(unsigned int x)
{
	return generic_fls(x) - 1;
}

/*
 * Address Mask Register
 */
#define IFC_AMASK_MASK			0xFFFF0000
#define IFC_AMASK_SHIFT			16
#define IFC_AMASK(n)			(IFC_AMASK_MASK << \
					(__ilog2(n) - IFC_AMASK_SHIFT))
#define IFC_NOR_AMASK		IFC_AMASK(128*1024*1024)

#define IFC_NOR_CSPR    	((IFC_NOR_BUF_BASE & IFC_NOR_BUF_MASK)\
				| CSPR_PORT_SIZE_8 \
                                | CSPR_MSEL_NOR        \
                                | CSPR_V)
#define IFC_NOR_CSPR_EXT	0x0
#define IFC_NOR_CSOR	        CSOR_NOR_ADM_SHIFT(10)
#define IFC_NOR_FTIM0	        (FTIM0_NOR_TACSE(0x1) | \
				 FTIM0_NOR_TEADC(0x1) | \
				 FTIM0_NOR_TEAHC(0x1))
#define IFC_NOR_FTIM1	        (FTIM1_NOR_TACO(0x1) | \
			 	 FTIM1_NOR_TRAD_NOR(0x1))
#define IFC_NOR_FTIM2	        (FTIM2_NOR_TCS(0x0) | \
				 FTIM2_NOR_TCH(0x0) | \
				 FTIM2_NOR_TWP(0x1))
#define IFC_NOR_FTIM3     	0x04000000

#define IFC_NOR_CSPR0		IFC_NOR_CSPR
#define IFC_NOR_AMASK0		IFC_NOR_AMASK
#define IFC_NOR_CSOR0		IFC_NOR_CSOR

#define IFC_SRAM_BUF_SIZE	0x4000

typedef enum {
	IFC_CS0 = 0,
	IFC_CS1,
	IFC_CS2,
	IFC_CS3,
	IFC_CS4,
	IFC_CS5,
	IFC_CS6,
	IFC_CS7,
} FSL_IFC_CHIP_SEL;

typedef enum {
	IFC_FTIM0 = 0,
	IFC_FTIM1,
	IFC_FTIM2,
	IFC_FTIM3,
} FSL_IFC_FTIMS;

/*
 * Instruction opcodes to be programmed
 * in FIR registers- 6bits
 */

enum ifc_nand_fir_opcodes {
	IFC_FIR_OP_NOP,
	IFC_FIR_OP_CA0,
	IFC_FIR_OP_CA1,
	IFC_FIR_OP_CA2,
	IFC_FIR_OP_CA3,
	IFC_FIR_OP_RA0,
	IFC_FIR_OP_RA1,
	IFC_FIR_OP_RA2,
	IFC_FIR_OP_RA3,
	IFC_FIR_OP_CMD0,
	IFC_FIR_OP_CMD1,
	IFC_FIR_OP_CMD2,
	IFC_FIR_OP_CMD3,
	IFC_FIR_OP_CMD4,
	IFC_FIR_OP_CMD5,
	IFC_FIR_OP_CMD6,
	IFC_FIR_OP_CMD7,
	IFC_FIR_OP_CW0,
	IFC_FIR_OP_CW1,
	IFC_FIR_OP_CW2,
	IFC_FIR_OP_CW3,
	IFC_FIR_OP_CW4,
	IFC_FIR_OP_CW5,
	IFC_FIR_OP_CW6,
	IFC_FIR_OP_CW7,
	IFC_FIR_OP_WBCD,
	IFC_FIR_OP_RBCD,
	IFC_FIR_OP_BTRD,
	IFC_FIR_OP_RDSTAT,
	IFC_FIR_OP_NWAIT,
	IFC_FIR_OP_WFR,
	IFC_FIR_OP_SBRD,
	IFC_FIR_OP_UA,
	IFC_FIR_OP_RB,
};

typedef struct {
	UINT32 cspr_ext;
	UINT32 cspr;
	UINT32 res;
} FSL_IFC_CSPR;

typedef struct {
	UINT32 amask;
	UINT32 res[0x2];
} FSL_IFC_AMASK;

typedef struct {
	UINT32 csor;
	UINT32 csor_ext;
	UINT32 res;
} FSL_IFC_CSOR;

typedef struct {
	UINT32 ftim[4];
	UINT32 res[0x8];
}FSL_IFC_FTIM ;

typedef struct {
	UINT32 ncfgr;
	UINT32 res1[0x4];
	UINT32 nand_fcr0;
	UINT32 nand_fcr1;
	UINT32 res2[0x8];
	UINT32 row0;
	UINT32 res3;
	UINT32 col0;
	UINT32 res4;
	UINT32 row1;
	UINT32 res5;
	UINT32 col1;
	UINT32 res6;
	UINT32 row2;
	UINT32 res7;
	UINT32 col2;
	UINT32 res8;
	UINT32 row3;
	UINT32 res9;
	UINT32 col3;
	UINT32 res10[0x24];
	UINT32 nand_fbcr;
	UINT32 res11;
	UINT32 nand_fir0;
	UINT32 nand_fir1;
	UINT32 nand_fir2;
	UINT32 res12[0x10];
	UINT32 nand_csel;
	UINT32 res13;
	UINT32 nandseq_strt;
	UINT32 res14;
	UINT32 nand_evter_stat;
	UINT32 res15;
	UINT32 pgrdcmpl_evt_stat;
	UINT32 res16[0x2];
	UINT32 nand_evter_en;
	UINT32 res17[0x2];
	UINT32 nand_evter_intr_en;
	UINT32 res18[0x2];
	UINT32 nand_erattr0;
	UINT32 nand_erattr1;
	UINT32 res19[0x10];
	UINT32 nand_fsr;
	UINT32 res20;
	UINT32 nand_eccstat[4];
	UINT32 res21[0x20];
	UINT32 nanndcr;
	UINT32 res22[0x2];
	UINT32 nand_autoboot_trgr;
	UINT32 res23;
	UINT32 nand_mdr;
	UINT32 res24[0x5C];
} FSL_IFC_NAND;

/*
 * IFC controller NOR Machine registers
 */
typedef struct {
	UINT32 nor_evter_stat;
	UINT32 res1[0x2];
	UINT32 nor_evter_en;
	UINT32 res2[0x2];
	UINT32 nor_evter_intr_en;
	UINT32 res3[0x2];
	UINT32 nor_erattr0;
	UINT32 nor_erattr1;
	UINT32 nor_erattr2;
	UINT32 res4[0x4];
	UINT32 norcr;
	UINT32 res5[0xEF];
} FSL_IFC_NOR;

/*
 * IFC Controller Registers
 */
typedef struct {
	UINT32 ifc_rev;
	UINT32 res1[0x2];
	FSL_IFC_CSPR cspr_cs[FSL_IFC_BANK_COUNT];
	UINT8 res2[IFC_CSPR_REG_LEN - IFC_CSPR_USED_LEN];
	FSL_IFC_AMASK amask_cs[FSL_IFC_BANK_COUNT];
	UINT8 res3[IFC_AMASK_REG_LEN - IFC_AMASK_USED_LEN];
	FSL_IFC_CSOR csor_cs[FSL_IFC_BANK_COUNT];
	UINT8 res4[IFC_CSOR_REG_LEN - IFC_CSOR_USED_LEN];
	FSL_IFC_FTIM ftim_cs[FSL_IFC_BANK_COUNT];
	UINT8 res5[IFC_FTIM_REG_LEN - IFC_FTIM_USED_LEN];
	UINT32 rb_stat;
	UINT32 res6[0x2];
	UINT32 ifc_gcr;
	UINT32 res7[0x2];
	UINT32 cm_evter_stat;
	UINT32 res8[0x2];
	UINT32 cm_evter_en;
	UINT32 res9[0x2];
	UINT32 cm_evter_intr_en;
	UINT32 res10[0x2];
	UINT32 cm_erattr0;
	UINT32 cm_erattr1;
	UINT32 res11[0x2];
	UINT32 ifc_ccr;
	UINT32 ifc_csr;
	UINT32 res12[0x2EB];
	FSL_IFC_NAND ifc_nand;
	FSL_IFC_NOR ifc_nor;
} FSL_IFC_REGS;

VOID
IfcNorInit (
  VOID
  );

#endif //__FLASH_H__
