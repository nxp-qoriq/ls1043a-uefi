/** @CpldLib.c
  Cpld specific Library for LS1043A-RDB board, containing functions to
  program and read the Cpld registers.

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
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>

#include <LS1043aRdb.h>
#include <CpldLib.h>
#include <Library/FslIfc.h>

UINT8
CpldRead (
  OUT UINTN reg
  )
{
	VOID *p = (VOID *)CONFIG_CPLD_BASE;

	return MmioRead8((UINTN)(p + reg));
}

VOID
CpldWrite (
  IN UINTN reg,
  IN UINT8 value
  )
{
	VOID *p = (VOID *)CONFIG_CPLD_BASE;

	MmioWrite8((UINTN)(p + reg), value);
}

/* Set the boot bank to the alternate bank */
VOID
CpldSetAltbank (
  VOID
  )
{
	UINT8 reg4 = CPLD_READ(soft_mux_on);
	UINT8 reg7 = CPLD_READ(vbank);

	CPLD_WRITE(soft_mux_on, reg4 | CPLD_SW_MUX_BANK_SEL);

	reg7 = (reg7 & ~CPLD_BANK_SEL_MASK) | CPLD_BANK_SEL_ALTBANK;
	CPLD_WRITE(vbank, reg7);

	CPLD_WRITE(system_rst, 1);
}

/* Set the boot bank to the default bank */
VOID
CpldSetDefaultBank (
  VOID
  )
{
	CPLD_WRITE(global_rst, 1);
}

VOID
CpldDumpRegs (
  VOID
  )
{
	DEBUG((EFI_D_INFO, "cpld_ver	= %x\n", CPLD_READ(cpld_ver)));
	DEBUG((EFI_D_INFO, "cpld_ver_sub	= %x\n", CPLD_READ(cpld_ver_sub)));
	DEBUG((EFI_D_INFO, "pcba_ver	= %x\n", CPLD_READ(pcba_ver)));
	DEBUG((EFI_D_INFO, "soft_mux_on	= %x\n", CPLD_READ(soft_mux_on)));
	DEBUG((EFI_D_INFO, "cfg_rcw_src1	= %x\n", CPLD_READ(cfg_rcw_src1)));
	DEBUG((EFI_D_INFO, "cfg_rcw_src2	= %x\n", CPLD_READ(cfg_rcw_src2)));
	DEBUG((EFI_D_INFO, "vbank		= %x\n", CPLD_READ(vbank)));
	DEBUG((EFI_D_INFO, "sysclk_sel	= %x\n", CPLD_READ(sysclk_sel)));
	DEBUG((EFI_D_INFO, "uart_sel	= %x\n", CPLD_READ(uart_sel)));
	DEBUG((EFI_D_INFO, "sd1refclk_sel	= %x\n", CPLD_READ(sd1refclk_sel)));
	DEBUG((EFI_D_INFO, "tdmclk_mux_sel	= %x\n", CPLD_READ(tdmclk_mux_sel)));
	DEBUG((EFI_D_INFO, "sdhc_spics_sel	= %x\n", CPLD_READ(sdhc_spics_sel)));
	DEBUG((EFI_D_INFO, "status_led	= %x\n", CPLD_READ(status_led)));
}

VOID
CpldRevBit (
  OUT UINT8 *value
  )
{
	UINT8 rev_val, val;
	UINTN i;

	val = *value;
	rev_val = val & 1;
	for (i = 1; i <= 7; i++) {
		val >>= 1;
		rev_val <<= 1;
		rev_val |= val & 1;
	}

	*value = rev_val;
}

VOID
DoCpld (
  IN CpldCmd Cmd
  )
{
	switch (Cmd) {
	case RESET:
		CpldSetDefaultBank();
		break;
	case RESET_ALTBANK:
		CpldSetAltbank();
		break;
	case DUMP_REGISTERS:
		CpldDumpRegs();
		break;
	default:
		DEBUG((EFI_D_ERROR, "Error: Unknown Cpld Command!\n"));
		break;
	}
}

VOID
CpldInit (
  VOID
  )
{
	MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->cspr_cs[IFC_CS2].cspr_ext, IFC_CPLD_CSPR_EXT);

	MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->ftim_cs[IFC_CS2].ftim[IFC_FTIM0],
			IFC_CPLD_FTIM0);
	MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->ftim_cs[IFC_CS2].ftim[IFC_FTIM1],
			IFC_CPLD_FTIM1);
	MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->ftim_cs[IFC_CS2].ftim[IFC_FTIM2],
			IFC_CPLD_FTIM2);
	MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->ftim_cs[IFC_CS2].ftim[IFC_FTIM3],
			IFC_CPLD_FTIM3);

	MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->csor_cs[IFC_CS2].csor, IFC_CPLD_CSOR);
	MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->amask_cs[IFC_CS2].amask, IFC_CPLD_AMASK);
	MmioWriteBe32((UINTN) &(IFC_REGS_BASE)->cspr_cs[IFC_CS2].cspr, IFC_CPLD_CSPR);
}
