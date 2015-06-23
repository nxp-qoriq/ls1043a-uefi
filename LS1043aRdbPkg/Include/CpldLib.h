/** CpldLib.h
*  Header defining the LS1043a Cpld specific constants (Base addresses, sizes, flags)
*
*  Copyright (c) 2015, Freescale Ltd. All rights reserved.
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

#ifndef __LS1043A_CPLD_H__
#define __LS1043A_CPLD_H__

#include <Common.h>

/*
 * CPLD register set of LS1043ARDB board-specific.
 */
struct cpld_data {
	UINT8 cpld_ver;		/* 0x0 - CPLD Major Revision Register */
	UINT8 cpld_ver_sub;	/* 0x1 - CPLD Minor Revision Register */
	UINT8 pcba_ver;		/* 0x2 - PCBA Revision Register */
	UINT8 system_rst;	/* 0x3 - system reset register */
	UINT8 soft_mux_on;	/* 0x4 - Switch Control Enable Register */
	UINT8 cfg_rcw_src1;	/* 0x5 - Reset config word 1 */
	UINT8 cfg_rcw_src2;	/* 0x6 - Reset config word 1 */
	UINT8 vbank;		/* 0x7 - Flash bank selection Control */
	UINT8 sysclk_sel;	/* 0x8 - */
	UINT8 uart_sel;		/* 0x9 - */
	UINT8 sd1refclk_sel;	/* 0xA - */
	UINT8 tdmclk_mux_sel;	/* 0xB - */
	UINT8 sdhc_spics_sel;	/* 0xC - */
	UINT8 status_led;	/* 0xD - */
	UINT8 global_rst;	/* 0xE - */
};

/*
 * Reset the board, Reset to alternate bank or Dump registers:
 * RESET - reset to default bank
 * RESET_ALTBANK - reset to alternate bank
 * DUMP_REGISTERS - display the CPLD registers
 */
typedef enum {
	RESET = 0,
	RESET_ALTBANK,
	DUMP_REGISTERS
} CpldCmd;

UINT8 CpldRead(UINTN reg);
VOID CpldWrite(UINTN reg, UINT8 value);
VOID CpldRevBit(UINT8 *value);
VOID DoCpld (CpldCmd Cmd);
VOID CpldInit (VOID);

#define CPLD_READ(reg) CpldRead(offsetof(struct cpld_data, reg))
#define CPLD_WRITE(reg, value)  \
	CpldWrite(offsetof(struct cpld_data, reg), value)

/* CPLD on IFC */
#define CPLD_SW_MUX_BANK_SEL	0x40
#define CPLD_BANK_SEL_MASK	0x07
#define CPLD_BANK_SEL_ALTBANK	0x04

#endif
