/** @DdrLib.c
  Ddr Library containing functions to initialize ddr controller

  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <LS1043aSocLib.h>
#include <Ddr.h>
#include <I2c.h>
#include <Library/BaseLib.h>

#define Uswap32(X) \
       ((((X) & 0xff000000) >> 24) | \
        (((X) & 0x00ff0000) >>  8) | \
        (((X) & 0x0000ff00) <<  8) | \
        (((X) & 0x000000ff) << 24))
#define DDRMC_DELAY 10000

UINT32 DdrcWrite(
		IN		UINTN Address,
		IN		UINT32 Value
		)
{
  *(volatile UINT32*)Address = Uswap32(Value);
  return Value;
}

/**
  Function to dump DDRC registers

**/

VOID
DdrRegDump (
  VOID
  )
{

  struct CcsrDdr *Ddr = (VOID *)CONFIG_SYS_FSL_DDR_ADDR;

  DEBUG((EFI_D_RELEASE, "Cs0Bnds = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs0Bnds)));
  DEBUG((EFI_D_RELEASE, "Cs1Bnds = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs1Bnds)));
  DEBUG((EFI_D_RELEASE, "Cs2Bnds = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs2Bnds)));
  DEBUG((EFI_D_RELEASE, "Cs3Bnds = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs3Bnds)));
  DEBUG((EFI_D_RELEASE, "Cs0Config = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs0Config)));
  DEBUG((EFI_D_RELEASE, "Cs1Config = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs1Config)));
  DEBUG((EFI_D_RELEASE, "Cs2Config = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs2Config)));
  DEBUG((EFI_D_RELEASE, "Cs3Config = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs3Config)));
  DEBUG((EFI_D_RELEASE, "Cs0Config2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs0Config2)));
  DEBUG((EFI_D_RELEASE, "Cs1Config2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs1Config2)));
  DEBUG((EFI_D_RELEASE, "Cs2Config2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs2Config2)));
  DEBUG((EFI_D_RELEASE, "Cs3Config2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Cs3Config2)));
  DEBUG((EFI_D_RELEASE, "TimingCfg3 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->TimingCfg3)));
  DEBUG((EFI_D_RELEASE, "TimingCfg0 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->TimingCfg0)));
  DEBUG((EFI_D_RELEASE, "TimingCfg1 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->TimingCfg1)));
  DEBUG((EFI_D_RELEASE, "TimingCfg2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->TimingCfg2)));
  DEBUG((EFI_D_RELEASE, "SdramCfg = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramCfg)));
  DEBUG((EFI_D_RELEASE, "SdramCfg2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramCfg2)));
  DEBUG((EFI_D_RELEASE, "SdramMode = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode)));
  DEBUG((EFI_D_RELEASE, "SdramMode2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode2)));
  DEBUG((EFI_D_RELEASE, "SdramMdCntl = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMdCntl)));
  DEBUG((EFI_D_RELEASE, "SdramInterval = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramInterval)));
  DEBUG((EFI_D_RELEASE, "SdramDataInit = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramDataInit)));
  DEBUG((EFI_D_RELEASE, "SdramClkCntl = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramClkCntl)));
  DEBUG((EFI_D_RELEASE, "InitAddr = 0x%x\n",MmioReadBe32((UINTN)&Ddr->InitAddr)));
  DEBUG((EFI_D_RELEASE, "InitExtAddr = 0x%x\n",MmioReadBe32((UINTN)&Ddr->InitExtAddr)));
  DEBUG((EFI_D_RELEASE, "TimingCfg4 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->TimingCfg4)));
  DEBUG((EFI_D_RELEASE, "TimingCfg5 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->TimingCfg5)));
  DEBUG((EFI_D_RELEASE, "TimingCfg6 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->TimingCfg6)));
  DEBUG((EFI_D_RELEASE, "TimingCfg7 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->TimingCfg7)));
  DEBUG((EFI_D_RELEASE, "DdrZqCntl = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrZqCntl)));
  DEBUG((EFI_D_RELEASE, "DdrWrlvlCntl = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrWrlvlCntl)));
  DEBUG((EFI_D_RELEASE, "DdrSrCntr = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrSrCntr)));
  DEBUG((EFI_D_RELEASE, "DdrSdramRcw1 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrSdramRcw1)));
  DEBUG((EFI_D_RELEASE, "DdrSdramRcw2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrSdramRcw2)));
  DEBUG((EFI_D_RELEASE, "DdrWrlvlCntl2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrWrlvlCntl2)));
  DEBUG((EFI_D_RELEASE, "DdrWrlvlCntl3 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrWrlvlCntl3)));
  DEBUG((EFI_D_RELEASE, "DdrSdramRcw3 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrSdramRcw3)));
  DEBUG((EFI_D_RELEASE, "DdrSdramRcw4 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrSdramRcw4)));
  DEBUG((EFI_D_RELEASE, "DdrSdramRcw5 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrSdramRcw5)));
  DEBUG((EFI_D_RELEASE, "DdrSdramRcw6 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrSdramRcw6)));
  DEBUG((EFI_D_RELEASE, "SdramMode3 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode3)));
  DEBUG((EFI_D_RELEASE, "SdramMode4 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode4)));
  DEBUG((EFI_D_RELEASE, "SdramMode5 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode5)));
  DEBUG((EFI_D_RELEASE, "SdramMode6 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode6)));
  DEBUG((EFI_D_RELEASE, "SdramMode7 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode7)));
  DEBUG((EFI_D_RELEASE, "SdramMode8 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode8)));
  DEBUG((EFI_D_RELEASE, "SdramMode9 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode9)));
  DEBUG((EFI_D_RELEASE, "SdramMode10 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode10)));
  DEBUG((EFI_D_RELEASE, "SdramMode11 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode11)));
  DEBUG((EFI_D_RELEASE, "SdramMode12 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode12)));
  DEBUG((EFI_D_RELEASE, "SdramMode13 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode13)));
  DEBUG((EFI_D_RELEASE, "SdramMode14 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode14)));
  DEBUG((EFI_D_RELEASE, "SdramMode15 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode15)));
  DEBUG((EFI_D_RELEASE, "SdramMode16 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramMode16)));
  DEBUG((EFI_D_RELEASE, "TimingCfg8 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->TimingCfg8)));
  DEBUG((EFI_D_RELEASE, "SdramCfg3 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->SdramCfg3)));
  DEBUG((EFI_D_RELEASE, "DqMap0 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DqMap0)));
  DEBUG((EFI_D_RELEASE, "DqMap1 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DqMap1)));
  DEBUG((EFI_D_RELEASE, "DqMap2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DqMap2)));
  DEBUG((EFI_D_RELEASE, "DqMap3 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DqMap3)));
  DEBUG((EFI_D_RELEASE, "DdrDsr1 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrDsr1)));
  DEBUG((EFI_D_RELEASE, "DdrDsr2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrDsr2)));
  DEBUG((EFI_D_RELEASE, "DdrCdr1 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrCdr1)));
  DEBUG((EFI_D_RELEASE, "DdrCdr2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DdrCdr2)));
  DEBUG((EFI_D_RELEASE, "IpRev1 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->IpRev1)));
  DEBUG((EFI_D_RELEASE, "IpRev2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->IpRev2)));
  DEBUG((EFI_D_RELEASE, "Eor = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Eor)));
  DEBUG((EFI_D_RELEASE, "Mtcr = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtcr)));
  DEBUG((EFI_D_RELEASE, "Mtp1 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtp1)));
  DEBUG((EFI_D_RELEASE, "Mtp2 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtp2)));
  DEBUG((EFI_D_RELEASE, "Mtp3 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtp3)));
  DEBUG((EFI_D_RELEASE, "Mtp4 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtp4)));
  DEBUG((EFI_D_RELEASE, "Mtp5 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtp5)));
  DEBUG((EFI_D_RELEASE, "Mtp6 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtp6)));
  DEBUG((EFI_D_RELEASE, "Mtp7 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtp7)));
  DEBUG((EFI_D_RELEASE, "Mtp8 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtp8)));
  DEBUG((EFI_D_RELEASE, "Mtp9 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtp9)));
  DEBUG((EFI_D_RELEASE, "Mtp10 = 0x%x\n",MmioReadBe32((UINTN)&Ddr->Mtp10)));
  DEBUG((EFI_D_RELEASE, "DataErrInjectHi = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DataErrInjectHi)));
  DEBUG((EFI_D_RELEASE, "DataErrInjectLo = 0x%x\n",MmioReadBe32((UINTN)&Ddr->DataErrInjectLo)));
  DEBUG((EFI_D_RELEASE, "EccErrInject = 0x%x\n",MmioReadBe32((UINTN)&Ddr->EccErrInject)));
  DEBUG((EFI_D_RELEASE, "CaptureDataHi = 0x%x\n",MmioReadBe32((UINTN)&Ddr->CaptureDataHi)));
  DEBUG((EFI_D_RELEASE, "CaptureDataLo = 0x%x\n",MmioReadBe32((UINTN)&Ddr->CaptureDataLo)));
  DEBUG((EFI_D_RELEASE, "CaptureEcc = 0x%x\n",MmioReadBe32((UINTN)&Ddr->CaptureEcc)));
  DEBUG((EFI_D_RELEASE, "ErrDetect = 0x%x\n",MmioReadBe32((UINTN)&Ddr->ErrDetect)));
  DEBUG((EFI_D_RELEASE, "ErrDisable = 0x%x\n",MmioReadBe32((UINTN)&Ddr->ErrDisable)));
  DEBUG((EFI_D_RELEASE, "ErrIntEn = 0x%x\n",MmioReadBe32((UINTN)&Ddr->ErrIntEn)));
  DEBUG((EFI_D_RELEASE, "CaptureAttributes = 0x%x\n",MmioReadBe32((UINTN)&Ddr->CaptureAttributes)));
  DEBUG((EFI_D_RELEASE, "CaptureAddress = 0x%x\n",MmioReadBe32((UINTN)&Ddr->CaptureAddress)));
  DEBUG((EFI_D_RELEASE, "CaptureExtAddress = 0x%x\n",MmioReadBe32((UINTN)&Ddr->CaptureExtAddress)));
  DEBUG((EFI_D_RELEASE, "ErrSbe = 0x%x\n",MmioReadBe32((UINTN)&Ddr->ErrSbe)));
  DEBUG((EFI_D_ERROR,"\n"));
}

/**
  Function to initialize DDR
 **/
VOID
DramInit (
  )
{
  struct CcsrDdr *Ddr;
	UINT32 Count, Delay = DDRMC_DELAY;
  Ddr = (VOID *)CONFIG_SYS_FSL_DDR_ADDR;

  DdrcWrite((UINTN)&Ddr->SdramCfg, CONFIG_DDR_SDRAM_CFG);

  DdrcWrite((UINTN)&Ddr->Cs0Bnds, CONFIG_CS0_BNDS);
  DdrcWrite((UINTN)&Ddr->Cs0Config, CONFIG_CS0_CONFIG);

  DdrcWrite((UINTN)&Ddr->TimingCfg0, CONFIG_TIMING_CFG_0);
  DdrcWrite((UINTN)&Ddr->TimingCfg1, CONFIG_TIMING_CFG_1);
  DdrcWrite((UINTN)&Ddr->TimingCfg2, CONFIG_TIMING_CFG_2);
  DdrcWrite((UINTN)&Ddr->TimingCfg3, CONFIG_TIMING_CFG_3);
  DdrcWrite((UINTN)&Ddr->TimingCfg4, CONFIG_TIMING_CFG_4);
  DdrcWrite((UINTN)&Ddr->TimingCfg5, CONFIG_TIMING_CFG_5);
  DdrcWrite((UINTN)&Ddr->TimingCfg7, CONFIG_TIMING_CFG_7);
  DdrcWrite((UINTN)&Ddr->TimingCfg8, CONFIG_TIMING_CFG_8);

  DdrcWrite((UINTN)&Ddr->SdramCfg2, CONFIG_DDR_SDRAM_CFG_2);

  DdrcWrite((UINTN)&Ddr->SdramMode, CONFIG_DDR_SDRAM_MODE);
  DdrcWrite((UINTN)&Ddr->SdramMode2, 0);
  DdrcWrite((UINTN)&Ddr->SdramInterval, CONFIG_DDR_SDRAM_INTERVAL);

  DdrcWrite((UINTN)&Ddr->DdrWrlvlCntl, CONFIG_DDR_WRLVL_CNTL);
  DdrcWrite((UINTN)&Ddr->DdrWrlvlCntl2, CONFIG_DDR_WRLVL_CNTL_2);
  DdrcWrite((UINTN)&Ddr->DdrWrlvlCntl3, 0);

  DdrcWrite((UINTN)&Ddr->DdrCdr1, CONFIG_DDRCDR_1);
  DdrcWrite((UINTN)&Ddr->DdrCdr2, CONFIG_DDRCDR_2);

  DdrcWrite((UINTN)&Ddr->SdramClkCntl, CONFIG_DDR_SDRAM_CLK_CNTL);

  DdrcWrite((UINTN)&Ddr->DdrZqCntl, CONFIG_DDR_ZQ_CNTL);

  DdrcWrite((UINTN)&Ddr->SdramMode9, CONFIG_DDR_SDRAM_MODE_9);
  DdrcWrite((UINTN)&Ddr->SdramMode10, CONFIG_DDR_SDRAM_MODE_10);

  DdrcWrite((UINTN)&Ddr->Cs0Config2, 0);

  for(Count = 0; Count < Delay; Count++)
		;

  DdrcWrite((UINTN)&Ddr->SdramCfg, CONFIG_DDR_SDRAM_CFG
                     | CONFIG_DDR_SDRAM_CFG_MEM_EN);

  return;
}

