#include <Library/Sdxc.h>

extern struct Mmc *gMmc;

EFI_STATUS
DetectMmcPresence (
  OUT UINT8*  Data
  )
{
  if (MmcGetcd() == 0) {
    DEBUG((EFI_D_ERROR, "MMC, No Card Present\n"));
    *Data = FALSE;
    return EFI_NO_MEDIA;
  } else {
    *Data = TRUE;
    return EFI_SUCCESS;
  }
}

EFI_STATUS
MmcRcvResp (
  IN   UINT32   RespType,
  OUT  UINT32*  Buffer
  )
{
  EFI_STATUS Err;

  Err = ReceiveResponse(NULL, RespType, Buffer);

  return Err;
}

EFI_STATUS
MmcSendCommand (
  IN  struct MmcCmd *Cmd
  )
{
  return SdxcSendCmd(gMmc, Cmd, NULL);
}

EFI_STATUS
MmcSendReset (
  VOID
  )
{
  return MmcGoIdle(gMmc);
}

EFI_STATUS
InitMmc (
  IN EFI_BLOCK_IO_MEDIA *Media
  )
{
  EFI_STATUS Status;

  Status = DoMmcInfo();

  if (Status == EFI_SUCCESS) {
    Media->BlockSize = gMmc->BlockDev.Blksz;
    Media->LastBlock = gMmc->BlockDev.Lba;
  } else
    DEBUG((EFI_D_ERROR, "Failed to start MMC\n"));

  return Status;
}

VOID
DestroyMmc (
  IN VOID
  )
{
  if (gMmc) {
    FreePool(gMmc->Cfg);
    FreePool(gMmc->Priv);
    FreePool(gMmc);
  }
}

VOID
SelectSdxc (
  IN VOID
  )
{
  UINT8 Data = 0;

  /* Enable soft mux */
  Data = CPLD_READ(soft_mux_on);
  DEBUG((EFI_D_INFO,"soft_mux_on 0x%x\n", Data));
  if ((Data & (ENABLE_SDXC_SOFT_MUX | ENABLE_RCW_SOFT_MUX)) 
	!= (ENABLE_SDXC_SOFT_MUX | ENABLE_RCW_SOFT_MUX))
    CPLD_WRITE(soft_mux_on, (Data | (ENABLE_SDXC_SOFT_MUX |
					ENABLE_RCW_SOFT_MUX)));
  else
    DEBUG((EFI_D_INFO, "Enabled\n"));

  Data = CPLD_READ(soft_mux_on);
  DEBUG((EFI_D_INFO,"soft_mux_on 0x%x\n", Data));

  /* Enable sdhc */
  Data = CPLD_READ(sdhc_spics_sel);
  DEBUG((EFI_D_INFO,"sdhc_spics_sel 0x%x\n", Data));
  if ((Data & 0x01) == 0x00)
    DEBUG((EFI_D_INFO,"SDXC selected\n"));
  else
    CPLD_WRITE(sdhc_spics_sel, (Data & 0xFE));

  Data = CPLD_READ(sdhc_spics_sel);
  DEBUG((EFI_D_INFO,"sdhc_spics_sel 0x%x\n", Data));

  /* Enable sdhc clock */
  Data = CPLD_READ(tdmclk_mux_sel);
  DEBUG((EFI_D_INFO,"tdmclk_mux_sel 0x%x\n", Data));
  if ((Data & 0x01) == 0x01)
    DEBUG((EFI_D_INFO,"SDXC CLK selected\n"));
  else
    CPLD_WRITE(tdmclk_mux_sel, (Data | 0x01));

  Data = CPLD_READ(tdmclk_mux_sel);
  DEBUG((EFI_D_INFO,"tdmclk_mux_sel 0x%x\n", Data));

  /* configure SW4 and SW5 for SDXC/eMMC */
  Data = CPLD_READ(cfg_rcw_src1);
  DEBUG((EFI_D_INFO,"cfg_rcw_src1 0x%x\n", Data));
  if ((Data & SELECT_SW4_SDXC) != SELECT_SW4_SDXC)
    CPLD_WRITE(cfg_rcw_src1, SELECT_SW4_SDXC);
  else
    DEBUG((EFI_D_INFO, "Enabled 2\n"));

  Data = CPLD_READ(cfg_rcw_src1);
  DEBUG((EFI_D_INFO,"cfg_rcw_src1 0x%x\n", Data));

  Data = CPLD_READ(cfg_rcw_src2);
  DEBUG((EFI_D_INFO,"cfg_rcw_src2 0x%x\n", Data));
  if ((Data & SELECT_SW5_SDXC) != SELECT_SW5_SDXC)
    CPLD_WRITE(cfg_rcw_src2, SELECT_SW5_SDXC);
  else
    DEBUG((EFI_D_INFO, "Enabled 3\n"));

  Data = CPLD_READ(cfg_rcw_src2);
  DEBUG((EFI_D_INFO,"cfg_rcw_src2 0x%x\n", Data));
}

EFI_STATUS
MmcInitialize (
  VOID
  )
{
  SelectSdxc();

  if (BoardMmcInit() < 0)
    if (SdxcMmcInit() < 0)
      return EFI_DEVICE_ERROR;

  return EFI_SUCCESS;
}
