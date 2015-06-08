#/** LS1043A board package.
#
# Copyright (c) 2014, Freescale Ltd. All rights reserved.
# Author: Bhupesh Sharma <bhupesh.sharma@freescale.com>
#
#    This program and the accompanying materials
#    are licensed and made available under the terms and conditions of the BSD License
#    which accompanies this distribution. The full text of the license may be found at
#    http://opensource.org/licenses/bsd-license.php
#
#    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#**/

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = LS1043aRdbPkg
  PLATFORM_GUID                  = 60169ec4-d2b4-44f8-825e-f8684fd42e4f
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/LS1043aRdb
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = LS1043aRdbPkg/LS1043aRdbPkg.fdf

[LibraryClasses.common.SEC]
  ArmLib|ArmPkg/Library/ArmLib/AArch64/AArch64LibSec.inf

[LibraryClasses.common]
  ArmLib|ArmPkg/Library/ArmLib/AArch64/AArch64Lib.inf
  ArmCpuLib|ArmPkg/Drivers/ArmCpuLib/ArmCortexAEMv8Lib/ArmCortexAEMv8Lib.inf
  ArmSmcLib|ArmPkg/Library/ArmSmcLib/ArmSmcLib.inf

  ArmGenericTimerCounterLib|ArmPkg/Library/ArmGenericTimerPhyCounterLib/ArmGenericTimerPhyCounterLib.inf
  TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf

  LS1043aSocLib|LS1043aRdbPkg/Library/LS1043aSocLib/LS1043aSocLib.inf
  ArmTrustZoneLib|LS1043aRdbPkg/Library/ArmTrustZone/ArmTrustZone.inf
  ArmPlatformLib|LS1043aRdbPkg/Library/LS1043aRdbLib/LS1043aRdbLib.inf
  ArmPlatformStackLib|ArmPlatformPkg/Library/ArmPlatformStackLib/ArmPlatformStackLib.inf

  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf

  # ARM PL310 L2 Cache Driver
  L2X0CacheLib|ArmPlatformPkg/Drivers/PL310L2Cache/PL310L2CacheSec.inf
  # ARM PL301 Axi Driver
  PL301AxiLib|ArmPlatformPkg/Drivers/PL301Axi/PL301Axi.inf
  # ARM GIC400 General Interrupt Driver
  ArmGicLib|ArmPkg/Drivers/ArmGic/ArmGicLib.inf


!if $(TARGET) == RELEASE
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  UncachedMemoryAllocationLib|ArmPkg/Library/UncachedMemoryAllocationLib/UncachedMemoryAllocationLib.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  UncachedMemoryAllocationLib|ArmPkg/Library/UncachedMemoryAllocationLib/UncachedMemoryAllocationLib.inf

  # I2c Library
  I2cLib|LS1043aRdbPkg/Library/I2cLib/I2cLib.inf

  # Ddr Library
  DdrLib|LS1043aRdbPkg/Library/DdrLib/DdrLib.inf
!endif
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf

  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|ArmPkg/Library/BaseMemoryLibStm/BaseMemoryLibStm.inf

  EfiResetSystemLib|LS1043aRdbPkg/Library/ResetSystemLib/ResetSystemLib.inf

  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf

  EfiFileLib|EmbeddedPkg/Library/EfiFileLib/EfiFileLib.inf

  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf

  PeCoffExtraActionLib|ArmPkg/Library/DebugPeCoffExtraActionLib/DebugPeCoffExtraActionLib.inf

  CacheMaintenanceLib|ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLib.inf
  CpuExceptionHandlerLib|MdeModulePkg/Library/CpuExceptionHandlerLibNull/CpuExceptionHandlerLibNull.inf
  PrePiLib|EmbeddedPkg/Library/PrePiLib/PrePiLib.inf
  RealTimeClockLib|LS1043aRdbPkg/Library/RealTimeClockLib/RealTimeClockLib.inf
  SemihostLib|ArmPkg/Library/SemihostLib/SemihostLib.inf

  # UART Driver
  DUartPortLib|LS1043aRdbPkg/Drivers/DUart/DUart.inf
  SerialPortLib|LS1043aRdbPkg/Library/DUartPortLib/DUartPortLib.inf
  SerialPortExtLib|LS1043aRdbPkg/Library/DUartPortLib/DUartPortExtLib.inf

  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf

  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf

  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf

  NorFlashLib|LS1043aRdbPkg/Library/IfcNorFlashLib/IfcNorFlashLib.inf
  NandFlashLib|LS1043aRdbPkg/Library/FslIfcNandLib/FslIfcNandLib.inf
  FileSystemLib|LS1043aRdbPkg/Library/LS1043aFileSystemLib/LS1043aFileSystemLib.inf
  DspiFlashLib|LS1043aRdbPkg/Library/DspiFlashLib/DspiFlashLib.inf
  SdxcLib|LS1043aRdbPkg/Library/SdxcLib/SdxcLib.inf

