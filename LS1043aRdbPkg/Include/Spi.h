/** @Spi.h
  Header Defining The Spi Flash Controller Constants, Structures etc

  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This Program And The Accompanying Materials
  Are Licensed And Made Available Under The Terms And Conditions Of The BSD
  License Which Accompanies This Distribution. The Full Text Of The License
  May Be Found At
  http://Opensource.Org/Licenses/Bsd-License.Php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SPI_H__
#define __SPI_H__

/* SPI Mode Flags */
#define SPI_CPHA			0x01	/* Clock Phase */
#define SPI_CPOL			0x02	/* Clock Polarity */
#define SPI_MODE_0			(0|0)	/* (Original MicroWire) */
#define SPI_MODE_1			(0|SPI_CPHA)
#define SPI_MODE_2			(SPI_CPOL|0)
#define SPI_MODE_3			(SPI_CPOL|SPI_CPHA)
#define SPI_CS_HIGH			0x04	/* CS Active High */
#define SPI_LSB_FIRST		0x08	/* Per-Word Bits-On-Wire */
#define SPI_3WIRE			0x10	/* SI/SO Signals Shared */
#define SPI_LOOP			0x20	/* Loopback Mode */
#define SPI_SLAVE			0x40	/* Slave Mode */
#define SPI_PREAMBLE			0x80	/* Skip Preamble Bytes */

/* SPI Transfer Flags */
#define SPI_XFER_BEGIN 		0x01	/* Assert CS Before Transfer */
#define SPI_XFER_END 		0x02 	/* Deassert CS After Transfer */
#define SPI_XFER_MMAP 		0x08 	/* Memory Mapped Start */
#define SPI_XFER_MMAP_END 		0x10 	/* Memory Mapped End */
#define SPI_XFER_ONCE 		(SPI_XFER_BEGIN | SPI_XFER_END)
#define SPI_XFER_U_PAGE 		(1 << 5)

#define CONFIG_SPI_IDLE_VAL 	0x0

#define SPI_FLASH_3B_ADDR_LEN 	3
#define SPI_FLASH_CMD_LEN		(1 + SPI_FLASH_3B_ADDR_LEN)
#define SPI_FLASH_16MB_BOUN		0x1000000

#define SPI_DEFAULT_WORDLEN 	8

/* Spi Specific Mode */
#define SPI_MODE_MOD 		0x00200000
#define SPI_DBLRATE			0x00100000

/* Flash Timeout Values */
#define CONFIG_SYS_HZ 			1000
#define SPI_FLASH_PROG_TIMEOUT		(2 * CONFIG_SYS_HZ)
#define SPI_FLASH_PAGE_ERASE_TIMEOUT 	(5 * CONFIG_SYS_HZ)
#define SPI_FLASH_SECTOR_ERASE_TIMEOUT	(10 * CONFIG_SYS_HZ)

/* CFI Manufacture ID'S */
#define SPI_FLASH_CFI_MFR_SPANSION 	0x01
#define SPI_FLASH_CFI_MFR_STMICRO 		0x20
#define SPI_FLASH_CFI_MFR_MACRONIX 	0xc2
#define SPI_FLASH_CFI_MFR_WINBOND 		0xef

/* SPI TX Operation Modes */
#define SPI_OPM_TX_QPP 			(1 << 0)

/* Enum list - Full read commands */
enum SpiReadCmds {
  ARRAY_SLOW           = 1 << 0,
  ARRAY_FAST           = 1 << 1,
  DUAL_OUTPUT_FAST     = 1 << 2,
  DUAL_IO_FAST         = 1 << 3,
  QUAD_OUTPUT_FAST     = 1 << 4,
  QUAD_IO_FAST         = 1 << 5,
};

/* Normal - Extended - Full command set */
#define RD_NORM      (ARRAY_SLOW | ARRAY_FAST)
#define RD_EXTN      (RD_NORM | DUAL_OUTPUT_FAST | DUAL_IO_FAST)
#define RD_FULL      (RD_EXTN | QUAD_OUTPUT_FAST | QUAD_IO_FAST)

/* Dual SPI Flash Memories */
enum SpiDualFlash {
  SF_SINGLE_FLASH = 0,
  SF_DUAL_STACKED_FLASH = 1 << 0,
  SF_DUAL_PARALLEL_FLASH = 1 << 1,
};

/**
 * struct DspiFlashParameters - Flash Device Parameter Structure
 *
 * @Name:            Device Name ([MANUFLETTER][DEVTYPE][DENSITY][EXTRAINFO])
 * @Jedec:           Device Jedec ID (0x[1byteManufId][2byteDevId])
 * @ExtJedec:        Device ExtJedec ID
 * @SectorSize:      Sector Size Of This Device
 * @NoOfSectors:       No.Of Sectors On This Device
 * @EnumRdCmd:          enum List for Read Commands
 * @Flags:           Important Param, for Flash Specific Behaviour
 */
struct DspiFlashParameters {
  CONST INT8 *Name;
  UINT32 Jedec;
  UINT16 ExtJedec;
  UINT32 SectorSize;
  UINT32 NoOfSectors;
  UINT8  EnumRdCmd;
  UINT16 Flags;
};

/**
 * struct SpiSlave - Representation Of Slave device connected to SPI controller
 *
 * Drivers Are Expected To Extend This With Controller-Specific Data.
 *
 * @Bus:           ID Of The Bus That The Slave Is Attached To.
 * @Cs:            ID Of The Chip Select Connected To The Slave.
 * @OpModeRx:      SPI RX Operation Mode.
 * @OpModeTx:      SPI TX Operation Mode.
 * @Wordlen:       Size Of SPI Word In Number Of Bits
 * @MaxWriteSize:  if Non-Zero, The Maximum Number Of Bytes Which Can
 *                 Be Written At Once, Excluding Command Bytes.
 * @MemoryMap:     Address Of Read-Only SPI Flash Access.
 * @Option:        Varies SPI Bus Options - Separate, Shared Bus.
 * @Flags:         Indication Of SPI Flags.
 */
struct SpiSlave {
  UINT32 Bus;
  UINT32 Cs;
  UINT8  OpModeRx;
  UINT8  OpModeTx;
  UINT32 Wordlen;
  UINT32 MaxWriteSize;
  VOID   *MemoryMap;
  UINT8  Option;
  UINT8  Flags;
};

#endif
