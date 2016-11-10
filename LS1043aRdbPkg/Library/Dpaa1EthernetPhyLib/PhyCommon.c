/** PhyCommon.c
  DPAA1 Ethernet PHY common services implementation

  Copyright (c) 2016, Freescale Semiconductor, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/Dpaa1DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <LS1043aRdb.h>

#include"Dpaa1EthernetPhyPrivate.h"

#define MDIO_CLOCK_DIVIDER  258
#define MDIO_PHY_DEV_ADDR   0x0

/**
   Initializes the given DPAA1 PHY MDIO bus

   @param[in] MdioBus   Pointer to MDIO bus object

   @retval EFI_SUCCESS, on success
   @retval error code, on failure

 **/
STATIC EFI_STATUS
Dpaa1PhyMdioBusInit (
  IN  DPAA1_PHY_MDIO_BUS *MdioBus
  )
{
  MEMAC_MDIO_BUS_REGS *CONST MdioBusRegs = MdioBus->IoRegs;

  UINT32 RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioStat);

  RegValue |= MDIO_STAT_CLKDIV(MDIO_CLOCK_DIVIDER) | MDIO_STAT_NEG;
  DPAA1_DEBUG_MSG(" MDIO Init : 0x%x : 0x%x \n", &MdioBusRegs->MdioStat, RegValue);
  MmioWriteBe32((UINTN)&MdioBusRegs->MdioStat, RegValue);

  return EFI_SUCCESS;
}

/**
   Initializes the MDIO buses used by DPAA1 mEMACs to reach their
   corresponding PHYs.

   @param[in] MdioBuses         Pointer to array of MDIO buses to initialize
   @param[in] MdioBusesCount    Number of entries in MdioBuses[]

   @retval EFI_SUCCESS, on success
   @retval error code, on failure

 **/
EFI_STATUS
Dpaa1PhyMdioBusesInit (
  IN  DPAA1_PHY_MDIO_BUS MdioBuses[],
  UINT8 MdioBusesCount
  )
{
  EFI_STATUS Status;
  UINT8 I;

  for (I = 0; I < MdioBusesCount; I ++) {
    Status = Dpaa1PhyMdioBusInit(&MdioBuses[I]);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

STATIC
VOID
Dpaa1PhyMdioBusWrite (
  IN DPAA1_PHY_MDIO_BUS *MdioBus,
  IN UINT8 PhyAddress,
  IN UINT8 MdioCtlDevAddr,
  IN UINT16 PhyRegNum,
  IN UINT16 Value
  )
{
  UINT32 RegValue;
  MEMAC_MDIO_BUS_REGS *CONST MdioBusRegs = MdioBus->IoRegs;
  DPAA1_DEBUG_MSG("MDIO bus WRITE for PHY addr 0x%x, dev addr 0x%x, "
                   "reg num 0x%x (MDIO stat reg: 0x%x)\n",
                   PhyAddress, MdioCtlDevAddr, PhyRegNum, Value);

  ASSERT(MdioBus->Signature == DPAA1_PHY_MDIO_BUS_SIGNATURE);

  RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioStat);
  RegValue |= MDIO_STAT_ENC;
  MmioWriteBe32((UINTN)&MdioBusRegs->MdioStat, RegValue);

  /*
   * Wait until the MDIO bus is not busy
   */
  do {
    RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioStat);
  } while (RegValue & MDIO_STAT_BSY);

  /*
   * Specify the target PHY and PHY control device:
   */
  RegValue = MDIO_CTL_PORT_ADDR(PhyAddress) |
             MDIO_CTL_DEV_ADDR(MdioCtlDevAddr);
  MmioWriteBe32((UINTN)&MdioBusRegs->MdioCtl, RegValue);

  /*
   * Specify the target PHY register:
   */
  MmioWriteBe32((UINTN)&MdioBusRegs->MdioAddr, PhyRegNum);

  /*
   * Wait until the MDIO bus is not busy:
   */
  do {
    RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioStat);
  } while (RegValue & MDIO_STAT_BSY);

  /*
   * Write the value to the PHY register
   */
  MmioWriteBe32((UINTN)&MdioBusRegs->MdioData, MDIO_DATA(Value));

  /*
   * Wait until the MDIO write is complete
   */
  do {
    RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioData);
  } while (RegValue & MDIO_DATA_BSY);
}

