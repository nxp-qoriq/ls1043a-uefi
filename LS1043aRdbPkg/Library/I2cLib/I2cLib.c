/** I2cLib.c
  I2c Library containing functions for read, write, initialize, set speed etc

  Copyright (c) 2014, Freescale Ltd. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <I2c.h>
#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseMemoryLib/MemLibInternals.h>
#include <Library/BaseLib.h>

UINT16 I2cClkDiv[60][2] = {
  { 20,  0x00 }, { 22, 0x01 },  { 24, 0x02 },  { 26, 0x03 },
  { 28,  0x04 }, { 30,  0x05 }, { 32,  0x09 }, { 34, 0x06 },
  { 36,  0x0A }, { 40, 0x07 },  { 44, 0x0C },  { 48, 0x0D },
  { 52,  0x43 }, { 56,  0x0E }, { 60, 0x45 },  { 64, 0x12 },
  { 68,  0x0F }, { 72,  0x13 }, { 80,  0x14 }, { 88,  0x15 },
  { 96,  0x19 }, { 104, 0x16 }, { 112, 0x1A }, { 128, 0x17 },
  { 136, 0x4F }, { 144, 0x1C }, { 160, 0x1D }, { 176, 0x55 },
  { 192, 0x1E }, { 208, 0x56 }, { 224, 0x22 }, { 228, 0x24 },
  { 240, 0x1F }, { 256, 0x23 }, { 288, 0x5C }, { 320, 0x25 },
  { 384, 0x26 }, { 448, 0x2A }, { 480, 0x27 }, { 512, 0x2B },
  { 576, 0x2C }, { 640, 0x2D }, { 768, 0x31 }, { 896, 0x32 },
  { 960, 0x2F }, { 1024, 0x33 }, { 1152, 0x34 },{ 1280, 0x35 },
  { 1536, 0x36 }, { 1792, 0x3A }, { 1920, 0x37 }, { 2048, 0x3B },
  { 2304, 0x3C }, { 2560, 0x3D }, { 3072, 0x3E }, { 3584, 0x7A },
  { 3840, 0x3F }, { 4096, 0x7B }, { 5120, 0x7D }, { 6144, 0x7E },
};


/**
  Calculate and set proper clock divider

  @param   Rate	clock rate

  @retval  ClkDiv	Value used to get frequency divider value

**/
UINT8
I2cGetClk(
  IN   UINT32		Rate
  )
{
  UINTN I2cClkRate;
  UINT32 Div;
  UINT8 ClkDiv;
  struct SysInfo SocSysInfo;

  /* Divider value calculation */
  GetSysInfo(&SocSysInfo);
  I2cClkRate = SocSysInfo.FreqSystemBus;
  Div = (I2cClkRate + Rate - 1) / Rate;
  if (Div < I2cClkDiv[0][0])
    ClkDiv = 0;
  else if (Div > I2cClkDiv[ARRAY_SIZE(I2cClkDiv) - 1][0])
    ClkDiv = ARRAY_SIZE(I2cClkDiv) - 1;
  else
    for (ClkDiv = 0; I2cClkDiv[ClkDiv][0] < Div; ClkDiv++);

  return ClkDiv;
}


/**
  Function to reset I2C module

**/

VOID
I2cReset (
  VOID
  )
{
  struct I2cRegs *I2cRegs = (VOID *)I2C0_BASE_ADDRESS;

  /** Reset module */
  MmioWrite8((UINTN)&I2cRegs->I2Cr, I2CR_IDIS);
  MmioWrite8((UINTN)&I2cRegs->I2Sr, 0);
}

/**
  Function to set I2c bus speed

  @param   BaseAddress             Base address of I2c controller
  @param   Speed                   value to be set

**/
EFI_STATUS
EFIAPI
BusI2cSetBusSpeed (
  IN   VOID 		*BaseAddress,
  IN   UINT32 	Speed
  )
{
  struct I2cRegs *I2cRegs = BaseAddress;
  UINT8 ClkIdx = I2cGetClk(Speed);
  UINT8 Idx = I2cClkDiv[ClkIdx][1];

  /** Store divider value */
  MmioWrite8((UINTN)&I2cRegs->IFdr, Idx);
  I2cReset();

  return EFI_SUCCESS;
}


