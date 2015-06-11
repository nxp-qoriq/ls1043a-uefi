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
  FLASH_DEFINITION               = LS1043aRdbPkg/LS1043aRdbPkgXipBoot.fdf

!include LS1043aRdbPkg.dsc
