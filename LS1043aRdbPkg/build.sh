#!/bin/bash

# UEFI build script for LS1043A SoC from Freescale
#
# Copyright (c) 2014, Freescale Ltd. All rights reserved.
# Author: Bhupesh Sharma <bhupesh.sharma@freescale.com>
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

if [[ $2 == "NONXIP" ]]; then
	BootSuffix="NonXipBoot.dsc"
	echo "Compiling for NON-XIP boot"
	if [[ $3 != "clean" ]]; then
		echo "Enter the absolute path for mkimage utility: "
		echo "(Hint: Usually this is the path of the u-boot/tools directory) "
		read MKIMAGE_PATH
		echo "mkimage utility is available at this path: $MKIMAGE_PATH"
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
		echo "Error ! Incorrect 3rd argument to build script."
		print_usage_banner
		exit
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
build -p "$WORKSPACE/LS1043aRdbPkg/LS1043aRdbPkg$BootSuffix" -a $ARCH -t $TARGET_TOOLS -b $1
if [[ $2 == "NONXIP" ]]; then
$MKIMAGE_PATH/mkimage -n $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/ls1043ardb_rcw_nand.cfg -R $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/ls1043ardb_pbi.cfg -T pblimage -A arm -a 0x10000000 -d $WORKSPACE/Build/LS1043aRdb/$1_GCC48/FV/LS1043ARDBPI_EFI.fd $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/LS1043ARDBPI_EFI.pbl
echo "PBL image created at $WORKSPACE/LS1043aRdbPkg/Library/LS1043aPrePiOcram/LS1043ARDBPI_EFI.pbl"
fi
