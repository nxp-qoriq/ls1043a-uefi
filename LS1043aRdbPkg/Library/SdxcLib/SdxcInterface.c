/** SdxcInterface.C
  Functions to provide Read, Write, Erase , Initialize etc APIs. 

  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This Program And The Accompanying Materials
  Are Licensed And Made Available Under The Terms And Conditions Of The BSD License
  Which Accompanies This Distribution.  The Full Text Of The License May Be Found At
  Http://Opensource.Org/Licenses/Bsd-License.Php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/Sdxc.h>

extern struct Mmc *gMmc;

UINT32
__Div64_32 (
  IN   UINT64		*n,
  IN   UINT32		Base
  )
{
  UINT64 Rem = *n;
  UINT64 B = Base;
  UINT64 Res, D = 1;
  UINT32 High = Rem >> 32;

  /** Reduce The Thing A Bit First */
  Res = 0;
  if (High >= Base) {
    High /= Base;
    Res = (UINT64) High << 32;
    Rem -= (UINT64) (High*Base) << 32;
  }

  while ((UINTN)B > 0 && B < Rem) {
    B = B+B;
    D = D+D;
  }

  do {
    if (Rem >= B) {
      Rem -= B;
      Res += D;
    }
    B >>= 1;
    D >>= 1;
  } while (D);

  *n = Res;
  return Rem;
}

static struct Mmc *
InitDevice (
  IN  BOOLEAN FInit
  )
{
  EFI_STATUS Status;
  struct Mmc *Mmc = gMmc;

  if (FInit)
    Mmc->HasInit = 0;

  Status = MmcInit(Mmc);
  if (Status != EFI_SUCCESS)
    return NULL;

  return Mmc;
}

/*
 * PrINT32 Sizes As "Xxx KiB", "Xxx.Y KiB", "Xxx MiB", "Xxx.Y MiB",
 * Xxx GiB, Xxx.Y GiB, Etc As Needed; Allow for Optional Trailing String
 * (Like "\n")
 */
VOID
PrintSizeSdxc (
  IN  UINT64 Size
  )
{
  UINT64 M = 0, n;
  UINT64 F;
  static CONST INT8 Names[] = {'E', 'P', 'T', 'G', 'M', 'K'};
  UINT64 D = 10 * ARRAY_SIZE(Names);
  CHAR8 C = 0;
  UINT32 I;

  for (I = 0; I < ARRAY_SIZE(Names); I++, D -= 10) {
    if (Size >> D) {
      C = Names[I];
      break;
    }
  }

  if (!C) {
    DEBUG((EFI_D_RELEASE, "%Ld Bytes", Size));
    return;
  }

  n = Size >> D;
  F = Size & ((1ULL << D) - 1);

  /* if There'S A Remainder, Deal With It */
  if (F) {
    M = (10ULL * F + (1ULL << (D - 1))) >> D;

    if (M >= 10) {
           M -= 10;
           n += 1;
    }
  }

  DEBUG((EFI_D_RELEASE, "%Ld", n));
  if (M) {
    DEBUG((EFI_D_RELEASE, ".%Ld", M));
  }
  DEBUG((EFI_D_RELEASE, " %ciB", C));
}

VOID
PrintSdxcInfo (
  IN  struct Mmc *Mmc
  )
{
  DEBUG((EFI_D_RELEASE, "Device: %a\n", Mmc->Cfg->Name));
  DEBUG((EFI_D_RELEASE, "Manufacturer ID: %x\n", Mmc->Cid[0] >> 24));
  DEBUG((EFI_D_RELEASE, "OEM: %x\n", (Mmc->Cid[0] >> 8) & 0xffff));
  DEBUG((EFI_D_RELEASE, "Name: %c%c%c%c%c \n", Mmc->Cid[0] & 0xff,
		(Mmc->Cid[1] >> 24), (Mmc->Cid[1] >> 16) & 0xff,
		(Mmc->Cid[1] >> 8) & 0xff, Mmc->Cid[1] & 0xff));

  DEBUG((EFI_D_RELEASE, "Card: %x\n", (Mmc->Cid[0] >> 16) & 0x03));
  DEBUG((EFI_D_RELEASE, "Product revision: %d.%d\n", (Mmc->Cid[2] >> 20) & 0xf,
                                                  (Mmc->Cid[2] >> 16) & 0xf));
  DEBUG((EFI_D_RELEASE, "Product Serial No. : %04x%04x\n",
                                 (Mmc->Cid[2]) & 0xffff,
                                 (Mmc->Cid[3] >> 16) & 0xffff));
  DEBUG((EFI_D_RELEASE, "Manufacturing date : %d:%d\n",
                                 (Mmc->Cid[3] >> 12) & 0xf,
                                 ((Mmc->Cid[3] >> 8) & 0xf) + 1997));

  DEBUG((EFI_D_RELEASE, "Tran Speed: %d\n", Mmc->TranSpeed));
  DEBUG((EFI_D_RELEASE, "Rd Block Len: %d\n", Mmc->ReadBlkLen));

  DEBUG((EFI_D_RELEASE, "%a Version %d.%d\n", IS_SD(Mmc) ? "SD" : "MMC",
		(Mmc->Version >> 8) & 0xf, Mmc->Version & 0xff));

  DEBUG((EFI_D_RELEASE, "High Capacity: %a\n", Mmc->HighCapacity ? "Yes" : "No"));
  DEBUG((EFI_D_RELEASE, "Capacity: "));
  PrintSizeSdxc(Mmc->Capacity);

  DEBUG((EFI_D_RELEASE, "\nBus Width: %d-Bit%a\n", Mmc->BusWidth,
		Mmc->DdrMode ? " DDR" : ""));
}