#
# Assume everything is fixed at build
#
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf

  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf

  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf

  TimerLib|LS1043aRdbPkg/Library/TimerLib/TimerLib.inf

  ArmDisassemblerLib|ArmPkg/Library/ArmDisassemblerLib/ArmDisassemblerLib.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  DmaLib|ArmPkg/Library/ArmDmaLib/ArmDmaLib.inf

  BdsLib|ArmPkg/Library/BdsLib/BdsLib.inf
  FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf

  #ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  #ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
  #FileHandleLib|ShellPkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  #ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  #SortLib|ShellPkg/Library/UefiSortLib/UefiSortLib.inf
  #HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf

  #PathLib|ShellPkg/Library/BasePathLib/BasePathLib.inf


[LibraryClasses.common.SEC]
  ArmLib|ArmPkg/Library/ArmLib/AArch64/AArch64Lib.inf
  ArmPlatformGlobalVariableLib|ArmPlatformPkg/Library/ArmPlatformGlobalVariableLib/PrePi/PrePiArmPlatformGlobalVariableLib.inf

  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  ExtractGuidedSectionLib|EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
  LzmaDecompressLib|IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf

  # Temp work around for Movt relocation issue.
  #PeCoffLib|ArmPkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf

  HobLib|EmbeddedPkg/Library/PrePiHobLib/PrePiHobLib.inf
  PrePiHobListPointerLib|ArmPlatformPkg/Library/PrePiHobListPointerLib/PrePiHobListPointerLib.inf
  MemoryAllocationLib|EmbeddedPkg/Library/PrePiMemoryAllocationLib/PrePiMemoryAllocationLib.inf
  PerformanceLib|MdeModulePkg/Library/PeiPerformanceLib/PeiPerformanceLib.inf
  PlatformPeiLib|ArmPlatformPkg/PlatformPei/PlatformPeiLib.inf
  MemoryInitPeiLib|ArmPlatformPkg/MemoryInitPei/MemoryInitPeiLib.inf

  # 1/123 faster than Stm or Vstm version
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf

  # Uncomment to turn on GDB stub in SEC. 
  #DebugAgentLib|EmbeddedPkg/Library/GdbDebugAgent/GdbDebugAgent.inf

[LibraryClasses.common.PEI_CORE]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf

[LibraryClasses.common.DXE_CORE]
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/DxeReportStatusCodeLibFramework/DxeReportStatusCodeLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  PeCoffLib|EmbeddedPkg/Library/DxeHobPeCoffLib/DxeHobPeCoffLib.inf

  PerformanceLib|MdeModulePkg/Library/DxeCorePerformanceLib/DxeCorePerformanceLib.inf

[LibraryClasses.common.DXE_DRIVER]
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/DxeReportStatusCodeLibFramework/DxeReportStatusCodeLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  ArmPlatformGlobalVariableLib|ArmPlatformPkg/Library/ArmPlatformGlobalVariableLib/Dxe/DxeArmPlatformGlobalVariableLib.inf
  MemoryInitPeiLib|ArmPlatformPkg/MemoryInitPei/MemoryInitPeiLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/DxeReportStatusCodeLibFramework/DxeReportStatusCodeLib.inf
  UefiDecompressLib|IntelFrameworkModulePkg/Library/BaseUefiTianoCustomDecompressLib/BaseUefiTianoCustomDecompressLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/DxeReportStatusCodeLibFramework/DxeReportStatusCodeLib.inf
  UefiDecompressLib|IntelFrameworkModulePkg/Library/BaseUefiTianoCustomDecompressLib/BaseUefiTianoCustomDecompressLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/DxeReportStatusCodeLibFramework/DxeReportStatusCodeLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  PeCoffLib|EmbeddedPkg/Library/DxeHobPeCoffLib/DxeHobPeCoffLib.inf

