/** @Sdxc.h
  Header Defining The Sdxc Memory Controller Constants (Base Addresses, Sizes,
  Flags), Function Prototype, Structures Etc

  Copyright (C) 2015, Freescale Ltd. All Rights Reserved.

  This Program And The Accompanying Materials
  Are Licensed And Made Available Under The Terms And Conditions Of The BSD
  License Which Accompanies This Distribution. The Full Text Of The License
  May Be Found At
  Http://Opensource.Org/Licenses/Bsd-License.Php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SDXC_H__
#define __SDXC_H__

#include "LS1043aRdb.h"
#include "LS1043aSocLib.h"
#include "Common.h"
#include "Bitops.h"
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
#include <Library/PrintLib.h>


#define EFI_SDXC_SIGNATURE SIGNATURE_32('S','D','X','C')

/**
  Set Block Count Limit Because Of 16 Bit Register Limit On Some Hardware
**/
#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
#define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif

/**
  SDXC-Specific Constants
**/
#define SYSCTL              	0x0002e02c
#define SYSCTL_INITA        	0x08000000
#define SYSCTL_TIMEOUT_MASK 	0x000f0000
#define SYSCTL_CLOCK_MASK   	0x0000fff0
#define SYSCTL_CKEN         	0x00000008
#define SYSCTL_PEREN        	0x00000004
#define SYSCTL_HCKEN        	0x00000002
#define SYSCTL_IPGEN        	0x00000001
#define SYSCTL_RSTA         	0x01000000
#define SYSCTL_RSTC         	0x02000000
#define SYSCTL_RSTD         	0x04000000

/**
  Host Capabilities
**/
#define SDXC_HOSTCAPBLT_VS18      0x04000000
#define SDXC_HOSTCAPBLT_VS30      0x02000000
#define SDXC_HOSTCAPBLT_VS33      0x01000000
#define SDXC_HOSTCAPBLT_SRS       0x00800000
#define SDXC_HOSTCAPBLT_DMAS      0x00400000
#define SDXC_HOSTCAPBLT_HSS       0x00200000

/**
  VDD Voltage Range
**/
#define MMC_VDD_165_195            0x00000080    // VDD Voltage 1.65 - 1.95
#define MMC_VDD_20_21              0x00000100    // VDD Voltage 2.0 ~ 2.1
#define MMC_VDD_21_22              0x00000200    // VDD Voltage 2.1 ~ 2.2
#define MMC_VDD_22_23              0x00000400    // VDD Voltage 2.2 ~ 2.3
#define MMC_VDD_23_24              0x00000800    // VDD Voltage 2.3 ~ 2.4
#define MMC_VDD_24_25              0x00001000    // VDD Voltage 2.4 ~ 2.5
#define MMC_VDD_25_26              0x00002000    // VDD Voltage 2.5 ~ 2.6
#define MMC_VDD_26_27              0x00004000    // VDD Voltage 2.6 ~ 2.7
#define MMC_VDD_27_28              0x00008000    // VDD Voltage 2.7 ~ 2.8
#define MMC_VDD_28_29              0x00010000    // VDD Voltage 2.8 ~ 2.9
#define MMC_VDD_29_30              0x00020000    // VDD Voltage 2.9 ~ 3.0
#define MMC_VDD_30_31              0x00040000    // VDD Voltage 3.0 ~ 3.1
#define MMC_VDD_31_32              0x00080000    // VDD Voltage 3.1 ~ 3.2
#define MMC_VDD_32_33              0x00100000    // VDD Voltage 3.2 ~ 3.3
#define MMC_VDD_33_34              0x00200000    // VDD Voltage 3.3 ~ 3.4
#define MMC_VDD_34_35              0x00400000    // VDD Voltage 3.4 ~ 3.5
#define MMC_VDD_35_36              0x00800000    // VDD Voltage 3.5 ~ 3.6