/**
   Writes a value to a PHY register

   @param[in] Dpaa1Phy          Pointer to PHY object
   @param[in] MdioCtlDevAddr    MDIO control device selector
   @param[in] PhyRegNum         Phy register index
   @param[in] Value             Value to write to the PHY register

   @retval EFI_SUCCESS, on success
   @retval error code, on failure

 **/
VOID
Dpaa1PhyRegisterWrite (
  IN DPAA1_PHY *Dpaa1Phy,
  IN UINT8 MdioCtlDevAddr,
  IN UINT16 PhyRegNum,
  IN UINT16 Value
  )
{
  ASSERT(Dpaa1Phy->Signature == DPAA1_PHY_SIGNATURE);
  ASSERT(MdioCtlDevAddr == MDIO_PHY_DEV_ADDR ||
         MdioCtlDevAddr == MDIO_CTL_DEV_PMAPMD ||
         MdioCtlDevAddr == MDIO_CTL_DEV_AUTO_NEGOTIATION);
  ASSERT(PhyRegNum == PHY_CONTROL_REG || PhyRegNum == PHY_STATUS_REG);

  // only XFI is supported
  ASSERT(Dpaa1Phy->PhyInterfaceType == PHY_INTERFACE_XFI);
  Dpaa1PhyMdioBusWrite(Dpaa1Phy->MdioBus,
                       Dpaa1Phy->PhyAddress,
                       MdioCtlDevAddr,
                       PhyRegNum,
                       Value);
}

STATIC
UINT16
Dpaa1PhyMdioBusRead (
  IN DPAA1_PHY_MDIO_BUS *MdioBus,
  IN UINT8 PhyAddress,
  IN UINT8 MdioCtlDevAddr,
  IN UINT16 PhyRegNum
  )
{
  UINT32 RegValue;
  MEMAC_MDIO_BUS_REGS *CONST MdioBusRegs = MdioBus->IoRegs;

  ASSERT(MdioBus->Signature == DPAA1_PHY_MDIO_BUS_SIGNATURE);

  RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioStat);
  RegValue |= MDIO_STAT_ENC;
  MmioWriteBe32((UINTN)&MdioBusRegs->MdioStat, RegValue);

  /*
   * Wait until the MDIO bus is not busy
   */
  do {
    RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioStat);
  } while (RegValue & MDIO_STAT_BSY);

  /*
   * Specify the target PHY and PHY control device:
   */
  RegValue = MDIO_CTL_PORT_ADDR(PhyAddress) |
             MDIO_CTL_DEV_ADDR(MdioCtlDevAddr);
  MmioWriteBe32((UINTN)&MdioBusRegs->MdioCtl, RegValue);

  /*
   * Specify the target PHY register:
   */
  MmioWriteBe32((UINTN)&MdioBusRegs->MdioAddr, PhyRegNum & 0xFFFF);

  /*
   * Wait until the MDIO bus is not busy:
   */
  do {
    RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioStat);
  } while (RegValue & MDIO_STAT_BSY);

  /*
   * Initiate the MDIO read:
   */
  RegValue = MDIO_CTL_PORT_ADDR(PhyAddress) |
             MDIO_CTL_DEV_ADDR(MdioCtlDevAddr) |
             MDIO_CTL_READ;
  MmioWriteBe32((UINTN)&MdioBusRegs->MdioCtl, RegValue);

  /*
   * Wait until the MDIO read is complete
   */
  do {
    RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioData);
  } while (RegValue & MDIO_DATA_BSY);

  RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioStat);

  if (RegValue & MDIO_STAT_RD_ER) {
    /*
     * If read failed, return -1:
     */
    DPAA1_ERROR_MSG("MDIO bus read failed for PHY addr 0x%x, dev addr 0x%x, "
                    "reg num 0x%x (MDIO stat reg: 0x%x)\n",
                    PhyAddress, MdioCtlDevAddr, PhyRegNum, RegValue);
    return (UINT16)(-1);
  }
  RegValue = MmioReadBe32((UINTN)&MdioBusRegs->MdioData);
  DPAA1_DEBUG_MSG("MDIO bus read for PHY addr 0x%x, dev addr 0x%x, "
                    "reg num 0x%x (MDIO stat reg: 0x%x)\n",
                    PhyAddress, MdioCtlDevAddr, PhyRegNum, RegValue);

  return (UINT16)RegValue;
}