EFI_STATUS
DoMmcInfo (
  VOID
  )
{
  struct Mmc *Mmc;

  SelectSdxc();

  Mmc = InitDevice(FALSE);
  if (!Mmc)
    return EFI_NO_MAPPING;

  PrintSdxcInfo(Mmc);

  return EFI_SUCCESS;
}

EFI_STATUS
DoMmcRead (
  OUT  VOID * InAddr,
  IN  UINT32 StartBlk,
  IN  UINT32 Count
  )
{
  UINT32 BlocksRead;

  DEBUG((EFI_D_INFO, "MMC Read: Block # %d, Count %d ...\n", StartBlk, Count));

  SelectSdxc();

  BlocksRead = gMmc->BlockDev.BlkRead(StartBlk, Count, InAddr);
  /* Flush Cache After Read */
  // TODO FlushCache((Ulong)InAddr, Count * 512); /* FIXME */
  DEBUG((EFI_D_INFO, "%d Blocks Read: %a\n",
	BlocksRead, (BlocksRead == Count) ? "OK" : "ERROR"));

  return (BlocksRead == Count) ? EFI_SUCCESS : EFI_TIMEOUT;
}

EFI_STATUS
DoMmcWrite (
  IN  VOID * InAddr,
  IN  UINT32 StartBlk,
  IN  UINT32 Count
  )
{
  UINT32 BlkWrtn;

  DEBUG((EFI_D_INFO, "MMC Write: Block # %d, Count %d ... \n", StartBlk, Count));

  SelectSdxc();

  if (MmcGetwp(gMmc) == 1) {
    DEBUG((EFI_D_ERROR, "Error: Card Is Write Protected!\n"));
    return EFI_WRITE_PROTECTED;
  }
  BlkWrtn = gMmc->BlockDev.BlkWrite(StartBlk, Count, InAddr);
  DEBUG((EFI_D_INFO, "%d Blocks Written: %a\n",
	BlkWrtn, (BlkWrtn == Count) ? "OK" : "ERROR"));

  return (BlkWrtn == Count) ? EFI_SUCCESS : EFI_TIMEOUT;
}

EFI_STATUS
DoMmcErase (
  IN UINT32 StartBlk,
  IN UINT32 Count
  )
{
  UINT32 BlkErsd;

  DEBUG((EFI_D_INFO, "MMC Erase: Block # %d, Count %d\n", StartBlk, Count));

  SelectSdxc();

  if (MmcGetwp(gMmc) == 1) {
     DEBUG((EFI_D_ERROR, "Error: Card Is Write Protected!\n"));
     return EFI_WRITE_PROTECTED;
  }

  BlkErsd = gMmc->BlockDev.BlkErase(StartBlk, Count);
  DEBUG((EFI_D_INFO, "%d Blocks Erased: %a\n",
	BlkErsd, (BlkErsd == Count) ? "OK" : "ERROR"));

  return (BlkErsd == Count) ? EFI_SUCCESS : EFI_TIMEOUT;
}

VOID
CreateBootStruct (
  IN struct SdxcBoot *Boot
  )
{
  struct SdxcCfg *Cfg = gMmc->Private;

  Boot->SdxcBase = (struct SdxcRegs *)Cfg->SdxcBase ;
  Boot->BMax = gMmc->Cfg->BMax ;
  Boot->Lba = gMmc->BlockDev.Lba ;
  Boot->Clock = gMmc->Clock ;
  Boot->Rca = gMmc->Rca ;
  Boot->HighCapacity = gMmc->HighCapacity ;
  Boot->ReadBlkLen = gMmc->ReadBlkLen ;
}