/**
  MMC Operating Modes
**/
#define MMC_MODE_HS         	(1 << 0)
#define MMC_MODE_HS_52MHz   	(1 << 1)
#define MMC_MODE_4BIT       	(1 << 2)
#define MMC_MODE_8BIT       	(1 << 3)
#define MMC_MODE_SPI        	(1 << 4)
#define MMC_MODE_HC         	(1 << 5)
#define MMC_MODE_DDR_52MHz  	(1 << 6)

#define SD_DATA_4BIT 		0x00040000

#define MMC_SWITCH_MODE_CMD_SET    0x00 // Change The Command Set
#define MMC_SWITCH_MODE_SET_BITS   0x01 // Set Bits In EXT_CSD Byte
                                        // Addressed By Index Which Are
                                        // 1 In Value Field
#define MMC_SWITCH_MODE_CLEAR_BITS 0x02 // Clear Bits In EXT_CSD Byte
                                        // Addressed By Index, Which Are
                                        // 1 In Value Field
#define MMC_SWITCH_MODE_WRITE_BYTE 0x03 // Set Target Byte To Value

#define SD_SWITCH_CHECK            0
#define SD_SWITCH_SWITCH    	1

#define MMC_DATA_READ              1
#define MMC_DATA_WRITE             2

#define MmcHostIsSpi(Mmc)       	0

#define SD_VERSION_SD       	0x20000
#define SD_VERSION_3 		(SD_VERSION_SD | 0x300)
#define SD_VERSION_2 		(SD_VERSION_SD | 0x200)
#define SD_VERSION_1_0      	(SD_VERSION_SD | 0x100)
#define SD_VERSION_1_10     	(SD_VERSION_SD | 0x10a)
#define MMC_VERSION_MMC            0x10000
#define MMC_VERSION_UNKNOWN 	(MMC_VERSION_MMC)
#define MMC_VERSION_1_2            (MMC_VERSION_MMC | 0x102)
#define MMC_VERSION_1_4            (MMC_VERSION_MMC | 0x104)
#define MMC_VERSION_2_2            (MMC_VERSION_MMC | 0x202)
#define MMC_VERSION_3              (MMC_VERSION_MMC | 0x300)
#define MMC_VERSION_4              (MMC_VERSION_MMC | 0x400)
#define MMC_VERSION_4_1            (MMC_VERSION_MMC | 0x401)
#define MMC_VERSION_4_2            (MMC_VERSION_MMC | 0x402)
#define MMC_VERSION_4_3            (MMC_VERSION_MMC | 0x403)
#define MMC_VERSION_4_41    	(MMC_VERSION_MMC | 0x429)
#define MMC_VERSION_4_5            (MMC_VERSION_MMC | 0x405)
#define MMC_VERSION_5_0            (MMC_VERSION_MMC | 0x500)

#define IS_SD(X) 			(X->Version & SD_VERSION_SD)

/**
  Maximum Block Size for MMC
**/
#define MMC_MAX_BLOCK_LEN   512

#define MMCPART_NOAVAILABLE (0xff)
#define PART_ACCESS_MASK    (0x7)
#define PART_SUPPORT        (0x1)
#define PART_ENH_ATTRIB     (0x1f)

/**
  SCR Definitions In Different Words
**/
#define SD_HIGHSPEED_BUSY   	0x00020000
#define SD_HIGHSPEED_SUPPORTED     0x00020000

#define OCR_BUSY            	0x80000000
#define OCR_HCS                    0x40000000
#define OCR_VOLTAGE_MASK    	0x007FFF80
#define OCR_ACCESS_MODE            0x60000000

#define SECURE_ERASE        	0x80000000

#define SD_CMD_SEND_RELATIVE_ADDR  3
#define SD_CMD_SEND_OP_COND        5
#define SD_CMD_SWITCH_FUNC         6
#define SD_CMD_SEND_IF_COND        8
#define SD_CMD_APP_SET_BUS_WIDTH   6

