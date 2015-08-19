/** @file
  Implement EFI RealTimeClock runtime services via RTC Lib.

  Currently this driver does not support runtime virtual calling.

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/RealTimeClockLib.h>
#include <I2c.h>


#define Bin(Bcd) ((Bcd) & 0x0f) + ((Bcd) >> 4) * 10
#define Bcd(Bin) (((Bin / 10) << 4) | (Bin % 10))

#define DS1307_I2C_ADDR			0x68

/*
 * RTC register addresses
 */
#define DS1307_SEC_REG_ADDR	0x00
#define DS1307_MIN_REG_ADDR	0x01
#define DS1307_HR_REG_ADDR		0x02
#define DS1307_DAY_REG_ADDR	0x03
#define DS1307_DATE_REG_ADDR	0x04
#define DS1307_MON_REG_ADDR	0x05
#define DS1307_YR_REG_ADDR		0x06
#define DS1307_CTL_REG_ADDR	0x07

#define DS1307_SEC_BIT_CH		0x80	/* Clock Halt (in Register 0)   */

#define DS1307_CTL_BIT_RS0		0x01	/* Rate select 0                */
#define DS1307_CTL_BIT_RS1		0x02	/* Rate select 1                */
#define DS1307_CTL_BIT_SQWE	0x10	/* Square Wave Enable           */
#define DS1307_CTL_BIT_OUT		0x80	/* Output Control               */

UINT8 RtcRead(
		UINT8 RtcRegAddr
)
{
	INT32 Status;
	UINT8 Val = 0;
	Status = I2cDataRead((VOID*)I2C0_BASE_ADDRESS, DS1307_I2C_ADDR, RtcRegAddr, 0x1, &Val, sizeof(Val));
	if(EFI_ERROR(Status))
		DEBUG((EFI_D_ERROR, "RTC read error at Addr:0x%x\n", RtcRegAddr));
	return Val;
}

VOID RtcWrite(
		UINT8 RtcRegAddr,
		UINT8 Val)
{
	INT32 Status;
	Status = I2cDataWrite((VOID*)I2C0_BASE_ADDRESS, DS1307_I2C_ADDR, RtcRegAddr, 0x1, &Val, sizeof(Val));
	if(EFI_ERROR(Status))
		DEBUG((EFI_D_ERROR, "RTC write error at Addr:0x%x\n", RtcRegAddr));

}

/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param  Time                  A pointer to storage to receive a snapshot of the current time.
  @param  Capabilities          An optional pointer to a buffer to receive the real time clock
                                device's capabilities.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER Time is NULL.
  @retval EFI_DEVICE_ERROR      The time could not be retrieved due to hardware error.

**/

EFI_STATUS
EFIAPI
LibGetTime (
  OUT EFI_TIME                *Time,
  OUT  EFI_TIME_CAPABILITIES  *Capabilities
  )
{
	EFI_STATUS Status = EFI_SUCCESS;
	CHAR16 Second, Minute, Hour, Day, Month, Year;

	Second = RtcRead (DS1307_SEC_REG_ADDR);
	Minute = RtcRead (DS1307_MIN_REG_ADDR);
	Hour = RtcRead (DS1307_HR_REG_ADDR);
	Day = RtcRead (DS1307_DATE_REG_ADDR);
	Month = RtcRead (DS1307_MON_REG_ADDR);
	Year = RtcRead (DS1307_YR_REG_ADDR);

	if (Second & DS1307_SEC_BIT_CH) {
		DEBUG((EFI_D_ERROR, "### Warning: RTC oscillator has stopped\n"));
		/* clear the CH flag */
		RtcWrite (DS1307_SEC_REG_ADDR,
			   RtcRead (DS1307_SEC_REG_ADDR) & ~DS1307_SEC_BIT_CH);
		Status = EFI_DEVICE_ERROR;
	}

	Time->Second  = Bin (Second & 0x7F);
	Time->Minute  = Bin (Minute & 0x7F);
	Time->Hour = Bin (Hour & 0x3F);
	Time->Day = Bin (Day & 0x3F);
	Time->Month  = Bin (Month & 0x1F);
	Time->Year = Bin (Year) + ( Bin (Year) >= 70 ? 1900 : 2000);

  return Status;
}


/**
  Sets the current local time and date information.

  @param  Time                  A pointer to the current time.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The time could not be set due due to hardware error.

**/
EFI_STATUS
EFIAPI
LibSetTime (
  IN EFI_TIME                *Time
  )
{
	if (Time->Year < 1970 || Time->Year > 2069)
		DEBUG((EFI_D_ERROR, "WARNING: Year should be between 1970 and 2069!\n"));

	RtcWrite (DS1307_YR_REG_ADDR, Bcd (Time->Year % 100));
	RtcWrite (DS1307_MON_REG_ADDR, Bcd (Time->Month));
	RtcWrite (DS1307_DATE_REG_ADDR, Bcd (Time->Day));
	RtcWrite (DS1307_HR_REG_ADDR, Bcd (Time->Hour));
	RtcWrite (DS1307_MIN_REG_ADDR, Bcd (Time->Minute));
	RtcWrite (DS1307_SEC_REG_ADDR, Bcd (Time->Second));

  return EFI_SUCCESS;
}


/**
  Returns the current wakeup alarm clock setting.

  @param  Enabled               Indicates if the alarm is currently enabled or disabled.
  @param  Pending               Indicates if the alarm signal is pending and requires acknowledgement.
  @param  Time                  The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to a hardware error.

**/
EFI_STATUS
EFIAPI
LibGetWakeupTime (
  OUT BOOLEAN     *Enabled,
  OUT BOOLEAN     *Pending,
  OUT EFI_TIME    *Time
  )
{
  // Not a required feature
  return EFI_UNSUPPORTED;
}


/**
  Sets the system wakeup alarm clock time.

  @param  Enabled               Enable or disable the wakeup alarm.
  @param  Time                  If Enable is TRUE, the time to set the wakeup alarm for.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was enabled. If
                                Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be set due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
LibSetWakeupTime (
  IN BOOLEAN      Enabled,
  OUT EFI_TIME    *Time
  )
{
  // Not a required feature
  return EFI_UNSUPPORTED;
}



/**
  This is the declaration of an EFI image entry point. This can be the entry point to an application
  written to this specification, an EFI boot service driver, or an EFI runtime driver.

  @param  ImageHandle           Handle that identifies the loaded image.
  @param  SystemTable           System Table for this image.

  @retval EFI_SUCCESS           The operation completed successfully.

**/
EFI_STATUS
EFIAPI
LibRtcInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  //
  // Do some initialization if reqruied to turn on the RTC
  //
  return EFI_SUCCESS;
}


/**
  Fixup internal data so that EFI can be call in virtual mode.
  Call the passed in Child Notify event and convert any pointers in
  lib to virtual mode.

  @param[in]    Event   The Event that is being processed
  @param[in]    Context Event Context
**/
VOID
EFIAPI
LibRtcVirtualNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  //
  // Only needed if you are going to support the OS calling RTC functions in virtual mode.
  // You will need to call EfiConvertPointer (). To convert any stored physical addresses
  // to virtual address. After the OS transistions to calling in virtual mode, all future
  // runtime calls will be made in virtual mode.
  //
  return;
}
