/** LS1043aBoardMem.c
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

#include <LS1043aBoard.h>

#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS          20

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

  // DRAM
  VirtualMemoryTable[Index].PhysicalBase = PcdGet64(PcdSystemMemoryBase);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64(PcdSystemMemoryBase);
  VirtualMemoryTable[Index].Length       = PcdGet64(PcdSystemMemorySize);
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;

  // ROM1
  VirtualMemoryTable[++Index].PhysicalBase = ROMCP_BASE1_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = ROMCP_BASE1_ADDR;
  VirtualMemoryTable[Index].Length       = ROMCP_SIZE1;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  // ROM2
  VirtualMemoryTable[++Index].PhysicalBase = ROMCP_BASE2_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = ROMCP_BASE2_ADDR;
  VirtualMemoryTable[Index].Length       = ROMCP_SIZE2;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

   // GIC
  VirtualMemoryTable[++Index].PhysicalBase = GIC_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = GIC_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = GIC_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // GIC interrupt distributor
  VirtualMemoryTable[++Index].PhysicalBase = GICDIST_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = GICDIST_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = GICDIST_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

   // OCRAM1
  VirtualMemoryTable[++Index].PhysicalBase = OCRAM1_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = OCRAM1_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = OCRAM1_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

   // OCRAM2
  VirtualMemoryTable[++Index].PhysicalBase = OCRAM2_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = OCRAM2_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = OCRAM2_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = DUART_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = DUART_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = DUART_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = WDOG1_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = WDOG1_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = WDOG_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = WDOG2_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = WDOG2_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = WDOG_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = WDOG3_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = WDOG3_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = WDOG_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = WDOG4_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = WDOG4_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = WDOG_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = WDOG5_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = WDOG5_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = WDOG_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_SYS_FSL_DDR_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_SYS_FSL_DDR_ADDR;
  VirtualMemoryTable[Index].Length       = DDRC_MEMORY_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = I2C0_BASE_ADDRESS;
  VirtualMemoryTable[Index].VirtualBase  = I2C0_BASE_ADDRESS;
  VirtualMemoryTable[Index].Length       = I2C_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = IFC_MEM1_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = IFC_MEM1_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = IFC_MEM1_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = IFC_REG_BASE_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = IFC_REG_BASE_ADDR;
  VirtualMemoryTable[Index].Length       = IFC_REG_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;

  VirtualMemoryTable[++Index].PhysicalBase = CONFIG_SYS_FSL_DSPI_ADDR;
  VirtualMemoryTable[Index].VirtualBase  = CONFIG_SYS_FSL_DSPI_ADDR;
  VirtualMemoryTable[Index].Length       = DSPI_MEMORY_SIZE;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED;


  // End of Table
  VirtualMemoryTable[++Index].PhysicalBase = 0;
  VirtualMemoryTable[Index].VirtualBase  = 0;
  VirtualMemoryTable[Index].Length       = 0;
  VirtualMemoryTable[Index].Attributes   = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT((Index + 1) <= MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
}
