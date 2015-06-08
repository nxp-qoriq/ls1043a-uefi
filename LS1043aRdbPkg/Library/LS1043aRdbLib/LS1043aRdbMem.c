/** LS1043aRdbMem.c
*
*  Copyright (c) 2013, Freescale Ltd. All rights reserved.
*  Author: Bhupesh Sharma <bhupesh.sharma@freescale.com>

*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>

#include <LS1043aRdb.h>

#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS          25

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                                    Virtual Memory mapping. This array must be ended by a zero-filled
                                    entry

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR** VirtualMemoryMap
  )
{
  ARM_MEMORY_REGION_ATTRIBUTES  CacheAttributes;
  UINTN                         Index = 0;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;

  ASSERT(VirtualMemoryMap != NULL);

  VirtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR*)AllocatePages(EFI_SIZE_TO_PAGES (sizeof(ARM_MEMORY_REGION_DESCRIPTOR) * MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS));
  if (VirtualMemoryTable == NULL) {
    return;
  }

  if (FeaturePcdGet(PcdCacheEnable) == TRUE) {
    CacheAttributes = DDR_ATTRIBUTES_CACHED;
  } else {
    CacheAttributes = DDR_ATTRIBUTES_UNCACHED;
  }

  // DRAM1 (Must be 1st entry)
  VirtualMemoryTable[Index].PhysicalBase = CONFIG_DRAM1_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_DRAM1_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_DRAM1_SIZE;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;

  // Secure BootROM
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_SECURE_BOOTROM_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_SECURE_BOOTROM_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_SECURE_BOOTROM_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // Extended BootROM
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_EXT_BOOTROM_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_EXT_BOOTROM_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_EXT_BOOTROM_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // CCSR Space
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_CCSR_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_CCSR_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_CCSR_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // OCRAM1 Space
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_OCRAM1_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_OCRAM1_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_OCRAM1_SIZE;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;
  
  // OCRAM2 Space
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_OCRAM2_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_OCRAM2_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_OCRAM2_SIZE;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;

  // FIXME: To Add QSPI

  // IFC region 1
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_IFC_REGION1_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_IFC_REGION1_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_IFC_REGION1_BASE_SIZE;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;

  // QMAN SWP
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_QMAN_SWP_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_QMAN_SWP_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_QMAN_SWP_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // BMAN SWP
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_BMAN_SWP_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_BMAN_SWP_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_BMAN_SWP_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // IFC region 2
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_IFC_REGION2_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_IFC_REGION2_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_IFC_REGION2_BASE_SIZE;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;

  // DRAM2
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_DRAM2_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_DRAM2_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_DRAM2_SIZE;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;

  // PCIe1
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_PCI_EXP1_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_PCI_EXP1_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_PCI_EXP1_BASE_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // PCIe2
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_PCI_EXP2_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_PCI_EXP2_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_PCI_EXP2_BASE_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // PCIe3
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_PCI_EXP3_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_PCI_EXP3_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_PCI_EXP3_BASE_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // DRAM3
  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_DRAM3_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_DRAM3_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = CONFIG_DRAM3_SIZE;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;
 
  // End of Table
  VirtualMemoryTable[++Index].PhysicalBase = 0;
  VirtualMemoryTable[Index].VirtualBase  = 0;
  VirtualMemoryTable[Index].Length       = 0;
  VirtualMemoryTable[Index].Attributes   = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT((Index + 1) <= MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
}
