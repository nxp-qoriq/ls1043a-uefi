#!/bin/bash

# UEFI build script for LS1043A SoC from Freescale
#
# Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

print_usage_banner()
{
	echo "This shell script expects:"
	echo "	Arg 1 (mandatory): Build candidate (can be RELEASE or DEBUG). By
		default we build the RELEASE candidate."
	echo "	Arg 2 (mandatory): Boot source type (can be XIP, FATXIP or NONXIP. By default 
		we build the XIP candidate. FATXIP is used for FAT
		filesystem based XIP boot case, while NONXIP is used for
		NAND boot currently"
	echo "	Arg 3 (mandatory for NONXIP boot): absolute path of mkimage utility."
	echo "	Arg 3 (optional): clean - To do a 'make clean' operation."
}

# Actual stuff starts from here
echo ".........................................."
echo "Welcome to LS1043A UEFI Build environment"
echo ".........................................."

# Check for input arguments
if [[ $1 == "" ]]; then
	echo "Error ! No build target specified."
	print_usage_banner
	exit
fi
if [[ $2 == "" ]]; then
	echo "Error ! No boot source type specified."
	print_usage_banner
	exit
fi

# Check for input arguments
if [[ $1 != "RELEASE" ]]; then
	if [[ $1 != "DEBUG" ]]; then
		echo "Error ! Incorrect build target specified."
		print_usage_banner
		exit
	fi
fi

if [[ $2 == "NAND" || $2 == "SD" ]]; then
	BootSuffix="NonXipBoot.dsc"
	echo "Compiling for $1 boot"
	if [[ $3 == "" ]]; then
		echo "Error ! Incorrect 3rd argument to build script."
		print_usage_banner
		exit
	fi

	if [[ $3 != "clean" ]]; then
		echo "Using mkimage utility is available at this path: $3"
		echo "(Hint: Usually this is the absolute path of the u-boot/tools directory)"
	fi
else
	if [[ $2 == "XIP" ]]; then
			BootSuffix="XipBoot.dsc"
			echo "Compiling for XIP boot"
	else
		if [[ $2 == "FATXIP" ]]; then
			BootSuffix="FatXipBoot.dsc"
			echo "Compiling for FAT filesystem based XIP boot"
		else
			echo "Bad boot type argument. Use NONXIP, FATXIP or XIP"
			print_usage_banner
			exit
		fi
	fi
fi

if [[ $3 == "clean" ]]; then
	echo "Cleaning up the build directory '../Build/LS1043aRdb/'.."
	rm -rf ../Build/LS1043aRdb/*
	exit
else
	if [[ $3 == "" ]]; then
		# Do nothing as argument 3 is optional.
		echo " "
	else
		if [[ $2 != "NAND" && $2 != "SD" ]]; then
			echo "Error ! Incorrect 3rd argument to build script."
			print_usage_banner
			exit
		fi
	fi
fi

# Clean-up
set -e
shopt -s nocasematch

#
# Setup workspace now
#
echo Initializing workspace
cd ..

# Use the BaseTools in edk2
export EDK_TOOLS_PATH=`pwd`/BaseTools
source edksetup.sh BaseTools

# Global Defaults
ARCH=AARCH64
TARGET_TOOLS=GCC48

# Actual build command
if [[ $2 == "NAND" ]]; then
build -p "$WORKSPACE/LS1043aRdbPkg/LS1043aRdbPkg$BootSuffix" -a $ARCH -t $TARGET_TOOLS -b $1 -DNAND_BOOT_ENABLE=TRUE
elif [[ $2 == "SD" ]]; then
build -p "$WORKSPACE/LS1043aRdbPkg/LS1043aRdbPkg$BootSuffix" -a $ARCH -t $TARGET_TOOLS -b $1 -DSD_BOOT_ENABLE=TRUE
else
build -p "$WORKSPACE/LS1043aRdbPkg/LS1043aRdbPkg$BootSuffix" -a $ARCH -t $TARGET_TOOLS -b $1
fi

if [[ $2 == "XIP" || $2 == "FATXIP" ]]; then
 	cat $WORKSPACE/Build/LS1043aRdb/$1_GCC48/FV/LS1043ARDB_EFI.fd >> $WORKSPACE/Build/LS1043aRdb/$1_GCC48/FV/LS1043ARDBPI_EFI.fd 
 	mv $WORKSPACE/Build/LS1043aRdb/$1_GCC48/FV/LS1043ARDBPI_EFI.fd $WORKSPACE/Build/LS1043aRdb/$1_GCC48/FV/LS1043ARDB_EFI.fd 
fi

if [[ $2 == "NAND" ]]; then
$3/mkimage -n $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/ls1043ardb_rcw_nand.cfg -R $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/ls1043ardb_pbi.cfg -T pblimage -A arm -a 0x10000000 -d $WORKSPACE/Build/LS1043aRdb/$1_GCC48/FV/LS1043ARDBPI_EFI.fd $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/LS1043ARDBPI_NAND_EFI.pbl
echo "PBL image created at $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/LS1043ARDBPI_NAND_EFI.pbl"
elif [[ $2 == "SD" ]]; then
$3/mkimage -n $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/ls1043ardb_rcw_sd.cfg -R $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/ls1043ardb_pbi.cfg -T pblimage -A arm -a 0x10000000 -d $WORKSPACE/Build/LS1043aRdb/$1_GCC48/FV/LS1043ARDBPI_EFI.fd $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/LS1043ARDBPI_SD_EFI.pbl
echo "PBL image created at $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/LS1043ARDBPI_SD_EFI.pbl"
fi
