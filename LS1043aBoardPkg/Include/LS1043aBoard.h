/** LS1043aBoard.h
*  Header defining the LS1043aBoard constants (Base addresses, sizes, flags)
*
*  Copyright (c) 2013, Freescale Ltd. All rights reserved.
*  Author: Bhupesh Sharma <bhupesh.sharma@freescale.com>
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

#ifndef __LS1043aBOARD_PLATFORM_H__
#define __LS1043aBOARD_PLATFORM_H__

// Types
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// DDR attributes
#define DDR_ATTRIBUTES_CACHED                ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define DDR_ATTRIBUTES_UNCACHED              ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED

#define CONFIG_SYS_CACHELINE_SIZE	32

#define ROMCP_BASE1_ADDR		0x00000000
#define ROMCP_SIZE1			0x00100000
#define ROMCP_BASE2_ADDR		0x00100000
#define ROMCP_SIZE2			0x00F00000

#define GIC_BASE_ADDR		0x01402000
#define GIC_SIZE			0x1000
#define GICDIST_BASE_ADDR		0x01401000
#define GICDIST_SIZE			0x1000

/* FIXME:Remove this?
#define L2_CACHE_BASE_ADDR		0x10900000
#define L2_CACHE_END_ADDR		0x10903FFF
*/
#define OCRAM1_BASE_ADDR		0x10000000
#define OCRAM1_SIZE			0x10000
#define OCRAM2_BASE_ADDR		0x10010000
#define OCRAM2_SIZE			0x10000

#define DUART_BASE_ADDR		0x21C0000
#define DUART_SIZE			0x1000

#define WDOG1_BASE_ADDR		0x02AD0000
#define WDOG2_BASE_ADDR		0x02AE0000
#define WDOG3_BASE_ADDR		0x02A70000
#define WDOG4_BASE_ADDR		0x02A80000
#define WDOG5_BASE_ADDR		0x02A90000

#define WDOG_SIZE			0x1000

#define WDOG_WCR_OFFSET		0
#define WDOG_WSR_OFFSET		2
#define WDOG_WRSR_OFFSET		4
#define WDOG_WICR_OFFSET		6
#define WDOG_WMCR_OFFSET		8

#define WDOG_WMCR_PDE		(1 << 0)
#define WDOG_WCR_WT			(0xFF << 8)
#define WDOG_WCR_WDE			(1 << 2)

#define WDOG_SERVICE_SEQ1		0x5555
#define WDOG_SERVICE_SEQ2		0xAAAA

#define I2C0_BASE_ADDRESS		0x02180000
#define I2C1_BASE_ADDRESS		0x02190000
#define I2C2_BASE_ADDRESS		0x021A0000
#define I2C3_BASE_ADDRESS		0x02183000

#define I2C_SIZE			0x10000
#define DSPI_MEMORY_SIZE		0x10000
#define DDRC_MEMORY_SIZE		0x10000
#define SDXC_MEMORY_SIZE		0x10000

#define CONFIG_SYS_IMMR		0x01000000
#define CONFIG_SYS_FSL_DDR_ADDR	0x01080000
#define CONFIG_SYS_FSL_DSPI_ADDR	0x02100000
#define CONFIG_SYS_FSL_SDXC_ADDR	0x01560000

#define IFC_MEM1_BASE_ADDR         0x60000000
#define IFC_MEM1_SIZE              0x20000000

#define IFC_REG_BASE_ADDR          0x1530000
#define IFC_REG_SIZE               0x0003000
#define SCFG_BASE_ADDR             0x1570000
#define SCFG_SIZE                  0x0010000

#endif