/**
  Function used to check if i2c is in mentioned state or not

  @param   I2cRegs		Pointer to I2C registers
  @param   State		i2c state need to be checked

  @retval  EFI_NOT_READY	Arbitration was lost
  @retval  EFI_TIMEOUT	Timeout occured
  @retval  Sr			Value of state register

**/
EFI_STATUS
WaitForSrState (
  IN   struct I2cRegs 	*I2cRegs,
  IN   UINT32 		State
  )
{
  UINT8 Sr;
  UINT64 Cnt = 0;

  for (Cnt = 0; Cnt < 50; Cnt++) {
    Sr = MmioRead8((UINTN)&I2cRegs->I2Sr);
    if (Sr & I2SR_IAL) {
       MmioWrite8((UINTN)&I2cRegs->I2Sr, Sr | I2SR_IAL);
  	return EFI_NOT_READY;
    }
    if ((Sr & (State >> 8)) == (UINT8)State)
      	return Sr;

    //WatchdogReset();
    MicroSecondDelay(300);
  }
  return EFI_TIMEOUT;
}


/**
  Function to transfer byte on i2c

  @param   I2cRegs		Pointer to i2c registers
  @param   Byte		Byte to be transferred on i2c bus

  @retval  EFI_NOT_READY	Arbitration was lost
  @retval  EFI_TIMEOUT	Timeout occured
  @retval  EFI_NOT_FOUND	ACK was not recieved
  @retval  EFI_SUCCESS	Data transfer was succesful

**/
EFI_STATUS
TxByte (
  IN   struct I2cRegs 	*I2cRegs,
  IN   UINT8 			Byte
  )
{
  EFI_STATUS Ret;

  MmioWrite8((UINTN)&I2cRegs->I2Sr, I2SR_IIF_CLEAR);
  MmioWrite8((UINTN)&I2cRegs->I2Dr, Byte);

  Ret = WaitForSrState(I2cRegs, ST_IIF);
  if ((Ret == EFI_TIMEOUT) || (Ret == EFI_NOT_READY))
    return Ret;

  if (Ret & I2SR_RX_NO_AK) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}


/**
  Function to stop transaction on i2c bus

  @param   I2cRegs          Pointer to i2c registers

  @retval  EFI_NOT_READY    Arbitration was lost
  @retval  EFI_TIMEOUT      Timeout occured
  @retval  EFI_SUCCESS      Stop operation was successful

**/
EFI_STATUS
I2cStop (
  IN   struct I2cRegs 	*I2cRegs
  )
{
  INT32 Ret;
  UINT32 Temp = MmioRead8((UINTN)&I2cRegs->I2Cr);

  Temp &= ~(I2CR_MSTA | I2CR_MTX);
  MmioWrite8((UINTN)&I2cRegs->I2Cr, Temp);
  Ret = WaitForSrState(I2cRegs, ST_BUS_IDLE);
  if (Ret < 0)
    return Ret;
  else
    return EFI_SUCCESS;
}


/**
  Function to send start signal, Chip Address and
  memory offset

  @param   I2cRegs	Pointer to i2c base registers
  @param   Chip	Chip Address
  @param   Offset	Slave memory's offset
  @param   Alen	length of chip address

  @retval  EFI_NOT_READY           Arbitration lost
  @retval  EFI_TIMEOUT             Failed to initialize data transfer in predefined time
  @retval  EFI_NOT_FOUND           ACK was not recieved
  @retval  EFI_SUCCESS             Read was successful

**/
EFI_STATUS
I2cInitTransfer_ (
  IN  	struct I2cRegs	*I2cRegs,
  IN	UINT8			Chip,
  IN	UINT32			Offset,
  IN	INT32			Alen
  )
{
  UINT32 Temp;
  EFI_STATUS Ret;

  /** Enable I2C controller */
  if (MmioRead8((UINTN)&I2cRegs->I2Cr) & I2CR_IDIS)
    MmioWrite8((UINTN)&I2cRegs->I2Cr, I2CR_IEN);

  if (MmioRead8((UINTN)&I2cRegs->IAdr) == (Chip << 1))
    MmioWrite8((UINTN)&I2cRegs->IAdr, (Chip << 1) ^ 2);

  MmioWrite8((UINTN)&I2cRegs->I2Sr, I2SR_IIF_CLEAR);
  Ret = WaitForSrState(I2cRegs, ST_BUS_IDLE);
  if ((Ret == EFI_TIMEOUT) || (Ret == EFI_NOT_READY))
    return Ret;

  /** Start I2C transaction */
  Temp = MmioRead8((UINTN)&I2cRegs->I2Cr);
  /** set to master mode */
  Temp |= I2CR_MSTA;
  MmioWrite8((UINTN)&I2cRegs->I2Cr, Temp);

  Ret = WaitForSrState(I2cRegs, ST_BUS_BUSY);
  if ((Ret == EFI_TIMEOUT) || (Ret == EFI_NOT_READY))
    return Ret;

  Temp |= I2CR_MTX | I2CR_TX_NO_AK;
  MmioWrite8((UINTN)&I2cRegs->I2Cr, Temp);

  /** write slave Address */
  Ret = TxByte(I2cRegs, Chip << 1);
  if (Ret != EFI_SUCCESS)
    return Ret;

  if (Alen >= 0) {
    while (Alen--) {
      Ret = TxByte(I2cRegs, (Offset >> (Alen * 8)) & 0xff);
      if (Ret != EFI_SUCCESS)
        return Ret;
    }
  }
  return EFI_SUCCESS;
}


