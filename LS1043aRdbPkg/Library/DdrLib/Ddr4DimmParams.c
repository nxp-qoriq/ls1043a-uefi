/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * Calculate The Organization And Timing Parameter
 * From Ddr3 Spd, Please Refer To The Spec
 * JEDEC Standard No.21-C 4_01_02_12R23A.Pdf
 *
 *
 */

#include <Common.h>
#include <Ddr.h>

/*
 * Calculate The Density Of Each Physical Rank.
 * Returned Size Is In Bytes.
 *
 * Total DIMM Size =
 * Sdram Capacity(Bit) / 8 * Primary Bus Width / Sdram Width
 *                     * Logical Ranks Per DIMM
 *
 * Where: Sdram Capacity  = Spd Byte4[3:0]
 *        Primary Bus Width = Spd Byte13[2:0]
 *        Sdram Width = Spd Byte12[2:0]
 *        Logical Ranks Per DIMM = Spd Byte12[5:3] for SDP, DDP, QDP
 *                                 Spd Byte12{5:3] * Spd Byte6[6:4] for 3DS
 *
 * To Simplify Each Rank Size = Total DIMM Size / Number Of Package Ranks
 * Where Number Of Package Ranks = Spd Byte12[5:3]
 *
 * SPD Byte4 - Sdram Density And Banks
 *	Bit[3:0]	Size(Bit)	Size(Byte)
 *	0000		256Mb		32MB
 *	0001		512Mb		64MB
 *	0010		1Gb		128MB
 *	0011		2Gb		256MB
 *	0100		4Gb		512MB
 *	0101		8Gb		1GB
 *	0110		16Gb		2GB
 *      0111		32Gb		4GB
 *
 * SPD Byte13 - Module Memory Bus Width
 *	Bit[2:0]	Primary Bus Width
 *	000		8bits
 *	001		16bits
 *	010		32bits
 *	011		64bits
 *
 * SPD Byte12 - Module Organization
 *	Bit[2:0]	Sdram Device Width
 *	000		4bits
 *	001		8bits
 *	010		16bits
 *	011		32bits
 *
 * SPD Byte12 - Module Organization
 *	Bit[5:3]	Number Of Package Ranks Per DIMM
 *	000		1
 *	001		2
 *	010		3
 *	011		4
 *
 * SPD Byte6 - SDRAM Package Type
 *	Bit[6:4]	Die Count
 *	000		1
 *	001		2
 *	010		3
 *	011		4
 *	100		5
 *	101		6
 *	110		7
 *	111		8
 *
 * SPD Byte6 - SRAM Package Type
 *	Bit[1:0]	Signal Loading
 *	00		Not Specified
 *	01		Multi Load Stack
 *	10		Sigle Load Stack (3DS)
 *	11		Reserved
 */
static UINT64
ComputeRanksize  (
  IN  CONST struct Ddr4SpdEepromS *Spd
  )
{
  UINT64 Bsize;

  INT32 NbitSdramCapBsize = 0;
  INT32 NbitPrimaryBusWidth = 0;
  INT32 NbitSdramWidth = 0;
  INT32 DieCount = 0;
  BOOLEAN Package_3ds;

  if ((Spd->DensityBanks & 0xf) <= 7)
    NbitSdramCapBsize = (Spd->DensityBanks & 0xf) + 28;
  if ((Spd->BusWidth & 0x7) < 4)
    NbitPrimaryBusWidth = (Spd->BusWidth & 0x7) + 3;
  if ((Spd->Organization & 0x7) < 4)
    NbitSdramWidth = (Spd->Organization & 0x7) + 2;
    Package_3ds = (Spd->PackageType & 0x3) == 0x2;
  if (Package_3ds)
    DieCount = (Spd->PackageType >> 4) & 0x7;

  Bsize = 1ULL << (NbitSdramCapBsize - 3 +
	 NbitPrimaryBusWidth - NbitSdramWidth +
	 DieCount);

  DEBUG((EFI_D_INFO,"DDR: DDR III Rank Density = 0x%16llx\n", Bsize));

  return Bsize;
}

#define SpdToPs(Mtb, Ftb)	\
	(Mtb * Pdimm->MtbPs + (Ftb * Pdimm->Ftb10thPs) / 10)
/*
  Ddr4ComputeDimmParameters for DDR4 SPD
 
  Compute DIMM Parameters Based Upon The SPD Information In Spd.
  Writes The Results To The DimmParamsT Structure Pointed By Pdimm.
 
  DdrComputeDimmParameters for DDR3 SPD

  Compute DIMM Parameters Based Upon The SPD Information In Spd.
  Writes The Results To The DimmParamsT Structure Pointed By Pdimm.

  @Param   Spd		Pointer To SPD Parameters
  @Param   DimmNumber	Dimm Number

  @Retval  Dimm Parameters In Pdimm Pointer
 **/

