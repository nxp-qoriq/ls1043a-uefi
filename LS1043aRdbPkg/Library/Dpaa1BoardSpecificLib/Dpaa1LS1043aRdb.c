/** Dpaa1LS1043aRdb.c
  DPAA1 definitions specific for the LS1043aRDB board

  Copyright (c) 2016, Freescale Semiconductor, Inc. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/Dpaa1BoardSpecificLib.h>
#include <Library/Dpaa1DebugLib.h>
#include <LS1043aRdb.h>

/**
 * Mapping of FMAN MEMACs to Ethernet PHYs
 */
typedef struct _MEMAC_PHY_MAPPING {
  /**
   * Pointer to the MDIO bus that connects a MEMAC to a PHY
   */
  DPAA1_PHY_MDIO_BUS *MdioBus;

  /**
   * PHY address of the associated PHY
   */
  UINT8 PhyAddress;
} MEMAC_PHY_MAPPING;

/**
 * PHY MDIO buses
 */
DPAA1_PHY_MDIO_BUS gDpaa1MdioBuses[] = {
  [0] = {
    .Signature = DPAA1_PHY_MDIO_BUS_SIGNATURE,
    .IoRegs = (MEMAC_MDIO_BUS_REGS *)DPAA1_FMAN_MDIO1_ADDR,
  },

  [1] = {
    .Signature = DPAA1_PHY_MDIO_BUS_SIGNATURE,
    .IoRegs = (MEMAC_MDIO_BUS_REGS *)DPAA1_FMAN_MDIO2_ADDR,
  },
};

C_ASSERT(ARRAY_SIZE(gDpaa1MdioBuses) == DPAA1_MDIO_BUSES_COUNT);

/**
 * Table of mappings of FMAN MEMACs to PHYs
 */
STATIC CONST MEMAC_PHY_MAPPING gMemacToPhyMap[] = {
  [FM1_DTSEC_1] = {
    .MdioBus = &gDpaa1MdioBuses[0],
    .PhyAddress = QSGMII_PORT1_PHY_ADDR,
  },

  [FM1_DTSEC_2] = {
    .MdioBus = &gDpaa1MdioBuses[0],
    .PhyAddress = QSGMII_PORT2_PHY_ADDR,
  },

  [FM1_DTSEC_3] = {
    .MdioBus = &gDpaa1MdioBuses[0],
    .PhyAddress = RGMII_PHY1_ADDR,
  },

  [FM1_DTSEC_4] = {
    .MdioBus = &gDpaa1MdioBuses[0],
    .PhyAddress = RGMII_PHY2_ADDR,
  },

  [FM1_DTSEC_5] = {
    .MdioBus = &gDpaa1MdioBuses[0],
    .PhyAddress = QSGMII_PORT3_PHY_ADDR,
  },

  [FM1_DTSEC_6] = {
    .MdioBus = &gDpaa1MdioBuses[0],
    .PhyAddress = QSGMII_PORT4_PHY_ADDR,
  },

  [FM1_DTSEC_9] = {
    .MdioBus = &gDpaa1MdioBuses[1],
    .PhyAddress = FM1_10GEC1_PHY_ADDR,
  },
};

C_ASSERT(ARRAY_SIZE(gMemacToPhyMap) <= NUM_FMAN_MEMACS);

/**
   Returns MAC ID and PHY type for given SerDes lane protocol

   @param[in] LaneProtocol	SerDes lane protocol representing a device
   
   @retval MemacId, NULL if lane protocol not found
   @retval PhyInterfaceType, NULL if lane protocol not found

 **/
VOID
GetMemacIdAndPhyType(SERDES_LANE_PROTOCOL LaneProtocol,
                     FMAN_MEMAC_ID *MemacId,
                     PHY_INTERFACE_TYPE *PhyInterfaceType)
{
  if (LaneProtocol >= SGMII_FM1_DTSEC1 && LaneProtocol <= SGMII_2500_FM1_DTSEC9) {
    switch(LaneProtocol){
      case XFI_FM1_MAC9:
        *MemacId = FM1_DTSEC_9; // XFI on lane A, MAC 9
        *PhyInterfaceType = PHY_INTERFACE_XFI;
        break;
      case SGMII_FM1_DTSEC1:
        *MemacId = FM1_DTSEC_1;
        *PhyInterfaceType = PHY_INTERFACE_SGMII;
        break;
      case SGMII_FM1_DTSEC2:
        *MemacId = FM1_DTSEC_2;
        *PhyInterfaceType = PHY_INTERFACE_SGMII;
        break;
      case SGMII_FM1_DTSEC3:
        *MemacId = FM1_DTSEC_3;
        *PhyInterfaceType = PHY_INTERFACE_SGMII;
        break;
      case SGMII_FM1_DTSEC4:
        *MemacId = FM1_DTSEC_4;
        *PhyInterfaceType = PHY_INTERFACE_SGMII;
        break;
      case SGMII_FM1_DTSEC5:
        *MemacId = FM1_DTSEC_5;
        *PhyInterfaceType = PHY_INTERFACE_SGMII;
        break;
      case SGMII_FM1_DTSEC6:
        *MemacId = FM1_DTSEC_6;
        *PhyInterfaceType = PHY_INTERFACE_SGMII;
        break;
      case QSGMII_FM1_A:
        *MemacId = INVALID_FMAN_MEMAC_ID;	//TODO
        *PhyInterfaceType = PHY_INTERFACE_QSGMII;
        break;
      case SGMII_2500_FM1_DTSEC2:
        *MemacId = FM1_DTSEC_2;
        *PhyInterfaceType = PHY_INTERFACE_SGMII_2500;
        break;
      case SGMII_2500_FM1_DTSEC9:
        *MemacId = FM1_DTSEC_9;
        *PhyInterfaceType = PHY_INTERFACE_SGMII_2500;
        break;
	default:
	 break;
    }
  }      
}
/**
   SerDes lane probe callback

   @param[in] LaneProtocol	SerDes lane protocol representing a device
   @param[in] Arg		Memacs list pointer to be populated

   @retval Lane index, if found
   @retval -1, if not found

 **/
VOID
Dpaa1DiscoverFmanMemac(SERDES_LANE_PROTOCOL LaneProtocol,
                       VOID *Arg)
{
  FMAN_MEMAC_ID MemacId = (FMAN_MEMAC_ID)INVALID_FMAN_MEMAC_ID;
  PHY_INTERFACE_TYPE PhyInterfaceType = (PHY_INTERFACE_TYPE)PHY_INTERFACE_NONE;

  GetMemacIdAndPhyType(LaneProtocol, &MemacId, &PhyInterfaceType);
  DPAA1_DEBUG_MSG("Dpaa1DiscoverFmanMemac lane protocol 0x%x" 
			"MemacId %d PhyInterfaceType %d \n", 
			LaneProtocol, MemacId, PhyInterfaceType);
  
  if (MemacId != (FMAN_MEMAC_ID)INVALID_FMAN_MEMAC_ID && 
	PhyInterfaceType != (PHY_INTERFACE_TYPE)PHY_INTERFACE_NONE) {
    FmanMemacInit(MemacId,
                  LaneProtocol,
                  PhyInterfaceType,
                  gMemacToPhyMap[MemacId].MdioBus,
                  gMemacToPhyMap[MemacId].PhyAddress,
                  Arg); 
  }  
}  
