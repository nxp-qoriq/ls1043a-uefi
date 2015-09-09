/** @file
**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/Cpu.h>

/**
  Initialize IFC Nand Flash interface
**/

EFI_STATUS
FslIfcNandFlashInit (
  OUT EFI_BLOCK_IO_MEDIA *gNandFlashMedia;
);

/**
  Write BufferSize bytes from Lba into Buffer.

  @param  This       Indicates a pointer to the calling context.
  @param  MediaId    The media ID that the write request is for.
  @param  Lba        The starting logical block address to be written.
		     The caller is responsible for writing to only legitimate
		     locations.
  @param  BufferSize Size of Buffer, must be a multiple of device block size.
  @param  Buffer     A pointer to the source buffer for the data.

  @retval EFI_SUCCESS           The data was written correctly to the device.
  @retval EFI_WRITE_PROTECTED   The device can not be written to.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing
				the write.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.
  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size
				of the device.
  @retval EFI_INVALID_PARAMETER The write request contains LBAs that are not
				valid, or the buffer is not on proper alignment.

**/

EFI_STATUS
EFIAPI
FslIfcNandFlashWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  IN VOID                           *Buffer
  );

/**
  Read BufferSize bytes from Lba into Buffer.

  @param  This		Indicates a pointer to the calling context.
  @param  MediaId    	Id of the media, changes every time media is replaced.
  @param  Lba        	The starting Logical Block Address to read from
  @param  BufferSize	Size of Buffer, must be a multiple of device block size.
  @param  Buffer	A pointer to the destination buffer for the data. The
			caller is responsible for either having implicit or
			explicit ownership of the buffer.

  @retval EFI_SUCCESS           The data was read correctly from the device.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing
				the read.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHANGED     The MediaId does not matched the current device.
  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size
				of the device.
  @retval EFI_INVALID_PARAMETER The read request contains LBAs that are not
				valid, or the buffer is not on proper alignment.
**/

EFI_STATUS
EFIAPI
FslIfcNandFlashReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  OUT VOID                          *Buffer
  );

/**
  Reset the Block Device.

  @param  This			Indicates a pointer to the calling context.
  @param  ExtendedVerification	Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS		The device was reset.
  @retval EFI_DEVICE_ERROR	The device is not functioning properly and could
				not be reset.
**/

EFI_STATUS
EFIAPI
FslIfcNandFlashReset (
  IN EFI_BLOCK_IO_PROTOCOL          *This,
  IN BOOLEAN                        ExtendedVerification
  );

/**
  Flush the Block Device.

  @param  This			Indicates a pointer to the calling context.

  @retval EFI_SUCCESS		All outstanding data was written to the device
  @retval EFI_DEVICE_ERROR	The device reported an error while writting back
				the data
  @retval EFI_NO_MEDIA		There is no media in the device.
**/

EFI_STATUS
EFIAPI
FslIfcNandFlashFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  );

typedef struct {
  UINT8 ManufactureId;
  UINT8 DeviceId;
  UINT8 BlockAddressStart; //Start of the Block address in actual NAND
  UINT8 PageAddressStart; //Start of the Page address in actual NAND
} NAND_PART_INFO_TABLE;

typedef struct {
  UINT8		    ManufactureId;
  UINT8		    DeviceId;
  UINT8		    Organization; //x8 or x16
  UINT32	    PageSize;
  UINT32	    SparePageSize;
  UINT32	    BlockSize;
  UINT32	    NumPagesPerBlock;
  UINT8		    BlockAddressStart;//Start of Block address in actual NAND
  UINT8		    PageAddressStart; //Start of Page address in actual NAND
  VOID*		    BufBase;
  FSL_IFC_REGS*     IfcRegs;
  UINT32	    cs_nand;
} NAND_FLASH_INFO;
