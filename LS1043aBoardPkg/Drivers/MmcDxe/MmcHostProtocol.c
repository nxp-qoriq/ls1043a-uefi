#include <Drivers/Mmc.h>


extern MMC_HOST_INSTANCE       *gMmcHostInstance;

BOOLEAN
MMCIsCardPresent (
  IN EFI_MMC_HOST_PROTOCOL     *This
  )
{
  EFI_STATUS  Status;
  UINT8       Data;

  Status = DetectMmcPresence (&Data);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
MMCIsReadOnly (
  IN EFI_MMC_HOST_PROTOCOL     *This
  )
{
  /* Note:
  //FIXME return response of CMD30
  */

  return FALSE;
}


EFI_STATUS
MMCBuildDevicePath (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN EFI_DEVICE_PATH_PROTOCOL  **DevicePath
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
MMCNotifyState (
  IN EFI_MMC_HOST_PROTOCOL    *This,
  IN MMC_STATE                State
  )
{
  //EFI_STATUS              Status;

  switch(State) {
    case MmcInvalidState:
      ASSERT(0);
      break;
    case MmcHwInitializationState:
	InitMmc(gMmcHostInstance->BlockIo.Media);
	return EFI_SUCCESS;
      break;
    case MmcIdleState:
      break;
    case MmcReadyState:
      break;
    case MmcIdentificationState:
      break;
    case MmcStandByState:
     // CalculateCardCLKD (&FreqSel);
     // UpdateMMCHSClkFrequency (FreqSel);
      break;
    case MmcTransferState:
      break;
    case MmcSendingDataState:
      break;
    case MmcReceiveDataState:
      break;
    case MmcProgrammingState:
      break;
    case MmcDisconnectState:
    default:
      ASSERT(0);
  }
  return EFI_SUCCESS;
}

EFI_STATUS
MMCSendCommand (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_CMD                   MmcCmd,
  IN UINT32                    Argument
  )
{
  struct MmcCmd Cmd;
  EFI_STATUS Status;

  Cmd.CmdIdx = MmcCmd;
  Cmd.RespType = 0xFF;
  Cmd.CmdArg = Argument;

  Status = MmcSendCommand(&Cmd);

  return Status;
}


EFI_STATUS
MMCReceiveResponse (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_RESPONSE_TYPE         Type,
  IN UINT32*                   Buffer
  )
{
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  return MmcRcvResp(Type, Buffer);
}

EFI_STATUS
MMCReadBlockData (
  IN EFI_MMC_HOST_PROTOCOL      *This,
  IN EFI_LBA                    Lba,
  IN UINTN                      Length,
  IN UINT32*                    Buffer
  )
{
  DEBUG ((DEBUG_BLKIO, "MMCReadBlockData(LBA: 0x%x, Length: 0x%x, Buffer: 0x%x)\n", Lba, Length, Buffer));

  return EFI_SUCCESS;
}

EFI_STATUS
MMCWriteBlockData (
  IN EFI_MMC_HOST_PROTOCOL    *This,
  IN EFI_LBA                  Lba,
  IN UINTN                    Length,
  IN UINT32*                  Buffer
  )
{
  return EFI_SUCCESS;
}

