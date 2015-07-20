/*++

Copyright (c) 2005 - 2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the Software
License Agreement which accompanies this distribution.


Module Name:

  Init.c

Abstract:

  Initialization routines

--*/

#include "Fat.h"

EFI_STATUS
FatAllocateVolume (
  IN  EFI_HANDLE                Handle,
  IN  EFI_DISK_IO_PROTOCOL      *DiskIo,
  IN  EFI_DISK_IO2_PROTOCOL     *DiskIo2,
  IN  EFI_BLOCK_IO_PROTOCOL     *BlockIo
  )
/*++

Routine Description:

  Allocates volume structure, detects FAT file system, installs protocol,
  and initialize cache.

Arguments:

  Handle                - The handle of parent device.
  DiskIo                - The DiskIo of parent device.
  BlockIo               - The BlockIo of parent devicel

Returns:

  EFI_SUCCESS           - Allocate a new volume successfully.
  EFI_OUT_OF_RESOURCES  - Can not allocate the memory.
  Others                - Allocating a new volume failed.

--*/
{
  EFI_STATUS  Status;
  FAT_VOLUME  *Volume;

  //
  // Allocate a volume structure
  //
  Volume = AllocateZeroPool (sizeof (FAT_VOLUME));
  if (Volume == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize the structure
  //
  Volume->Signature                   = FAT_VOLUME_SIGNATURE;
  Volume->Handle                      = Handle;
  Volume->DiskIo                      = DiskIo;
  Volume->DiskIo2                     = DiskIo2;
  Volume->BlockIo                     = BlockIo;
  Volume->MediaId                     = BlockIo->Media->MediaId;
  Volume->ReadOnly                    = BlockIo->Media->ReadOnly;
  Volume->VolumeInterface.Revision    = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
  Volume->VolumeInterface.OpenVolume  = FatOpenVolume;
  InitializeListHead (&Volume->CheckRef);
  InitializeListHead (&Volume->DirCacheList);
  //
  // Initialize Root Directory entry
  //
  Volume->RootDirEnt.FileString       = Volume->RootFileString;
  Volume->RootDirEnt.Entry.Attributes = FAT_ATTRIBUTE_DIRECTORY;
DEBUG((EFI_D_INFO, "Volume->MediaId 0x%x \n", Volume->MediaId));
DEBUG((EFI_D_INFO, "SDXC->MediaId 0x%x \n", SIGNATURE_32('m','m','c','o')));

  //
  // Check to see if there's a file system on the volume
  //
  Status = FatOpenDevice (Volume);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Initialize cache
  //
  Status = FatInitializeDiskCache (Volume);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Install our protocol interfaces on the device's handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Volume->Handle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  &Volume->VolumeInterface,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Volume installed
  //
  Volume->Valid = TRUE;

Done:
  if (EFI_ERROR (Status)) {
    FatFreeVolume (Volume);
  }

  return Status;
}

EFI_STATUS
FatAbandonVolume (
  IN FAT_VOLUME *Volume
  )
/*++

Routine Description:

  Called by FatDriverBindingStop(), Abandon the volume.

Arguments:

  Volume                - The volume to be abandoned.

Returns:

  EFI_SUCCESS           - Abandoned the volume successfully.
  Others                - Can not uninstall the protocol interfaces.

--*/
{
  EFI_STATUS  Status;
  BOOLEAN     LockedByMe;

  //
  // Uninstall the protocol interface.
  //
  if (Volume->Handle != NULL) {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                    Volume->Handle,
                    &gEfiSimpleFileSystemProtocolGuid,
                    &Volume->VolumeInterface,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  LockedByMe = FALSE;

  //
  // Acquire the lock.
  // If the caller has already acquired the lock (which
  // means we are in the process of some Fat operation),
  // we can not acquire again.
  //
  Status = FatAcquireLockOrFail ();
  if (!EFI_ERROR (Status)) {
    LockedByMe = TRUE;
  }
  //
  // The volume is still being used. Hence, set error flag for all OFiles still in
  // use. In two cases, we could get here. One is EFI_MEDIA_CHANGED, the other is
  // EFI_NO_MEDIA.
  //
  if (Volume->Root != NULL) {
    FatSetVolumeError (
      Volume->Root,
      Volume->BlockIo->Media->MediaPresent ? EFI_MEDIA_CHANGED : EFI_NO_MEDIA
      );
  }

  Volume->Valid = FALSE;

  //
  // Release the lock.
  // If locked by me, this means DriverBindingStop is NOT
  // called within an on-going Fat operation, so we should
  // take responsibility to cleanup and free the volume.
  // Otherwise, the DriverBindingStop is called within an on-going
  // Fat operation, we shouldn't check reference, so just let outer
  // FatCleanupVolume do the task.
  //
  if (LockedByMe) {
    FatCleanupVolume (Volume, NULL, EFI_SUCCESS, NULL);
    FatReleaseLock ();
  }

  return EFI_SUCCESS;
}

BOOLEAN
PartitionValidMbr (
  IN  MASTER_BOOT_RECORD      *Mbr,
  IN  EFI_LBA                 LastLba
  )
{
  UINT32  StartingLBA;
  UINT32  EndingLBA;
  UINT32  NewEndingLBA;
  INTN    Index1;
  INTN    Index2;
  BOOLEAN MbrValid;
DEBUG((EFI_D_INFO,"Mbr->Signature 0x%x \n" ,Mbr->Signature));

  if (Mbr->Signature != MBR_SIGNATURE) {
    return FALSE;
  }
  //
  // The BPB also has this signature, so it can not be used alone.
  //
  MbrValid = FALSE;
  for (Index1 = 0; Index1 < MAX_MBR_PARTITIONS; Index1++) {
    if (Mbr->Partition[Index1].OSIndicator == 0x00 || UNPACK_UINT32 (Mbr->Partition[Index1].SizeInLBA) == 0) {
      continue;
    }
DEBUG((EFI_D_INFO," in for index %d\n",Index1));

    MbrValid    = TRUE;
    StartingLBA = UNPACK_UINT32 (Mbr->Partition[Index1].StartingLBA);
    EndingLBA   = StartingLBA + UNPACK_UINT32 (Mbr->Partition[Index1].SizeInLBA) - 1;
DEBUG((EFI_D_INFO,"StartingLBA 0x%x, EndingLBA 0x%x, LastLba 0x%x \n", StartingLBA, EndingLBA, LastLba));
    if (EndingLBA > LastLba) {
      //
      // Compatibility Errata:
      //  Some systems try to hide drive space with their INT 13h driver
      //  This does not hide space from the OS driver. This means the MBR
      //  that gets created from DOS is smaller than the MBR created from
      //  a real OS (NT & Win98). This leads to BlockIo->LastBlock being
      //  wrong on some systems FDISKed by the OS.
      //
      // return FALSE since no block devices on a system are implemented
      // with INT 13h
      //
      return FALSE;
    }

    for (Index2 = Index1 + 1; Index2 < MAX_MBR_PARTITIONS; Index2++) {
      if (Mbr->Partition[Index2].OSIndicator == 0x00 || UNPACK_UINT32 (Mbr->Partition[Index2].SizeInLBA) == 0) {
        continue;
      }

      NewEndingLBA = UNPACK_UINT32 (Mbr->Partition[Index2].StartingLBA) + UNPACK_UINT32 (Mbr->Partition[Index2].SizeInLBA) - 1;
DEBUG((EFI_D_INFO," NewEndingLBA 0x%x, StartingLBA 0x%x, EndingLBA 0x%x, lba 0x%x \n", NewEndingLBA, StartingLBA, EndingLBA, UNPACK_UINT32 (Mbr->Partition[Index2].StartingLBA)));
      if (NewEndingLBA >= StartingLBA && UNPACK_UINT32 (Mbr->Partition[Index2].StartingLBA) <= EndingLBA) {
        //
        // This region overlaps with the Index1'th region
        //
DEBUG((EFI_D_INFO," Index2 %d\n", Index2));
        return FALSE;
      }
    }
  }
  //
  // None of the regions overlapped so MBR is O.K.
  //
  return MbrValid;
}


EFI_STATUS
FatOpenDevice (
  IN OUT FAT_VOLUME           *Volume
  )
/*++

Routine Description:

  Detects FAT file system on Disk and set relevant fields of Volume

Arguments:

  Volume                - The volume structure.

Returns:

  EFI_SUCCESS           - The Fat File System is detected successfully
  EFI_UNSUPPORTED       - The volume is not FAT file system.
  EFI_VOLUME_CORRUPTED  - The volume is corrupted.

--*/
{
  EFI_STATUS            Status;
  UINT32  		   StartingLBA;
  UINT32                BlockSize;
  UINT32                DirtyMask;
  EFI_DISK_IO_PROTOCOL  *DiskIo;
  FAT_BOOT_SECTOR       FatBs;
  FAT_VOLUME_TYPE       FatType;
  UINTN                 RootDirSectors;
  UINTN                 FatLba;
  UINTN                 RootLba;
  UINTN                 FirstClusterLba;
  UINTN                 Sectors;
  UINTN                 SectorsPerFat;
  UINT8                 SectorsPerClusterAlignment;
  UINT8                 BlockAlignment;
  MASTER_BOOT_RECORD    *Mbr;
#if 0
  UINT32 		   Arr[23];
  UINT32 		   Index = 0;
#endif

  //
  // Read the FAT_BOOT_SECTOR BPB info
  // This is the only part of FAT code that uses parent DiskIo,
  // Others use FatDiskIo which utilizes a Cache.
  //
  DiskIo  = Volume->DiskIo;

#if 0									//90
  Status  = DiskIo->ReadDisk (DiskIo, Volume->MediaId, 0x10000, sizeof (FatBs), &Arr);
#endif
  Status  = DiskIo->ReadDisk (DiskIo, Volume->MediaId, 0x10000, sizeof (FatBs), &FatBs);

#if 0
  for(Index = 0; Index < 23; Index++)
    Arr[Index] = Uswap32(Arr[Index]);

  CopyMem(&FatBs, Arr, sizeof (FatBs));
#endif
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INIT, "FatOpenDevice: read of part_lba failed %r\n", Status));
    return Status;
  }

  FatType = FatUndefined;

  DEBUG((EFI_D_INFO, "Ia32Jump 0x%x 0x%x 0x%x \n", FatBs.FatBsb.Ia32Jump[0], FatBs.FatBsb.Ia32Jump[1], FatBs.FatBsb.Ia32Jump[2]));
  DEBUG((EFI_D_INFO, "OEM ID %a \n", FatBs.FatBsb.OemId));
  DEBUG((EFI_D_INFO, "SectorSize %d \n", FatBs.FatBsb.SectorSize));
  DEBUG((EFI_D_INFO, "SectorsPerCluster %d \n", FatBs.FatBsb.SectorsPerCluster));
  DEBUG((EFI_D_INFO, "ReservedSectors %d \n", FatBs.FatBsb.ReservedSectors));
  DEBUG((EFI_D_INFO, "NumFats %d \n", FatBs.FatBsb.NumFats));
  DEBUG((EFI_D_INFO, "RootEntries %d \n", FatBs.FatBsb.RootEntries));
  DEBUG((EFI_D_INFO, "Sectors %d \n", FatBs.FatBsb.Sectors));
  DEBUG((EFI_D_INFO, "MediaType 0x%x \n", FatBs.FatBsb.Media));
  DEBUG((EFI_D_INFO, "SectorsPerFat %d \n", FatBs.FatBsb.SectorsPerFat));
  DEBUG((EFI_D_INFO, "SectorsPerTrack %d \n", FatBs.FatBsb.SectorsPerTrack));
  DEBUG((EFI_D_INFO, "Heads %d \n", FatBs.FatBsb.Heads));
  DEBUG((EFI_D_INFO, "HiddenSectors %ld \n", FatBs.FatBsb.HiddenSectors));
  DEBUG((EFI_D_INFO, "LargeSectors %ld \n\n", FatBs.FatBsb.LargeSectors));

  DEBUG((EFI_D_INFO, "LargeSectorsPerFat %d \n", FatBs.FatBse.Fat32Bse.LargeSectorsPerFat));
  DEBUG((EFI_D_INFO, "ExtendedFlags %d \n", FatBs.FatBse.Fat32Bse.ExtendedFlags));
  DEBUG((EFI_D_INFO, "FsVersion %d \n", FatBs.FatBse.Fat32Bse.FsVersion));
  DEBUG((EFI_D_INFO, "RootDirFirstCluster %d \n", FatBs.FatBse.Fat32Bse.RootDirFirstCluster));
  DEBUG((EFI_D_INFO, "FsInfoSector %d \n", FatBs.FatBse.Fat32Bse.FsInfoSector));
  DEBUG((EFI_D_INFO, "BackupBootSector %d \n", FatBs.FatBse.Fat32Bse.BackupBootSector));
  DEBUG((EFI_D_INFO, "Reserved %a \n", FatBs.FatBse.Fat32Bse.Reserved));
  DEBUG((EFI_D_INFO, "PhysicalDriveNumber 0x%x \n", FatBs.FatBse.Fat32Bse.PhysicalDriveNumber));
  DEBUG((EFI_D_INFO, "CurrentHead %d \n", FatBs.FatBse.Fat32Bse.CurrentHead));
  DEBUG((EFI_D_INFO, "Signature 0x%x \n", FatBs.FatBse.Fat32Bse.Signature));
  DEBUG((EFI_D_INFO, "Id %a \n", FatBs.FatBse.Fat32Bse.Id));
  DEBUG((EFI_D_INFO, "FatLabel %a \n", FatBs.FatBse.Fat32Bse.FatLabel));
  DEBUG((EFI_D_INFO, "SystemId %a \n", FatBs.FatBse.Fat32Bse.SystemId));
  //
  // Use LargeSectors if Sectors is 0
  //
  Sectors = FatBs.FatBsb.Sectors;
  if (Sectors == 0) {
    Sectors = FatBs.FatBsb.LargeSectors;
  }

  SectorsPerFat = FatBs.FatBsb.SectorsPerFat;
  if (SectorsPerFat == 0) {
    SectorsPerFat = FatBs.FatBse.Fat32Bse.LargeSectorsPerFat;
    FatType       = FAT32;
  }
  //
  // Is boot sector a fat sector?
  // (Note that so far we only know if the sector is FAT32 or not, we don't
  // know if the sector is Fat16 or Fat12 until later when we can compute
  // the volume size)
  //

  if (FatBs.FatBsb.ReservedSectors == 0 || FatBs.FatBsb.NumFats == 0 || Sectors == 0) {
    return EFI_UNSUPPORTED;
  }
  if ((FatBs.FatBsb.SectorSize & (FatBs.FatBsb.SectorSize - 1)) != 0) {
    return EFI_UNSUPPORTED;
  }

  BlockAlignment = (UINT8) HighBitSet32 (FatBs.FatBsb.SectorSize);
  if (BlockAlignment > MAX_BLOCK_ALIGNMENT || BlockAlignment < MIN_BLOCK_ALIGNMENT) {
    return EFI_UNSUPPORTED;
  }

  if ((FatBs.FatBsb.SectorsPerCluster & (FatBs.FatBsb.SectorsPerCluster - 1)) != 0) {
    return EFI_UNSUPPORTED;
  }

  SectorsPerClusterAlignment = (UINT8) HighBitSet32 (FatBs.FatBsb.SectorsPerCluster);
  if (SectorsPerClusterAlignment > MAX_SECTORS_PER_CLUSTER_ALIGNMENT) {
    return EFI_UNSUPPORTED;
  }

  if (FatBs.FatBsb.Media <= 0xf7 &&
      FatBs.FatBsb.Media != 0xf0 &&
      FatBs.FatBsb.Media != 0x00 &&
      FatBs.FatBsb.Media != 0x01
      ) {
    return EFI_UNSUPPORTED;
  }
  //
  // Initialize fields the volume information for this FatType
  //
  if (FatType != FAT32) {
    if (FatBs.FatBsb.RootEntries == 0) {
      return EFI_UNSUPPORTED;
    }
    //
    // Unpack fat12, fat16 info
    //
    Volume->RootEntries = FatBs.FatBsb.RootEntries;
  } else {
    //
    // If this is fat32, refuse to mount mirror-disabled volumes
    //
    if ((SectorsPerFat == 0 || FatBs.FatBse.Fat32Bse.FsVersion != 0) || (FatBs.FatBse.Fat32Bse.ExtendedFlags & 0x80)) {
      return EFI_UNSUPPORTED;
    }
    //
    // Unpack fat32 info
    //
    Volume->RootCluster = FatBs.FatBse.Fat32Bse.RootDirFirstCluster;
  }

  Volume->NumFats           = FatBs.FatBsb.NumFats;
  //
  // Compute some fat locations
  //
  BlockSize                 = FatBs.FatBsb.SectorSize;