[LibraryClasses.ARM]
  #
  # It is not possible to prevent the ARM compiler for generic intrinsic functions.
  # This library provides the instrinsic functions generate by a given compiler.
  # [LibraryClasses.ARM] and NULL mean link this library into all ARM images.
  #
  NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf

[BuildOptions]
  XCODE:*_*_ARM_PLATFORM_FLAGS == -arch armv7

  GCC:*_*_ARM_PLATFORM_FLAGS == -march=armv7-a

  RVCT:*_*_ARM_PLATFORM_FLAGS == --cpu cortex-a9

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsFeatureFlag.common]
  gEfiMdePkgTokenSpaceGuid.PcdComponentNameDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnosticsDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdComponentName2Disable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnostics2Disable|TRUE

  #
  # Control what commands are supported from the UI
  # Turn these on and off to add features or save size
  #
  gEmbeddedTokenSpaceGuid.PcdEmbeddedMacBoot|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedDirCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedHobCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedHwDebugCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPciDebugCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedIoEnable|FALSE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedScriptCmd|FALSE

  gEmbeddedTokenSpaceGuid.PcdCacheEnable|TRUE

  # Use the Vector Table location in CpuDxe. We will not copy the Vector Table at PcdCpuVectorBaseAddress
  gArmTokenSpaceGuid.PcdRelocateVectorTable|FALSE

  gEmbeddedTokenSpaceGuid.PcdPrePiProduceMemoryTypeInformationHob|TRUE
  gArmTokenSpaceGuid.PcdCpuDxeProduceDebugSupport|FALSE

  gEfiMdeModulePkgTokenSpaceGuid.PcdTurnOffUsbLegacySupport|TRUE

  ## If TRUE, Graphics Output Protocol will be installed on virtual handle created by ConsplitterDxe.
  #  It could be set FALSE to save size.
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|FALSE

[PcdsFixedAtBuild.common]
  gArmPlatformTokenSpaceGuid.PcdFirmwareVendor|"LS1043a Simulator"
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPrompt|"LS1043a"

  gArmPlatformTokenSpaceGuid.PcdCoreCount|1 # Only one core
  gArmPlatformTokenSpaceGuid.PcdCounterFrequency|12000000 #12Mhz

  gEmbeddedTokenSpaceGuid.PcdPrePiCpuMemorySize|32
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|0
  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdSpinLockTimeout|10000000
  gEfiMdePkgTokenSpaceGuid.PcdDebugClearMemoryValue|0xAF
  gEfiMdePkgTokenSpaceGuid.PcdPerformanceLibraryPropertyMask|1
  #gEfiMdePkgTokenSpaceGuid.PcdPerformanceLibraryPropertyMask|0 # turn off for now
  gEfiMdePkgTokenSpaceGuid.PcdPostCodePropertyMask|0
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|320

# DEBUG_ASSERT_ENABLED       0x01
# DEBUG_PRINT_ENABLED        0x02
# DEBUG_CODE_ENABLED         0x04
# CLEAR_MEMORY_ENABLED       0x08
# ASSERT_BREAKPOINT_ENABLED  0x10
# ASSERT_DEADLOOP_ENABLED    0x20
!if $(TARGET) == RELEASE
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x21
!else
  #gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2f
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2F
!endif

