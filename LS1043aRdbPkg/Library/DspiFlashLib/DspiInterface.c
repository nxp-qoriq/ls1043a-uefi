/** DspiInterface.C
  Dspi Flash Library Containing Functions for Read, Write, Initialize, Set Speed
  Etc

  Copyright (C) 2015, Freescale Ltd. All Rights Reserved.

  This Program And The Accompanying Materials
  Are Licensed And Made Available Under The Terms And Conditions Of The BSD
  License Which Accompanies This Distribution. The Full Text Of The License May
  Be Found At http://Opensource.Org/Licenses/Bsd-License.Php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "Library/Dspi.h"

struct DspiFlash *gFlash;

EFI_STATUS
DspiFlashErase (
  IN  UINT32 Offset,
  IN  UINT64 Len
  )
{
  /* Consistency Checking */
  if (Offset + Len > gFlash->Size) {
    DEBUG((EFI_D_ERROR, "ERROR: Attempting %S Past Flash Size (%#X)\n",
		Len, gFlash->Size));
    return 1;
  }

  return gFlash->Erase(gFlash, Offset, Len);
}

EFI_STATUS
DspiFlashWrite (
  IN  UINT32 Offset,
  IN  UINT64 Len,
  IN  CONST VOID *Buf
  )
{
  /* Consistency Checking */
  if (Offset + Len > gFlash->Size) {
    DEBUG((EFI_D_ERROR, "ERROR: Attempting %s Past Flash Size (0x%x)\n",
		Len, gFlash->Size));
    return EFI_INVALID_PARAMETER;
  }

  return gFlash->Write(gFlash, Offset, Len, Buf);
}

EFI_STATUS
DspiFlashRead (
  IN  UINT32 Offset,
  IN  UINT64 Len,
  OUT VOID *Buf
  )
{
  /* Consistency Checking */
  if (Offset + Len > gFlash->Size) {
    DEBUG((EFI_D_ERROR, "ERROR: Attempting %s Past Flash Size (0x%x)\n",
		Len, gFlash->Size));
    return EFI_INVALID_PARAMETER;
  }

  return gFlash->Read(gFlash, Offset, Len, Buf);
}

VOID
DspiFlashFree (
  VOID
  )
{
  FreePool(gFlash->Dspi);
  FreePool(gFlash);
}

VOID
SelectDspi (
  IN VOID
  )
{
  UINT8 Data = 0;

  /* Enable soft mux */
  Data = CPLD_READ(soft_mux_on);
  if ((Data & (ENABLE_SDXC_SOFT_MUX | ENABLE_RCW_SOFT_MUX)) 
	!= (ENABLE_SDXC_SOFT_MUX | ENABLE_RCW_SOFT_MUX))
    CPLD_WRITE(soft_mux_on, (Data | (ENABLE_SDXC_SOFT_MUX |
					ENABLE_RCW_SOFT_MUX)));

  /* Enable sdhc */
  Data = CPLD_READ(sdhc_spics_sel);
  if ((Data & 0x01) != 0x01)
    CPLD_WRITE(sdhc_spics_sel, (Data | 0x01));
}

EFI_STATUS
DspiDetect(
  VOID
  )
{

  struct DspiSlave *Dspi;

  UINT32 Bus = CONFIG_SF_DEFAULT_BUS;
  UINT32 Cs = CONFIG_SF_DEFAULT_CS;
  UINT32 Speed = CONFIG_SF_DEFAULT_SPEED;
  UINT32 Mode = CONFIG_SF_DEFAULT_MODE;
  struct DspiFlash *New = NULL;
  
  Dspi = DspiSetupSlave(Bus, Cs, Speed, Mode);
  if (!Dspi) {
    DEBUG((EFI_D_ERROR, "SF: Failed To Set Up Slave\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  New = DspiFlashProbeSlave(Dspi);

  if (!New)
    return EFI_OUT_OF_RESOURCES;

  if (gFlash)
    DspiFlashFree();
  gFlash = New;

  return EFI_SUCCESS;
}