#define SD_CMD_ERASE_WR_BLK_START  32
#define SD_CMD_ERASE_WR_BLK_END    33
#define SD_CMD_APP_SEND_OP_COND    41
#define SD_CMD_APP_SEND_SCR        51

/**
  EXT_CSD Fields
**/
#define EXT_CSD_GP_SIZE_MULT              143    // R/W
#define EXT_CSD_PARTITION_SETTING         155    // R/W
#define EXT_CSD_PARTITIONS_ATTRIBUTE      156    // R/W
#define EXT_CSD_PARTITIONING_SUPPORT      160    // RO
#define EXT_CSD_RST_N_FUNCTION            162    // R/W
#define EXT_CSD_RPMB_MULT          	168    // RO
#define EXT_CSD_ERASE_GROUP_DEF           175    // R/W
#define EXT_CSD_BOOT_BUS_WIDTH            177
#define EXT_CSD_PART_CONF          	179    // R/W
#define EXT_CSD_BUS_WIDTH          	183    // R/W
#define EXT_CSD_HS_TIMING          	185    // R/W
#define EXT_CSD_REV                	192    // RO
#define EXT_CSD_CARD_TYPE          	196    // RO
#define EXT_CSD_SEC_CNT                   212    // RO, 4 Bytes
#define EXT_CSD_HC_WP_GRP_SIZE            221    // RO
#define EXT_CSD_HC_ERASE_GRP_SIZE  	224    // RO
#define EXT_CSD_BOOT_MULT          	226    // RO

#define EXT_CSD_CMD_SET_NORMAL            (1 << 0)
#define EXT_CSD_CMD_SET_SECURE            (1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE   	(1 << 2)

#define EXT_CSD_CARD_TYPE_26       	(1 << 0)      // Card Can Run At 26MHz
#define EXT_CSD_CARD_TYPE_52       	(1 << 1)      // Card Can Run At 52MHz
#define EXT_CSD_CARD_TYPE_DDR_1_8V 	(1 << 2)
#define EXT_CSD_CARD_TYPE_DDR_1_2V 	(1 << 3)
#define EXT_CSD_CARD_TYPE_DDR_52   	(EXT_CSD_CARD_TYPE_DDR_1_8V \
                                   	| EXT_CSD_CARD_TYPE_DDR_1_2V)

#define EXT_CSD_BUS_WIDTH_1 		0      // Card Is In 1 Bit Mode
#define EXT_CSD_BUS_WIDTH_4 		1      // Card Is In 4 Bit Mode
#define EXT_CSD_BUS_WIDTH_8 		2      // Card Is In 8 Bit Mode
#define EXT_CSD_DDR_BUS_WIDTH_4    	5      // Card Is In 4 Bit DDR Mode
#define EXT_CSD_DDR_BUS_WIDTH_8    	6      // Card Is In 8 Bit DDR Mode

#define EXT_CSD_PARTITION_SETTING_COMPLETED      (1 << 0)

/**
  MMC Commands
**/
#define MMC_CMD_GO_IDLE_STATE             0
#define MMC_CMD_SEND_OP_COND              1
#define MMC_CMD_ALL_SEND_CID              2
#define MMC_CMD_SET_RELATIVE_ADDR  	3
#define MMC_CMD_SET_DSR                   4
#define MMC_CMD_SWITCH                    6
#define MMC_CMD_SELECT_CARD        	7
#define MMC_CMD_SEND_EXT_CSD              8
#define MMC_CMD_SEND_CSD           	9
#define MMC_CMD_SEND_CID           	10
#define MMC_CMD_STOP_TRANSMISSION  	12
#define MMC_CMD_SEND_STATUS        	13
#define MMC_CMD_SET_BLOCKLEN              16
#define MMC_CMD_READ_SINGLE_BLOCK  	17
#define MMC_CMD_READ_MULTIPLE_BLOCK       18
#define MMC_CMD_SET_BLOCK_COUNT         	23
#define MMC_CMD_WRITE_SINGLE_BLOCK 	24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK      25
#define MMC_CMD_ERASE_GROUP_START  	35
#define MMC_CMD_ERASE_GROUP_END           36
#define MMC_CMD_ERASE                     38
#define MMC_CMD_APP_CMD                   55
#define MMC_CMD_SPI_READ_OCR              58
#define MMC_CMD_SPI_CRC_ON_OFF            59
#define MMC_CMD_RES_MAN                   62

