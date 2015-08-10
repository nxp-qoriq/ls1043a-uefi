/** @Dspi.h
  Header Defining The Dspi Flash Controller Constants (Base Addresses, Sizes,
  Flags), Function Prototype, Structures etc

  Copyright (C) 2015, Freescale Ltd. All Rights Reserved.

  This Program And The Accompanying Materials
  Are Licensed And Made Available Under The Terms And Conditions Of The BSD
  License Which Accompanies This Distribution. The Full Text Of The License
  May Be Found At
  http://Opensource.Org/Licenses/Bsd-License.Php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __DSPI_H__
#define __DSPI_H__

#include "LS1043aRdb.h"
#include "LS1043aSocLib.h"
#include "Common.h"
#include "Bitops.h"
#include "Spi.h"
#include "CpldLib.h"
#include <Library/MemoryAllocationLib.h>
#include <PiPei.h>
#include <Uefi.h>
#include <Library/BaseMemoryLib/MemLibInternals.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Protocol/BlockIo.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/TimerLib.h>


#define SPI_BUS_FSL_DSPI1	 	1
#define CONFIG_SPI_FLASH_BAR

#define CONFIG_SF_DEFAULT_SPEED 	10000000
#define CONFIG_SF_DEFAULT_MODE 	SPI_MODE_0
#define CONFIG_SF_DEFAULT_CS 	0
#define CONFIG_SF_DEFAULT_BUS 	1

/* Subsectors in a sector */
#define NUM_OF_SUBSECTOR		16

/* Common Status */
#define STATUS_WIP			(1 << 0)
#define STATUS_QEB_WINSPAN		(1 << 1)
#define STATUS_QEB_MXIC		(1 << 6)
#define STATUS_PEC			(1 << 7)
#define STATUS_PROT			(1 << 1)
#define STATUS_ERASE			(1 << 5)

/* ATMEL Dataflash Commands*/
#define CMD_ATMEL_PAGE_PROGRAM	0x82
#define CMD_ATMEL_READ_STATUS	0xd7
#define CMD_ATMEL_FLAG_STATUS	0xd7
#define CMD_ATMEL_ERASE_32K		0x7c

/* Transfer Control - 32-Bit Access */
#define DSPI_TFR_CONT 	(0x80000000)
#define DSPI_TFR_CTAS(X)	(((X)&0x07)<<12)
#define DSPI_TFR_EOQ		(0x08000000)
#define DSPI_TFR_CTCNT	(0x04000000)
#define DSPI_TFR_CS7		(0x00800000)
#define DSPI_TFR_CS6		(0x00400000)
#define DSPI_TFR_CS5		(0x00200000)
#define DSPI_TFR_CS4		(0x00100000)
#define DSPI_TFR_CS3		(0x00080000)
#define DSPI_TFR_CS2		(0x00040000)
#define DSPI_TFR_CS1		(0x00020000)
#define DSPI_TFR_CS0		(0x00010000)

/* Module Configuration */
#define DSPI_MCR_MSTR		(0x80000000)
#define DSPI_MCR_CSCK		(0x40000000)
#define DSPI_MCR_DCONF(X)		(((X)&0x03)<<28)
#define DSPI_MCR_FRZ 		(0x08000000)
#define DSPI_MCR_MTFE 		(0x04000000)
#define DSPI_MCR_PCSSE		(0x02000000)
#define DSPI_MCR_ROOE 		(0x01000000)
#define DSPI_MCR_CSIS7		(0x00800000)
#define DSPI_MCR_CSIS6		(0x00400000)
#define DSPI_MCR_CSIS5		(0x00200000)
#define DSPI_MCR_CSIS4		(0x00100000)
#define DSPI_MCR_CSIS3		(0x00080000)
#define DSPI_MCR_CSIS2		(0x00040000)
#define DSPI_MCR_CSIS1		(0x00020000)
#define DSPI_MCR_CSIS0		(0x00010000)
#define DSPI_MCR_MDIS 		(0x00004000)
#define DSPI_MCR_DTXF 		(0x00002000)
#define DSPI_MCR_DRXF 		(0x00001000)
#define DSPI_MCR_CTXF 		(0x00000800)
#define DSPI_MCR_CRXF 		(0x00000400)
#define DSPI_MCR_SMPL_PT(X) 	(((X)&0x03)<<8)
#define DSPI_MCR_HALT		(0x00000001)

