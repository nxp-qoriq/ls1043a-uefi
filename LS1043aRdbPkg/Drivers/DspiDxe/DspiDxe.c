/** @DspiDxe.c
#
#  Driver for installing BlockIo protocol over DSPI
#
#  Copyright (c) 2015, Freescale Ltd. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD
#  License which accompanies this distribution. The full text of the license
#  may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
**/

#include <Library/Dspi.h>
#include <Library/LS1043aFileSystem.h>

EFI_BLOCK_IO_MEDIA gDspiMedia = {
  SIGNATURE_32('d', 's', 'p', 'i'),         // MediaId
  FALSE,                                    // RemovableMedia
  TRUE,                                     // MediaPresent
  FALSE,                                    // LogicalPartition
  FALSE,                                    // ReadOnly
  FALSE,                                    // WriteCaching
  0,                                        // BlockSize
  2,                                        // IoAlign
  0,                                        // Pad
  0                                         // LastBlock
};

// Device path for SemiHosting. It contains our autogenerated Caller ID GUID.
typedef struct {

  VENDOR_DEVICE_PATH        Guid;

  EFI_DEVICE_PATH_PROTOCOL  End;

  } FLASH_DEVICE_PATH;

FLASH_DEVICE_PATH gDevicePath = {
    {
      { HARDWARE_DEVICE_PATH, HW_VENDOR_DP,
        { sizeof (VENDOR_DEVICE_PATH), 0 } 
      },
      EFI_CALLER_ID_GUID
    },
    { END_DEVICE_PATH_TYPE,
      END_ENTIRE_DEVICE_PATH_SUBTYPE,
      { sizeof (EFI_DEVICE_PATH_PROTOCOL), 0}
    }
  };

EFI_EVENT EfiExitBootServicesEvent      = (EFI_EVENT)NULL;

EFI_STATUS
EFIAPI
DspiReset (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN BOOLEAN                        ExtendedVerification
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DspiReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  OUT VOID                          *Buffer
  )
{
  return DspiRead(This, MediaId, Lba, BufferSize, Buffer);
}

EFI_STATUS
EFIAPI
DspiWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  IN VOID                           *Buffer
  )
{
  return DspiWrite(This, MediaId, Lba, BufferSize, Buffer);
}

EFI_STATUS
EFIAPI
DspiFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
{
  return EFI_SUCCESS;
}

EFI_BLOCK_IO_PROTOCOL BlockIo =
{
  EFI_BLOCK_IO_INTERFACE_REVISION,	// Revision
  &gDspiMedia,			// *Media
  DspiReset,				// Reset
  DspiReadBlocks,			// ReadBlocks
  DspiWriteBlocks,			// WriteBlocks
  DspiFlushBlocks			// FlushBlocks
};

/**
  Shutdown our hardware

  @param[in]  Event   	The Event that is being processed
  @param[in]  Context 	Event Context
**/
VOID
EFIAPI
DspiExitBootServicesEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_HANDLE			ImageHandle = NULL;
  DspiUnload(ImageHandle);
  DspiFlashFree();
}

EFI_STATUS
DspiInitialize (
  IN EFI_HANDLE		ImageHandle,
  IN EFI_SYSTEM_TABLE	*SystemTable
  )
{
  EFI_STATUS              Status;

  Status = DspiInit(&gDspiMedia);
  if (Status != EFI_SUCCESS) {
    DEBUG((EFI_D_ERROR,"Failed to init MMC\n"));
    return Status;
  }

  //
  // Install driver model protocol(s).
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
             &ImageHandle,
             &gEfiDevicePathProtocolGuid, &gDevicePath,
             &gEfiBlockIoProtocolGuid, &BlockIo,
	      NULL
             );

  // Register for an DspiExitBootServicesEvent
  Status = gBS->CreateEvent (EVT_SIGNAL_EXIT_BOOT_SERVICES, TPL_NOTIFY,
		DspiExitBootServicesEvent, NULL, &EfiExitBootServicesEvent);

  return Status;
}

/**
  This is the unload handle for Dspi Controller module.

  Disconnect the driver specified by ImageHandle from all the devices in the
  handle database.
  Uninstall all the protocols installed in the driver entry point.

  @param[in] ImageHandle           The drivers' driver image.

  @retval    EFI_SUCCESS           The image is unloaded.
  @retval    Others                Failed to unload the image.

**/
EFI_STATUS
EFIAPI
DspiUnload (
  IN EFI_HANDLE             ImageHandle
  )
{
  EFI_STATUS				Status;
  EFI_HANDLE				*DeviceHandleBuffer;
  UINTN				DeviceHandleCount;
  UINTN				Index;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL	*FsProtocol;
  LS1043A_FILE_SYSTEM		*Instance;
  UINT32				MediaId;

  //
  // Get the list of all Dspi Controller handles in the handle database.
  // If there is an error getting the list, then the unload
  // operation fails.
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  &DeviceHandleCount,
                  &DeviceHandleBuffer
                  );

  if (!EFI_ERROR (Status)) {
    //
    // Disconnect the driver specified by Driver BindingHandle from all
    // the devices in the handle database.
    //
    for (Index = 0; Index < DeviceHandleCount; Index++) {
      Status = gBS->HandleProtocol (DeviceHandleBuffer[Index],
			&gEfiSimpleFileSystemProtocolGuid,
			(VOID **)&FsProtocol);

      if (EFI_ERROR (Status))
        goto Done;

      Instance = CR(FsProtocol, LS1043A_FILE_SYSTEM, Fs,
                LS1043A_FILE_SYSTEM_SIGNATURE);
      MediaId = Instance->BlockIo->Media->MediaId;

      if (MediaId == SIGNATURE_32('d','s','p','i')) {
        Status = gBS->DisconnectController (
                      DeviceHandleBuffer[Index],
                      NULL,
                      NULL
                      );
        if (EFI_ERROR (Status)) {
          goto Done;
	 }
      }
    }
  }

Done:
  //
  // Free the buffer containing the list of handles from the handle database
  //
  if (DeviceHandleBuffer != NULL) {
    gBS->FreePool (DeviceHandleBuffer);
  }

  return Status;
}
