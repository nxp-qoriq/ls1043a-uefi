/** @file
  DPAA library implementation

  Copyright (c) 2016, Freescale Semiconductor, Inc. All rights Reserved.

  This Program And The Accompanying Materials
  Are Licensed And Made Available Under The Terms And Conditions Of The BSD
  License Which Accompanies This Distribution. The Full Text Of The License
  May Be Found At
  http://Opensource.Org/Licenses/Bsd-License.Php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/Dpaa1DebugLib.h>
#include <Library/Dpaa1Lib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <LS1043aRdb.h>

#include "FrameManager.h"

/**
   Initializes the DPAA Frame Manager (FMan)

   @retval EFI_SUCCESS   on success
   @retval !EFI_SUCCESS  on failure.

 **/

VOID BmiRxPortInit (
  IN  BMI_RX_PORT *Port
  )
{
       /* set BMI to independent mode, Rx port disable */
       MmioWriteBe32((UINTN)&Port->FmanBmRcfg, FMBM_RCFG_IM);
       /* clear FOF in IM case */
       MmioWriteBe32((UINTN)&Port->FmanBmRim, 0);
       /* Rx frame next engine -RISC */
       MmioWriteBe32((UINTN)&Port->FmanBmRfne, NIA_ENG_RISC | NIA_RISC_AC_IM_RX);
       /* Rx command attribute - no order, MR[3] = 1 */
       MmioClearBitsBe32((UINTN)&Port->FmanBmRfca, FMBM_RFCA_ORDER | FMBM_RFCA_MR_MASK);
       MmioSetBitsBe32((UINTN)&Port->FmanBmRfca, FMBM_RFCA_MR(4));
       /* enable Rx statistic counters */
       MmioWriteBe32((UINTN)&Port->FmanBmRstc, FMBM_RSTC_EN);
       /* disable Rx performance counters */
       MmioWriteBe32((UINTN)&Port->FmanBmRpc, 0);
}

VOID BmiTxPortInit (
  IN  BMI_TX_PORT *Port
  )
{
       /* set BMI to independent mode, Tx port disable */
       MmioWriteBe32((UINTN)&Port->FmanBmTcfg, FMBM_TCFG_IM);
       /* Tx frame next engine -RISC */
       MmioWriteBe32((UINTN)&Port->FmanBmTfne, NIA_ENG_RISC | NIA_RISC_AC_IM_TX);
       MmioWriteBe32((UINTN)&Port->FmanBmTfene, NIA_ENG_RISC | NIA_RISC_AC_IM_TX);
       /* Tx command attribute - no order, MR[3] = 1 */
       MmioClearBitsBe32((UINTN)&Port->FmanBmTfca, FMBM_TFCA_ORDER | FMBM_TFCA_MR_MASK);
       MmioSetBitsBe32((UINTN)&Port->FmanBmTfca, FMBM_TFCA_MR(4));
       /* enable Tx statistic counters */
       MmioWriteBe32((UINTN)&Port->FmanBmTstc, FMBM_TSTC_EN);
       /* disable Tx performance counters */
       MmioWriteBe32((UINTN)&Port->FmanBmTpc, 0);
}

EFI_STATUS
DpaaFrameManagerInit (VOID)
{
  return FmanInit(0, (FMAN_CCSR *)FMAN_ADDR);
}