/**
  Function to check if i2c bud is idle

  @param   Base		Pointer to base address of I2c controller

  @retval  EFI_SUCCESS

**/
INT32
I2cIdleBus (
  IN   VOID 	*Base
  )
{
  return EFI_SUCCESS;
}


/**
  Function to initiate data transfer on i2c bus

  @param   I2cRegs	Pointer to i2c base registers
  @param   Chip	Chip Address
  @param   Offset	Slave memory's offset
  @param   Alen	length of chip address

  @retval  EFI_NOT_READY           Arbitration lost
  @retval  EFI_TIMEOUT             Failed to initialize data transfer in predefined time
  @retval  EFI_NOT_FOUND           ACK was not recieved
  @retval  EFI_SUCCESS             Read was successful
 
**/
EFI_STATUS
I2cInitTransfer (
  IN   struct I2cRegs 	*I2cRegs,
  IN   UINT8 			Chip,
  IN   UINT32 		Offset,
  IN   INT32 			Alen
  )
{
  INT32 Retry;
  EFI_STATUS Ret;

  for (Retry = 0; Retry < 3; Retry++) {
    Ret = I2cInitTransfer_(I2cRegs, Chip, Offset, Alen);
    if (Ret == EFI_SUCCESS)
      return EFI_SUCCESS;

    I2cStop(I2cRegs);

    if (EFI_NOT_FOUND == Ret) {
      return Ret;
    }

    /** Disable controller */
    if (Ret != EFI_NOT_READY)
      MmioWrite8((UINTN)&I2cRegs->I2Cr, I2CR_IDIS);

    if (I2cIdleBus(I2cRegs) < 0)
      break;
  }
  return Ret;
}