DEBUG((EFI_D_INFO,"BlockSize %d\n", BlockSize));
  Mbr = AllocatePool (BlockSize);
  if (Mbr == NULL) {
    return EFI_VOLUME_CORRUPTED;
  }
  Status  = DiskIo->ReadDisk (DiskIo, Volume->MediaId, 0, BlockSize, Mbr);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR,"Failed to read partition \n"));
    return Status;
  }

  Status = PartitionValidMbr(Mbr, Volume->BlockIo->Media->LastBlock);
  if (!Status)
    return EFI_VOLUME_CORRUPTED;
 
  StartingLBA = UNPACK_UINT32 (Mbr->Partition[0].StartingLBA);
  DEBUG((EFI_D_INFO,"StartingLBA 0x%x \n", StartingLBA));

  RootDirSectors            = ((Volume->RootEntries * sizeof (FAT_DIRECTORY_ENTRY)) + (BlockSize - 1)) / BlockSize;

  FatLba                    = StartingLBA + FatBs.FatBsb.ReservedSectors;
  RootLba                   = FatBs.FatBsb.NumFats * SectorsPerFat + FatLba;
  FirstClusterLba           = RootLba + RootDirSectors;

  Volume->FatPos            = FatLba * BlockSize;
  Volume->FatSize           = SectorsPerFat * BlockSize;

  Volume->VolumeSize        = LShiftU64 (Sectors, BlockAlignment);
  Volume->RootPos           = LShiftU64 (RootLba, BlockAlignment);
  Volume->FirstClusterPos   = LShiftU64 (FirstClusterLba, BlockAlignment);
  Volume->MaxCluster        = (Sectors - FirstClusterLba) >> SectorsPerClusterAlignment;
  Volume->ClusterAlignment  = (UINT8)(BlockAlignment + SectorsPerClusterAlignment);
  Volume->ClusterSize       = (UINTN)1 << (Volume->ClusterAlignment);

  //
  // If this is not a fat32, determine if it's a fat16 or fat12
  //
  if (FatType != FAT32) {
    if (Volume->MaxCluster >= FAT_MAX_FAT16_CLUSTER) {
      return EFI_VOLUME_CORRUPTED;
    }

    FatType = Volume->MaxCluster < FAT_MAX_FAT12_CLUSTER ? FAT12 : FAT16;
    //
    // fat12 & fat16 fat-entries are 2 bytes
    //
    Volume->FatEntrySize = sizeof (UINT16);
    DirtyMask            = FAT16_DIRTY_MASK;
  } else {
    if (Volume->MaxCluster < FAT_MAX_FAT16_CLUSTER) {
      return EFI_VOLUME_CORRUPTED;
    }
    //
    // fat32 fat-entries are 4 bytes
    //
    Volume->FatEntrySize = sizeof (UINT32);
    DirtyMask            = FAT32_DIRTY_MASK;
  }
  //
  // Get the DirtyValue and NotDirtyValue
  // We should keep the initial value as the NotDirtyValue
  // in case the volume is dirty already
  //
  if (FatType != FAT12) {
    Status = FatAccessVolumeDirty (Volume, READ_DISK, &Volume->NotDirtyValue);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Volume->DirtyValue = Volume->NotDirtyValue & DirtyMask;
  }
  //
  // If present, read the fat hint info
  //
  if (FatType == FAT32) {
    Volume->FreeInfoPos = FatBs.FatBse.Fat32Bse.FsInfoSector * BlockSize;
    if (FatBs.FatBse.Fat32Bse.FsInfoSector != 0) {
      FatDiskIo (Volume, READ_DISK, Volume->FreeInfoPos, sizeof (FAT_INFO_SECTOR), &Volume->FatInfoSector, NULL);
      if (Volume->FatInfoSector.Signature == FAT_INFO_SIGNATURE &&
          Volume->FatInfoSector.InfoBeginSignature == FAT_INFO_BEGIN_SIGNATURE &&
          Volume->FatInfoSector.InfoEndSignature == FAT_INFO_END_SIGNATURE &&
          Volume->FatInfoSector.FreeInfo.ClusterCount <= Volume->MaxCluster
          ) {
        Volume->FreeInfoValid = TRUE;
      }
    }
  }
  //
  // Just make up a FreeInfo.NextCluster for use by allocate cluster
  //
  if (FAT_MIN_CLUSTER > Volume->FatInfoSector.FreeInfo.NextCluster ||
     Volume->FatInfoSector.FreeInfo.NextCluster > Volume->MaxCluster + 1
     ) {
    Volume->FatInfoSector.FreeInfo.NextCluster = FAT_MIN_CLUSTER;
  }
  //
  // We are now defining FAT Type
  //
  Volume->FatType = FatType;
  ASSERT (FatType != FatUndefined);

  return EFI_SUCCESS;
}
