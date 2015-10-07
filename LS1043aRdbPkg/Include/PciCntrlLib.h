/** PciCntrlLib.c
 The Header file of the Pci Controller Driver 

 Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

 This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution. The full text of the license may be found 
 http://opensource.org/licenses/bsd-license.php

 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/ 

#ifndef _PCI_CNTRL_H_
#define _PCI_CNTRL_H_

#include <PciRootBridge.h>
#include <PciLib.h>
#include "LS1043aRdb.h"

#define CONFIG_SYS_SDRAM_BASE       0x80000000

#ifndef CONFIG_SYS_PCI_MEMORY_BUS
#define CONFIG_SYS_PCI_MEMORY_BUS CONFIG_SYS_SDRAM_BASE
#endif

#ifndef CONFIG_SYS_PCI_MEMORY_PHYS
#define CONFIG_SYS_PCI_MEMORY_PHYS CONFIG_SYS_SDRAM_BASE
#endif

#ifndef CONFIG_SYS_PCI_MEMORY_SIZE
#define CONFIG_SYS_PCI_MEMORY_SIZE (2 * 1024 * 1024 * 1024UL) /* 2G */
#endif

#define CONFIG_SYS_IMMR                         0x01000000
#define CONFIG_SYS_DCSRBAR                      0x20000000

#define CONFIG_SYS_FSL_SCFG_ADDR                (CONFIG_SYS_IMMR + 0x00570000)

/* iATU registers */
#define PCIE_ATU_VIEWPORT               0x900
#define PCIE_ATU_REGION_INBOUND         (0x1 << 31)
#define PCIE_ATU_REGION_OUTBOUND        (0x0 << 31)
#define PCIE_ATU_REGION_INDEX0          (0x0 << 0)
#define PCIE_ATU_REGION_INDEX1          (0x1 << 0)
#define PCIE_ATU_REGION_INDEX2          (0x2 << 0)
#define PCIE_ATU_REGION_INDEX3          (0x3 << 0)
#define PCIE_ATU_REGION_INDEX4          (0x4 << 0)
#define PCIE_ATU_CR1                    0x904
#define PCIE_ATU_TYPE_MEM               (0x0 << 0)
#define PCIE_ATU_TYPE_IO                (0x2 << 0)
#define PCIE_ATU_TYPE_CFG0              (0x4 << 0)
#define PCIE_ATU_TYPE_CFG1              (0x5 << 0)
#define PCIE_ATU_CR2                    0x908
#define PCIE_ATU_ENABLE                 (0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE        (0x1 << 30)
#define PCIE_ATU_LOWER_BASE             0x90C
#define PCIE_ATU_UPPER_BASE             0x910
#define PCIE_ATU_LIMIT                  0x914
#define PCIE_ATU_LOWER_TARGET           0x918
#define PCIE_ATU_BUS(x)                 (((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)                 (((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)                (((x) & 0x7) << 16)
#define PCIE_ATU_UPPER_TARGET           0x91C

/* LUT registers */
#define PCIE_LUT_BASE           0x80000
#define PCIE_LUT_DBG            0x7FC

/* DBI Read-Only Write Enable Register: Pg 1518 RM rev B*/
/* Set bit 0 of this register as 1.
 * Write to RO Registers Using DBI.
 * When you set this field to "1", then some RO and HwInit bits are writable
 * from the local application through the DBI.
 */
#define PCIE_DBI_RO_WR_EN       0x8bc

#define PCIE_LINK_CAP           0x7c
#define PCIE_LINK_SPEED_MASK    0xf
#define PCIE_LINK_STA           0x82

#define LTSSM_STATE_MASK        0x3f
#define LTSSM_PCIE_L0           0x11 /* L0 state */

#define PCIE_DBI_SIZE           0x100000 /* 1M */

#define PCI_CLASS_REVISION	0x08	/* High 24 bits are class, low 8
					   revision */
#define PCI_REVISION_ID		0x08	/* Revision ID */
#define PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE	0x0a	/* Device class */
#define PCI_CLASS_CODE		0x0b	/* Device class code */
#define PCI_CLASS_BRIDGE_PCI		0x0604

/**
 Macro that converts PCI Bus, PCI Device, PCI Function and PCI Register to an
 address that can be passed to the PCI Library functions.
 
 @param  Bus       PCI Bus number. Range 0..255.
 @param  Device    PCI Device number. Range 0..31.
 @param  Function  PCI Function number. Range 0..7.
 @param  Register  PCI Register number. Range 0..255 for PCI. Range 0..4095
 for PCI Express.
 
 @return The encoded PCI address.
 **/
#define PCI_LIB_ADDRESS(Bus,Device,Function,Register)   \
  (((Register) & 0xfff) | (((Function) & 0x07) << 12) | (((Device) & 0x1f) << 15) | (((Bus) & 0xff) << 20))

