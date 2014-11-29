/** @I2c.h
  Header defining the constant, base address amd function for I2C controller

  Copyright (c) 2014, Freescale Ltd. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __I2C_H___
#define __I2C_H__

#include <Uefi.h>
#include "LS1043aBoard.h"
#include <Library/TimerLib.h>

#define I2C0	0
#define I2C1	1
#define I2C2	2
#define I2C3	3

///
/// Define the I2C flags
///
/// I2C read operation when set
#define I2C_READ_FLAG               0x1
#define I2C_WRITE_FLAG              0x2

#define I2C_CLK	3
#define BUS_CLK	300000000

#define I2C_SPEED	0x186a0

#define EEPROM0_ADDRESS	0x51
#define EEPROM1_ADDRESS	0x52
#define EEPROM2_ADDRESS	0x53
#define EEPROM3_ADDRESS	0x54

#define I2CR_IDIS    (1 << 7)
#define I2CR_IEN     (0 << 7)
#define I2CR_MSSL    (1 << 5)
#define I2CR_MTX     (1 << 4)
#define I2CR_RSTA    (1 << 2)
#define I2CR_TX_NO_AK       (1 << 3)

#define ST_BUS_IDLE (0 | (I2SR_IBB << 8))
#define ST_BIF (I2SR_BIF | (I2SR_BIF << 8))
#define ST_BUS_BUSY (I2SR_IBB | (I2SR_IBB << 8))

#define I2SR_BIF_CLEAR      (1 << 1)
#define I2SR_IBB     (1 << 5)
#define I2SR_IAL     (1 << 4)
#define I2SR_BIF     (1 << 1)
#define I2SR_RX_NO_AK       (1 << 0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
  record defining i2c registers
**/

struct I2cRegs {
  UINT8     IAdr;
  UINT8     IFdr;
  UINT8     I2Cr;
  UINT8     I2Sr;
  UINT8     I2Dr;
};

/**
  Function to initialize i2c bus

  @param   I2c			parameter defining I2c controller no
  @param   Speed			I2C speed to be set

  @retval  EFI_INVALID_PARAMETER	Input parametr I2c was invalid
  @retval  EFI_SUCCESS		I2c was initialized successfully

**/
EFI_STATUS
EFIAPI
I2cInit (
  IN   INT16		I2c,
  IN   UINT32        Speed
  );

/**
  Function to read data usin i2c

  @param   Base			A pointer to the base address of I2c Controller
  @param   Chip			Address of slave device from where data to be read
  @param   Offset			Offset of slave memory
  @param   Alen			Address length of slave
  @param   Buffer			A pointer to the destination buffer for the data
  @param   Len			Length of data to be read

  @retval  EFI_NOT_READY		Arbitration lost
  @retval  EFI_TIMEOUT		Failed to initialize data transfer in predefined time
  @retval  EFI_NOT_FOUND		ACK was not recieved
  @retval  EFI_SUCCESS		Read was successful

**/
INT32
I2cRead (
  IN   VOID 		*Base,
  IN   UINT8 		Chip,
  IN   UINT32 	Offset,
  IN   UINT32 	Alen,
  OUT  UINT8 		*Buffer,
  IN   UINT32 	Len
  );

/**
  Function to set I2c bus speed

  @param   BaseAddress		Base address of I2c controller
  @param   Speed			value to be set

**/
EFI_STATUS
EFIAPI
BusI2cSetBusSpeed (
  IN   VOID		*BaseAddress,
  IN   UINT32		Speed
  );

/**
  Function to stop transaction on i2c bus

  @param   I2cRegs		Pointer to i2c registers

  @retval  EFI_NOT_READY    Arbitration was lost
  @retval  EFI_TIMEOUT      Timeout occured
  @retval  EFI_SUCCESS	Stop operation was successful

**/
EFI_STATUS
I2cStop (
  IN   struct I2cRegs       *I2cRegs
  );

/**
  Function to initiate data transfer on i2c bus

  @param   I2cRegs   Pointer to i2c base registers
  @param   Chip      Chip Address
  @param   Offset    Slave memory's offset
  @param   Alen      length of chip address

  @retval  EFI_NOT_READY           Arbitration lost
  @retval  EFI_TIMEOUT             Failed to initialize data transfer in predefined time
  @retval  EFI_NOT_FOUND           ACK was not recieved
  @retval  EFI_SUCCESS             Read was successful

**/
INT32
I2cInitTransfer (
  IN   struct I2cRegs       *I2cRegs,
  IN   UINT8                Chip,
  IN   UINT32               Offset,
  IN   INT32                Alen
  );


/**
  Function to write data using i2c bus

  @param   Base                    Pointer to the base address of I2c Controller
  @param   Chip                    Address of slave device where data to be written
  @param   Offset                  Offset of slave memory
  @param   Alen                    Address length of slave
  @param   Buffer                  A pointer to the source buffer for the data
  @param   Len                     Length of data to be write

  @retval  EFI_NOT_READY           Arbitration lost
  @retval  EFI_TIMEOUT             Failed to initialize data transfer in predefined time
  @retval  EFI_NOT_FOUND           ACK was not recieved
  @retval  EFI_SUCCESS             Read was successful

**/
INT32
I2cWrite (
  IN   VOID          *Base,
  IN   UINT8         Chip,
  IN   UINT32        Offset,
  IN   INT32         Alen,
  OUT  UINT8         *Buffer,
  IN   INT32         Len
  );


/**
  Function to reset I2c

**/

VOID
I2cReset (
  VOID
  );
#endif