#  DEBUG_INIT      0x00000001  // Initialization
#  DEBUG_WARN      0x00000002  // Warnings
#  DEBUG_LOAD      0x00000004  // Load events
#  DEBUG_FS        0x00000008  // EFI File system
#  DEBUG_POOL      0x00000010  // Alloc & Free's
#  DEBUG_PAGE      0x00000020  // Alloc & Free's
#  DEBUG_INFO      0x00000040  // Verbose
#  DEBUG_DISPATCH  0x00000080  // PEI/DXE Dispatchers
#  DEBUG_VARIABLE  0x00000100  // Variable
#  DEBUG_BM        0x00000400  // Boot Manager
#  DEBUG_BLKIO     0x00001000  // BlkIo Driver
#  DEBUG_NET       0x00004000  // SNI Driver
#  DEBUG_UNDI      0x00010000  // UNDI Driver
#  DEBUG_LOADFILE  0x00020000  // UNDI Driver
#  DEBUG_EVENT     0x00080000  // Event messages
#  DEBUG_ERROR     0x80000000  // Error
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x8008054F #0x8000000F #Print almost everything

  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07

  gEmbeddedTokenSpaceGuid.PcdEmbeddedAutomaticBootCommand|""
  gEmbeddedTokenSpaceGuid.PcdEmbeddedDefaultTextColor|0x07
  gEmbeddedTokenSpaceGuid.PcdEmbeddedMemVariableStoreSize|0x10000

#
# Optional feature to help prevent EFI memory map fragments
# Turned on and off via: PcdPrePiProduceMemoryTypeInformationHob
# Values are in EFI Pages (4K). DXE Core will make sure that
# at least this much of each type of memory can be allocated
# from a single memory range. This way you only end up with
# maximum of two fragements for each type in the memory map
# (the memory used, and the free memory that was prereserved
# but not used).
#
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIReclaimMemory|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIMemoryNVS|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiReservedMemoryType|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|80
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|40
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|400
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesData|3000
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderCode|10
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderData|0

  #
  # ARM Primecells
  #

  ## Serial Terminal
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|0x21D0500
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|115200

  #
  # ARM General Interrupt Controller
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x1401000
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x1402000

  #
  # ARM L2x0 PCDs
  gArmTokenSpaceGuid.PcdL2x0ControllerBase|0x10900000

  #
  # LS1043a board Specific PCDs
  # (DRAM - Region 1 2GB)
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x80000000

  #
  # LS1043a Soc Specific PCDs
  #
  gArmPlatformTokenSpaceGuid.PcdCounterFrequencyReal|TRUE
  gArmPlatformTokenSpaceGuid.PcdCsuInitialize|TRUE
  gArmPlatformTokenSpaceGuid.PcdTzc380Initialize|TRUE
  gArmPlatformTokenSpaceGuid.PcdCci400Initialize|TRUE
  gArmPlatformTokenSpaceGuid.PcdClockInitialize|TRUE

!if $(TARGET) == DEBUG
  gArmTokenSpaceGuid.PcdDdrInitialize|TRUE
!endif

  # Size of the region used by UEFI in permanent memory (Reserved 16MB)
  gArmPlatformTokenSpaceGuid.PcdSystemMemoryUefiRegionSize|0x01000000

  # Size of the region reserved for fixed address allocations (Reserved 32MB)
  gArmTokenSpaceGuid.PcdArmLinuxKernelMaxOffset|0x08000000
  gArmTokenSpaceGuid.PcdArmLinuxFdtMaxOffset|0x08000000

  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x94A00000
  gArmTokenSpaceGuid.PcdCpuResetAddress|0x94A00000

  # Timer
  gEmbeddedTokenSpaceGuid.PcdTimerPeriod|100000 # expressed in 100ns units, 100,000 x 100 ns = 10,000,000 ns = 10 ms
  gArmPlatformTokenSpaceGuid.PcdTimerPeriodicInterruptNum|54 # Timer interrupt mapped to GIC IRQ
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPerformanceCounterPeriodInNanoseconds|77
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPerformanceCounterFrequencyInHz|13000000

  #
  # ARM Pcds
  #
  gArmTokenSpaceGuid.PcdArmUncachedMemoryMask|0x0000000040000000

  gArmPlatformTokenSpaceGuid.PcdDefaultBootDescription|L"Linux FIT from DDR"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDevicePath|L"MemoryMapped(0x0,0xB0000000,0xB0F59000)"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootInitrdPath|L"MemoryMapped(0x0,0xB1000000,0xB1D47EA0)"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootArgument|"console=ttyS1,115200 root=/dev/ram0 earlyprintk=uart8250-8bit,0x21c0600"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootType|3 # Linux Kernel with FDT support
  gArmPlatformTokenSpaceGuid.PcdPlatformBootTimeOut|10 #0 no timeout
  gArmPlatformTokenSpaceGuid.PcdFdtDevicePath|L"MemoryMapped(0x0, 0XA0EE0000, 0XA0EE4000)"
  gArmPlatformTokenSpaceGuid.PcdDefaultFitConfiguration|"config@1"

  # Use the serial console for both ConIn & ConOut
  gArmPlatformTokenSpaceGuid.PcdDefaultConOutPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenPcAnsi();"
  gArmPlatformTokenSpaceGuid.PcdDefaultConInPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenPcAnsi()"

  #
  # ARM OS Loader
  #
  # LS1043aRdb machine type (OMAP3_LS1043a = 1546) required for ARM Linux:
  gArmTokenSpaceGuid.PcdArmMachineType|1546

  #gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
  #gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  #gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|16000