#define MMC_RSP_PRESENT 			(1 << 0)
#define MMC_RSP_136  			(1 << 1)             // 136 Bit Response
#define MMC_RSP_CRC  			(1 << 2)             // Expect Valid Crc
#define MMC_RSP_BUSY 			(1 << 3)             // Card May Send Busy
#define MMC_RSP_OPCODE      		(1 << 4)             // Response Contains Opcode

#define MMC_RSP_NONE 		(0)
#define MMC_RSP_R1   		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b  		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| \
                     		MMC_RSP_BUSY)
#define MMC_RSP_R2   		(MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3   		(MMC_RSP_PRESENT)
#define MMC_RSP_R4   		(MMC_RSP_PRESENT)
#define MMC_RSP_R5   		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6   		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7   		(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define WML_RD_WML_MAX      	0x10
#define WML_WR_WML_MAX      	0x80
#define WML_RD_WML_MAX_VAL  	0x10
#define WML_WR_WML_MAX_VAL  	0x80
#define WML_RD_WML_MASK     	0xff
#define WML_WR_WML_MASK     	0xff0000

#define XFERTYP                    0x0002e00c
#define XFERTYP_CMD(X)             ((X & 0x3f) << 24)
#define XFERTYP_CMDTYP_NORMAL      0x0
#define XFERTYP_CMDTYP_SUSPEND     0x00400000
#define XFERTYP_CMDTYP_RESUME      0x00800000
#define XFERTYP_CMDTYP_ABORT       0x00c00000
#define XFERTYP_DPSEL              0x00200000
#define XFERTYP_CICEN              0x00100000
#define XFERTYP_CCCEN              0x00080000
#define XFERTYP_RSPTYP_NONE 	0
#define XFERTYP_RSPTYP_136  	0x00010000
#define XFERTYP_RSPTYP_48   	0x00020000
#define XFERTYP_RSPTYP_48_BUSY     0x00030000
#define XFERTYP_MSBSEL             0x00000020
#define XFERTYP_DTDSEL             0x00000010
#define XFERTYP_AC12EN             0x00000004
#define XFERTYP_BCEN        	0x00000002
#define XFERTYP_DMAEN              0x00000001

#define PIO_TIMEOUT         10000000

#define IRQSTAT             (0x0002e030)
#define IRQSTAT_DMAE        (0x10000000)
#define IRQSTAT_AC12E       (0x01000000)
#define IRQSTAT_DEBE        (0x00400000)
#define IRQSTAT_DCE         (0x00200000)
#define IRQSTAT_DTOE        (0x00100000)
#define IRQSTAT_CIE         (0x00080000)
#define IRQSTAT_CEBE        (0x00040000)
#define IRQSTAT_CCE         (0x00020000)
#define IRQSTAT_CTOE        (0x00010000)
#define IRQSTAT_CINT        (0x00000100)
#define IRQSTAT_CRM         (0x00000080)
#define IRQSTAT_CINS        (0x00000040)
#define IRQSTAT_BRR         (0x00000020)
#define IRQSTAT_BWR         (0x00000010)
#define IRQSTAT_DINT        (0x00000008)
#define IRQSTAT_BGE         (0x00000004)
#define IRQSTAT_TC          (0x00000002)
#define IRQSTAT_CC          (0x00000001)