/**
   Reads the value of a PHY register

   @param[in] Dpaa1Phy          Pointer to PHY object
   @param[in] MdioCtlDevAddr    MDIO control device selector
   @param[in] PhyRegNum         Phy register index

   @retval Value of the PHY register, on success
   @retval -1, on failure

 **/
UINT16
Dpaa1PhyRegisterRead (
  IN DPAA1_PHY *Dpaa1Phy,
  IN UINT8 MdioCtlDevAddr,
  IN UINT16 PhyRegNum
  )
{
  ASSERT(Dpaa1Phy->Signature == DPAA1_PHY_SIGNATURE);
  ASSERT(MdioCtlDevAddr == MDIO_PHY_DEV_ADDR ||
         MdioCtlDevAddr == MDIO_CTL_DEV_PMAPMD ||
         MdioCtlDevAddr == MDIO_CTL_DEV_AUTO_NEGOTIATION);
  ASSERT(PhyRegNum == PHY_CONTROL_REG || PhyRegNum == PHY_STATUS_REG);

  // only XFI is supported
  ASSERT(Dpaa1Phy->PhyInterfaceType == PHY_INTERFACE_XFI);
  return Dpaa1PhyMdioBusRead(Dpaa1Phy->MdioBus,
                             Dpaa1Phy->PhyAddress,
                             MdioCtlDevAddr,
                             PhyRegNum);
}


/**
   Resets the PHY for a given DPAA1 MEMAC

   @param[in] Memac Pointer to MEMAC

   @retval EFI_SUCCESS, on success
   @retval error code, on failure

 **/

STATIC
EFI_STATUS
Dpaa1PhyReset (
  IN  DPAA1_PHY *Dpaa1Phy
  )
{
  UINT16 PhyRegValue;
  UINT32 TimeoutMsCount = 500;

  if (Dpaa1Phy->Flags & PHY_BROKEN_RESET) {
    return EFI_SUCCESS;
  }

  DPAA1_DEBUG_MSG("****Resetting PHY (PHY address: 0x%x) ...\n",
                  Dpaa1Phy->PhyAddress);

  ASSERT(Dpaa1Phy->PhyInterfaceType == PHY_INTERFACE_XFI);

  PhyRegValue = Dpaa1PhyRegisterRead(Dpaa1Phy,
			MDIO_PHY_DEV_ADDR, PHY_CONTROL_REG);
  if (PhyRegValue == (UINT16)(-1)) {
    return EFI_DEVICE_ERROR;
  }

  PhyRegValue |= PHY_CONTROL_RESET;
  Dpaa1PhyRegisterWrite(Dpaa1Phy,
		MDIO_PHY_DEV_ADDR, PHY_CONTROL_REG, PhyRegValue);

  /*
   * Poll the control register for the reset bit to go to 0 (it is
   * auto-clearing). This should happen within 0.5 seconds per the
   * IEEE spec.
   */
  for ( ; ; ) {
    PhyRegValue = Dpaa1PhyRegisterRead(Dpaa1Phy,
			MDIO_PHY_DEV_ADDR, PHY_CONTROL_REG);
    if (PhyRegValue == (UINT16)(-1)) {
      return EFI_DEVICE_ERROR;
    }

    if ((PhyRegValue & PHY_CONTROL_RESET) == 0) {
      break;
    }

    if (TimeoutMsCount == 0) {
      DPAA1_ERROR_MSG("Timeout resetting PHY (PHY address: 0x%x)\n",
                      Dpaa1Phy->PhyAddress);
      return EFI_NOT_READY;
    }

    MicroSecondDelay(1000);
    TimeoutMsCount --;
  }

  return EFI_SUCCESS;
}