/**
  Function to read data using i2c bus

  @param   Base                    A pointer to the base address of I2c Controller
  @param   Chip                    Address of slave device from where data to be read
  @param   Offset                  Offset of slave memory
  @param   Alen                    Address length of slave
  @param   Buffer                  A pointer to the destination buffer for the data
  @param   Len                     Length of data to be read

  @retval  EFI_NOT_READY           Arbitration lost
  @retval  EFI_TIMEOUT             Failed to initialize data transfer in predefined time
  @retval  EFI_NOT_FOUND           ACK was not recieved
  @retval  EFI_SUCCESS             Read was successful

**/
EFI_STATUS
I2cRead (
  IN   VOID 		*Base,
  IN   UINT8 		Chip,
  IN   UINT32 	Offset,
  IN   UINT32 	Alen,
  IN   UINT8 		*Buffer,
  IN   UINT32 	Len
  )
{
  EFI_STATUS Ret;
  UINT32 Temp;
  INT32 i;
  struct I2cRegs *I2cRegs = (VOID *)Base;
  Ret = I2cInitTransfer(I2cRegs, Chip, Offset, Alen);
  if (Ret != EFI_SUCCESS)
    return Ret;

  Temp = MmioRead8((UINTN)&I2cRegs->I2Cr);
  Temp |= I2CR_RSTA;
  MmioWrite8((UINTN)&I2cRegs->I2Cr, Temp);

  Ret = TxByte(I2cRegs, (Chip << 1) | 1);
  if (Ret != EFI_SUCCESS) {
    I2cStop(I2cRegs);
    return Ret;
  }
  /** setup bus to read data */
  Temp = MmioRead8((UINTN)&I2cRegs->I2Cr);
  Temp &= ~(I2CR_MTX | I2CR_TX_NO_AK);
  if (Len == 1)
     Temp |= I2CR_TX_NO_AK;
  MmioWrite8((UINTN)&I2cRegs->I2Cr, Temp);
  MmioWrite8((UINTN)&I2cRegs->I2Sr, I2SR_IIF_CLEAR);

  /** read data */
  /** Dummy Read to initiate recieve operation */
  MmioRead8((UINTN)&I2cRegs->I2Dr);

  for (i = 0; i < Len; i++) {
    Ret = WaitForSrState(I2cRegs, ST_IIF);
    if ((Ret == EFI_TIMEOUT) || (Ret == EFI_NOT_READY)) {
       I2cStop(I2cRegs);
       return Ret;
    }
    /**
      It must generate STOP before read I2DR to prevent
      controller from generating another clock cycle
    **/
    if (i == (Len - 1)) {
      I2cStop(I2cRegs);
    } else if (i == (Len - 2)) {
      	Temp = MmioRead8((UINTN)&I2cRegs->I2Cr);
      	Temp |= I2CR_TX_NO_AK;
      	MmioWrite8((UINTN)&I2cRegs->I2Cr, Temp);
    }
    MmioWrite8((UINTN)&I2cRegs->I2Sr, I2SR_IIF_CLEAR);
    Buffer[i] = MmioRead8((UINTN)&I2cRegs->I2Dr);
  }

  I2cStop(I2cRegs);
  return EFI_SUCCESS;
}


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
EFI_STATUS
I2cWrite (
  IN   VOID 		*Base,
  IN   UINT8 		Chip,
  IN   UINT32 	Offset,
  IN   INT32 		Alen,
  OUT  UINT8 		*Buffer,
  IN   INT32 		Len
  )
{
  EFI_STATUS Ret;
  INT32 I;
  struct I2cRegs *I2cRegs = (VOID *)Base;

  Ret = I2cInitTransfer(I2cRegs, Chip, Offset, Alen);
  if (Ret != EFI_SUCCESS)
    return Ret;

  /** write data */
  /** Dummy write to initiate write operation */
#if 0
  MmioWrite8((UINTN)&I2cRegs->I2Dr, 0x00);
#endif
  for (I = 0; I < Len; I++) {
    Ret = TxByte(I2cRegs, Buffer[I]);
    if (Ret != EFI_SUCCESS)
      break;
  }
  I2cStop(I2cRegs);
  return Ret;
}

/**
  Function to Probe i2c bus

  @param   I2c                     parameter defining I2c controller no

  @retval  EFI_INVALID_PARAMETER   Input parametr I2c was invalid
  @retval  EFI_SUCCESS             I2c was initialized successfully

**/
EFI_STATUS
EFIAPI
I2cProbe (
  IN   INT16		I2c,
  IN   UINT8		Chip
  )
{
  VOID * BaseAddress;

  switch (I2c) {
    case I2C0:
      	BaseAddress = (VOID *)I2C0_BASE_ADDRESS;
  	break;
    case I2C1:
  	BaseAddress = (VOID *)I2C1_BASE_ADDRESS;
  	break;
    case I2C2:
  	BaseAddress = (VOID *)I2C2_BASE_ADDRESS;
  	break;
    case I2C3:
  	BaseAddress = (VOID *)I2C3_BASE_ADDRESS;
    	break;
    default:
  	return EFI_INVALID_PARAMETER;
  }
  return I2cWrite(BaseAddress, Chip, 0, 0, NULL, 0);
}

/**
  Function to initialize i2c bus

  @param   I2c                     parameter defining I2c controller no
  @param   Speed                   I2C speed to be set

  @retval  EFI_INVALID_PARAMETER   Input parametr I2c was invalid
  @retval  EFI_SUCCESS             I2c was initialized successfully

**/
EFI_STATUS
EFIAPI
I2cInit (
  IN   INT16		I2c,
  IN   UINT32		Speed
  )
{
  VOID * BaseAddress;

  switch (I2c) {
    case I2C0:
      	BaseAddress = (VOID *)I2C0_BASE_ADDRESS;
  	break;
    case I2C1:
  	BaseAddress = (VOID *)I2C1_BASE_ADDRESS;
  	break;
    case I2C2:
  	BaseAddress = (VOID *)I2C2_BASE_ADDRESS;
  	break;
    case I2C3:
  	BaseAddress = (VOID *)I2C3_BASE_ADDRESS;
    	break;
    default:
  	return EFI_INVALID_PARAMETER;
  }
  return BusI2cSetBusSpeed(BaseAddress, Speed);
}