#!ifdef $(NO_SHELL_PROFILES)
  #gEfiShellPkgTokenSpaceGuid.PcdShellProfileMask|0x00
#!endif #$(NO_SHELL_PROFILES)


################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]

  #
  # SEC
  #
  LS1043aRdbPkg/Library/LS1043aPrePi/PeiMPCore.inf

  #
  # DXE
  #
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
      NULL|EmbeddedPkg/Library/LzmaHobCustomDecompressLib/LzmaHobCustomDecompressLib.inf    
  }

  #
  # Architectural Protocols
  #
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/Variable/EmuRuntimeDxe/EmuVariableRuntimeDxe.inf
  EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf

  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
#  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
#  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  EmbeddedPkg/SerialDxe/SerialDxe.inf

  EmbeddedPkg/ResetRuntimeDxe/ResetRuntimeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  LS1043aRdbPkg/Drivers/LS1043aWatchDog/LS1043aWatchDogDxe.inf
  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf

  EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf

  #
  # PPA
  #
  LS1043aRdbPkg/Drivers/PpaInitDxe/PpaInitDxe.inf

  #
  # Semi-hosting filesystem
  #
  ArmPkg/Filesystem/SemihostFs/SemihostFs.inf

  #
  # FAT filesystem + GPT/MBR partitioning
  #
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  
  #
  # Dspi
  LS1043aRdbPkg/Drivers/DspiDxe/DspiDxe.inf

  #
  # Nor Flash
  LS1043aRdbPkg/Drivers/NorFlashDxe/NorFlashDxe.inf
  
  #
  # Nand Flash
  LS1043aRdbPkg/Drivers/NandFlashDxe/NandFlashDxe.inf
  
  #
  # File System
  LS1043aRdbPkg/Drivers/LS1043aFileSystemDxe/LS1043aFileSystemDxe.inf

  #
  # MMC/SD
  LS1043aRdbPkg/Drivers/MmcDxe/MmcDxe.inf

  #
  # I2C
  #
  LS1043aRdbPkg/Drivers/I2c/I2cDxe.inf

  #
  # Bds
  #
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  ArmPlatformPkg/Bds/Bds.inf

  #
  # Example Application
  #
  MdeModulePkg/Application/HelloWorld/HelloWorld.inf

#  ShellPkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
#  ShellPkg/Library/UefiShellLib/UefiShellLib.inf
#  ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
#  ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
#  ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
#
#  ShellPkg/Library/UefiDpLib/UefiDpLib.inf {
#    <LibraryClasses>
#      TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
#      PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
#      DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
#  }
#
#  ShellPkg/Application/Shell/Shell.inf {
#    <LibraryClasses>
#      NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
#      NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
#      NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
#!ifndef $(NO_SHELL_PROFILES)
#      NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
#      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
#      NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
##      NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
#!ifdef $(INCLUDE_DP)
#      NULL|ShellPkg/Library/UefiDpLib/UefiDpLib.inf
#!endif #$(INCLUDE_DP)
#!endif #$(NO_SHELL_PROFILES)
#
###