/**
   Configures the PHY for a given DPAA1 MEMAC

   @param[in] Memac Pointer to MEMAC

   @retval EFI_SUCCESS, on success
   @retval error code, on failure

 **/
STATIC
EFI_STATUS
Dpaa1PhyConfig (
  IN  DPAA1_PHY *Dpaa1Phy
  )
{
  DPAA1_DEBUG_MSG("Configuring PHY (PHY address: 0x%x) ...\n",
                  Dpaa1Phy->PhyAddress);

  /*
   * Support Aquantia PHY only right now
   */
  if (Dpaa1Phy->PhyInterfaceType == PHY_INTERFACE_XFI) {
    return AquantiaPhyConfig(Dpaa1Phy);
  } else {
    DPAA1_INFO_MSG("SGMII/QSGMII PHY configuration is not supported yet\n");
    return EFI_UNSUPPORTED;
  }  
}


/**
   Initializes the PHY for a given DPAA1 MEMAC

   @param[in] Memac Pointer to MEMAC

   @retval EFI_SUCCESS, on success
   @retval error code, on failure

 **/
EFI_STATUS
Dpaa1PhyInit (
  IN  DPAA1_PHY *Dpaa1Phy
  )
{
  EFI_STATUS Status;

  DPAA1_DEBUG_MSG("Initializing PHY (PHY type: %d, address: 0x%x) ...\n",
                    Dpaa1Phy->PhyInterfaceType, Dpaa1Phy->PhyAddress);
                    
  if (Dpaa1Phy->PhyInterfaceType == PHY_INTERFACE_XFI) {
    Dpaa1Phy->Flags = 0;

    Status = Dpaa1PhyReset(Dpaa1Phy);
    if (EFI_ERROR(Status)) {
      return Status;
    } 

    Status = Dpaa1PhyConfig(Dpaa1Phy);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  else {
    DPAA1_INFO_MSG("SGMII/QSGMII initialization is not supported yet\n");
  }
  return EFI_SUCCESS;
}

/**
   Start the PHY for a given DPAA1 MEMAC

   @param[in] Memac Pointer to MEMAC

   @retval EFI_SUCCESS, on success
   @retval error code, on failure

 **/
EFI_STATUS
Dpaa1PhyStartup (
  IN  DPAA1_PHY *Dpaa1Phy
  )
{
  DPAA1_DEBUG_MSG("Starting up PHY (PHY address: 0x%x) ...\n",
                 Dpaa1Phy->PhyAddress);

  /*
   * Support Aquantia PHY only right now
   */
  if (Dpaa1Phy->PhyInterfaceType == PHY_INTERFACE_XFI) {
    return AquantiaPhyStartup(Dpaa1Phy);
  } else {
    DPAA1_INFO_MSG("SGMII/QSGMII startup is not supported yet\n");
  }  
  return EFI_SUCCESS;
}

/**
   Stop the PHY for a given DPAA1 MEMAC

   @param[in] Memac Pointer to MEMAC

 **/
VOID
Dpaa1PhyShutdown (
  IN  DPAA1_PHY *Dpaa1Phy
  )
{
  /*
   * For the Aquantia PHY there is no shutdown
   */
}

/**
   Get status of PHY for a given DPAA1 MEMAC

   @param[in] Memac Pointer to MEMAC

   @retval 	TRUE if link is up
		FALSE if link is down 
 **/
BOOLEAN
Dpaa1PhyStatus (
  IN  DPAA1_PHY *Dpaa1Phy
  )
{
  /*
   * Support Aquantia PHY only right now
   */
  if (Dpaa1Phy->PhyInterfaceType == PHY_INTERFACE_XFI) {
    return AquantiaPhyStatus(Dpaa1Phy);
  } else {
    DPAA1_INFO_MSG("SGMII/QSGMII status function is not supported yet\n");
  }  
  return EFI_SUCCESS;
}


