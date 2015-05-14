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
	echo "	Arg 2 (optional): clean - To do a 'make clean' operation."
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

if [[ $1 != "RELEASE" ]]; then
	if [[ $1 != "DEBUG" ]]; then
		echo "Error ! Incorrect build target specified."
		print_usage_banner
		exit
	fi
fi

if [[ $2 == "clean" ]]; then
	echo "Cleaning up the build directory '../Build/LS1043aRdb/'.."
	rm -rf ../Build/LS1043aRdb/*
	exit
else
	if [[ $2 == "" ]]; then
		# Do nothing as argument 2 is optional.
		echo " "
	else
		echo "Error ! Incorrect 2nd argument to build script."
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
build -p $WORKSPACE/LS1043aRdbPkg/LS1043aRdbPkg.dsc -a $ARCH -t $TARGET_TOOLS -b $1