#define CMD_ERR             (IRQSTAT_CIE | IRQSTAT_CEBE | IRQSTAT_CCE)
#define DATA_ERR     	(IRQSTAT_DEBE | IRQSTAT_DCE | IRQSTAT_DTOE | \
                            IRQSTAT_DMAE)
#define DATA_COMPLETE       (IRQSTAT_TC | IRQSTAT_DINT)

#define IRQSTATEN           (0x0002e034)
#define IRQSTATEN_DMAE      (0x10000000)
#define IRQSTATEN_AC12E     (0x01000000)
#define IRQSTATEN_DEBE      (0x00400000)
#define IRQSTATEN_DCE       (0x00200000)
#define IRQSTATEN_DTOE      (0x00100000)
#define IRQSTATEN_CIE       (0x00080000)
#define IRQSTATEN_CEBE      (0x00040000)
#define IRQSTATEN_CCE       (0x00020000)
#define IRQSTATEN_CTOE      (0x00010000)
#define IRQSTATEN_CINT      (0x00000100)
#define IRQSTATEN_CRM       (0x00000080)
#define IRQSTATEN_CINS      (0x00000040)
#define IRQSTATEN_BRR       (0x00000020)
#define IRQSTATEN_BWR       (0x00000010)
#define IRQSTATEN_DINT      (0x00000008)
#define IRQSTATEN_BGE       (0x00000004)
#define IRQSTATEN_TC        (0x00000002)
#define IRQSTATEN_CC        (0x00000001)

#define PRSSTAT             (0x0002e024)
#define PRSSTAT_DAT0        (0x01000000)
#define PRSSTAT_CLSL        (0x00800000)
#define PRSSTAT_WPSPL       (0x00080000)
#define PRSSTAT_CDPL        (0x00040000)
#define PRSSTAT_CINS        (0x00010000)
#define PRSSTAT_BREN        (0x00000800)
#define PRSSTAT_BWEN        (0x00000400)
#define PRSSTAT_DLA         (0x00000004)
#define PRSSTAT_CICHB       (0x00000002)
#define PRSSTAT_CIDHB       (0x00000001)

#define PROCTL              0x0002e028
#define PROCTL_INIT         0x00000020
#define PROCTL_DTW_4        0x00000002
#define PROCTL_DTW_8        0x00000004
#define PROCTL_BE           0x00000030

#define MMC_STATUS_MASK            (~0x0206BF7F)
#define MMC_STATUS_SWITCH_ERROR    (1 << 7)
#define MMC_STATUS_RDY_FOR_DATA 	(1 << 8)
#define MMC_STATUS_CURR_STATE      (0xf << 9)
#define MMC_STATUS_ERROR    	(1 << 19)
#define MMC_STATE_PRG              (7 << 9)

/* For CPLD Settings */
#define ENABLE_SDXC_SOFT_MUX	0x30
#define ENABLE_RCW_SOFT_MUX		0x01
#define SELECT_SW4_SDXC		0x40
#define SELECT_SW5_SDXC		0x01

/* Dma addresses are 32-bits wide.  */
typedef UINT32 DmaAddrT;