/* Clock And Transfer Attributes */
#define DSPI_CTAR_DBR 		(0x80000000)
#define DSPI_CTAR_TRSZ(X)		(((X)&0x0F)<<27)
#define DSPI_CTAR_CPOL		(0x04000000)
#define DSPI_CTAR_CPHA 		(0x02000000)
#define DSPI_CTAR_LSBFE		(0x01000000)
#define DSPI_CTAR_PCSSCK(X) 	(((X)&0x03)<<22)
#define DSPI_CTAR_PCSSCK_7CLK 	(0x00A00000)
#define DSPI_CTAR_PCSSCK_5CLK 	(0x00800000)
#define DSPI_CTAR_PCSSCK_3CLK 	(0x00400000)
#define DSPI_CTAR_PCSSCK_1CLK 	(0x00000000)
#define DSPI_CTAR_PASC(X) 		(((X)&0x03)<<20)
#define DSPI_CTAR_PASC_7CL		(0x00300000)
#define DSPI_CTAR_PASC_5CL		(0x00200000)
#define DSPI_CTAR_PASC_3CL		(0x00100000)
#define DSPI_CTAR_PASC_1CL		(0x00000000)
#define DSPI_CTAR_PDT(X) 		(((X)&0x03)<<18)
#define DSPI_CTAR_PDT_7CLK		(0x000A0000)
#define DSPI_CTAR_PDT_5CLK		(0x00080000)
#define DSPI_CTAR_PDT_3CLK		(0x00040000)
#define DSPI_CTAR_PDT_1CLK		(0x00000000)
#define DSPI_CTAR_PBR(X) 		(((X)&0x03)<<16)
#define DSPI_CTAR_PBR_7CLK		(0x00030000)
#define DSPI_CTAR_PBR_5CLK		(0x00020000)
#define DSPI_CTAR_PBR_3CLK		(0x00010000)
#define DSPI_CTAR_PBR_1CLK		(0x00000000)
#define DSPI_CTAR_CSSCK(X)		(((X)&0x0F)<<12)
#define DSPI_CTAR_ASC(X)		(((X)&0x0F)<<8)
#define DSPI_CTAR_DT(X)		(((X)&0x0F)<<4)
#define DSPI_CTAR_BR(X)		(((X)&0x0F))

/* Write Commands */
#define CMD_WRITE_STATUS		0x01
#define CMD_BYTE_PROGRAM		0x02
#define CMD_AAI_PROGRAM		0xad
#define CMD_WRITE_DISABLE		0x04
#define CMD_READ_STATUS		0x05
#define CMD_QUAD_PAGE_PROGRAM	0x32
#define CMD_READ_STATUS1		0x35
#define CMD_WRITE_ENABLE		0x06
#define CMD_READ_CONFIG		0x35
#define CMD_FLAG_STATUS		0x70
#define CMD_CLEAR_FLAG_STATUS	0x50

/* Read Commands */
#define CMD_READ_ARRAY_SLOW		0x03
#define CMD_READ_ARRAY_FAST		0x0b
#define CMD_READ_DUAL_OUTPUT_FAST	0x3b
#define CMD_READ_DUAL_IO_FAST	0xbb
#define CMD_READ_QUAD_OUTPUT_FAST	0x6b
#define CMD_READ_QUAD_IO_FAST	0xeb
#define CMD_READ_JEDEC_ID		0x9f

/* Erase Commands */
#define CMD_ERASE_STATUS		0x50
#define CMD_ERASE_4K			0x20
#define CMD_ERASE_32K		0x52
#define CMD_ERASE_CHIP		0xc7
#define CMD_ERASE_64K		0xd8

#define DSPI_PUSHR_CTAS_MASK     (UINT32)(0x70000000)
#define DSPI_PUSHR_PCS_MASK      (UINT32)(0x003f0000)

/* Sf Param Flags */
enum {
  SECT_4K    = 1 << 0,
  SECT_32K   = 1 << 1,
  E_FSR      = 1 << 2,
  SST_BP     = 1 << 3,
  SST_WP     = 1 << 4,
  WR_QPP     = 1 << 5,
  AT45DB_CMD = 1 << 6
};

/* Bank addr access commands */
#ifdef CONFIG_SPI_FLASH_BAR
# define CMD_BANKADDR_BRWR         0x17
# define CMD_BANKADDR_BRRD         0x16
# define CMD_EXTNADDR_WREAR        0xC5
# define CMD_EXTNADDR_RDEAR        0xC8
#endif

#define CONFIG_SYS_DSPI_CTAR0 	(DSPI_CTAR_TRSZ(7) | DSPI_CTAR_PCSSCK_1CLK |\
					DSPI_CTAR_PASC(0) | DSPI_CTAR_PDT(0) | \
					DSPI_CTAR_CSSCK(0) | DSPI_CTAR_ASC(0) | \
					DSPI_CTAR_DT(0))