#define PCI_BUS_DEV(Bus,Device,Function)   \
  ((((Function) & 0x07) << 16) | (((Device) & 0x1f) << 19) | (((Bus) & 0xff) << 24))

VOID
PciSetRegion (
	IN struct PciRegion *Reg,
	IN PciAddrT BusStart,
	IN PhysAddrT PhysStart,
	IN PciSizeT Size,
	IN UINTN Flags
);

VOID
SetLSPcieInfo (
	IN struct LsPcieInfo *PcieInfo,
	IN UINTN Num
);


INTN
PcieLinkState (
	IN struct LsPcie *Pcie
);


INTN
PcieLinkUp (
	IN struct LsPcie *pcie
);


VOID
PcieCfg0SetBusdev (
	IN struct LsPcie *Pcie,
	IN UINT32 BusDev
);

VOID
PcieCfg1SetBusdev (
	IN struct LsPcie *Pcie,
	IN UINT32 BusDev
);


VOID
PcieIatuOutboundSet (
	IN struct LsPcie *Pcie,
	IN UINT32 Idx,
	IN UINT32 Type,
	IN UINT64 Phys,
	IN UINT64 BusAddr,
	IN UINT64 Size
);


EFI_STATUS
PciSkipDev (
	IN PCI_ROOT_BRIDGE_INSTANCE      *PrivateData,
	IN UINT32 Dev
);


EFI_STATUS
PcieAddrValid (
	IN PCI_ROOT_BRIDGE_INSTANCE      *PrivateData,
	IN UINT32 Dev
);


EFI_STATUS
PcieReadConfig (
	IN PCI_ROOT_BRIDGE_INSTANCE      *PrivateData,
	IN UINT32 Dev,
	IN INT32 Where,
	IN OUT UINT32 *Val
);

EFI_STATUS
PcieReadConfigByte (
  IN PCI_ROOT_BRIDGE_INSTANCE      *PrivateData,
  IN UINT32 Dev,
  IN INT32 Offset,
  IN OUT UINT8 *Val
);

EFI_STATUS
PcieReadConfigWord (
  IN PCI_ROOT_BRIDGE_INSTANCE      *PrivateData,
  IN UINT32 Dev,
  IN INT32 Offset,
  IN OUT UINT16 *Val
);

EFI_STATUS
PcieWriteConfigByte (
  IN PCI_ROOT_BRIDGE_INSTANCE      *PrivateData,
  IN UINT32 Dev,
  IN INT32 Offset,
  OUT UINT8 Val
);

EFI_STATUS
PcieWriteConfigWord (
  IN PCI_ROOT_BRIDGE_INSTANCE      *PrivateData,
  IN UINT32 Dev,
  IN INT32 Offset,
  OUT UINT16 Val
);

EFI_STATUS
PcieWriteConfig (
	IN PCI_ROOT_BRIDGE_INSTANCE      *PrivateData,
	IN UINT32 Dev,
	IN INT32 Where,
	IN UINT32 Val
);

VOID
PcieSetupAtu (
	IN struct LsPcie *Pcie,
	IN struct LsPcieInfo *Info
);

/*
VOID
PciAddRootBridge (
  IN PCI_ROOT_BRIDGE_INSTANCE      *PrivateData
);
*/
VOID
PcieSetupCntrl (
	IN PCI_ROOT_BRIDGE_INSTANCE      *PrivateData,
	IN struct LsPcie *Pcie,
	IN struct LsPcieInfo *Info
);

EFI_STATUS
RootBridgeIoMemRW (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     BOOLEAN                                Write,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
);


EFI_STATUS
RootBridgeIoIoRW (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     BOOLEAN                                Write,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
);


EFI_STATUS
RootBridgeIoPciRW (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN BOOLEAN                                Write,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN UINT64                                 Address,
  IN UINTN                                  Count,
  IN OUT VOID                               *Buffer
);


/**

  Initialize and create Pci Root Bridges for Board

  @param PrvateData       Driver instance of this Root Bridge IO protocol
  @param Protocol         Point to protocol instance
  @param Info 	          Point to Info field of this driver instance
  @param HostBridgeHandle Handle of host bridge
  @param Attri            Attribute of host bridge
  @param HostNo      	  HostNo for this Host Bridge
  @param Busno      	  Bus Number for the Host Bridge

  @retval EFI_SUCCESS Success to initialize the Pci Root Bridge.

**/

EFI_STATUS
PciRbInitialize (
  IN PCI_ROOT_BRIDGE_INSTANCE          *PrivateData,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL    *Protocol,
  IN struct LsPcieInfo     		*Info,
  IN EFI_HANDLE                         HostBridgeHandle,
  IN UINT64                             Attri,
  IN INTN				HostNo,
  IN INTN				Busno
);

#endif