struct FslSdxc {
       UINT32    Dsaddr;      // SDMA System Address Register
       UINT32    Blkattr;     // Block Attributes Register
       UINT32    CmdArg;      // Command Argument Register
       UINT32    Xfertyp;     // Transfer Type Register
       UINT32    Cmdrsp0;     // Command Response 0 Register
       UINT32    Cmdrsp1;     // Command Response 1 Register
       UINT32    Cmdrsp2;     // Command Response 2 Register
       UINT32    Cmdrsp3;     // Command Response 3 Register
       UINT32    Datport;     // Buffer Data Port Register
       UINT32    Prsstat;     // Present State Register
       UINT32    Proctl;      // Protocol Control Register
       UINT32    Sysctl;      // System Control Register
       UINT32    Irqstat;     // Interrupt Status Register
       UINT32    Irqstaten;   // Interrupt Status Enable Register
       UINT32    Irqsigen;    // Interrupt Signal Enable Register
       UINT32    Autoc12err;  // Auto CMD Error Status Register
       UINT32    Hostcapblt;  // Host Controller Capabilities Register
       UINT32    Wml;         // Watermark Level Register
       UINT32    Mixctrl;     // for USDHC
       CHAR8     Reserved1[4]; // Reserved
       UINT32    Fevt;        // Force Event Register
       UINT32    Admaes;      // ADMA Error Status Register
       UINT32    Adsaddr;     // ADMA System Address Register
       CHAR8     Reserved2[160];// Reserved
       UINT32    Hostver;     // Host Controller Version Register
       CHAR8     Reserved3[4];// Reserved
       UINT32    Dmaerraddr;  // DMA Error Address Register
       CHAR8     Reserved4[4];// Reserved
       UINT32    Dmaerrattr;  // DMA Error Attribute Register
       CHAR8     Reserved5[4];// Reserved
       UINT32    Hostcapblt2; // Host Controller Capabilities Register 2
       CHAR8     Reserved6[8];// Reserved
       UINT32    Tcr;         // Tuning Control Register
       CHAR8     Reserved7[28];// Reserved
       UINT32    Sddirctl;    // SD Direction Control Register
       CHAR8     Reserved8[712];// Reserved
       UINT32    Scr;         // SDXC Control Register
};

struct MmcData {
       union {
              CHAR8 *Dest;
              CONST CHAR8 *Src; // Src Buffers Don'T Get Written To
       };
       UINT32 Flags;
       UINT32 Blocks;
       UINT32 Blocksize;
};

struct MmcCmd {
       UINT16 CmdIdx;
       UINT32 RespType;
       UINT32 CmdArg;
       UINT32 Response[4];
};

typedef struct BlockDevDesc {
       INT32           IfType;      // Type Of The Interface
       UINT8 PartType;    // Partition Type
       UINT8 Target;              // Target SCSI ID
       UINT8 Lun;          // Target LUN
       UINT8 Type;         // Device Type
       UINT8 Removable;    // Removable Device
       UINT64      Lba;          // Number Of Blocks
       UINT64 Blksz;        // Block Size
       INT32           Log2blksz;    // for Convenience: Log2(Blksz)
       CHAR8          Vendor [40+1];       // IDE Model, SCSI Vendor
       CHAR8          Product[20+1];       // IDE Serial No, SCSI Product
       CHAR8          Revision[8+1];       // Firmware Revision
       UINT32 (*BlockRead)(UINT32 Start, UINT32 Blkcnt, VOID *Buffer);
       UINT32 (*BlockWrite)(UINT32 Start, UINT32 Blkcnt, CONST VOID *Buffer);
       UINT32 (*BlockErase)(UINT32 Start, UINT32 Blkcnt);
       VOID          *Priv;        // Driver Private struct Pointer
}BlockDevDescT;

struct Mmc {
	UINTN Signature;
       LIST_ENTRY Link;
       struct MmcConfig *Cfg;      // Provided Configuration
       UINT32 Version;
       VOID   *Priv;
       UINT32 HasInit;
       INT32  HighCapacity;
       UINT32 BusWidth;
       UINT32 Clock;
       UINT32 CardCaps;
       UINT32 Ocr;
       UINT32 Dsr;
       UINT32 DsrImp;
       UINT32 Scr[2];
       UINT32 Csd[4];
       UINT32 Cid[4];
       UINT16 Rca;
       CHAR8  PartConfig;
       CHAR8  PartNum;
       UINT32 TranSpeed;
       UINT32 ReadBlLen;
       UINT32 WriteBlLen;
       UINT32 EraseGrpSize;
       UINT64 Capacity;
       UINT64 CapacityUser;
       UINT64 CapacityBoot;
       UINT64 CapacityRpmb;
       UINT64 CapacityGp[4];
       BlockDevDescT BlockDev;
       CHAR8  OpCondPending;       // 1 if We Are Waiting On An OpCond Command
       CHAR8  InitInProgress;      // 1 if We Have Done MmcStartInit()
       CHAR8  Preinit;        // Start Init As Early As Possible
	INT32  DdrMode;
};