/* SST Specific */
# define SST_WP			0x01 /* Supports AAI Word Program */
# define CMD_SST_BP			0x02 /* Byte Program */
# define CMD_SST_AAI_WP		0xAD /* Auto Address Incr Word Program */

/* LAST BLOCK */
#define BLOCK_COUNT             	1024	/* For SST25WF040 
						(4MB/4K (sector erase size))*/
#define LAST_BLOCK              	BLOCK_COUNT

/**
 * struct Dspi - Dspi Controller Registers
 *
 * @Mcr:      Module Configuration Register	0x00
 * @Resv0:
 * @Tcr:      Transfer Count Register		0x08
 * @Ctar[4]:  Clock and Transfer Attributes
	 	Register 				0x0C - 0x18
 * @Resv1[4]:
 * @Sr:       Status Register			0x2C
 * @Irsr:     DMA/Interrupt Request Select
	 	and Enable Register			0x30
 * @Tfr:      PUSH TX FIFO Register In
	 	Master Mode				0x34
 * @Rfr:      POP RX FIFO Register			0x38
 * @Tfdr[16]: Transmit FIFO Registers		0x3C
 * @Rfdr[16]: Receive FIFO Registers		0x7C
 */
typedef struct Dspi {
  UINT32 Mcr;
  UINT32 Resv0;
  UINT32 Tcr;
  UINT32 Ctar[4];
  UINT32 Resv1[4];
  UINT32 Sr;
  UINT32 Irsr;
  UINT32 Tfr;
  UINT32 Rfr;
  UINT32 Tfdr[16];
  UINT32 Rfdr[16];
} DspiT;

/**
 * struct DspiSlave - Container of all parameters required to communicate
 * with connected slave device.
 *
 * @Slave: 	Instance of structure SpiSlave, represent slave device
		connected to controller
 * @Regs: 	Pointer to Dspi Registers structure
 * @Baudrate: Baudrate
 * @Charbit: 	Charbit
 */
struct DspiSlave {
  struct SpiSlave Slave;
  struct Dspi *Regs;
  UINT32 Baudrate;
  INT32 Charbit;
};

/**
 * struct DspiFlash - Structure to keep command ids and functions
 *			 used for read, write and erase operations.
 *
 * @Dspi:          Pointer to structure containing information about
 *		     DSPI controller register and slave device connected
 *		     to controller.
 * @Name:          Name Of SPI Flash
 * @DualFlash:     Indicates Dual Flash Memories - Dual Stacked, Parallel
 * @Shift:         Flash Shift Useful In Dual Parallel
 * @Size:          Total Flash Size
 * @PageSize:      Write (Page) Size
 * @SectorSize:    Sector Size
 * @EraseSize:     Erase Size
 * @BlockSize:     Block Size
 * @BankReadCmd:   Bank Read Cmd
 * @BankWriteCmd:  Bank Write Cmd
 * @BankCurr:      Current Flash Bank
 * @PollCmd:       Poll Cmd - for Flash Erase/Program
 * @EraseCmd:      Erase Cmd 4K, 32K, 64K
 * @ReadCmd:       Read Cmd - Array Fast, Extn Read And Quad Read.
 * @WriteCmd:      Write Cmd - Page And Quad Program.
 * @DummyByte:     Dummy Cycles for Read Operation.
 * @MemoryMap:     Address Of Read-Only SPI Flash Access
 * @Read:          Flash Read Ops: Read Len Bytes At Offset into Buf
 * @Write:         Flash Write Ops: Write Len Bytes From Buf into Offset
 * @Erase:         Flash Erase Ops: Erase Len Bytes From Offset
 */
struct DspiFlash {
  struct DspiSlave *Dspi;
  CONST INT8 *Name;
  UINT8 DualFlash;
  UINT8 Shift;
  UINT32 Size;
  UINT32 PageSize;
  UINT32 SectorSize;
  UINT32 EraseSize;
  UINT32 BlockSize;
  UINT8 BankReadCmd;
  UINT8 BankWriteCmd;
  UINT8 BankCurr;
  UINT8 PollCmd;
  UINT8 EraseCmd;
  UINT8 ReadCmd;
  UINT8 WriteCmd;
  UINT8 DummyByte;
  VOID *MemoryMap;
  EFI_STATUS (*Read)(struct DspiFlash *Flash, UINT32 Offset, UINT64 Len,
			VOID *Buf);
  EFI_STATUS (*Write)(struct DspiFlash *Flash, UINT32 Offset, UINT64 Len,
			CONST VOID *Buf);
  EFI_STATUS (*Erase)(struct DspiFlash *Flash, UINT32 Offset, UINT64 Len);
};

