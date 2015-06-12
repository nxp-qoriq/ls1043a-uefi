/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 *
 * This Program Is Free Software; You Can Redistribute It And/Or
 * Modify It Under The Terms Of The GNU General Public License
 * Version 2 As Published By The Free Software Foundation.
 */

#include <Ddr.h>

UINT32
Ddr4SpdCheck (
  IN  CONST Ddr4SpdEepromT *Spd
  )
{
       UINT8 *P = (UINT8 *)Spd;
       INT32 Csum16;
       INT32 Len;
       UINT8 CrcLsb; /* Byte 126 */
       UINT8 CrcMsb; /* Byte 127 */

       Len = 126;
       Csum16 = Crc16(P, Len);

       CrcLsb = (UINT8) (Csum16 & 0xff);
       CrcMsb = (UINT8) (Csum16 >> 8);

       if (Spd->Crc[0] != CrcLsb || Spd->Crc[1] != CrcMsb) {
              DEBUG((EFI_D_ERROR, "SPD Checksum Unexpected.\n"
                     "Checksum Lsb In SPD = %02x, Computed SPD = %02x\n"
                     "Checksum Msb In SPD = %02x, Computed SPD = %02x\n",
                     Spd->Crc[0], CrcLsb, Spd->Crc[1], CrcMsb));
              return 1;
       }

       P = (UINT8 *)((UINT64)Spd + 128);
       Len = 126;
       Csum16 = Crc16(P, Len);

       CrcLsb = (UINT8) (Csum16 & 0xff);
       CrcMsb = (UINT8) (Csum16 >> 8);

       if (Spd->ModSection.Uc[126] != CrcLsb ||
           Spd->ModSection.Uc[127] != CrcMsb) {
              DEBUG((EFI_D_ERROR, "SPD Checksum Unexpected.\n"
                     "Checksum Lsb In SPD = %02X, Computed SPD = %02X\n"
                     "Checksum Msb In SPD = %02X, Computed SPD = %02X\n",
                     Spd->ModSection.Uc[126],
                     CrcLsb, Spd->ModSection.Uc[127],
                     CrcMsb));
              return 1;

       }

       return 0;
}
