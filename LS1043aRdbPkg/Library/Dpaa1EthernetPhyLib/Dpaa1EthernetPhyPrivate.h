/** Dpaa1EthernetPhyPrivate.h
  DPAA1 Ethernet PHY private common declarations

  Copyright (c) 2016, Freescale Semiconductor, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __DPAA1_ETHERNET_PHY_PRIVATE_H__
#define __DPAA1_ETHERNET_PHY_PRIVATE_H__

#include <Library/Dpaa1EthernetPhyLib.h>
#include <Uefi.h>

/*
 * MDIO control device addresses
 */

/**
 * Physical Medium Attachment/Physical Medium Dependent
 */
#define MDIO_CTL_DEV_PMAPMD             1

/**
 * Auto-Negotiation
 */
#define MDIO_CTL_DEV_AUTO_NEGOTIATION   7

/*
 * PHY registers
 */

/**
 * PHY control register
 */
#define PHY_CONTROL_REG	0x00

/*
 * Fields of the PHY control register
 */
#define PHY_CONTROL_RESET	0x8000

/**
 * PHY status register
 */
#define PHY_STATUS_REG	0x01

/*
 * Fields of the PHY status register
 */
#define PHY_STATUS_LINK_STATUS		      0x0004
#define PHY_STATUS_AUTO_NEGOTIATION_COMPLETE    0x0020

/**
 * PHY autonegotiation timeout (milliseconds)
 */
#define PHY_AUTO_NEGOTIATION_TIMEOUT	5000

VOID
Dpaa1PhyRegisterWrite (
  IN DPAA1_PHY *Dpaa1Phy,
  IN UINT8 MdioCtlDevAddr,
  IN UINT16 PhyRegNum,
  IN UINT16 Value
  );

UINT16
Dpaa1PhyRegisterRead (
  IN DPAA1_PHY *Dpaa1Phy,
  IN UINT8 MdioCtlDevAddr,
  IN UINT16 PhyRegNum
  );

EFI_STATUS
AquantiaPhyConfig(DPAA1_PHY *Dpaa1Phy);

EFI_STATUS
AquantiaPhyStartup(DPAA1_PHY *Dpaa1Phy);

BOOLEAN
AquantiaPhyStatus (
  IN  DPAA1_PHY *Dpaa1Phy
  );

#endif /* __DPAA1_ETHERNET_PHY_PRIVATE_H__ */