/* Function Prototypes */

EFI_STATUS
DspiFlashReadCommon (
  IN  struct DspiFlash *Flash,
  IN  CONST UINT8 *Cmd,
  IN  UINT32 CmdLen,
  OUT VOID *Data,
  IN  UINT64 DataLen
  );

EFI_STATUS
DspiFlashWriteCommon (
  IN  struct DspiFlash *Flash,
  IN  CONST UINT8 *Cmd,
  IN  UINT32 CmdLen,
  IN  CONST VOID *Buf,
  IN  UINT32 BufLen
  );

EFI_STATUS
DspiFlashCmdWriteOps (
  IN  struct DspiFlash *Flash,
  IN  UINT32 Offset,
  IN  UINT64 Len,
  IN  CONST VOID *Buf
  );

EFI_STATUS
DspiFlashCmdEraseOps (
  IN  struct DspiFlash *Flash,
  IN  UINT32 Offset,
  IN  UINT64 Len
  );

EFI_STATUS
DspiFlashCmdReadOps (
  IN  struct DspiFlash *Flash,
  IN  UINT32 Offset,
  IN  UINT64 Len,
  OUT VOID *Data
  );

INT32
BoardSpiIsDataflash (
  IN  UINT32 Bus,
  IN  UINT32 Cs
  );

EFI_STATUS
DspiClaimBus (
  IN  struct DspiSlave *Slave
  );

EFI_STATUS
DspiFlashReadWrite (
  IN  struct DspiSlave *Dspi,
  IN  CONST UINT8 *Cmd,
  IN  UINT32 CmdLen,
  IN  CONST UINT8 *DataOut,
  OUT UINT8 *DataIn,
  IN  UINT64 DataLen
  );

EFI_STATUS
DspiFlashSetQeb (
  IN  struct DspiFlash *Flash,
  IN  UINT8 Idcode0
  );

VOID
PrintSize (
  IN  UINT64 Size,
  IN  CONST INT8 *S
  );

VOID
DspiReleaseBus (
  IN  struct DspiSlave *Slave
  );

VOID
DspiFreeSlave (
  IN  struct DspiSlave *Slave
  );

struct DspiFlash *
DspiFlashProbeSlave (
  IN  struct DspiSlave *Dspi
  );

struct DspiSlave *
DspiSetupSlave (
  IN  UINT32 Bus,
  IN  UINT32 Cs,
  IN  UINT32 MaxHz,
  IN  UINT32 Mode
  );

/**
  This API detect the slave device connected to dspi controller and
  initializes slave device structure.

  @retval EFI_SUCCESS       	Slave device is attached to DSPI Controller.
  @retval EFI_OUT_OF_RESOURCES	Failed to allocate memory.
**/
EFI_STATUS
DspiDetect(
  VOID
  );

/**
  This API read the length bytes of data starting from specified offset
  from connected flash memory device to an input memory buffer

  @param[in]  Offset    	Offset of Flash memory to start with.
  @param[in]  Len	 	Length of data to be read.
  @param[out] Buf		Pointer to memory buffer to keep read data.

  @retval EFI_SUCCESS       	Read operation is successful.
  @retval EFI_INVALID_PARAMETER 	Input Parameter is invalid.
**/
EFI_STATUS
DspiFlashRead (
  IN  UINT32 Offset,
  IN  UINT64 Len,
  OUT VOID *Buf
  );

/**
  This API write the length bytes of data from inputted memory
  buffer to connected flash memory device starting from specified offset.

  @param[in]  Offset    	Offset of Flash memory to start with.
  @param[in]  Len	 	Length of data to write.
  @param[in]  Buf		Pointer to memory buffer to write data from.

  @retval EFI_SUCCESS       	Write operation is successful.
  @retval EFI_INVALID_PARAMETER 	Input Parameter is invalid.
**/
EFI_STATUS
DspiFlashWrite (
  IN  UINT32 Offset,
  IN  UINT64 Len,
  IN  CONST VOID *Buf
  );

/**
  This API erase length bytes of flash memory device starting from
  input offset value.

  @param[in]  Offset    	Offset of Flash memory to start with.
  @param[in]  Len	 	Length of flash memory need to erase.

  @retval EFI_SUCCESS       	Erase operation is successful.
  @retval EFI_INVALID_PARAMETER 	Input Parameter is invalid.
**/
EFI_STATUS
DspiFlashErase (
  IN  UINT32 Offset,
  IN  UINT64 Len
  );