UINT32
Ddr4ComputeDimmParameters (
  IN  CONST GenericSpdEepromT 	*Spd,
  OUT DimmParamsT 			*Pdimm,
  IN  UINT32 				DimmNumber
  )
{
  UINT32 Retval;
  INT32 I;
  CONST UINT8 UdimmRcEDq[18] = {
	0x0c, 0x2c, 0x15, 0x35, 0x15, 0x35, 0x0b, 0x2c, 0x15,
	0x35, 0x0b, 0x35, 0x0b, 0x2c, 0x0b, 0x35, 0x15, 0x36
  };
  INT32 SpdError = 0;
  UINT8 *Ptr;

  if (Spd->MemType) {
    if (Spd->MemType != SPD_MEMTYPE_DDR4) {
      DEBUG((EFI_D_ERROR,"DIMM %d: Is Not a DDR4 SPD.\n",
           DimmNumber));
      return 1;
    }
  } else {
    InternalMemZeroMem(Pdimm, sizeof(DimmParamsT));
    return 1;
  }

  Retval = Ddr4SpdCheck(Spd);
  if (Retval != 0) {
	DEBUG((EFI_D_ERROR,"DIMM %d: Failed Checksum\n", DimmNumber));
	return 2;
  }

  /*
   * The Part Name In ASCII In The SPD EEPROM Is Not Null Terminated.
   * Guarantee Null Termination Here By Presetting All Bytes To 0
   * And Copying The Part Name In ASCII From The SPD Onto It
   */
  InternalMemZeroMem(Pdimm->Mpart, sizeof(Pdimm->Mpart));
  if ((Spd->InfoSizeCrc & 0xF) > 2)
    InternalMemCopyMem(Pdimm->Mpart, Spd->Mpart, sizeof(Pdimm->Mpart) - 1);

  /* DIMM Organization Parameters */
  Pdimm->NRanks = ((Spd->Organization >> 3) & 0x7) + 1;
  Pdimm->RankDensity = ComputeRanksize(Spd);
  Pdimm->Capacity = Pdimm->NRanks * Pdimm->RankDensity;
  Pdimm->PrimarySdramWidth = 1 << (3 + (Spd->BusWidth & 0x7));
  if ((Spd->BusWidth >> 3) & 0x3)
    Pdimm->EcSdramWidth = 8;
  else
    Pdimm->EcSdramWidth = 0;

  Pdimm->DataWidth = Pdimm->PrimarySdramWidth
		  + Pdimm->EcSdramWidth;
  Pdimm->DeviceWidth = 1 << ((Spd->Organization & 0x7) + 2);

  /* These Are The Types defined By The JEDEC SPD Spec */
  Pdimm->MirroredDimm = 0;
  Pdimm->RegisteredDimm = 0;
  switch (Spd->ModuleType & DDR4_SPD_MODULETYPE_MASK) {
    case DDR4_SPD_MODULETYPE_RDIMM:
      /* Registered/Buffered DIMMs */
      Pdimm->RegisteredDimm = 1;
      break;

    case DDR4_SPD_MODULETYPE_UDIMM:
    case DDR4_SPD_MODULETYPE_SO_DIMM:
      /* Unbuffered DIMMs */
      if (Spd->ModSection.Unbuffered.AddrMapping & 0x1)
     	 Pdimm->MirroredDimm = 1;
      if ((Spd->ModSection.Unbuffered.ModHeight & 0xe0) == 0 &&
         (Spd->ModSection.Unbuffered.RefRawCard == 0x04)) {
	 /* Fix SPD Error Found On DIMMs With Raw Card E0 */
	 for (I = 0; I < 18; I++) {
	   if (Spd->Mapping[I] == UdimmRcEDq[I])
	     continue;
	   SpdError = 1;
	   DEBUG((EFI_D_ERROR, "SPD Byte %d: 0x%x, Should Be 0x%x\n",
			      60 + I, Spd->Mapping[I],
			      UdimmRcEDq[I]));
	   Ptr = (UINT8 *)&Spd->Mapping[I];
	   *Ptr = UdimmRcEDq[I];
	 }
	 if (SpdError)
	   DEBUG((EFI_D_INFO,"SPD DQ Mapping Error Fixed\n"));
      }
      break;

    default:
      DEBUG((EFI_D_ERROR,"Unknown ModuleType 0x%02X\n", Spd->ModuleType));
      return 1;
  }

  /* SDRAM Device Parameters */
  Pdimm->NRowAddr = ((Spd->Addressing >> 3) & 0x7) + 12;
  Pdimm->NColAddr = (Spd->Addressing & 0x7) + 9;
  Pdimm->BankAddrBits = (Spd->DensityBanks >> 4) & 0x3;
  Pdimm->BankGroupBits = (Spd->DensityBanks >> 6) & 0x3;

  /*
   * The SPD Spec Has Not The ECC Bit,
   * We Consider The DIMM As ECC Capability
   * When The Extension Bus Exist
   */
  if (Pdimm->EcSdramWidth)
    Pdimm->EdcConfig = 0x02;
  else
    Pdimm->EdcConfig = 0x00;

  /*
   * The SPD Spec Has Not The Burst Length Byte
   * But DDR4 Spec Has Nature BL8 And BC4,
   * BL8 -Bit3, BC4 -Bit2
   */
  Pdimm->BurstLengthsBitmask = 0x0c;
  Pdimm->RowDensity = __ILog2(Pdimm->RankDensity);

  /* MTB - Medium Timebase
   * The MTB In The SPD Spec Is 125ps,
   *
   * FTB - Fine Timebase
   * Use 1/10th Of Ps As Our Unit To Avoid Floating Point
   * Eg, 10 for 1ps, 25 for 2.5ps, 50 for 5ps
   */
  if ((Spd->Timebases & 0xf) == 0x0) {
    Pdimm->MtbPs = 125;
    Pdimm->Ftb10thPs = 10;
  } else {
    DEBUG((EFI_D_ERROR,"Unknown Timebases\n"));
  }

  /* Sdram Minimum Cycle Time */
  Pdimm->TckminXPs = SpdToPs(Spd->TckMin, Spd->FineTckMin);

  /* Sdram Max Cycle Time */
  Pdimm->TckmaxPs = SpdToPs(Spd->TckMax, Spd->FineTckMax);

  /*
   * CAS Latency Supported
   * Bit0 - CL7
   * Bit4 - CL11
   * Bit8 - CL15
   * Bit12- CL19
   * Bit16- CL23
   */
  Pdimm->CaslatX  = (Spd->CaslatB1 << 7)	|
		   (Spd->CaslatB2 << 15)	|
		   (Spd->CaslatB3 << 23);

  ASSERT(Spd->CaslatB4 != 0);

  /*
   * Min CAS Latency Time
   */
  Pdimm->TaaPs = SpdToPs(Spd->TaaMin, Spd->FineTaaMin);

  /*
   * Min RAS To CAS Delay Time
   */
  Pdimm->TrcdPs = SpdToPs(Spd->TrcdMin, Spd->FineTrcdMin);

  /*
   * Min Row Precharge Delay Time
   */
  Pdimm->TrpPs = SpdToPs(Spd->TrpMin, Spd->FineTrpMin);

  /* Min Active To Precharge Delay Time */
  Pdimm->TrasPs = (((Spd->TrasTrcExt & 0xf) << 8) +
		  Spd->TrasMinLsb) * Pdimm->MtbPs;

  /* Min Active To Actice/Refresh Delay Time */
  Pdimm->TrcPs = SpdToPs((((Spd->TrasTrcExt & 0xf0) << 4) +
			   Spd->TrcMinLsb), Spd->FineTrcMin);
  /* Min Refresh Recovery Delay Time */
  Pdimm->Trfc1Ps = ((Spd->Trfc1MinMsb << 8) | (Spd->Trfc1MinLsb)) *
		Pdimm->MtbPs;
  Pdimm->Trfc2Ps = ((Spd->Trfc2MinMsb << 8) | (Spd->Trfc2MinLsb)) *
		Pdimm->MtbPs;
  Pdimm->Trfc4Ps = ((Spd->Trfc4MinMsb << 8) | (Spd->Trfc4MinLsb)) *
		Pdimm->MtbPs;
  /* Min Four Active Window Delay Time */
  Pdimm->TfawPs = (((Spd->TfawMsb & 0xf) << 8) | Spd->TfawMin) *
		Pdimm->MtbPs;

  /* Min Row Active To Row Active Delay Time, Different Bank Group */
  Pdimm->TrrdsPs = SpdToPs(Spd->TrrdsMin, Spd->FineTrrdsMin);
  /* Min Row Active To Row Active Delay Time, Same Bank Group */
  Pdimm->TrrdlPs = SpdToPs(Spd->TrrdlMin, Spd->FineTrrdlMin);
  /* Min CAS To CAS Delay Time (TCCD_Lmin), Same Bank Group */
  Pdimm->TccdlPs = SpdToPs(Spd->TccdlMin, Spd->FineTccdlMin);

  /*
   * Average Periodic Refresh Interval
   * TREFI = 7.8 Us At Normal Temperature Range
   */
  Pdimm->RefreshRatePs = 7800000;

  for (I = 0; I < 18; I++)
    Pdimm->DqMapping[I] = Spd->Mapping[I];

  Pdimm->DqMappingOrs = ((Spd->Mapping[0] >> 6) & 0x3) == 0 ? 1 : 0;

  return 0;
}