struct MmcOps {
       EFI_STATUS (*SendCmd)(struct Mmc *Mmc,
                     struct MmcCmd *Cmd, struct MmcData *Data);
       VOID (*SetIos)(struct Mmc *Mmc);
       INT32 (*Init)(struct Mmc *Mmc);
       INT32 (*Getcd)(struct Mmc *Mmc);
       INT32 (*Getwp)(struct Mmc *Mmc);
};

struct MmcConfig {
       CHAR8 *Name;
       struct MmcOps *Ops;
       UINT32 HostCaps;
       UINT32 Voltages;
       UINT32 FMin;
       UINT32 FMax;
       UINT32 BMax;
       UINT8 PartType;
};

struct FslSdxcCfg {
       VOID *    SdxcBase;
       UINT32    SdhcClk;
       UINT8     MaxBusWidth;
       struct MmcConfig Cfg;
};

struct Mmc *
MmcCreate (
  IN struct MmcConfig *Cfg,
  IN VOID *Priv
  );

VOID
MmcSetClock (
  IN  struct Mmc *Mmc,
  IN  UINT32 Clock
  );

EFI_STATUS
SdxcSendCmd (
  IN  struct Mmc *Mmc,
  IN  struct MmcCmd *Cmd,
  IN  struct MmcData *Data
  );

VOID
SdxcSetIos (
  IN  struct Mmc *Mmc
  );

INT32
SdxcInit (
  IN  struct Mmc *Mmc
  );

INT32
SdxcGetcd (
  IN  struct Mmc *Mmc
  );

EFI_STATUS
FslSdxcInitialize (
  IN struct FslSdxcCfg *Cfg
  );

INT32
SdxcMmcInit (
  );

INT32
BoardMmcInit (
  VOID
  );

EFI_STATUS
MmcInitialize (
  VOID
  );

EFI_STATUS
MmcInit (
  IN  struct Mmc *Mmc
  );

INT32
MmcGetwp (
  IN  struct Mmc *Mmc
  );

EFI_STATUS
DetectMmcPresence (
  OUT UINT8*  Data
  );

INT32
MmcGetcd (
  VOID
  );

EFI_STATUS
DoMmcInfo (
  VOID
  );

EFI_STATUS
MmcSendCommand (
  IN  struct MmcCmd *Cmd
  );

EFI_STATUS
MmcRcvResp (
  IN   UINT32   RespType,
  OUT  UINT32*  Buffer
  );

EFI_STATUS
ReceiveResponse(
  IN  struct MmcData *Data,
  IN  UINT32 RespType,
  OUT UINT32 *Response
  );

EFI_STATUS
DoMmcWrite (
  IN  VOID * InAddr,
  IN  UINT32 StartBlk,
  IN  UINT32 Count
  );

EFI_STATUS
DoMmcRead (
  OUT VOID * InAddr,
  IN  UINT32 StartBlk,
  IN  UINT32 Count
  );

EFI_STATUS
MmcSendReset (
  VOID
  );

EFI_STATUS
MmcGoIdle (
  IN  struct Mmc *Mmc
  );

EFI_STATUS
DoMmcErase (
  IN UINT32 StartBlk,
  IN UINT32 Count
  );

EFI_STATUS
InitMmc (
  IN EFI_BLOCK_IO_MEDIA *Media
  );

VOID
PrintMmcInfo (
  IN  struct Mmc *Mmc
  );

VOID
DestroyMmc (
  IN VOID
  );

VOID
SelectSdxc (
  IN VOID
  );

#endif