/**
  This API detect the slave device connected to dspi controller
  and initialize block IO parameter.

  @param[in]  gDspiMedia	Indicates a pointer to the block IO media
				device.

  @retval EFI_SUCCESS       	Slave device is attached to DSPI Controller.
  @retval EFI_OUT_OF_RESOURCES	Failed to allocate memory.
**/
EFI_STATUS
DspiInit (
  EFI_BLOCK_IO_MEDIA *gDspiMedia
  );

/**
  This API Erases BufferSize * NUM_OF_SUBSECTOR bytes starting from Lba.

  @param[in]  This    	Indicates a pointer to the calling context.
  @param[in]  MediaId 	Id of the media, changes every time media is replaced.
  @param[in]  Lba		The starting Logical Block Address to be erased
  @param[in]  BufferSize	Size of Buffer, must be a multiple of device
  				block size.

  @retval EFI_SUCCESS       	The data was erased successfully.
  @retval EFI_WRITE_PROTECTED	The device cannot be written to.
  @retval EFI_DEVICE_ERROR   	The device reported an error while performing
					the erase.
  @retval EFI_NO_MEDIA         	There is no media in the device.
  @retval EFI_MEDIA_CHANGED     	The MediaId does not matched the current
  					device.
  @retval EFI_BAD_BUFFER_SIZE   	Buffer not a multiple of the block size
  					of the device.
  @retval EFI_INVALID_PARAMETER 	The erase request contains LBAs that are
  					not valid.
**/
EFI_STATUS
EFIAPI
DspiErase (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                Lba,
  IN  UINTN                  BufferSize
);

/**
  This API Writes BufferSize bytes from Lba into Buffer.

  @param[in]  This    	Indicates a pointer to the calling context.
  @param[in]  MediaId 	Id of the media, changes every time media is replaced.
  @param[in]  Lba		The starting Logical Block Address to be written
				The caller is responsible for writing to only
				legitimate locations.
  @param[in]  BufferSize	Size of Buffer, must be a multiple of device
  				block size.
  @param[in]  Buffer		A pointer to the source buffer for the data.

  @retval EFI_SUCCESS       	The data was written correctly to the device.
  @retval EFI_WRITE_PROTECTED	The device cannot be written to.
  @retval EFI_DEVICE_ERROR   	The device reported an error while performing
					the write.
  @retval EFI_NO_MEDIA         	There is no media in the device.
  @retval EFI_MEDIA_CHANGED     	The MediaId does not matched the current
  					device.
  @retval EFI_BAD_BUFFER_SIZE   	Buffer not a multiple of the block size
  					of the device.
  @retval EFI_INVALID_PARAMETER 	The write request contains LBAs that are
  					not valid, or the buffer is not on
					proper alignment
**/
EFI_STATUS
EFIAPI
DspiWrite (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                Lba,
  IN  UINTN                  BufferSize,
  IN  VOID                   *Buffer
);

/**
  This API read the BuferSize bytes of from Lba into Buffer.

  @param[in]  This    	Indicates a pointer to the calling context.
  @param[in]  MediaId 	Id of the media, changes every time media is replaced.
  @param[in]  Lba		The starting Logical Block Address to read from.
  @param[in]  BufferSize	Size of Buffer, must be a multiple of device
  				block size.
  @param[out] Buffer		A pointer to the destination buffer for the
  				data. The caller is responsible for either
				having implicit or explicit ownership of the
				buffer.

  @retval EFI_SUCCESS       	The data was read correctly from the device.
  @retval EFI_DEVICE_ERROR   	The device reported an error while performing
					the read.
  @retval EFI_NO_MEDIA         	There is no media in the device.
  @retval EFI_MEDIA_CHANGED     	The MediaId does not matched the current
  					device.
  @retval EFI_BAD_BUFFER_SIZE   	Buffer not a multiple of the block size
  					of the device.
  @retval EFI_INVALID_PARAMETER 	The read request contains LBAs that are
  					not valid, or the buffer is not on
					proper alignment
**/
EFI_STATUS
EFIAPI
DspiRead (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  OUT VOID                          *Buffer
  );

/**
  This API free the global Dspi flash memory structure(DspiFlash).
**/
VOID
DspiFlashFree (
  VOID
  );

VOID
SelectDspi (
  IN VOID
  );

#endif
