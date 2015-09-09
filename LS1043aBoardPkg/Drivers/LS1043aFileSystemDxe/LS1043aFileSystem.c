/** @file
#
#  Driver for insalling SimpleFileSystem and DiskIo protocol over
#  BlockIo protocol
#
#  Copyright (c) 2014, Freescale Ltd. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
**/

#include <Library/LS1043aFileSystem.h>

LS1043A_FILE_SYSTEM gLS1043aFileSystemTemplate =
{
  LS1043A_FILE_SYSTEM_SIGNATURE,
  {
    EFI_DISK_IO_PROTOCOL_REVISION,
    DiskIoReadDisk,
    DiskIoWriteDisk
  },
  {
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION,
    OpenVolume
  },
  {
    EFI_FILE_REVISION,
    Open,
    Close,
    Delete,
    Read,
    Write,
    GetPosition,
    SetPosition,
    GetInfo,
    SetInfo,
    Flush,
    OpenEx,
    ReadEx,
    WriteEx,
    FlushEx
  },
  NULL,
  NULL
};

EFI_STATUS
EFIAPI
LS1043aFileSystemDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS            Status;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlockIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test.
  //
  gBS->CloseProtocol (
         ControllerHandle,
         &gEfiBlockIoProtocolGuid,
         This->DriverBindingHandle,
         ControllerHandle
         );
  return EFI_SUCCESS;
}

/**
  Start this driver on ControllerHandle by opening a Block IO protocol and
  installing a Disk IO protocol on ControllerHandle.

  @param  This                 Protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS		This driver is added to ControllerHandle
  @retval EFI_ALREADY_STARTED	This driver is already running on
				ControllerHandle
  @retval other			This driver does not support this device

**/
EFI_STATUS
EFIAPI
LS1043aFileSystemDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS  Status;
  LS1043A_FILE_SYSTEM *Instance;

  Instance = NULL;

  //
  // Initialize the FileSystem instance.
  //
  Instance = AllocateCopyPool (sizeof (LS1043A_FILE_SYSTEM),
		  &gLS1043aFileSystemTemplate);
  if (Instance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Connect to the Block IO interface on ControllerHandle.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &Instance->BlockIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ControllerHandle,
		  &gEfiDiskIoProtocolGuid, &Instance->DiskIo,
		  &gEfiSimpleFileSystemProtocolGuid, &Instance->Fs,
                  NULL
                  );
  return Status;
}

/**
  Stop this driver on ControllerHandle by removing Disk IO protocol and closing
  the Block IO protocol on ControllerHandle.

  @param  This              Protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
LS1043aFileSystemDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
{
  EFI_STATUS            Status;
  LS1043A_FILE_SYSTEM 	*Instance;
  EFI_DISK_IO_PROTOCOL  *DiskIo;

  //
  // Get our context back.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDiskIoProtocolGuid,
                  (VOID **) &DiskIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Instance = CR(DiskIo, LS1043A_FILE_SYSTEM, DiskIo,
		  LS1043A_FILE_SYSTEM_SIGNATURE);

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  &ControllerHandle,
		  &gEfiDiskIoProtocolGuid, &Instance->DiskIo,
		  &gEfiSimpleFileSystemProtocolGuid, &Instance->Fs,
                  NULL
                  );
  FreePool(Instance);
  return Status;
}

EFI_DRIVER_BINDING_PROTOCOL gLS1043aFileSystemDriverBinding = {
  LS1043aFileSystemDriverBindingSupported,
  LS1043aFileSystemDriverBindingStart,
  LS1043aFileSystemDriverBindingStop,
  0xa,
  NULL,
  NULL
};

EFI_STATUS LS1043aFileSystemInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
)
{
  EFI_STATUS Status;
  gLS1043aFileSystemDriverBinding.DriverBindingHandle = ImageHandle;
  gLS1043aFileSystemDriverBinding.ImageHandle = ImageHandle;

  Status = gBS->InstallMultipleProtocolInterfaces (
	     &ImageHandle,
	     &gEfiDriverBindingProtocolGuid, &gLS1043aFileSystemDriverBinding,
	     NULL
	);
  return Status;
}

