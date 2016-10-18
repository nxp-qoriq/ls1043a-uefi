/** @file
  Architectural MSR Definitions.

  Provides defines for Machine Specific Registers(MSR) indexes. Data structures
  are provided for MSRs that contain one or more bit fields.  If the MSR value
  returned is a single 32-bit or 64-bit value, then a data structure is not
  provided for that MSR.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Specification Reference:
  Intel(R) 64 and IA-32 Architectures Software Developer's Manual, Volume 3,
  December 2015, Chapter 35 Model-Specific-Registers (MSR), Section 35-1.

**/

#ifndef __ARCHITECTURAL_MSR_H__
#define __ARCHITECTURAL_MSR_H__

/**
  See Section 35.20, "MSRs in Pentium Processors.". Pentium Processor (05_01H).

  @param  ECX  MSR_IA32_P5_MC_ADDR (0x00000000)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_P5_MC_ADDR);
  AsmWriteMsr64 (MSR_IA32_P5_MC_ADDR, Msr);
  @endcode
**/
#define MSR_IA32_P5_MC_ADDR                      0x00000000


/**
  See Section 35.20, "MSRs in Pentium Processors.". DF_DM = 05_01H.

  @param  ECX  MSR_IA32_P5_MC_TYPE (0x00000001)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_P5_MC_TYPE);
  AsmWriteMsr64 (MSR_IA32_P5_MC_TYPE, Msr);
  @endcode
**/
#define MSR_IA32_P5_MC_TYPE                      0x00000001


/**
  See Section 8.10.5, "Monitor/Mwait Address Range Determination.". Introduced
  at Display Family / Display Model 0F_03H.

  @param  ECX  MSR_IA32_MONITOR_FILTER_SIZE (0x00000006)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MONITOR_FILTER_SIZE);
  AsmWriteMsr64 (MSR_IA32_MONITOR_FILTER_SIZE, Msr);
  @endcode
**/
#define MSR_IA32_MONITOR_FILTER_SIZE             0x00000006


/**
  See Section 17.14, "Time-Stamp Counter.". Introduced at Display Family /
  Display Model 05_01H.

  @param  ECX  MSR_IA32_TIME_STAMP_COUNTER (0x00000010)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_TIME_STAMP_COUNTER);
  AsmWriteMsr64 (MSR_IA32_TIME_STAMP_COUNTER, Msr);
  @endcode
**/
#define MSR_IA32_TIME_STAMP_COUNTER              0x00000010


/**
  Platform ID (RO)  The operating system can use this MSR to determine "slot"
  information for the processor and the proper microcode update to load.
  Introduced at Display Family / Display Model 06_01H.

  @param  ECX  MSR_IA32_PLATFORM_ID (0x00000017)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PLATFORM_ID_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PLATFORM_ID_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PLATFORM_ID_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PLATFORM_ID);
  @endcode
**/
#define MSR_IA32_PLATFORM_ID                     0x00000017

/**
  MSR information returned for MSR index #MSR_IA32_PLATFORM_ID
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    UINT32  Reserved1:32;
    UINT32  Reserved2:18;
    ///
    /// [Bits 52:50] Platform Id (RO)  Contains information concerning the
    /// intended platform for the processor.
    ///   52 51 50
    ///   -- -- --
    ///    0  0  0  Processor Flag 0.
    ///    0  0  1  Processor Flag 1
    ///    0  1  0  Processor Flag 2
    ///    0  1  1  Processor Flag 3
    ///    1  0  0  Processor Flag 4
    ///    1  0  1  Processor Flag 5
    ///    1  1  0  Processor Flag 6
    ///    1  1  1  Processor Flag 7
    ///
    UINT32  PlatformId:3;
    UINT32  Reserved3:11;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PLATFORM_ID_REGISTER;


/**
  06_01H.

  @param  ECX  MSR_IA32_APIC_BASE (0x0000001B)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_APIC_BASE_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_APIC_BASE_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_APIC_BASE_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_APIC_BASE);
  AsmWriteMsr64 (MSR_IA32_APIC_BASE, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_APIC_BASE                       0x0000001B

/**
  MSR information returned for MSR index #MSR_IA32_APIC_BASE
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    UINT32  Reserved1:8;
    ///
    /// [Bit 8] BSP flag (R/W).
    ///
    UINT32  BSP:1;
    UINT32  Reserved2:1;
    ///
    /// [Bit 10] Enable x2APIC mode. Introduced at Display Family / Display
    /// Model 06_1AH.
    ///
    UINT32  EXTD:1;
    ///
    /// [Bit 11] APIC Global Enable (R/W).
    ///
    UINT32  EN:1;
    ///
    /// [Bits 31:12] APIC Base (R/W).
    ///
    UINT32  ApicBase:20;
    ///
    /// [Bits 63:32] APIC Base (R/W).
    ///
    UINT32  ApicBaseHi:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_APIC_BASE_REGISTER;


/**
  Control Features in Intel 64 Processor (R/W). If any one enumeration
  condition for defined bit field holds.

  @param  ECX  MSR_IA32_FEATURE_CONTROL (0x0000003A)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_FEATURE_CONTROL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_FEATURE_CONTROL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_FEATURE_CONTROL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_FEATURE_CONTROL);
  AsmWriteMsr64 (MSR_IA32_FEATURE_CONTROL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_FEATURE_CONTROL                 0x0000003A

/**
  MSR information returned for MSR index #MSR_IA32_FEATURE_CONTROL
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Lock bit (R/WO): (1 = locked). When set, locks this MSR from
    /// being written, writes to this bit will result in GP(0). Note: Once the
    /// Lock bit is set, the contents of this register cannot be modified.
    /// Therefore the lock bit must be set after configuring support for Intel
    /// Virtualization Technology and prior to transferring control to an
    /// option ROM or the OS. Hence, once the Lock bit is set, the entire
    /// IA32_FEATURE_CONTROL contents are preserved across RESET when PWRGOOD
    /// is not deasserted. If any one enumeration condition for defined bit
    /// field position greater than bit 0 holds.
    ///
    UINT32  Lock:1;
    ///
    /// [Bit 1] Enable VMX inside SMX operation (R/WL): This bit enables a
    /// system executive to use VMX in conjunction with SMX to support
    /// Intel(R) Trusted Execution Technology. BIOS must set this bit only
    /// when the CPUID function 1 returns VMX feature flag and SMX feature
    /// flag set (ECX bits 5 and 6 respectively). If CPUID.01H:ECX[5] = 1 &&
    /// CPUID.01H:ECX[6] = 1.
    ///
    UINT32  EnableVmxInsideSmx:1;
    ///
    /// [Bit 2] Enable VMX outside SMX operation (R/WL): This bit enables VMX
    /// for system executive that do not require SMX. BIOS must set this bit
    /// only when the CPUID function 1 returns VMX feature flag set (ECX bit
    /// 5). If CPUID.01H:ECX[5] = 1.
    ///
    UINT32  EnableVmxOutsideSmx:1;
    UINT32  Reserved1:5;
    ///
    /// [Bits 14:8] SENTER Local Function Enables (R/WL): When set, each bit
    /// in the field represents an enable control for a corresponding SENTER
    /// function. This bit is supported only if CPUID.1:ECX.[bit 6] is set. If
    /// CPUID.01H:ECX[6] = 1.
    ///
    UINT32  SenterLocalFunctionEnables:7;
    ///
    /// [Bit 15] SENTER Global Enable (R/WL): This bit must be set to enable
    /// SENTER leaf functions. This bit is supported only if CPUID.1:ECX.[bit
    /// 6] is set. If CPUID.01H:ECX[6] = 1.
    ///
    UINT32  SenterGlobalEnable:1;
    UINT32  Reserved2:2;
    ///
    /// [Bit 18] SGX Global Enable (R/WL): This bit must be set to enable SGX
    /// leaf functions. This bit is supported only if CPUID.1:ECX.[bit 6] is
    /// set. If CPUID.(EAX=07H, ECX=0H): EBX[2] = 1.
    ///
    UINT32  SgxEnable:1;
    UINT32  Reserved3:1;
    ///
    /// [Bit 20] LMCE On (R/WL): When set, system software can program the
    /// MSRs associated with LMCE to configure delivery of some machine check
    /// exceptions to a single logical processor. If IA32_MCG_CAP[27] = 1.
    ///
    UINT32  LmceOn:1;
    UINT32  Reserved4:11;
    UINT32  Reserved5:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_FEATURE_CONTROL_REGISTER;


/**
  Per Logical Processor TSC Adjust (R/Write to clear). If CPUID.(EAX=07H,
  ECX=0H): EBX[1] = 1. THREAD_ADJUST:  Local offset value of the IA32_TSC for
  a logical processor. Reset value is Zero. A write to IA32_TSC will modify
  the local offset in IA32_TSC_ADJUST and the content of IA32_TSC, but does
  not affect the internal invariant TSC hardware.

  @param  ECX  MSR_IA32_TSC_ADJUST (0x0000003B)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_TSC_ADJUST);
  AsmWriteMsr64 (MSR_IA32_TSC_ADJUST, Msr);
  @endcode
**/
#define MSR_IA32_TSC_ADJUST                      0x0000003B


/**
  BIOS Update Trigger (W) Executing a WRMSR instruction to this MSR causes a
  microcode update to be loaded into the processor. See Section 9.11.6,
  "Microcode Update Loader." A processor may prevent writing to this MSR when
  loading guest states on VM entries or saving guest states on VM exits.
  Introduced at Display Family / Display Model 06_01H.

  @param  ECX  MSR_IA32_BIOS_UPDT_TRIG (0x00000079)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = 0;
  AsmWriteMsr64 (MSR_IA32_BIOS_UPDT_TRIG, Msr);
  @endcode
**/
#define MSR_IA32_BIOS_UPDT_TRIG                  0x00000079


/**
  BIOS Update Signature (RO) Returns the microcode update signature following
  the execution of CPUID.01H. A processor may prevent writing to this MSR when
  loading guest states on VM entries or saving guest states on VM exits.
  Introduced at Display Family / Display Model 06_01H.

  @param  ECX  MSR_IA32_BIOS_SIGN_ID (0x0000008B)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_BIOS_SIGN_ID_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_BIOS_SIGN_ID_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_BIOS_SIGN_ID_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_BIOS_SIGN_ID);
  @endcode
**/
#define MSR_IA32_BIOS_SIGN_ID                    0x0000008B

/**
  MSR information returned for MSR index #MSR_IA32_BIOS_SIGN_ID
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    UINT32  Reserved:32;
    ///
    /// [Bits 63:32] Microcode update signature. This field contains the
    /// signature of the currently loaded microcode update when read following
    /// the execution of the CPUID instruction, function 1. It is required
    /// that this register field be pre-loaded with zero prior to executing
    /// the CPUID, function 1. If the field remains equal to zero, then there
    /// is no microcode update loaded. Another nonzero value will be the
    /// signature.
    ///
    UINT32  MicrocodeUpdateSignature:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_BIOS_SIGN_ID_REGISTER;


/**
  SMM Monitor Configuration (R/W). If CPUID.01H: ECX[5]=1. CPUID.01H: ECX[6] =
  1.

  @param  ECX  MSR_IA32_SMM_MONITOR_CTL (0x0000009B)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_SMM_MONITOR_CTL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_SMM_MONITOR_CTL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_SMM_MONITOR_CTL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_SMM_MONITOR_CTL);
  AsmWriteMsr64 (MSR_IA32_SMM_MONITOR_CTL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_SMM_MONITOR_CTL                 0x0000009B

/**
  MSR information returned for MSR index #MSR_IA32_SMM_MONITOR_CTL
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Valid (R/W).  The STM may be invoked using VMCALL only if this
    /// bit is 1. Because VMCALL is used to activate the dual-monitor treatment
    /// (see Section 34.15.6), the dual-monitor treatment cannot be activated
    /// if the bit is 0. This bit is cleared when the logical processor is
    /// reset.
    ///
    UINT32  Valid:1;
    UINT32  Reserved1:1;
    ///
    /// [Bit 2] Determines whether executions of VMXOFF unblock SMIs under the
    /// default treatment of SMIs and SMM.  Executions of VMXOFF unblock SMIs
    /// unless bit 2 is 1 (the value of bit 0 is irrelevant).
    ///
    UINT32  BlockSmi:1;
    UINT32  Reserved2:9;
    ///
    /// [Bits 31:12] MSEG Base (R/W).
    ///
    UINT32  MsegBase:20;
    UINT32  Reserved3:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_SMM_MONITOR_CTL_REGISTER;


/**
  Base address of the logical processor's SMRAM image (RO, SMM only). If
  IA32_VMX_MISC[15].

  @param  ECX  MSR_IA32_SMBASE (0x0000009E)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_SMBASE);
  @endcode
**/
#define MSR_IA32_SMBASE                          0x0000009E


/**
  General Performance Counters (R/W).
  MSR_IA32_PMCn is supported if CPUID.0AH: EAX[15:8] > n.

  @param  ECX  MSR_IA32_PMCn
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_PMC0);
  AsmWriteMsr64 (MSR_IA32_PMC0, Msr);
  @endcode
  @{
**/
#define MSR_IA32_PMC0                            0x000000C1
#define MSR_IA32_PMC1                            0x000000C2
#define MSR_IA32_PMC2                            0x000000C3
#define MSR_IA32_PMC3                            0x000000C4
#define MSR_IA32_PMC4                            0x000000C5
#define MSR_IA32_PMC5                            0x000000C6
#define MSR_IA32_PMC6                            0x000000C7
#define MSR_IA32_PMC7                            0x000000C8
/// @}


/**
  TSC Frequency Clock Counter (R/Write to clear). If CPUID.06H: ECX[0] = 1.
  C0_MCNT: C0 TSC Frequency Clock Count Increments at fixed interval (relative
  to TSC freq.) when the logical processor is in C0. Cleared upon overflow /
  wrap-around of IA32_APERF.

  @param  ECX  MSR_IA32_MPERF (0x000000E7)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MPERF);
  AsmWriteMsr64 (MSR_IA32_MPERF, Msr);
  @endcode
**/
#define MSR_IA32_MPERF                           0x000000E7


/**
  Actual Performance Clock Counter (R/Write to clear). If CPUID.06H: ECX[0] =
  1. C0_ACNT: C0 Actual Frequency Clock Count Accumulates core clock counts at
  the coordinated clock frequency, when the logical processor is in C0.
  Cleared upon overflow / wrap-around of IA32_MPERF.

  @param  ECX  MSR_IA32_APERF (0x000000E8)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_APERF);
  AsmWriteMsr64 (MSR_IA32_APERF, Msr);
  @endcode
**/
#define MSR_IA32_APERF                           0x000000E8


/**
  MTRR Capability (RO) Section 11.11.2.1, "IA32_MTRR_DEF_TYPE MSR.".
  Introduced at Display Family / Display Model 06_01H.

  @param  ECX  MSR_IA32_MTRRCAP (0x000000FE)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_MTRRCAP_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_MTRRCAP_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_MTRRCAP_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_MTRRCAP);
  @endcode
**/
#define MSR_IA32_MTRRCAP                         0x000000FE

/**
  MSR information returned for MSR index #MSR_IA32_MTRRCAP
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 7:0] VCNT: The number of variable memory type ranges in the
    /// processor.
    ///
    UINT32  VCNT:8;
    ///
    /// [Bit 8] Fixed range MTRRs are supported when set.
    ///
    UINT32  FIX:1;
    UINT32  Reserved1:1;
    ///
    /// [Bit 10] WC Supported when set.
    ///
    UINT32  WC:1;
    ///
    /// [Bit 11] SMRR Supported when set.
    ///
    UINT32  SMRR:1;
    UINT32  Reserved2:20;
    UINT32  Reserved3:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_MTRRCAP_REGISTER;


/**
  SYSENTER_CS_MSR (R/W). Introduced at Display Family / Display Model 06_01H.

  @param  ECX  MSR_IA32_SYSENTER_CS (0x00000174)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_SYSENTER_CS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_SYSENTER_CS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_SYSENTER_CS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_SYSENTER_CS);
  AsmWriteMsr64 (MSR_IA32_SYSENTER_CS, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_SYSENTER_CS                     0x00000174

/**
  MSR information returned for MSR index #MSR_IA32_SYSENTER_CS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 15:0] CS Selector.
    ///
    UINT32  CS:16;
    UINT32  Reserved1:16;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_SYSENTER_CS_REGISTER;


/**
  SYSENTER_ESP_MSR (R/W). Introduced at Display Family / Display Model 06_01H.

  @param  ECX  MSR_IA32_SYSENTER_ESP (0x00000175)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_SYSENTER_ESP);
  AsmWriteMsr64 (MSR_IA32_SYSENTER_ESP, Msr);
  @endcode
**/
#define MSR_IA32_SYSENTER_ESP                    0x00000175


/**
  SYSENTER_EIP_MSR (R/W). Introduced at Display Family / Display Model 06_01H.

  @param  ECX  MSR_IA32_SYSENTER_EIP (0x00000176)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_SYSENTER_EIP);
  AsmWriteMsr64 (MSR_IA32_SYSENTER_EIP, Msr);
  @endcode
**/
#define MSR_IA32_SYSENTER_EIP                    0x00000176


/**
  Global Machine Check Capability (RO). Introduced at Display Family / Display
  Model 06_01H.

  @param  ECX  MSR_IA32_MCG_CAP (0x00000179)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_MCG_CAP_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_MCG_CAP_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_MCG_CAP_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_MCG_CAP);
  @endcode
**/
#define MSR_IA32_MCG_CAP                         0x00000179

/**
  MSR information returned for MSR index #MSR_IA32_MCG_CAP
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 7:0] Count: Number of reporting banks.
    ///
    UINT32  Count:8;
    ///
    /// [Bit 8] MCG_CTL_P: IA32_MCG_CTL is present if this bit is set.
    ///
    UINT32  MCG_CTL_P:1;
    ///
    /// [Bit 9] MCG_EXT_P: Extended machine check state registers are present
    /// if this bit is set.
    ///
    UINT32  MCG_EXT_P:1;
    ///
    /// [Bit 10] MCP_CMCI_P: Support for corrected MC error event is present.
    /// Introduced at Display Family / Display Model 06_01H.
    ///
    UINT32  MCP_CMCI_P:1;
    ///
    /// [Bit 11] MCG_TES_P: Threshold-based error status register are present
    /// if this bit is set.
    ///
    UINT32  MCG_TES_P:1;
    UINT32  Reserved1:4;
    ///
    /// [Bits 23:16] MCG_EXT_CNT: Number of extended machine check state
    /// registers present.
    ///
    UINT32  MCG_EXT_CNT:8;
    ///
    /// [Bit 24] MCG_SER_P: The processor supports software error recovery if
    /// this bit is set.
    ///
    UINT32  MCG_SER_P:1;
    UINT32  Reserved2:1;
    ///
    /// [Bit 26] MCG_ELOG_P: Indicates that the processor allows platform
    /// firmware to be invoked when an error is detected so that it may
    /// provide additional platform specific information in an ACPI format
    /// "Generic Error Data Entry" that augments the data included in machine
    /// check bank registers. Introduced at Display Family / Display Model
    /// 06_3EH.
    ///
    UINT32  MCG_ELOG_P:1;
    ///
    /// [Bit 27] MCG_LMCE_P: Indicates that the processor support extended
    /// state in IA32_MCG_STATUS and associated MSR necessary to configure
    /// Local Machine Check Exception (LMCE). Introduced at Display Family /
    /// Display Model 06_3EH.
    ///
    UINT32  MCG_LMCE_P:1;
    UINT32  Reserved3:4;
    UINT32  Reserved4:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_MCG_CAP_REGISTER;


/**
  Global Machine Check Status (R/W0). Introduced at Display Family / Display
  Model 06_01H.

  @param  ECX  MSR_IA32_MCG_STATUS (0x0000017A)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_MCG_STATUS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_MCG_STATUS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_MCG_STATUS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_MCG_STATUS);
  AsmWriteMsr64 (MSR_IA32_MCG_STATUS, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_MCG_STATUS                      0x0000017A

/**
  MSR information returned for MSR index #MSR_IA32_MCG_STATUS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] RIPV. Restart IP valid. Introduced at Display Family / Display
    /// Model 06_01H.
    ///
    UINT32  RIPV:1;
    ///
    /// [Bit 1] EIPV. Error IP valid. Introduced at Display Family / Display
    /// Model 06_01H.
    ///
    UINT32  EIPV:1;
    ///
    /// [Bit 2] MCIP. Machine check in progress. Introduced at Display Family
    /// / Display Model 06_01H.
    ///
    UINT32  MCIP:1;
    ///
    /// [Bit 3] LMCE_S. If IA32_MCG_CAP.LMCE_P[2 7] =1.
    ///
    UINT32  LMCE_S:1;
    UINT32  Reserved1:28;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_MCG_STATUS_REGISTER;


/**
  Global Machine Check Control (R/W). If IA32_MCG_CAP.CTL_P[8] =1.

  @param  ECX  MSR_IA32_MCG_CTL (0x0000017B)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MCG_CTL);
  AsmWriteMsr64 (MSR_IA32_MCG_CTL, Msr);
  @endcode
**/
#define MSR_IA32_MCG_CTL                         0x0000017B


/**
  Performance Event Select Register n (R/W). If CPUID.0AH: EAX[15:8] > n.

  @param  ECX  MSR_IA32_PERFEVTSELn
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PERFEVTSEL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PERFEVTSEL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PERFEVTSEL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PERFEVTSEL0);
  AsmWriteMsr64 (MSR_IA32_PERFEVTSEL0, Msr.Uint64);
  @endcode
  @{
**/
#define MSR_IA32_PERFEVTSEL0                     0x00000186
#define MSR_IA32_PERFEVTSEL1                     0x00000187
#define MSR_IA32_PERFEVTSEL2                     0x00000188
#define MSR_IA32_PERFEVTSEL3                     0x00000189
/// @}

/**
  MSR information returned for MSR indexes #MSR_IA32_PERFEVTSEL0 to
  #MSR_IA32_PERFEVTSEL3
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 7:0] Event Select: Selects a performance event logic unit.
    ///
    UINT32  EventSelect:8;
    ///
    /// [Bits 15:8] UMask: Qualifies the microarchitectural condition to
    /// detect on the selected event logic.
    ///
    UINT32  UMASK:8;
    ///
    /// [Bit 16] USR: Counts while in privilege level is not ring 0.
    ///
    UINT32  USR:1;
    ///
    /// [Bit 17] OS: Counts while in privilege level is ring 0.
    ///
    UINT32  OS:1;
    ///
    /// [Bit 18] Edge: Enables edge detection if set.
    ///
    UINT32  E:1;
    ///
    /// [Bit 19] PC: enables pin control.
    ///
    UINT32  PC:1;
    ///
    /// [Bit 20] INT: enables interrupt on counter overflow.
    ///
    UINT32  INT:1;
    ///
    /// [Bit 21] AnyThread: When set to 1, it enables counting the associated
    /// event conditions occurring across all logical processors sharing a
    /// processor core. When set to 0, the counter only increments the
    /// associated event conditions occurring in the logical processor which
    /// programmed the MSR.
    ///
    UINT32  ANY:1;
    ///
    /// [Bit 22] EN: enables the corresponding performance counter to commence
    /// counting when this bit is set.
    ///
    UINT32  EN:1;
    ///
    /// [Bit 23] INV: invert the CMASK.
    ///
    UINT32  INV:1;
    ///
    /// [Bits 31:24] CMASK: When CMASK is not zero, the corresponding
    /// performance counter increments each cycle if the event count is
    /// greater than or equal to the CMASK.
    ///
    UINT32  CMASK:8;
    UINT32  Reserved:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PERFEVTSEL_REGISTER;


/**
  Current performance state(P-State) operating point (RO). Introduced at
  Display Family / Display Model 0F_03H.

  @param  ECX  MSR_IA32_PERF_STATUS (0x00000198)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_STATUS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_STATUS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PERF_STATUS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PERF_STATUS);
  @endcode
**/
#define MSR_IA32_PERF_STATUS                     0x00000198

/**
  MSR information returned for MSR index #MSR_IA32_PERF_STATUS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 15:0] Current performance State Value.
    ///
    UINT32  State:16;
    UINT32  Reserved1:16;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PERF_STATUS_REGISTER;


/**
  (R/W). Introduced at Display Family / Display Model 0F_03H.

  @param  ECX  MSR_IA32_PERF_CTL (0x00000199)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_CTL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_CTL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PERF_CTL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PERF_CTL);
  AsmWriteMsr64 (MSR_IA32_PERF_CTL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PERF_CTL                        0x00000199

/**
  MSR information returned for MSR index #MSR_IA32_PERF_CTL
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 15:0] Target performance State Value.
    ///
    UINT32  TargetState:16;
    UINT32  Reserved1:16;
    ///
    /// [Bit 32] IDA Engage. (R/W) When set to 1: disengages IDA. 06_0FH
    /// (Mobile only).
    ///
    UINT32  IDA:1;
    UINT32  Reserved2:31;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PERF_CTL_REGISTER;


/**
  Clock Modulation Control (R/W) See Section 14.7.3, "Software Controlled
  Clock Modulation.". Introduced at Display Family / Display Model 0F_0H.

  @param  ECX  MSR_IA32_CLOCK_MODULATION (0x0000019A)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_CLOCK_MODULATION_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_CLOCK_MODULATION_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_CLOCK_MODULATION_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_CLOCK_MODULATION);
  AsmWriteMsr64 (MSR_IA32_CLOCK_MODULATION, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_CLOCK_MODULATION                0x0000019A

/**
  MSR information returned for MSR index #MSR_IA32_CLOCK_MODULATION
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Extended On-Demand Clock Modulation Duty Cycle:. If
    /// CPUID.06H:EAX[5] = 1.
    ///
    UINT32  ExtendedOnDemandClockModulationDutyCycle:1;
    ///
    /// [Bits 3:1] On-Demand Clock Modulation Duty Cycle: Specific encoded
    /// values for target duty cycle modulation.
    ///
    UINT32  OnDemandClockModulationDutyCycle:3;
    ///
    /// [Bit 4] On-Demand Clock Modulation Enable: Set 1 to enable modulation.
    ///
    UINT32  OnDemandClockModulationEnable:1;
    UINT32  Reserved1:27;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_CLOCK_MODULATION_REGISTER;


/**
  Thermal Interrupt Control (R/W) Enables and disables the generation of an
  interrupt on temperature transitions detected with the processor's thermal
  sensors and thermal monitor. See Section 14.7.2, "Thermal Monitor.".
  Introduced at Display Family / Display Model 0F_0H.

  @param  ECX  MSR_IA32_THERM_INTERRUPT (0x0000019B)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_THERM_INTERRUPT_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_THERM_INTERRUPT_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_THERM_INTERRUPT_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_THERM_INTERRUPT);
  AsmWriteMsr64 (MSR_IA32_THERM_INTERRUPT, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_THERM_INTERRUPT                 0x0000019B

/**
  MSR information returned for MSR index #MSR_IA32_THERM_INTERRUPT
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] High-Temperature Interrupt Enable.
    ///
    UINT32  HighTempEnable:1;
    ///
    /// [Bit 1] Low-Temperature Interrupt Enable.
    ///
    UINT32  LowTempEnable:1;
    ///
    /// [Bit 2] PROCHOT# Interrupt Enable.
    ///
    UINT32  PROCHOT_Enable:1;
    ///
    /// [Bit 3] FORCEPR# Interrupt Enable.
    ///
    UINT32  FORCEPR_Enable:1;
    ///
    /// [Bit 4] Critical Temperature Interrupt Enable.
    ///
    UINT32  CriticalTempEnable:1;
    UINT32  Reserved1:3;
    ///
    /// [Bits 14:8] Threshold #1 Value.
    ///
    UINT32  Threshold1:7;
    ///
    /// [Bit 15] Threshold #1 Interrupt Enable.
    ///
    UINT32  Threshold1Enable:1;
    ///
    /// [Bits 22:16] Threshold #2 Value.
    ///
    UINT32  Threshold2:7;
    ///
    /// [Bit 23] Threshold #2 Interrupt Enable.
    ///
    UINT32  Threshold2Enable:1;
    ///
    /// [Bit 24] Power Limit Notification Enable. If CPUID.06H:EAX[4] = 1.
    ///
    UINT32  PowerLimitNotificationEnable:1;
    UINT32  Reserved2:7;
    UINT32  Reserved3:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_THERM_INTERRUPT_REGISTER;


/**
  Thermal Status Information (RO) Contains status information about the
  processor's thermal sensor and automatic thermal monitoring facilities. See
  Section 14.7.2, "Thermal Monitor". Introduced at Display Family / Display
  Model 0F_0H.

  @param  ECX  MSR_IA32_THERM_STATUS (0x0000019C)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_THERM_STATUS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_THERM_STATUS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_THERM_STATUS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_THERM_STATUS);
  @endcode
**/
#define MSR_IA32_THERM_STATUS                    0x0000019C

/**
  MSR information returned for MSR index #MSR_IA32_THERM_STATUS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Thermal Status (RO):.
    ///
    UINT32  ThermalStatus:1;
    ///
    /// [Bit 1] Thermal Status Log (R/W):.
    ///
    UINT32  ThermalStatusLog:1;
    ///
    /// [Bit 2] PROCHOT # or FORCEPR# event (RO).
    ///
    UINT32  PROCHOT_FORCEPR_Event:1;
    ///
    /// [Bit 3] PROCHOT # or FORCEPR# log (R/WC0).
    ///
    UINT32  PROCHOT_FORCEPR_Log:1;
    ///
    /// [Bit 4] Critical Temperature Status (RO).
    ///
    UINT32  CriticalTempStatus:1;
    ///
    /// [Bit 5] Critical Temperature Status log (R/WC0).
    ///
    UINT32  CriticalTempStatusLog:1;
    ///
    /// [Bit 6] Thermal Threshold #1 Status (RO). If CPUID.01H:ECX[8] = 1.
    ///
    UINT32  ThermalThreshold1Status:1;
    ///
    /// [Bit 7] Thermal Threshold #1 log (R/WC0). If CPUID.01H:ECX[8] = 1.
    ///
    UINT32  ThermalThreshold1Log:1;
    ///
    /// [Bit 8] Thermal Threshold #2 Status (RO). If CPUID.01H:ECX[8] = 1.
    ///
    UINT32  ThermalThreshold2Status:1;
    ///
    /// [Bit 9] Thermal Threshold #2 log (R/WC0). If CPUID.01H:ECX[8] = 1.
    ///
    UINT32  ThermalThreshold2Log:1;
    ///
    /// [Bit 10] Power Limitation Status (RO). If CPUID.06H:EAX[4] = 1.
    ///
    UINT32  PowerLimitStatus:1;
    ///
    /// [Bit 11] Power Limitation log (R/WC0). If CPUID.06H:EAX[4] = 1.
    ///
    UINT32  PowerLimitLog:1;
    ///
    /// [Bit 12] Current Limit Status (RO). If CPUID.06H:EAX[7] = 1.
    ///
    UINT32  CurrentLimitStatus:1;
    ///
    /// [Bit 13] Current Limit log (R/WC0). If CPUID.06H:EAX[7] = 1.
    ///
    UINT32  CurrentLimitLog:1;
    ///
    /// [Bit 14] Cross Domain Limit Status (RO). If CPUID.06H:EAX[7] = 1.
    ///
    UINT32  CrossDomainLimitStatus:1;
    ///
    /// [Bit 15] Cross Domain Limit log (R/WC0). If CPUID.06H:EAX[7] = 1.
    ///
    UINT32  CrossDomainLimitLog:1;
    ///
    /// [Bits 22:16] Digital Readout (RO). If CPUID.06H:EAX[0] = 1.
    ///
    UINT32  DigitalReadout:7;
    UINT32  Reserved1:4;
    ///
    /// [Bits 30:27] Resolution in Degrees Celsius (RO). If CPUID.06H:EAX[0] =
    /// 1.
    ///
    UINT32  ResolutionInDegreesCelsius:4;
    ///
    /// [Bit 31] Reading Valid (RO). If CPUID.06H:EAX[0] = 1.
    ///
    UINT32  ReadingValid:1;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_THERM_STATUS_REGISTER;


/**
  Enable Misc. Processor Features (R/W)  Allows a variety of processor
  functions to be enabled and disabled.

  @param  ECX  MSR_IA32_MISC_ENABLE (0x000001A0)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_MISC_ENABLE_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_MISC_ENABLE_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_MISC_ENABLE_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_MISC_ENABLE                     0x000001A0

/**
  MSR information returned for MSR index #MSR_IA32_MISC_ENABLE
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Fast-Strings Enable When set, the fast-strings feature (for
    /// REP MOVS and REP STORS) is enabled (default); when clear, fast-strings
    /// are disabled. Introduced at Display Family / Display Model 0F_0H.
    ///
    UINT32  FastStrings:1;
    UINT32  Reserved1:2;
    ///
    /// [Bit 3] Automatic Thermal Control Circuit Enable (R/W)  1 = Setting
    /// this bit enables the thermal control circuit (TCC) portion of the
    /// Intel Thermal Monitor feature. This allows the processor to
    /// automatically reduce power consumption in response to TCC activation.
    /// 0 = Disabled. Note: In some products clearing this bit might be
    /// ignored in critical thermal conditions, and TM1, TM2 and adaptive
    /// thermal throttling will still be activated. Introduced at Display
    /// Family / Display Model 0F_0H.
    ///
    UINT32  AutomaticThermalControlCircuit:1;
    UINT32  Reserved2:3;
    ///
    /// [Bit 7] Performance Monitoring Available (R)  1 = Performance
    /// monitoring enabled 0 = Performance monitoring disabled. Introduced at
    /// Display Family / Display Model 0F_0H.
    ///
    UINT32  PerformanceMonitoring:1;
    UINT32  Reserved3:3;
    ///
    /// [Bit 11] Branch Trace Storage Unavailable (RO) 1 = Processor doesn't
    /// support branch trace storage (BTS) 0 = BTS is supported. Introduced at
    /// Display Family / Display Model 0F_0H.
    ///
    UINT32  BTS:1;
    ///
    /// [Bit 12] Precise Event Based Sampling (PEBS)  Unavailable (RO)  1 =
    /// PEBS is not supported; 0 = PEBS is supported. Introduced at Display
    /// Family / Display Model 06_0FH.
    ///
    UINT32  PEBS:1;
    UINT32  Reserved4:3;
    ///
    /// [Bit 16] Enhanced Intel SpeedStep Technology  Enable (R/W) 0= Enhanced
    /// Intel SpeedStep Technology disabled 1 = Enhanced Intel SpeedStep
    /// Technology enabled. If CPUID.01H: ECX[7] =1.
    ///
    UINT32  EIST:1;
    UINT32  Reserved5:1;
    ///
    /// [Bit 18] ENABLE MONITOR FSM (R/W) When this bit is set to 0, the
    /// MONITOR feature flag is not set (CPUID.01H:ECX[bit 3] = 0). This
    /// indicates that MONITOR/MWAIT are not supported. Software attempts to
    /// execute MONITOR/MWAIT will cause #UD when this bit is 0. When this bit
    /// is set to 1 (default), MONITOR/MWAIT are supported (CPUID.01H:ECX[bit
    /// 3] = 1). If the SSE3 feature flag ECX[0] is not set (CPUID.01H:ECX[bit
    /// 0] = 0), the OS must not attempt to alter this bit. BIOS must leave it
    /// in the default state. Writing this bit when the SSE3 feature flag is
    /// set to 0 may generate a #GP exception. Introduced at Display Family /
    /// Display Model 0F_03H.
    ///
    UINT32  MONITOR:1;
    UINT32  Reserved6:3;
    ///
    /// [Bit 22] Limit CPUID Maxval (R/W) When this bit is set to 1, CPUID.00H
    /// returns a maximum value in EAX[7:0] of 3. BIOS should contain a setup
    /// question that allows users to specify when the installed OS does not
    /// support CPUID functions greater than 3. Before setting this bit, BIOS
    /// must execute the CPUID.0H and examine the maximum value returned in
    /// EAX[7:0]. If the maximum value is greater than 3, the bit is
    /// supported. Otherwise, the bit is not supported. Writing to this bit
    /// when the maximum value is greater than 3 may generate a #GP exception.
    /// Setting this bit may cause unexpected behavior in software that
    /// depends on the availability of CPUID leaves greater than 3. Introduced
    /// at Display Family / Display Model 0F_03H.
    ///
    UINT32  LimitCpuidMaxval:1;
    ///
    /// [Bit 23] xTPR Message Disable (R/W) When set to 1, xTPR messages are
    /// disabled. xTPR messages are optional messages that allow the processor
    /// to inform the chipset of its priority. if CPUID.01H:ECX[14] = 1.
    ///
    UINT32  xTPR_Message_Disable:1;
    UINT32  Reserved7:8;
    UINT32  Reserved8:2;
    ///
    /// [Bit 34] XD Bit Disable (R/W) When set to 1, the Execute Disable Bit
    /// feature (XD Bit) is disabled and the XD Bit extended feature flag will
    /// be clear (CPUID.80000001H: EDX[20]=0). When set to a 0 (default), the
    /// Execute Disable Bit feature (if available) allows the OS to enable PAE
    /// paging and take advantage of data only pages. BIOS must not alter the
    /// contents of this bit location, if XD bit is not supported. Writing
    /// this bit to 1 when the XD Bit extended feature flag is set to 0 may
    /// generate a #GP exception. if CPUID.80000001H:EDX[2 0] = 1.
    ///
    UINT32  XD:1;
    UINT32  Reserved9:29;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_MISC_ENABLE_REGISTER;


/**
  Performance Energy Bias Hint (R/W). if CPUID.6H:ECX[3] = 1.

  @param  ECX  MSR_IA32_ENERGY_PERF_BIAS (0x000001B0)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_ENERGY_PERF_BIAS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_ENERGY_PERF_BIAS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_ENERGY_PERF_BIAS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_ENERGY_PERF_BIAS);
  AsmWriteMsr64 (MSR_IA32_ENERGY_PERF_BIAS, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_ENERGY_PERF_BIAS                0x000001B0

/**
  MSR information returned for MSR index #MSR_IA32_ENERGY_PERF_BIAS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 3:0] Power Policy Preference: 0 indicates preference to highest
    /// performance. 15 indicates preference to maximize energy saving.
    ///
    UINT32  PowerPolicyPreference:4;
    UINT32  Reserved1:28;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_ENERGY_PERF_BIAS_REGISTER;


/**
  Package Thermal Status Information (RO) Contains status information about
  the package's thermal sensor. See Section 14.8, "Package Level Thermal
  Management.". If CPUID.06H: EAX[6] = 1.

  @param  ECX  MSR_IA32_PACKAGE_THERM_STATUS (0x000001B1)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PACKAGE_THERM_STATUS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PACKAGE_THERM_STATUS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PACKAGE_THERM_STATUS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PACKAGE_THERM_STATUS);
  @endcode
**/
#define MSR_IA32_PACKAGE_THERM_STATUS            0x000001B1

/**
  MSR information returned for MSR index #MSR_IA32_PACKAGE_THERM_STATUS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Pkg Thermal Status (RO):.
    ///
    UINT32  ThermalStatus:1;
    ///
    /// [Bit 1] Pkg Thermal Status Log (R/W):.
    ///
    UINT32  ThermalStatusLog:1;
    ///
    /// [Bit 2] Pkg PROCHOT # event (RO).
    ///
    UINT32  PROCHOT_Event:1;
    ///
    /// [Bit 3] Pkg PROCHOT # log (R/WC0).
    ///
    UINT32  PROCHOT_Log:1;
    ///
    /// [Bit 4] Pkg Critical Temperature Status (RO).
    ///
    UINT32  CriticalTempStatus:1;
    ///
    /// [Bit 5] Pkg Critical Temperature Status log (R/WC0).
    ///
    UINT32  CriticalTempStatusLog:1;
    ///
    /// [Bit 6] Pkg Thermal Threshold #1 Status (RO).
    ///
    UINT32  ThermalThreshold1Status:1;
    ///
    /// [Bit 7] Pkg Thermal Threshold #1 log (R/WC0).
    ///
    UINT32  ThermalThreshold1Log:1;
    ///
    /// [Bit 8] Pkg Thermal Threshold #2 Status (RO).
    ///
    UINT32  ThermalThreshold2Status:1;
    ///
    /// [Bit 9] Pkg Thermal Threshold #1 log (R/WC0).
    ///
    UINT32  ThermalThreshold2Log:1;
    ///
    /// [Bit 10] Pkg Power Limitation Status (RO).
    ///
    UINT32  PowerLimitStatus:1;
    ///
    /// [Bit 11] Pkg Power Limitation log (R/WC0).
    ///
    UINT32  PowerLimitLog:1;
    UINT32  Reserved1:4;
    ///
    /// [Bits 22:16] Pkg Digital Readout (RO).
    ///
    UINT32  DigitalReadout:7;
    UINT32  Reserved2:9;
    UINT32  Reserved3:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PACKAGE_THERM_STATUS_REGISTER;


/**
  Pkg Thermal Interrupt Control (R/W) Enables and disables the generation of
  an interrupt on temperature transitions detected with the package's thermal
  sensor. See Section 14.8, "Package Level Thermal Management.". If CPUID.06H:
  EAX[6] = 1.

  @param  ECX  MSR_IA32_PACKAGE_THERM_INTERRUPT (0x000001B2)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PACKAGE_THERM_INTERRUPT_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PACKAGE_THERM_INTERRUPT_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PACKAGE_THERM_INTERRUPT_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PACKAGE_THERM_INTERRUPT);
  AsmWriteMsr64 (MSR_IA32_PACKAGE_THERM_INTERRUPT, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PACKAGE_THERM_INTERRUPT         0x000001B2

/**
  MSR information returned for MSR index #MSR_IA32_PACKAGE_THERM_INTERRUPT
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Pkg High-Temperature Interrupt Enable.
    ///
    UINT32  HighTempEnable:1;
    ///
    /// [Bit 1] Pkg Low-Temperature Interrupt Enable.
    ///
    UINT32  LowTempEnable:1;
    ///
    /// [Bit 2] Pkg PROCHOT# Interrupt Enable.
    ///
    UINT32  PROCHOT_Enable:1;
    UINT32  Reserved1:1;
    ///
    /// [Bit 4] Pkg Overheat Interrupt Enable.
    ///
    UINT32  OverheatEnable:1;
    UINT32  Reserved2:3;
    ///
    /// [Bits 14:8] Pkg Threshold #1 Value.
    ///
    UINT32  Threshold1:7;
    ///
    /// [Bit 15] Pkg Threshold #1 Interrupt Enable.
    ///
    UINT32  Threshold1Enable:1;
    ///
    /// [Bits 22:16] Pkg Threshold #2 Value.
    ///
    UINT32  Threshold2:7;
    ///
    /// [Bit 23] Pkg Threshold #2 Interrupt Enable.
    ///
    UINT32  Threshold2Enable:1;
    ///
    /// [Bit 24] Pkg Power Limit Notification Enable.
    ///
    UINT32  PowerLimitNotificationEnable:1;
    UINT32  Reserved3:7;
    UINT32  Reserved4:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PACKAGE_THERM_INTERRUPT_REGISTER;


/**
  Trace/Profile Resource Control (R/W). Introduced at Display Family / Display
  Model 06_0EH.

  @param  ECX  MSR_IA32_DEBUGCTL (0x000001D9)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_DEBUGCTL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_DEBUGCTL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_DEBUGCTL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_DEBUGCTL);
  AsmWriteMsr64 (MSR_IA32_DEBUGCTL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_DEBUGCTL                        0x000001D9

/**
  MSR information returned for MSR index #MSR_IA32_DEBUGCTL
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] LBR: Setting this bit to 1 enables the processor to record a
    /// running trace of the most recent branches taken by the processor in
    /// the LBR stack. Introduced at Display Family / Display Model 06_01H.
    ///
    UINT32  LBR:1;
    ///
    /// [Bit 1] BTF: Setting this bit to 1 enables the processor to treat
    /// EFLAGS.TF as single-step on branches instead of single-step on
    /// instructions. Introduced at Display Family / Display Model 06_01H.
    ///
    UINT32  BTF:1;
    UINT32  Reserved1:4;
    ///
    /// [Bit 6] TR: Setting this bit to 1 enables branch trace messages to be
    /// sent. Introduced at Display Family / Display Model 06_0EH.
    ///
    UINT32  TR:1;
    ///
    /// [Bit 7] BTS: Setting this bit enables branch trace messages (BTMs) to
    /// be logged in a BTS buffer. Introduced at Display Family / Display
    /// Model 06_0EH.
    ///
    UINT32  BTS:1;
    ///
    /// [Bit 8] BTINT: When clear, BTMs are logged in a BTS buffer in circular
    /// fashion. When this bit is set, an interrupt is generated by the BTS
    /// facility when the BTS buffer is full. Introduced at Display Family /
    /// Display Model 06_0EH.
    ///
    UINT32  BTINT:1;
    ///
    /// [Bit 9] BTS_OFF_OS: When set, BTS or BTM is skipped if CPL = 0.
    /// Introduced at Display Family / Display Model 06_0FH.
    ///
    UINT32  BTS_OFF_OS:1;
    ///
    /// [Bit 10] BTS_OFF_USR: When set, BTS or BTM is skipped if CPL > 0.
    /// Introduced at Display Family / Display Model 06_0FH.
    ///
    UINT32  BTS_OFF_USR:1;
    ///
    /// [Bit 11] FREEZE_LBRS_ON_PMI: When set, the LBR stack is frozen on a
    /// PMI request. If CPUID.01H: ECX[15] = 1 && CPUID.0AH: EAX[7:0] > 1.
    ///
    UINT32  FREEZE_LBRS_ON_PMI:1;
    ///
    /// [Bit 12] FREEZE_PERFMON_ON_PMI: When set, each ENABLE bit of the
    /// global counter control MSR are frozen (address 38FH) on a PMI request.
    /// If CPUID.01H: ECX[15] = 1 && CPUID.0AH: EAX[7:0] > 1.
    ///
    UINT32  FREEZE_PERFMON_ON_PMI:1;
    ///
    /// [Bit 13] ENABLE_UNCORE_PMI: When set, enables the logical processor to
    /// receive and generate PMI on behalf of the uncore. Introduced at
    /// Display Family / Display Model 06_1AH.
    ///
    UINT32  ENABLE_UNCORE_PMI:1;
    ///
    /// [Bit 14] FREEZE_WHILE_SMM: When set, freezes perfmon and trace
    /// messages while in SMM. If IA32_PERF_CAPABILITIES[ 12] = 1.
    ///
    UINT32  FREEZE_WHILE_SMM:1;
    ///
    /// [Bit 15] RTM_DEBUG: When set, enables DR7 debug bit on XBEGIN. If
    /// (CPUID.(EAX=07H, ECX=0):EBX[11] = 1).
    ///
    UINT32  RTM_DEBUG:1;
    UINT32  Reserved2:16;
    UINT32  Reserved3:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_DEBUGCTL_REGISTER;


/**
  SMRR Base Address (Writeable only in SMM)  Base address of SMM memory range.
  If IA32_MTRRCAP.SMRR[11] = 1.

  @param  ECX  MSR_IA32_SMRR_PHYSBASE (0x000001F2)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_SMRR_PHYSBASE_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_SMRR_PHYSBASE_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_SMRR_PHYSBASE_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_SMRR_PHYSBASE);
  AsmWriteMsr64 (MSR_IA32_SMRR_PHYSBASE, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_SMRR_PHYSBASE                   0x000001F2

/**
  MSR information returned for MSR index #MSR_IA32_SMRR_PHYSBASE
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 7:0] Type. Specifies memory type of the range.
    ///
    UINT32  Type:8;
    UINT32  Reserved1:4;
    ///
    /// [Bits 31:12] PhysBase.  SMRR physical Base Address.
    ///
    UINT32  PhysBase:20;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_SMRR_PHYSBASE_REGISTER;


/**
  SMRR Range Mask. (Writeable only in SMM)  Range Mask of SMM memory range. If
  IA32_MTRRCAP[SMRR] = 1.

  @param  ECX  MSR_IA32_SMRR_PHYSMASK (0x000001F3)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_SMRR_PHYSMASK_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_SMRR_PHYSMASK_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_SMRR_PHYSMASK_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_SMRR_PHYSMASK);
  AsmWriteMsr64 (MSR_IA32_SMRR_PHYSMASK, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_SMRR_PHYSMASK                   0x000001F3

/**
  MSR information returned for MSR index #MSR_IA32_SMRR_PHYSMASK
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    UINT32  Reserved1:11;
    ///
    /// [Bit 11] Valid Enable range mask.
    ///
    UINT32  Valid:1;
    ///
    /// [Bits 31:12] PhysMask SMRR address range mask.
    ///
    UINT32  PhysMask:20;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_SMRR_PHYSMASK_REGISTER;


/**
  DCA Capability (R). If CPUID.01H: ECX[18] = 1.

  @param  ECX  MSR_IA32_PLATFORM_DCA_CAP (0x000001F8)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_PLATFORM_DCA_CAP);
  @endcode
**/
#define MSR_IA32_PLATFORM_DCA_CAP                0x000001F8


/**
  If set, CPU supports Prefetch-Hint type. If CPUID.01H: ECX[18] = 1.

  @param  ECX  MSR_IA32_CPU_DCA_CAP (0x000001F9)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_CPU_DCA_CAP);
  AsmWriteMsr64 (MSR_IA32_CPU_DCA_CAP, Msr);
  @endcode
**/
#define MSR_IA32_CPU_DCA_CAP                     0x000001F9


/**
  DCA type 0 Status and Control register. If CPUID.01H: ECX[18] = 1.

  @param  ECX  MSR_IA32_DCA_0_CAP (0x000001FA)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_DCA_0_CAP_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_DCA_0_CAP_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_DCA_0_CAP_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_DCA_0_CAP);
  AsmWriteMsr64 (MSR_IA32_DCA_0_CAP, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_DCA_0_CAP                       0x000001FA

/**
  MSR information returned for MSR index #MSR_IA32_DCA_0_CAP
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] DCA_ACTIVE: Set by HW when DCA is fuseenabled and no
    /// defeatures are set.
    ///
    UINT32  DCA_ACTIVE:1;
    ///
    /// [Bits 2:1] TRANSACTION.
    ///
    UINT32  TRANSACTION:2;
    ///
    /// [Bits 6:3] DCA_TYPE.
    ///
    UINT32  DCA_TYPE:4;
    ///
    /// [Bits 10:7] DCA_QUEUE_SIZE.
    ///
    UINT32  DCA_QUEUE_SIZE:4;
    UINT32  Reserved1:2;
    ///
    /// [Bits 16:13] DCA_DELAY: Writes will update the register but have no HW
    /// side-effect.
    ///
    UINT32  DCA_DELAY:4;
    UINT32  Reserved2:7;
    ///
    /// [Bit 24] SW_BLOCK: SW can request DCA block by setting this bit.
    ///
    UINT32  SW_BLOCK:1;
    UINT32  Reserved3:1;
    ///
    /// [Bit 26] HW_BLOCK: Set when DCA is blocked by HW (e.g. CR0.CD = 1).
    ///
    UINT32  HW_BLOCK:1;
    UINT32  Reserved4:5;
    UINT32  Reserved5:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_DCA_0_CAP_REGISTER;


/**
  MTRRphysBasen.  See Section 11.11.2.3, "Variable Range MTRRs".
  If CPUID.01H: EDX.MTRR[12] = 1 and IA32_MTRRCAP[7:0] > n.

  @param  ECX  MSR_IA32_MTRR_PHYSBASEn
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_MTRR_PHYSBASE_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_MTRR_PHYSBASE_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_MTRR_PHYSBASE_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_MTRR_PHYSBASE0);
  AsmWriteMsr64 (MSR_IA32_MTRR_PHYSBASE0, Msr.Uint64);
  @endcode
  @{
**/
#define MSR_IA32_MTRR_PHYSBASE0                  0x00000200
#define MSR_IA32_MTRR_PHYSBASE1                  0x00000202
#define MSR_IA32_MTRR_PHYSBASE2                  0x00000204
#define MSR_IA32_MTRR_PHYSBASE3                  0x00000206
#define MSR_IA32_MTRR_PHYSBASE4                  0x00000208
#define MSR_IA32_MTRR_PHYSBASE5                  0x0000020A
#define MSR_IA32_MTRR_PHYSBASE6                  0x0000020C
#define MSR_IA32_MTRR_PHYSBASE7                  0x0000020E
#define MSR_IA32_MTRR_PHYSBASE8                  0x00000210
#define MSR_IA32_MTRR_PHYSBASE9                  0x00000212
/// @}

/**
  MSR information returned for MSR indexes #MSR_IA32_MTRR_PHYSBASE0 to
  #MSR_IA32_MTRR_PHYSBASE9
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 7:0] Type. Specifies memory type of the range.
    ///
    UINT32  Type:8;
    UINT32  Reserved1:4;
    ///
    /// [Bits 31:12] PhysBase.  MTRR physical Base Address.
    ///
    UINT32  PhysBase:20;
    ///
    /// [Bits MAXPHYSADDR:32] PhysBase.  Upper bits of MTRR physical Base Address.
    /// MAXPHYADDR: The bit position indicated by MAXPHYADDR depends on the
    /// maximum physical address range supported by the processor. It is
    /// reported by CPUID leaf function 80000008H. If CPUID does not support
    /// leaf 80000008H, the processor supports 36-bit physical address size,
    /// then bit PhysMask consists of bits 35:12, and bits 63:36 are reserved.
    ///
    UINT32  PhysBaseHi:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_MTRR_PHYSBASE_REGISTER;


/**
  MTRRphysMaskn.  See Section 11.11.2.3, "Variable Range MTRRs".
  If CPUID.01H: EDX.MTRR[12] = 1 and IA32_MTRRCAP[7:0] > n.

  @param  ECX  MSR_IA32_MTRR_PHYSMASKn
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_MTRR_PHYSMASK_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_MTRR_PHYSMASK_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_MTRR_PHYSMASK_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_MTRR_PHYSMASK0);
  AsmWriteMsr64 (MSR_IA32_MTRR_PHYSMASK0, Msr.Uint64);
  @endcode
  @{
**/
#define MSR_IA32_MTRR_PHYSMASK0                  0x00000201
#define MSR_IA32_MTRR_PHYSMASK1                  0x00000203
#define MSR_IA32_MTRR_PHYSMASK2                  0x00000205
#define MSR_IA32_MTRR_PHYSMASK3                  0x00000207
#define MSR_IA32_MTRR_PHYSMASK4                  0x00000209
#define MSR_IA32_MTRR_PHYSMASK5                  0x0000020B
#define MSR_IA32_MTRR_PHYSMASK6                  0x0000020D
#define MSR_IA32_MTRR_PHYSMASK7                  0x0000020F
#define MSR_IA32_MTRR_PHYSMASK8                  0x00000211
#define MSR_IA32_MTRR_PHYSMASK9                  0x00000213
/// @}

/**
  MSR information returned for MSR indexes #MSR_IA32_MTRR_PHYSMASK0 to
  #MSR_IA32_MTRR_PHYSMASK9
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    UINT32  Reserved1:11;
    ///
    /// [Bit 11] Valid Enable range mask.
    ///
    UINT32  V:8;
    ///
    /// [Bits 31:12] PhysMask.  MTRR address range mask.
    ///
    UINT32  PhysMask:20;
    ///
    /// [Bits MAXPHYSADDR:32] PhysMask.  Upper bits of MTRR address range mask.
    /// MAXPHYADDR: The bit position indicated by MAXPHYADDR depends on the
    /// maximum physical address range supported by the processor. It is
    /// reported by CPUID leaf function 80000008H. If CPUID does not support
    /// leaf 80000008H, the processor supports 36-bit physical address size,
    /// then bit PhysMask consists of bits 35:12, and bits 63:36 are reserved.
    ///
    UINT32  PhysMaskHi:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_MTRR_PHYSMASK_REGISTER;


/**
  MTRRfix64K_00000. If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX64K_00000 (0x00000250)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX64K_00000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX64K_00000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX64K_00000               0x00000250


/**
  MTRRfix16K_80000. If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX16K_80000 (0x00000258)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX16K_80000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX16K_80000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX16K_80000               0x00000258


/**
  MTRRfix16K_A0000. If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX16K_A0000 (0x00000259)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX16K_A0000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX16K_A0000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX16K_A0000               0x00000259


/**
  See Section 11.11.2.2, "Fixed Range MTRRs.". If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX4K_C0000 (0x00000268)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX4K_C0000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX4K_C0000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX4K_C0000                0x00000268


/**
  MTRRfix4K_C8000. If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX4K_C8000 (0x00000269)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX4K_C8000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX4K_C8000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX4K_C8000                0x00000269


/**
  MTRRfix4K_D0000. If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX4K_D0000 (0x0000026A)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX4K_D0000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX4K_D0000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX4K_D0000                0x0000026A


/**
  MTRRfix4K_D8000. If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX4K_D8000 (0x0000026B)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX4K_D8000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX4K_D8000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX4K_D8000                0x0000026B


/**
  MTRRfix4K_E0000. If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX4K_E0000 (0x0000026C)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX4K_E0000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX4K_E0000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX4K_E0000                0x0000026C


/**
  MTRRfix4K_E8000. If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX4K_E8000 (0x0000026D)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX4K_E8000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX4K_E8000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX4K_E8000                0x0000026D


/**
  MTRRfix4K_F0000. If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX4K_F0000 (0x0000026E)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX4K_F0000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX4K_F0000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX4K_F0000                0x0000026E


/**
  MTRRfix4K_F8000. If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_FIX4K_F8000 (0x0000026F)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MTRR_FIX4K_F8000);
  AsmWriteMsr64 (MSR_IA32_MTRR_FIX4K_F8000, Msr);
  @endcode
**/
#define MSR_IA32_MTRR_FIX4K_F8000                0x0000026F


/**
  IA32_PAT (R/W). If CPUID.01H: EDX.MTRR[16] =1.

  @param  ECX  MSR_IA32_PAT (0x00000277)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PAT_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PAT_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PAT_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PAT);
  AsmWriteMsr64 (MSR_IA32_PAT, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PAT                             0x00000277

/**
  MSR information returned for MSR index #MSR_IA32_PAT
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 2:0] PA0.
    ///
    UINT32  PA0:3;
    UINT32  Reserved1:5;
    ///
    /// [Bits 10:8] PA1.
    ///
    UINT32  PA1:3;
    UINT32  Reserved2:5;
    ///
    /// [Bits 18:16] PA2.
    ///
    UINT32  PA2:3;
    UINT32  Reserved3:5;
    ///
    /// [Bits 26:24] PA3.
    ///
    UINT32  PA3:3;
    UINT32  Reserved4:5;
    ///
    /// [Bits 34:32] PA4.
    ///
    UINT32  PA4:3;
    UINT32  Reserved5:5;
    ///
    /// [Bits 42:40] PA5.
    ///
    UINT32  PA5:3;
    UINT32  Reserved6:5;
    ///
    /// [Bits 50:48] PA6.
    ///
    UINT32  PA6:3;
    UINT32  Reserved7:5;
    ///
    /// [Bits 58:56] PA7.
    ///
    UINT32  PA7:3;
    UINT32  Reserved8:5;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PAT_REGISTER;


/**
  Provides the programming interface to use corrected MC error signaling
  capability (R/W). If IA32_MCG_CAP[10] = 1 && IA32_MCG_CAP[7:0] > n.

  @param  ECX  MSR_IA32_MCn_CTL2
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_MC_CTL2_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_MC_CTL2_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_MC_CTL2_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_MC0_CTL2);
  AsmWriteMsr64 (MSR_IA32_MC0_CTL2, Msr.Uint64);
  @endcode
  @{
**/
#define MSR_IA32_MC0_CTL2                        0x00000280
#define MSR_IA32_MC1_CTL2                        0x00000281
#define MSR_IA32_MC2_CTL2                        0x00000282
#define MSR_IA32_MC3_CTL2                        0x00000283
#define MSR_IA32_MC4_CTL2                        0x00000284
#define MSR_IA32_MC5_CTL2                        0x00000285
#define MSR_IA32_MC6_CTL2                        0x00000286
#define MSR_IA32_MC7_CTL2                        0x00000287
#define MSR_IA32_MC8_CTL2                        0x00000288
#define MSR_IA32_MC9_CTL2                        0x00000289
#define MSR_IA32_MC10_CTL2                       0x0000028A
#define MSR_IA32_MC11_CTL2                       0x0000028B
#define MSR_IA32_MC12_CTL2                       0x0000028C
#define MSR_IA32_MC13_CTL2                       0x0000028D
#define MSR_IA32_MC14_CTL2                       0x0000028E
#define MSR_IA32_MC15_CTL2                       0x0000028F
#define MSR_IA32_MC16_CTL2                       0x00000290
#define MSR_IA32_MC17_CTL2                       0x00000291
#define MSR_IA32_MC18_CTL2                       0x00000292
#define MSR_IA32_MC19_CTL2                       0x00000293
#define MSR_IA32_MC20_CTL2                       0x00000294
#define MSR_IA32_MC21_CTL2                       0x00000295
#define MSR_IA32_MC22_CTL2                       0x00000296
#define MSR_IA32_MC23_CTL2                       0x00000297
#define MSR_IA32_MC24_CTL2                       0x00000298
#define MSR_IA32_MC25_CTL2                       0x00000299
#define MSR_IA32_MC26_CTL2                       0x0000029A
#define MSR_IA32_MC27_CTL2                       0x0000029B
#define MSR_IA32_MC28_CTL2                       0x0000029C
#define MSR_IA32_MC29_CTL2                       0x0000029D
#define MSR_IA32_MC30_CTL2                       0x0000029E
#define MSR_IA32_MC31_CTL2                       0x0000029F
/// @}

/**
  MSR information returned for MSR indexes #MSR_IA32_MC0_CTL2
  to #MSR_IA32_MC31_CTL2
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 14:0] Corrected error count threshold.
    ///
    UINT32  CorrectedErrorCountThreshold:15;
    UINT32  Reserved1:15;
    ///
    /// [Bit 30] CMCI_EN.
    ///
    UINT32  CMCI_EN:1;
    UINT32  Reserved2:1;
    UINT32  Reserved3:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_MC_CTL2_REGISTER;


/**
  MTRRdefType (R/W). If CPUID.01H: EDX.MTRR[12] =1.

  @param  ECX  MSR_IA32_MTRR_DEF_TYPE (0x000002FF)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_MTRR_DEF_TYPE_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_MTRR_DEF_TYPE_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_MTRR_DEF_TYPE_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_MTRR_DEF_TYPE);
  AsmWriteMsr64 (MSR_IA32_MTRR_DEF_TYPE, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_MTRR_DEF_TYPE                   0x000002FF

/**
  MSR information returned for MSR index #MSR_IA32_MTRR_DEF_TYPE
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 2:0] Default Memory Type.
    ///
    UINT32  Type:3;
    UINT32  Reserved1:7;
    ///
    /// [Bit 10] Fixed Range MTRR Enable.
    ///
    UINT32  FE:1;
    ///
    /// [Bit 11] MTRR Enable.
    ///
    UINT32  E:1;
    UINT32  Reserved2:20;
    UINT32  Reserved3:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_MTRR_DEF_TYPE_REGISTER;


/**
  Fixed-Function Performance Counter 0 (R/W): Counts Instr_Retired.Any. If
  CPUID.0AH: EDX[4:0] > 0.

  @param  ECX  MSR_IA32_FIXED_CTR0 (0x00000309)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_FIXED_CTR0);
  AsmWriteMsr64 (MSR_IA32_FIXED_CTR0, Msr);
  @endcode
**/
#define MSR_IA32_FIXED_CTR0                      0x00000309


/**
  Fixed-Function Performance Counter 1 0 (R/W): Counts CPU_CLK_Unhalted.Core.
  If CPUID.0AH: EDX[4:0] > 1.

  @param  ECX  MSR_IA32_FIXED_CTR1 (0x0000030A)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_FIXED_CTR1);
  AsmWriteMsr64 (MSR_IA32_FIXED_CTR1, Msr);
  @endcode
**/
#define MSR_IA32_FIXED_CTR1                      0x0000030A


/**
  Fixed-Function Performance Counter 0 0 (R/W): Counts CPU_CLK_Unhalted.Ref.
  If CPUID.0AH: EDX[4:0] > 2.

  @param  ECX  MSR_IA32_FIXED_CTR2 (0x0000030B)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_FIXED_CTR2);
  AsmWriteMsr64 (MSR_IA32_FIXED_CTR2, Msr);
  @endcode
**/
#define MSR_IA32_FIXED_CTR2                      0x0000030B


/**
  RO. If CPUID.01H: ECX[15] = 1.

  @param  ECX  MSR_IA32_PERF_CAPABILITIES (0x00000345)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_CAPABILITIES_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_CAPABILITIES_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PERF_CAPABILITIES_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PERF_CAPABILITIES);
  AsmWriteMsr64 (MSR_IA32_PERF_CAPABILITIES, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PERF_CAPABILITIES               0x00000345

/**
  MSR information returned for MSR index #MSR_IA32_PERF_CAPABILITIES
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 5:0] LBR format.
    ///
    UINT32  LBR_FMT:6;
    ///
    /// [Bit 6] PEBS Trap.
    ///
    UINT32  PEBS_TRAP:1;
    ///
    /// [Bit 7] PEBSSaveArchRegs.
    ///
    UINT32  PEBS_ARCH_REG:1;
    ///
    /// [Bits 11:8] PEBS Record Format.
    ///
    UINT32  PEBS_REC_FMT:4;
    ///
    /// [Bit 12] 1: Freeze while SMM is supported.
    ///
    UINT32  SMM_FREEZE:1;
    ///
    /// [Bit 13] 1: Full width of counter writable via IA32_A_PMCx.
    ///
    UINT32  FW_WRITE:1;
    UINT32  Reserved1:18;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PERF_CAPABILITIES_REGISTER;


/**
  Fixed-Function Performance Counter Control (R/W) Counter increments while
  the results of ANDing respective enable bit in IA32_PERF_GLOBAL_CTRL with
  the corresponding OS or USR bits in this MSR is true. If CPUID.0AH: EAX[7:0]
  > 1.

  @param  ECX  MSR_IA32_FIXED_CTR_CTRL (0x0000038D)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_FIXED_CTR_CTRL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_FIXED_CTR_CTRL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_FIXED_CTR_CTRL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_FIXED_CTR_CTRL);
  AsmWriteMsr64 (MSR_IA32_FIXED_CTR_CTRL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_FIXED_CTR_CTRL                  0x0000038D

/**
  MSR information returned for MSR index #MSR_IA32_FIXED_CTR_CTRL
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] EN0_OS: Enable Fixed Counter 0 to count while CPL = 0.
    ///
    UINT32  EN0_OS:1;
    ///
    /// [Bit 1] EN0_Usr: Enable Fixed Counter 0 to count while CPL > 0.
    ///
    UINT32  EN0_Usr:1;
    ///
    /// [Bit 2] AnyThread: When set to 1, it enables counting the associated
    /// event conditions occurring across all logical processors sharing a
    /// processor core. When set to 0, the counter only increments the
    /// associated event conditions occurring in the logical processor which
    /// programmed the MSR. If CPUID.0AH: EAX[7:0] > 2.
    ///
    UINT32  AnyThread0:1;
    ///
    /// [Bit 3] EN0_PMI: Enable PMI when fixed counter 0 overflows.
    ///
    UINT32  EN0_PMI:1;
    ///
    /// [Bit 4] EN1_OS: Enable Fixed Counter 1 to count while CPL = 0.
    ///
    UINT32  EN1_OS:1;
    ///
    /// [Bit 5] EN1_Usr: Enable Fixed Counter 1 to count while CPL > 0.
    ///
    UINT32  EN1_Usr:1;
    ///
    /// [Bit 6] AnyThread: When set to 1, it enables counting the associated
    /// event conditions occurring across all logical processors sharing a
    /// processor core. When set to 0, the counter only increments the
    /// associated event conditions occurring in the logical processor which
    /// programmed the MSR. If CPUID.0AH: EAX[7:0] > 2.
    ///
    UINT32  AnyThread1:1;
    ///
    /// [Bit 7] EN1_PMI: Enable PMI when fixed counter 1 overflows.
    ///
    UINT32  EN1_PMI:1;
    ///
    /// [Bit 8] EN2_OS: Enable Fixed Counter 2 to count while CPL = 0.
    ///
    UINT32  EN2_OS:1;
    ///
    /// [Bit 9] EN2_Usr: Enable Fixed Counter 2 to count while CPL > 0.
    ///
    UINT32  EN2_Usr:1;
    ///
    /// [Bit 10] AnyThread: When set to 1, it enables counting the associated
    /// event conditions occurring across all logical processors sharing a
    /// processor core. When set to 0, the counter only increments the
    /// associated event conditions occurring in the logical processor which
    /// programmed the MSR. If CPUID.0AH: EAX[7:0] > 2.
    ///
    UINT32  AnyThread2:1;
    ///
    /// [Bit 11] EN2_PMI: Enable PMI when fixed counter 2 overflows.
    ///
    UINT32  EN2_PMI:1;
    UINT32  Reserved1:20;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_FIXED_CTR_CTRL_REGISTER;


/**
  Global Performance Counter Status (RO). If CPUID.0AH: EAX[7:0] > 0.

  @param  ECX  MSR_IA32_PERF_GLOBAL_STATUS (0x0000038E)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_STATUS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_STATUS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PERF_GLOBAL_STATUS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PERF_GLOBAL_STATUS);
  @endcode
**/
#define MSR_IA32_PERF_GLOBAL_STATUS              0x0000038E

/**
  MSR information returned for MSR index #MSR_IA32_PERF_GLOBAL_STATUS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Ovf_PMC0: Overflow status of IA32_PMC0. If CPUID.0AH:
    /// EAX[15:8] > 0.
    ///
    UINT32  Ovf_PMC0:1;
    ///
    /// [Bit 1] Ovf_PMC1: Overflow status of IA32_PMC1. If CPUID.0AH:
    /// EAX[15:8] > 1.
    ///
    UINT32  Ovf_PMC1:1;
    ///
    /// [Bit 2] Ovf_PMC2: Overflow status of IA32_PMC2. If CPUID.0AH:
    /// EAX[15:8] > 2.
    ///
    UINT32  Ovf_PMC2:1;
    ///
    /// [Bit 3] Ovf_PMC3: Overflow status of IA32_PMC3. If CPUID.0AH:
    /// EAX[15:8] > 3.
    ///
    UINT32  Ovf_PMC3:1;
    UINT32  Reserved1:28;
    ///
    /// [Bit 32] Ovf_FixedCtr0: Overflow status of IA32_FIXED_CTR0. If
    /// CPUID.0AH: EAX[7:0] > 1.
    ///
    UINT32  Ovf_FixedCtr0:1;
    ///
    /// [Bit 33] Ovf_FixedCtr1: Overflow status of IA32_FIXED_CTR1. If
    /// CPUID.0AH: EAX[7:0] > 1.
    ///
    UINT32  Ovf_FixedCtr1:1;
    ///
    /// [Bit 34] Ovf_FixedCtr2: Overflow status of IA32_FIXED_CTR2. If
    /// CPUID.0AH: EAX[7:0] > 1.
    ///
    UINT32  Ovf_FixedCtr2:1;
    UINT32  Reserved2:20;
    ///
    /// [Bit 55] Trace_ToPA_PMI: A PMI occurred due to a ToPA entry memory
    /// buffer was completely filled. If (CPUID.(EAX=07H, ECX=0):EBX[25] = 1)
    /// && IA32_RTIT_CTL.ToPA = 1.
    ///
    UINT32  Trace_ToPA_PMI:1;
    UINT32  Reserved3:2;
    ///
    /// [Bit 58] LBR_Frz: LBRs are frozen due to -
    /// IA32_DEBUGCTL.FREEZE_LBR_ON_PMI=1, -  The LBR stack overflowed. If
    /// CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  LBR_Frz:1;
    ///
    /// [Bit 59] CTR_Frz: Performance counters in the core PMU are frozen due
    /// to -  IA32_DEBUGCTL.FREEZE_PERFMON_ON_ PMI=1, -  one or more core PMU
    /// counters overflowed. If CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  CTR_Frz:1;
    ///
    /// [Bit 60] ASCI: Data in the performance counters in the core PMU may
    /// include contributions from the direct or indirect operation intel SGX
    /// to protect an enclave. If CPUID.(EAX=07H, ECX=0):EBX[2] = 1.
    ///
    UINT32  ASCI:1;
    ///
    /// [Bit 61] Ovf_Uncore: Uncore counter overflow status. If CPUID.0AH:
    /// EAX[7:0] > 2.
    ///
    UINT32  Ovf_Uncore:1;
    ///
    /// [Bit 62] OvfBuf: DS SAVE area Buffer overflow status. If CPUID.0AH:
    /// EAX[7:0] > 0.
    ///
    UINT32  OvfBuf:1;
    ///
    /// [Bit 63] CondChgd: status bits of this register has changed. If
    /// CPUID.0AH: EAX[7:0] > 0.
    ///
    UINT32  CondChgd:1;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PERF_GLOBAL_STATUS_REGISTER;


/**
  Global Performance Counter Control (R/W) Counter increments while the result
  of ANDing respective enable bit in this MSR with the corresponding OS or USR
  bits in the general-purpose or fixed counter control MSR is true. If
  CPUID.0AH: EAX[7:0] > 0.

  @param  ECX  MSR_IA32_PERF_GLOBAL_CTRL (0x0000038F)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_CTRL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_CTRL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PERF_GLOBAL_CTRL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PERF_GLOBAL_CTRL);
  AsmWriteMsr64 (MSR_IA32_PERF_GLOBAL_CTRL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PERF_GLOBAL_CTRL                0x0000038F

/**
  MSR information returned for MSR index #MSR_IA32_PERF_GLOBAL_CTRL
**/
typedef union {
  ///
  /// Individual bit fields
///
  struct {
    ///
    /// [Bits 31:0] EN_PMCn. If CPUID.0AH: EAX[15:8] > n.
    /// Enable bitmask.  Only the first n-1 bits are valid.
    /// Bits n..31 are reserved.
    ///
    UINT32  EN_PMCn:32;
    ///
    /// [Bits 63:32] EN_FIXED_CTRn. If CPUID.0AH: EDX[4:0] > n.
    /// Enable bitmask.  Only the first n-1 bits are valid.
    /// Bits 31:n are reserved.
    ///
    UINT32  EN_FIXED_CTRn:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PERF_GLOBAL_CTRL_REGISTER;


/**
  Global Performance Counter Overflow Control (R/W). If CPUID.0AH: EAX[7:0] >
  0 && CPUID.0AH: EAX[7:0] <= 3.

  @param  ECX  MSR_IA32_PERF_GLOBAL_OVF_CTRL (0x00000390)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_OVF_CTRL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_OVF_CTRL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PERF_GLOBAL_OVF_CTRL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PERF_GLOBAL_OVF_CTRL);
  AsmWriteMsr64 (MSR_IA32_PERF_GLOBAL_OVF_CTRL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PERF_GLOBAL_OVF_CTRL            0x00000390

/**
  MSR information returned for MSR index #MSR_IA32_PERF_GLOBAL_OVF_CTRL
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 31:0] Set 1 to Clear Ovf_PMC0 bit. If CPUID.0AH: EAX[15:8] > n.
    /// Clear bitmask.  Only the first n-1 bits are valid.
    /// Bits 31:n are reserved.
    ///
    UINT32  Ovf_PMCn:32;
    ///
    /// [Bits 54:32] Set 1 to Clear Ovf_FIXED_CTR0 bit.
    /// If CPUID.0AH: EDX[4:0] > n.
    /// Clear bitmask.  Only the first n-1 bits are valid.
    /// Bits 22:n are reserved.
    ///
    UINT32  Ovf_FIXED_CTRn:23;
    ///
    /// [Bit 55] Set 1 to Clear Trace_ToPA_PMI bit. If (CPUID.(EAX=07H,
    /// ECX=0):EBX[25] = 1) && IA32_RTIT_CTL.ToPA = 1.
    ///
    UINT32  Trace_ToPA_PMI:1;
    UINT32  Reserved2:5;
    ///
    /// [Bit 61] Set 1 to Clear Ovf_Uncore bit. Introduced at Display Family /
    /// Display Model 06_2EH.
    ///
    UINT32  Ovf_Uncore:1;
    ///
    /// [Bit 62] Set 1 to Clear OvfBuf: bit. If CPUID.0AH: EAX[7:0] > 0.
    ///
    UINT32  OvfBuf:1;
    ///
    /// [Bit 63] Set to 1to clear CondChgd: bit. If CPUID.0AH: EAX[7:0] > 0.
    ///
    UINT32  CondChgd:1;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PERF_GLOBAL_OVF_CTRL_REGISTER;


/**
  Global Performance Counter Overflow Reset Control (R/W). If CPUID.0AH:
  EAX[7:0] > 3.

  @param  ECX  MSR_IA32_PERF_GLOBAL_STATUS_RESET (0x00000390)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_STATUS_RESET_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_STATUS_RESET_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PERF_GLOBAL_STATUS_RESET_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PERF_GLOBAL_STATUS_RESET);
  AsmWriteMsr64 (MSR_IA32_PERF_GLOBAL_STATUS_RESET, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PERF_GLOBAL_STATUS_RESET        0x00000390

/**
  MSR information returned for MSR index #MSR_IA32_PERF_GLOBAL_STATUS_RESET
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 31:0] Set 1 to Clear Ovf_PMC0 bit. If CPUID.0AH: EAX[15:8] > n.
    /// Clear bitmask.  Only the first n-1 bits are valid.
    /// Bits 31:n are reserved.
    ///
    UINT32  Ovf_PMCn:32;
    ///
    /// [Bits 54:32] Set 1 to Clear Ovf_FIXED_CTR0 bit.
    /// If CPUID.0AH: EDX[4:0] > n.
    /// Clear bitmask.  Only the first n-1 bits are valid.
    /// Bits 22:n are reserved.
    ///
    UINT32  Ovf_FIXED_CTRn:23;
    ///
    /// [Bit 55] Set 1 to Clear Trace_ToPA_PMI bit. If (CPUID.(EAX=07H,
    /// ECX=0):EBX[25] = 1) && IA32_RTIT_CTL.ToPA[8] = 1.
    ///
    UINT32  Trace_ToPA_PMI:1;
    UINT32  Reserved2:2;
    ///
    /// [Bit 58] Set 1 to Clear LBR_Frz bit. If CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  LBR_Frz:1;
    ///
    /// [Bit 59] Set 1 to Clear CTR_Frz bit. If CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  CTR_Frz:1;
    ///
    /// [Bit 60] Set 1 to Clear ASCI bit. If CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  ASCI:1;
    ///
    /// [Bit 61] Set 1 to Clear Ovf_Uncore bit. Introduced at Display Family /
    /// Display Model 06_2EH.
    ///
    UINT32  Ovf_Uncore:1;
    ///
    /// [Bit 62] Set 1 to Clear OvfBuf: bit. If CPUID.0AH: EAX[7:0] > 0.
    ///
    UINT32  OvfBuf:1;
    ///
    /// [Bit 63] Set to 1to clear CondChgd: bit. If CPUID.0AH: EAX[7:0] > 0.
    ///
    UINT32  CondChgd:1;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PERF_GLOBAL_STATUS_RESET_REGISTER;


/**
  Global Performance Counter Overflow Set Control (R/W). If CPUID.0AH:
  EAX[7:0] > 3.

  @param  ECX  MSR_IA32_PERF_GLOBAL_STATUS_SET (0x00000391)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_STATUS_SET_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_STATUS_SET_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PERF_GLOBAL_STATUS_SET_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PERF_GLOBAL_STATUS_SET);
  AsmWriteMsr64 (MSR_IA32_PERF_GLOBAL_STATUS_SET, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PERF_GLOBAL_STATUS_SET          0x00000391

/**
  MSR information returned for MSR index #MSR_IA32_PERF_GLOBAL_STATUS_SET
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 31:0] Set 1 to cause Ovf_PMCn = 1. If CPUID.0AH: EAX[7:0] > n.
    /// Set bitmask.  Only the first n-1 bits are valid.
    /// Bits 31:n are reserved.
    ///
    UINT32  Ovf_PMCn:32;
    ///
    /// [Bits 54:32] Set 1 to cause Ovf_FIXED_CTRn = 1.
    /// If CPUID.0AH: EAX[7:0] > n.
    /// Set bitmask.  Only the first n-1 bits are valid.
    /// Bits 22:n are reserved.
    ///
    UINT32  Ovf_FIXED_CTRn:23;
    ///
    /// [Bit 55] Set 1 to cause Trace_ToPA_PMI = 1. If CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  Trace_ToPA_PMI:1;
    UINT32  Reserved2:2;
    ///
    /// [Bit 58] Set 1 to cause LBR_Frz = 1. If CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  LBR_Frz:1;
    ///
    /// [Bit 59] Set 1 to cause CTR_Frz = 1. If CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  CTR_Frz:1;
    ///
    /// [Bit 60] Set 1 to cause ASCI = 1. If CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  ASCI:1;
    ///
    /// [Bit 61] Set 1 to cause Ovf_Uncore = 1. If CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  Ovf_Uncore:1;
    ///
    /// [Bit 62] Set 1 to cause OvfBuf = 1. If CPUID.0AH: EAX[7:0] > 3.
    ///
    UINT32  OvfBuf:1;
    UINT32  Reserved3:1;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PERF_GLOBAL_STATUS_SET_REGISTER;


/**
  Indicator of core perfmon interface is in use (RO). If CPUID.0AH: EAX[7:0] >
  3.

  @param  ECX  MSR_IA32_PERF_GLOBAL_INUSE (0x00000392)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_INUSE_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PERF_GLOBAL_INUSE_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PERF_GLOBAL_INUSE_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PERF_GLOBAL_INUSE);
  @endcode
**/
#define MSR_IA32_PERF_GLOBAL_INUSE               0x00000392

/**
  MSR information returned for MSR index #MSR_IA32_PERF_GLOBAL_INUSE
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 31:0] IA32_PERFEVTSELn in use.  If CPUID.0AH: EAX[7:0] > n.
    /// Status bitmask.  Only the first n-1 bits are valid.
    /// Bits 31:n are reserved.
    ///
    UINT32  IA32_PERFEVTSELn:32;
    ///
    /// [Bits 62:32] IA32_FIXED_CTRn in use.
    /// If CPUID.0AH: EAX[7:0] > n.
    /// Status bitmask.  Only the first n-1 bits are valid.
    /// Bits 30:n are reserved.
    ///
    UINT32  IA32_FIXED_CTRn:31;
    ///
    /// [Bit 63] PMI in use.
    ///
    UINT32  PMI:1;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PERF_GLOBAL_INUSE_REGISTER;


/**
  PEBS Control (R/W).

  @param  ECX  MSR_IA32_PEBS_ENABLE (0x000003F1)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PEBS_ENABLE_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PEBS_ENABLE_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PEBS_ENABLE_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PEBS_ENABLE);
  AsmWriteMsr64 (MSR_IA32_PEBS_ENABLE, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PEBS_ENABLE                     0x000003F1

/**
  MSR information returned for MSR index #MSR_IA32_PEBS_ENABLE
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Enable PEBS on IA32_PMC0. Introduced at Display Family /
    /// Display Model 06_0FH.
    ///
    UINT32  Enable:1;
    ///
    /// [Bits 3:1] Reserved or Model specific.
    ///
    UINT32  Reserved1:3;
    UINT32  Reserved2:28;
    ///
    /// [Bits 35:32] Reserved or Model specific.
    ///
    UINT32  Reserved3:4;
    UINT32  Reserved4:28;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PEBS_ENABLE_REGISTER;


/**
  MCn_CTL. If IA32_MCG_CAP.CNT > n.

  @param  ECX  MSR_IA32_MCn_CTL
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MC0_CTL);
  AsmWriteMsr64 (MSR_IA32_MC0_CTL, Msr);
  @endcode
  @{
**/
#define MSR_IA32_MC0_CTL                         0x00000400
#define MSR_IA32_MC1_CTL                         0x00000404
#define MSR_IA32_MC2_CTL                         0x00000408
#define MSR_IA32_MC3_CTL                         0x0000040C
#define MSR_IA32_MC4_CTL                         0x00000410
#define MSR_IA32_MC5_CTL                         0x00000414
#define MSR_IA32_MC6_CTL                         0x00000418
#define MSR_IA32_MC7_CTL                         0x0000041C
#define MSR_IA32_MC8_CTL                         0x00000420
#define MSR_IA32_MC9_CTL                         0x00000424
#define MSR_IA32_MC10_CTL                        0x00000428
#define MSR_IA32_MC11_CTL                        0x0000042C
#define MSR_IA32_MC12_CTL                        0x00000430
#define MSR_IA32_MC13_CTL                        0x00000434
#define MSR_IA32_MC14_CTL                        0x00000438
#define MSR_IA32_MC15_CTL                        0x0000043C
#define MSR_IA32_MC16_CTL                        0x00000440
#define MSR_IA32_MC17_CTL                        0x00000444
#define MSR_IA32_MC18_CTL                        0x00000448
#define MSR_IA32_MC19_CTL                        0x0000044C
#define MSR_IA32_MC20_CTL                        0x00000450
#define MSR_IA32_MC21_CTL                        0x00000454
#define MSR_IA32_MC22_CTL                        0x00000458
#define MSR_IA32_MC23_CTL                        0x0000045C
#define MSR_IA32_MC24_CTL                        0x00000460
#define MSR_IA32_MC25_CTL                        0x00000464
#define MSR_IA32_MC26_CTL                        0x00000468
#define MSR_IA32_MC27_CTL                        0x0000046C
#define MSR_IA32_MC28_CTL                        0x00000470
/// @}


/**
  MCn_STATUS. If IA32_MCG_CAP.CNT > n.

  @param  ECX  MSR_IA32_MCn_STATUS
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MC0_STATUS);
  AsmWriteMsr64 (MSR_IA32_MC0_STATUS, Msr);
  @endcode
  @{
**/
#define MSR_IA32_MC0_STATUS                      0x00000401
#define MSR_IA32_MC1_STATUS                      0x00000405
#define MSR_IA32_MC2_STATUS                      0x00000409
#define MSR_IA32_MC3_STATUS                      0x0000040D
#define MSR_IA32_MC4_STATUS                      0x00000411
#define MSR_IA32_MC5_STATUS                      0x00000415
#define MSR_IA32_MC6_STATUS                      0x00000419
#define MSR_IA32_MC7_STATUS                      0x0000041D
#define MSR_IA32_MC8_STATUS                      0x00000421
#define MSR_IA32_MC9_STATUS                      0x00000425
#define MSR_IA32_MC10_STATUS                     0x00000429
#define MSR_IA32_MC11_STATUS                     0x0000042D
#define MSR_IA32_MC12_STATUS                     0x00000431
#define MSR_IA32_MC13_STATUS                     0x00000435
#define MSR_IA32_MC14_STATUS                     0x00000439
#define MSR_IA32_MC15_STATUS                     0x0000043D
#define MSR_IA32_MC16_STATUS                     0x00000441
#define MSR_IA32_MC17_STATUS                     0x00000445
#define MSR_IA32_MC18_STATUS                     0x00000449
#define MSR_IA32_MC19_STATUS                     0x0000044D
#define MSR_IA32_MC20_STATUS                     0x00000451
#define MSR_IA32_MC21_STATUS                     0x00000455
#define MSR_IA32_MC22_STATUS                     0x00000459
#define MSR_IA32_MC23_STATUS                     0x0000045D
#define MSR_IA32_MC24_STATUS                     0x00000461
#define MSR_IA32_MC25_STATUS                     0x00000465
#define MSR_IA32_MC26_STATUS                     0x00000469
#define MSR_IA32_MC27_STATUS                     0x0000046D
#define MSR_IA32_MC28_STATUS                     0x00000471
/// @}


/**
  MCn_ADDR. If IA32_MCG_CAP.CNT > n.

  @param  ECX  MSR_IA32_MCn_ADDR
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MC0_ADDR);
  AsmWriteMsr64 (MSR_IA32_MC0_ADDR, Msr);
  @endcode
  @{
**/
#define MSR_IA32_MC0_ADDR                        0x00000402
#define MSR_IA32_MC1_ADDR                        0x00000406
#define MSR_IA32_MC2_ADDR                        0x0000040A
#define MSR_IA32_MC3_ADDR                        0x0000040E
#define MSR_IA32_MC4_ADDR                        0x00000412
#define MSR_IA32_MC5_ADDR                        0x00000416
#define MSR_IA32_MC6_ADDR                        0x0000041A
#define MSR_IA32_MC7_ADDR                        0x0000041E
#define MSR_IA32_MC8_ADDR                        0x00000422
#define MSR_IA32_MC9_ADDR                        0x00000426
#define MSR_IA32_MC10_ADDR                       0x0000042A
#define MSR_IA32_MC11_ADDR                       0x0000042E
#define MSR_IA32_MC12_ADDR                       0x00000432
#define MSR_IA32_MC13_ADDR                       0x00000436
#define MSR_IA32_MC14_ADDR                       0x0000043A
#define MSR_IA32_MC15_ADDR                       0x0000043E
#define MSR_IA32_MC16_ADDR                       0x00000442
#define MSR_IA32_MC17_ADDR                       0x00000446
#define MSR_IA32_MC18_ADDR                       0x0000044A
#define MSR_IA32_MC19_ADDR                       0x0000044E
#define MSR_IA32_MC20_ADDR                       0x00000452
#define MSR_IA32_MC21_ADDR                       0x00000456
#define MSR_IA32_MC22_ADDR                       0x0000045A
#define MSR_IA32_MC23_ADDR                       0x0000045E
#define MSR_IA32_MC24_ADDR                       0x00000462
#define MSR_IA32_MC25_ADDR                       0x00000466
#define MSR_IA32_MC26_ADDR                       0x0000046A
#define MSR_IA32_MC27_ADDR                       0x0000046E
#define MSR_IA32_MC28_ADDR                       0x00000472
/// @}


/**
  MCn_MISC. If IA32_MCG_CAP.CNT > n.

  @param  ECX  MSR_IA32_MCn_MISC
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_MC0_MISC);
  AsmWriteMsr64 (MSR_IA32_MC0_MISC, Msr);
  @endcode
  @{
**/
#define MSR_IA32_MC0_MISC                        0x00000403
#define MSR_IA32_MC1_MISC                        0x00000407
#define MSR_IA32_MC2_MISC                        0x0000040B
#define MSR_IA32_MC3_MISC                        0x0000040F
#define MSR_IA32_MC4_MISC                        0x00000413
#define MSR_IA32_MC5_MISC                        0x00000417
#define MSR_IA32_MC6_MISC                        0x0000041B
#define MSR_IA32_MC7_MISC                        0x0000041F
#define MSR_IA32_MC8_MISC                        0x00000423
#define MSR_IA32_MC9_MISC                        0x00000427
#define MSR_IA32_MC10_MISC                       0x0000042B
#define MSR_IA32_MC11_MISC                       0x0000042F
#define MSR_IA32_MC12_MISC                       0x00000433
#define MSR_IA32_MC13_MISC                       0x00000437
#define MSR_IA32_MC14_MISC                       0x0000043B
#define MSR_IA32_MC15_MISC                       0x0000043F
#define MSR_IA32_MC16_MISC                       0x00000443
#define MSR_IA32_MC17_MISC                       0x00000447
#define MSR_IA32_MC18_MISC                       0x0000044B
#define MSR_IA32_MC19_MISC                       0x0000044F
#define MSR_IA32_MC20_MISC                       0x00000453
#define MSR_IA32_MC21_MISC                       0x00000457
#define MSR_IA32_MC22_MISC                       0x0000045B
#define MSR_IA32_MC23_MISC                       0x0000045F
#define MSR_IA32_MC24_MISC                       0x00000463
#define MSR_IA32_MC25_MISC                       0x00000467
#define MSR_IA32_MC26_MISC                       0x0000046B
#define MSR_IA32_MC27_MISC                       0x0000046F
#define MSR_IA32_MC28_MISC                       0x00000473
/// @}


/**
  Reporting Register of Basic VMX  Capabilities (R/O) See Appendix A.1, "Basic
  VMX Information.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_BASIC (0x00000480)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_BASIC);
  @endcode
**/
#define MSR_IA32_VMX_BASIC                       0x00000480


/**
  Capability Reporting Register of Pinbased VM-execution Controls (R/O) See
  Appendix A.3.1, "Pin-Based VMExecution Controls.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_PINBASED_CTLS (0x00000481)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_PINBASED_CTLS);
  @endcode
**/
#define MSR_IA32_VMX_PINBASED_CTLS               0x00000481


/**
  Capability Reporting Register of Primary  Processor-based VM-execution
  Controls (R/O) See Appendix A.3.2, "Primary Processor- Based VM-Execution
  Controls.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_PROCBASED_CTLS (0x00000482)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_PROCBASED_CTLS);
  @endcode
**/
#define MSR_IA32_VMX_PROCBASED_CTLS              0x00000482


/**
  Capability Reporting Register of VM-exit  Controls (R/O) See Appendix A.4,
  "VM-Exit Controls.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_EXIT_CTLS (0x00000483)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_EXIT_CTLS);
  @endcode
**/
#define MSR_IA32_VMX_EXIT_CTLS                   0x00000483


/**
  Capability Reporting Register of VMentry Controls (R/O) See Appendix A.5,
  "VM-Entry Controls.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_ENTRY_CTLS (0x00000484)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_ENTRY_CTLS);
  @endcode
**/
#define MSR_IA32_VMX_ENTRY_CTLS                  0x00000484


/**
  Reporting Register of Miscellaneous VMX Capabilities (R/O) See Appendix A.6,
  "Miscellaneous Data.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_MISC (0x00000485)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_MISC);
  @endcode
**/
#define MSR_IA32_VMX_MISC                        0x00000485


/**
  Capability Reporting Register of CR0 Bits Fixed to 0 (R/O) See Appendix A.7,
  "VMX-Fixed Bits in CR0.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_CR0_FIXED0 (0x00000486)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_CR0_FIXED0);
  @endcode
**/
#define MSR_IA32_VMX_CR0_FIXED0                  0x00000486


/**
  Capability Reporting Register of CR0 Bits Fixed to 1 (R/O) See Appendix A.7,
  "VMX-Fixed Bits in CR0.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_CR0_FIXED1 (0x00000487)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_CR0_FIXED1);
  @endcode
**/
#define MSR_IA32_VMX_CR0_FIXED1                  0x00000487


/**
  Capability Reporting Register of CR4 Bits Fixed to 0 (R/O) See Appendix A.8,
  "VMX-Fixed Bits in CR4.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_CR4_FIXED0 (0x00000488)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_CR4_FIXED0);
  @endcode
**/
#define MSR_IA32_VMX_CR4_FIXED0                  0x00000488


/**
  Capability Reporting Register of CR4 Bits Fixed to 1 (R/O) See Appendix A.8,
  "VMX-Fixed Bits in CR4.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_CR4_FIXED1 (0x00000489)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_CR4_FIXED1);
  @endcode
**/
#define MSR_IA32_VMX_CR4_FIXED1                  0x00000489


/**
  Capability Reporting Register of VMCS Field Enumeration (R/O) See Appendix
  A.9, "VMCS Enumeration.". If CPUID.01H:ECX.[5] = 1.

  @param  ECX  MSR_IA32_VMX_VMCS_ENUM (0x0000048A)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_VMCS_ENUM);
  @endcode
**/
#define MSR_IA32_VMX_VMCS_ENUM                   0x0000048A


/**
  Capability Reporting Register of  Secondary Processor-based  VM-execution
  Controls (R/O) See Appendix A.3.3, "Secondary Processor- Based VM-Execution
  Controls.". If ( CPUID.01H:ECX.[5] && IA32_VMX_PROCBASED_C TLS[63]).

  @param  ECX  MSR_IA32_VMX_PROCBASED_CTLS2 (0x0000048B)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_PROCBASED_CTLS2);
  @endcode
**/
#define MSR_IA32_VMX_PROCBASED_CTLS2             0x0000048B


/**
  Capability Reporting Register of EPT and  VPID (R/O) See Appendix A.10,
  "VPID and EPT Capabilities.". If ( CPUID.01H:ECX.[5] && IA32_VMX_PROCBASED_C
  TLS[63] && ( IA32_VMX_PROCBASED_C TLS2[33] IA32_VMX_PROCBASED_C TLS2[37]) ).

  @param  ECX  MSR_IA32_VMX_EPT_VPID_CAP (0x0000048C)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_EPT_VPID_CAP);
  @endcode
**/
#define MSR_IA32_VMX_EPT_VPID_CAP                0x0000048C


/**
  Capability Reporting Register of Pinbased VM-execution Flex Controls (R/O)
  See Appendix A.3.1, "Pin-Based VMExecution Controls.". If (
  CPUID.01H:ECX.[5] = 1 && IA32_VMX_BASIC[55] ).

  @param  ECX  MSR_IA32_VMX_TRUE_PINBASED_CTLS (0x0000048D)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_TRUE_PINBASED_CTLS);
  @endcode
**/
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS          0x0000048D


/**
  Capability Reporting Register of Primary  Processor-based VM-execution Flex
  Controls (R/O) See Appendix A.3.2, "Primary Processor- Based VM-Execution
  Controls.". If( CPUID.01H:ECX.[5] = 1 && IA32_VMX_BASIC[55] ).

  @param  ECX  MSR_IA32_VMX_TRUE_PROCBASED_CTLS (0x0000048E)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_TRUE_PROCBASED_CTLS);
  @endcode
**/
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS         0x0000048E


/**
  Capability Reporting Register of VM-exit  Flex Controls (R/O) See Appendix
  A.4, "VM-Exit Controls.". If( CPUID.01H:ECX.[5] = 1 && IA32_VMX_BASIC[55] ).

  @param  ECX  MSR_IA32_VMX_TRUE_EXIT_CTLS (0x0000048F)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_TRUE_EXIT_CTLS);
  @endcode
**/
#define MSR_IA32_VMX_TRUE_EXIT_CTLS              0x0000048F


/**
  Capability Reporting Register of VMentry Flex Controls (R/O) See Appendix
  A.5, "VM-Entry Controls.". If( CPUID.01H:ECX.[5] = 1 && IA32_VMX_BASIC[55] ).

  @param  ECX  MSR_IA32_VMX_TRUE_ENTRY_CTLS (0x00000490)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_TRUE_ENTRY_CTLS);
  @endcode
**/
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS             0x00000490


/**
  Capability Reporting Register of VMfunction Controls (R/O). If(
  CPUID.01H:ECX.[5] = 1 && IA32_VMX_BASIC[55] ).

  @param  ECX  MSR_IA32_VMX_VMFUNC (0x00000491)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_VMX_VMFUNC);
  @endcode
**/
#define MSR_IA32_VMX_VMFUNC                      0x00000491


/**
  Full Width Writable IA32_PMCn Alias (R/W). (If CPUID.0AH: EAX[15:8] > n) &&
  IA32_PERF_CAPABILITIES[ 13] = 1.

  @param  ECX  MSR_IA32_A_PMCn
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_A_PMC0);
  AsmWriteMsr64 (MSR_IA32_A_PMC0, Msr);
  @endcode
  @{
**/
#define MSR_IA32_A_PMC0                          0x000004C1
#define MSR_IA32_A_PMC1                          0x000004C2
#define MSR_IA32_A_PMC2                          0x000004C3
#define MSR_IA32_A_PMC3                          0x000004C4
#define MSR_IA32_A_PMC4                          0x000004C5
#define MSR_IA32_A_PMC5                          0x000004C6
#define MSR_IA32_A_PMC6                          0x000004C7
#define MSR_IA32_A_PMC7                          0x000004C8
/// @}


/**
  (R/W). If IA32_MCG_CAP.LMCE_P =1.

  @param  ECX  MSR_IA32_MCG_EXT_CTL (0x000004D0)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_MCG_EXT_CTL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_MCG_EXT_CTL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_MCG_EXT_CTL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_MCG_EXT_CTL);
  AsmWriteMsr64 (MSR_IA32_MCG_EXT_CTL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_MCG_EXT_CTL                     0x000004D0

/**
  MSR information returned for MSR index #MSR_IA32_MCG_EXT_CTL
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] LMCE_EN.
    ///
    UINT32  LMCE_EN:1;
    UINT32  Reserved1:31;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_MCG_EXT_CTL_REGISTER;


/**
  Status and SVN Threshold of SGX Support for ACM (RO). If CPUID.(EAX=07H,
  ECX=0H): EBX[2] = 1.

  @param  ECX  MSR_IA32_SGX_SVN_STATUS (0x00000500)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_SGX_SVN_STATUS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_SGX_SVN_STATUS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_SGX_SVN_STATUS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_SGX_SVN_STATUS);
  @endcode
**/
#define MSR_IA32_SGX_SVN_STATUS                  0x00000500

/**
  MSR information returned for MSR index #MSR_IA32_SGX_SVN_STATUS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Lock. See Section 42.12.3, "Interactions with Authenticated
    /// Code Modules (ACMs)".
    ///
    UINT32  Lock:1;
    UINT32  Reserved1:15;
    ///
    /// [Bits 23:16] SGX_SVN_SINIT. See Section 42.12.3, "Interactions with
    /// Authenticated Code Modules (ACMs)".
    ///
    UINT32  SGX_SVN_SINIT:8;
    UINT32  Reserved2:8;
    UINT32  Reserved3:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_SGX_SVN_STATUS_REGISTER;


/**
  Trace Output Base Register (R/W). If ((CPUID.(EAX=07H, ECX=0):EBX[25] = 1)
  && ( (CPUID.(EAX=14H,ECX=0): ECX[0] = 1) (CPUID.(EAX=14H,ECX=0): ECX[2] = 1)
  ) ).

  @param  ECX  MSR_IA32_RTIT_OUTPUT_BASE (0x00000560)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_OUTPUT_BASE_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_OUTPUT_BASE_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_RTIT_OUTPUT_BASE_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_RTIT_OUTPUT_BASE);
  AsmWriteMsr64 (MSR_IA32_RTIT_OUTPUT_BASE, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_RTIT_OUTPUT_BASE                0x00000560

/**
  MSR information returned for MSR index #MSR_IA32_RTIT_OUTPUT_BASE
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    UINT32  Reserved:7;
    ///
    /// [Bits 31:7] Base physical address.
    ///
    UINT32  Base:25;
    ///
    /// [Bits 63:32] Base physical address.
    ///
    UINT32  BaseHi:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_RTIT_OUTPUT_BASE_REGISTER;


/**
  Trace Output Mask Pointers Register (R/W). If ((CPUID.(EAX=07H,
  ECX=0):EBX[25] = 1) && ( (CPUID.(EAX=14H,ECX=0): ECX[0] = 1)
  (CPUID.(EAX=14H,ECX=0): ECX[2] = 1) ) ).

  @param  ECX  MSR_IA32_RTIT_OUTPUT_MASK_PTRS (0x00000561)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_OUTPUT_MASK_PTRS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_OUTPUT_MASK_PTRS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_RTIT_OUTPUT_MASK_PTRS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_RTIT_OUTPUT_MASK_PTRS);
  AsmWriteMsr64 (MSR_IA32_RTIT_OUTPUT_MASK_PTRS, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_RTIT_OUTPUT_MASK_PTRS           0x00000561

/**
  MSR information returned for MSR index #MSR_IA32_RTIT_OUTPUT_MASK_PTRS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    UINT32  Reserved:7;
    ///
    /// [Bits 31:7] MaskOrTableOffset.
    ///
    UINT32  MaskOrTableOffset:25;
    ///
    /// [Bits 63:32] Output Offset.
    ///
    UINT32  OutputOffset:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_RTIT_OUTPUT_MASK_PTRS_REGISTER;


/**
  Trace Control Register (R/W). If (CPUID.(EAX=07H, ECX=0):EBX[25] = 1).

  @param  ECX  MSR_IA32_RTIT_CTL (0x00000570)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_CTL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_CTL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_RTIT_CTL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_RTIT_CTL);
  AsmWriteMsr64 (MSR_IA32_RTIT_CTL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_RTIT_CTL                        0x00000570

/**
  MSR information returned for MSR index #MSR_IA32_RTIT_CTL
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] TraceEn.
    ///
    UINT32  TraceEn:1;
    ///
    /// [Bit 1] CYCEn. If (CPUID.(EAX=07H, ECX=0):EBX[1] = 1).
    ///
    UINT32  CYCEn:1;
    ///
    /// [Bit 2] OS.
    ///
    UINT32  OS:1;
    ///
    /// [Bit 3] User.
    ///
    UINT32  User:1;
    UINT32  Reserved1:2;
    ///
    /// [Bit 6] FabricEn. If (CPUID.(EAX=07H, ECX=0):ECX[3] = 1).
    ///
    UINT32  FabricEn:1;
    ///
    /// [Bit 7] CR3 filter.
    ///
    UINT32  CR3:1;
    ///
    /// [Bit 8] ToPA.
    ///
    UINT32  ToPA:1;
    ///
    /// [Bit 9] MTCEn. If (CPUID.(EAX=07H, ECX=0):EBX[3] = 1).
    ///
    UINT32  MTCEn:1;
    ///
    /// [Bit 10] TSCEn.
    ///
    UINT32  TSCEn:1;
    ///
    /// [Bit 11] DisRETC.
    ///
    UINT32  DisRETC:1;
    UINT32  Reserved2:1;
    ///
    /// [Bit 13] BranchEn.
    ///
    UINT32  BranchEn:1;
    ///
    /// [Bits 17:14] MTCFreq. If (CPUID.(EAX=07H, ECX=0):EBX[3] = 1).
    ///
    UINT32  MTCFreq:4;
    UINT32  Reserved3:1;
    ///
    /// [Bits 22:19] CYCThresh. If (CPUID.(EAX=07H, ECX=0):EBX[1] = 1).
    ///
    UINT32  CYCThresh:4;
    UINT32  Reserved4:1;
    ///
    /// [Bits 27:24] PSBFreq. If (CPUID.(EAX=07H, ECX=0):EBX[1] = 1).
    ///
    UINT32  PSBFreq:4;
    UINT32  Reserved5:4;
    ///
    /// [Bits 35:32] ADDR0_CFG. If (CPUID.(EAX=07H, ECX=1):EAX[2:0] > 0).
    ///
    UINT32  ADDR0_CFG:4;
    ///
    /// [Bits 39:36] ADDR1_CFG. If (CPUID.(EAX=07H, ECX=1):EAX[2:0] > 1).
    ///
    UINT32  ADDR1_CFG:4;
    ///
    /// [Bits 43:40] ADDR2_CFG. If (CPUID.(EAX=07H, ECX=1):EAX[2:0] > 2).
    ///
    UINT32  ADDR2_CFG:4;
    ///
    /// [Bits 47:44] ADDR3_CFG. If (CPUID.(EAX=07H, ECX=1):EAX[2:0] > 3).
    ///
    UINT32  ADDR3_CFG:4;
    UINT32  Reserved6:16;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_RTIT_CTL_REGISTER;


/**
  Tracing Status Register (R/W). If (CPUID.(EAX=07H, ECX=0):EBX[25] = 1).

  @param  ECX  MSR_IA32_RTIT_STATUS (0x00000571)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_STATUS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_STATUS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_RTIT_STATUS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_RTIT_STATUS);
  AsmWriteMsr64 (MSR_IA32_RTIT_STATUS, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_RTIT_STATUS                     0x00000571

/**
  MSR information returned for MSR index #MSR_IA32_RTIT_STATUS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] FilterEn, (writes ignored).
    /// If (CPUID.(EAX=07H, ECX=0):EBX[2] = 1).
    ///
    UINT32  FilterEn:1;
    ///
    /// [Bit 1] ContexEn, (writes ignored).
    ///
    UINT32  ContexEn:1;
    ///
    /// [Bit 2] TriggerEn, (writes ignored).
    ///
    UINT32  TriggerEn:1;
    UINT32  Reserved1:1;
    ///
    /// [Bit 4] Error.
    ///
    UINT32  Error:1;
    ///
    /// [Bit 5] Stopped.
    ///
    UINT32  Stopped:1;
    UINT32  Reserved2:26;
    ///
    /// [Bits 48:32] PacketByteCnt. If (CPUID.(EAX=07H, ECX=0):EBX[1] > 3).
    ///
    UINT32  PacketByteCnt:17;
    UINT32  Reserved3:15;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_RTIT_STATUS_REGISTER;


/**
  Trace Filter CR3 Match Register (R/W).
  If (CPUID.(EAX=07H, ECX=0):EBX[25] = 1).

  @param  ECX  MSR_IA32_RTIT_CR3_MATCH (0x00000572)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_CR3_MATCH_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_CR3_MATCH_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_RTIT_CR3_MATCH_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_RTIT_CR3_MATCH);
  AsmWriteMsr64 (MSR_IA32_RTIT_CR3_MATCH, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_RTIT_CR3_MATCH                  0x00000572

/**
  MSR information returned for MSR index #MSR_IA32_RTIT_CR3_MATCH
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    UINT32  Reserved:5;
    ///
    /// [Bits 31:5] CR3[63:5] value to match.
    ///
    UINT32  Cr3:27;
    ///
    /// [Bits 63:32] CR3[63:5] value to match.
    ///
    UINT32  Cr3Hi:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_RTIT_CR3_MATCH_REGISTER;


/**
  Region n Start Address (R/W). If (CPUID.(EAX=07H, ECX=1):EAX[2:0] > n).

  @param  ECX  MSR_IA32_RTIT_ADDRn_A
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_ADDR_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_ADDR_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_RTIT_ADDR_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_RTIT_ADDR0_A);
  AsmWriteMsr64 (MSR_IA32_RTIT_ADDR0_A, Msr.Uint64);
  @endcode
  @{
**/
#define MSR_IA32_RTIT_ADDR0_A                    0x00000580
#define MSR_IA32_RTIT_ADDR1_A                    0x00000582
#define MSR_IA32_RTIT_ADDR2_A                    0x00000584
#define MSR_IA32_RTIT_ADDR3_A                    0x00000586
/// @}


/**
  Region n End Address (R/W). If (CPUID.(EAX=07H, ECX=1):EAX[2:0] > n).

  @param  ECX  MSR_IA32_RTIT_ADDRn_B
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_ADDR_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_RTIT_ADDR_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_RTIT_ADDR_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_RTIT_ADDR0_B);
  AsmWriteMsr64 (MSR_IA32_RTIT_ADDR0_B, Msr.Uint64);
  @endcode
  @{
**/
#define MSR_IA32_RTIT_ADDR0_B                    0x00000581
#define MSR_IA32_RTIT_ADDR1_B                    0x00000583
#define MSR_IA32_RTIT_ADDR2_B                    0x00000585
#define MSR_IA32_RTIT_ADDR3_B                    0x00000587
/// @}


/**
  MSR information returned for MSR indexes
  #MSR_IA32_RTIT_ADDR0_A to #MSR_IA32_RTIT_ADDR3_A and
  #MSR_IA32_RTIT_ADDR0_B to #MSR_IA32_RTIT_ADDR3_B
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 31:0] Virtual Address.
    ///
    UINT32  VirtualAddress:32;
    ///
    /// [Bits 47:32] Virtual Address.
    ///
    UINT32  VirtualAddressHi:16;
    ///
    /// [Bits 63:48] SignExt_VA.
    ///
    UINT32  SignExt_VA:16;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_RTIT_ADDR_REGISTER;


/**
  DS Save Area (R/W)  Points to the linear address of the first byte of the DS
  buffer management area, which is used to manage the BTS and PEBS buffers.
  See Section 18.12.4, "Debug Store (DS) Mechanism.". If( CPUID.01H:EDX.DS[21]
  = 1.

    [Bits 31..0] The linear address of the first byte of the DS buffer
    management area, if not in IA-32e mode.

    [Bits 63..0] The linear address of the first byte of the DS buffer
    management area, if IA-32e mode is active.

  @param  ECX  MSR_IA32_DS_AREA (0x00000600)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_DS_AREA_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_DS_AREA_REGISTER.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_DS_AREA);
  AsmWriteMsr64 (MSR_IA32_DS_AREA, Msr);
  @endcode
**/
#define MSR_IA32_DS_AREA                         0x00000600


/**
  TSC Target of Local APIC's TSC Deadline Mode (R/W). If CPUID.01H:ECX.[24] =
  1.

  @param  ECX  MSR_IA32_TSC_DEADLINE (0x000006E0)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_TSC_DEADLINE);
  AsmWriteMsr64 (MSR_IA32_TSC_DEADLINE, Msr);
  @endcode
**/
#define MSR_IA32_TSC_DEADLINE                    0x000006E0


/**
  Enable/disable HWP (R/W). If CPUID.06H:EAX.[7] = 1.

  @param  ECX  MSR_IA32_PM_ENABLE (0x00000770)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PM_ENABLE_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PM_ENABLE_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PM_ENABLE_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PM_ENABLE);
  AsmWriteMsr64 (MSR_IA32_PM_ENABLE, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PM_ENABLE                       0x00000770

/**
  MSR information returned for MSR index #MSR_IA32_PM_ENABLE
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] HWP_ENABLE (R/W1-Once). See Section 14.4.2, "Enabling HWP". If
    /// CPUID.06H:EAX.[7] = 1.
    ///
    UINT32  HWP_ENABLE:1;
    UINT32  Reserved1:31;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PM_ENABLE_REGISTER;


/**
  HWP Performance Range Enumeration (RO). If CPUID.06H:EAX.[7] = 1.

  @param  ECX  MSR_IA32_HWP_CAPABILITIES (0x00000771)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_HWP_CAPABILITIES_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_HWP_CAPABILITIES_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_HWP_CAPABILITIES_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_HWP_CAPABILITIES);
  @endcode
**/
#define MSR_IA32_HWP_CAPABILITIES                0x00000771

/**
  MSR information returned for MSR index #MSR_IA32_HWP_CAPABILITIES
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 7:0] Highest_Performance See Section 14.4.3, "HWP Performance
    /// Range and Dynamic Capabilities". If CPUID.06H:EAX.[7] = 1.
    ///
    UINT32  Highest_Performance:8;
    ///
    /// [Bits 15:8] Guaranteed_Performance See Section 14.4.3, "HWP
    /// Performance Range and Dynamic Capabilities". If CPUID.06H:EAX.[7] = 1.
    ///
    UINT32  Guaranteed_Performance:8;
    ///
    /// [Bits 23:16] Most_Efficient_Performance See Section 14.4.3, "HWP
    /// Performance Range and Dynamic Capabilities". If CPUID.06H:EAX.[7] = 1.
    ///
    UINT32  Most_Efficient_Performance:8;
    ///
    /// [Bits 31:24] Lowest_Performance See Section 14.4.3, "HWP Performance
    /// Range and Dynamic Capabilities". If CPUID.06H:EAX.[7] = 1.
    ///
    UINT32  Lowest_Performance:8;
    UINT32  Reserved:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_HWP_CAPABILITIES_REGISTER;


/**
  Power Management Control Hints for All Logical Processors in a Package
  (R/W). If CPUID.06H:EAX.[11] = 1.

  @param  ECX  MSR_IA32_HWP_REQUEST_PKG (0x00000772)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_HWP_REQUEST_PKG_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_HWP_REQUEST_PKG_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_HWP_REQUEST_PKG_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_HWP_REQUEST_PKG);
  AsmWriteMsr64 (MSR_IA32_HWP_REQUEST_PKG, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_HWP_REQUEST_PKG                 0x00000772

/**
  MSR information returned for MSR index #MSR_IA32_HWP_REQUEST_PKG
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 7:0] Minimum_Performance See Section 14.4.4, "Managing HWP". If
    /// CPUID.06H:EAX.[11] = 1.
    ///
    UINT32  Minimum_Performance:8;
    ///
    /// [Bits 15:8] Maximum_Performance See Section 14.4.4, "Managing HWP". If
    /// CPUID.06H:EAX.[11] = 1.
    ///
    UINT32  Maximum_Performance:8;
    ///
    /// [Bits 23:16] Desired_Performance See Section 14.4.4, "Managing HWP".
    /// If CPUID.06H:EAX.[11] = 1.
    ///
    UINT32  Desired_Performance:8;
    ///
    /// [Bits 31:24] Energy_Performance_Preference See Section 14.4.4,
    /// "Managing HWP". If CPUID.06H:EAX.[11] = 1 && CPUID.06H:EAX.[10] = 1.
    ///
    UINT32  Energy_Performance_Preference:8;
    ///
    /// [Bits 41:32] Activity_Window See Section 14.4.4, "Managing HWP". If
    /// CPUID.06H:EAX.[11] = 1 && CPUID.06H:EAX.[9] = 1.
    ///
    UINT32  Activity_Window:10;
    UINT32  Reserved:22;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_HWP_REQUEST_PKG_REGISTER;


/**
  Control HWP Native Interrupts (R/W). If CPUID.06H:EAX.[8] = 1.

  @param  ECX  MSR_IA32_HWP_INTERRUPT (0x00000773)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_HWP_INTERRUPT_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_HWP_INTERRUPT_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_HWP_INTERRUPT_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_HWP_INTERRUPT);
  AsmWriteMsr64 (MSR_IA32_HWP_INTERRUPT, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_HWP_INTERRUPT                   0x00000773

/**
  MSR information returned for MSR index #MSR_IA32_HWP_INTERRUPT
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] EN_Guaranteed_Performance_Change. See Section 14.4.6, "HWP
    /// Notifications". If CPUID.06H:EAX.[8] = 1.
    ///
    UINT32  EN_Guaranteed_Performance_Change:1;
    ///
    /// [Bit 1] EN_Excursion_Minimum. See Section 14.4.6, "HWP Notifications".
    /// If CPUID.06H:EAX.[8] = 1.
    ///
    UINT32  EN_Excursion_Minimum:1;
    UINT32  Reserved1:30;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_HWP_INTERRUPT_REGISTER;


/**
  Power Management Control Hints to a Logical Processor (R/W). If
  CPUID.06H:EAX.[7] = 1.

  @param  ECX  MSR_IA32_HWP_REQUEST (0x00000774)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_HWP_REQUEST_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_HWP_REQUEST_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_HWP_REQUEST_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_HWP_REQUEST);
  AsmWriteMsr64 (MSR_IA32_HWP_REQUEST, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_HWP_REQUEST                     0x00000774

/**
  MSR information returned for MSR index #MSR_IA32_HWP_REQUEST
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 7:0] Minimum_Performance See Section 14.4.4, "Managing HWP". If
    /// CPUID.06H:EAX.[7] = 1.
    ///
    UINT32  Minimum_Performance:8;
    ///
    /// [Bits 15:8] Maximum_Performance See Section 14.4.4, "Managing HWP". If
    /// CPUID.06H:EAX.[7] = 1.
    ///
    UINT32  Maximum_Performance:8;
    ///
    /// [Bits 23:16] Desired_Performance See Section 14.4.4, "Managing HWP".
    /// If CPUID.06H:EAX.[7] = 1.
    ///
    UINT32  Desired_Performance:8;
    ///
    /// [Bits 31:24] Energy_Performance_Preference See Section 14.4.4,
    /// "Managing HWP". If CPUID.06H:EAX.[7] = 1 && CPUID.06H:EAX.[10] = 1.
    ///
    UINT32  Energy_Performance_Preference:8;
    ///
    /// [Bits 41:32] Activity_Window See Section 14.4.4, "Managing HWP". If
    /// CPUID.06H:EAX.[7] = 1 && CPUID.06H:EAX.[9] = 1.
    ///
    UINT32  Activity_Window:10;
    ///
    /// [Bit 42] Package_Control See Section 14.4.4, "Managing HWP". If
    /// CPUID.06H:EAX.[7] = 1 && CPUID.06H:EAX.[11] = 1.
    ///
    UINT32  Package_Control:1;
    UINT32  Reserved:21;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_HWP_REQUEST_REGISTER;


/**
  Log bits indicating changes to  Guaranteed & excursions to Minimum (R/W). If
  CPUID.06H:EAX.[7] = 1.

  @param  ECX  MSR_IA32_HWP_STATUS (0x00000777)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_HWP_STATUS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_HWP_STATUS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_HWP_STATUS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_HWP_STATUS);
  AsmWriteMsr64 (MSR_IA32_HWP_STATUS, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_HWP_STATUS                      0x00000777

/**
  MSR information returned for MSR index #MSR_IA32_HWP_STATUS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Guaranteed_Performance_Change (R/WC0). See Section 14.4.5,
    /// "HWP Feedback". If CPUID.06H:EAX.[7] = 1.
    ///
    UINT32  Guaranteed_Performance_Change:1;
    UINT32  Reserved1:1;
    ///
    /// [Bit 2] Excursion_To_Minimum (R/WC0). See Section 14.4.5, "HWP
    /// Feedback". If CPUID.06H:EAX.[7] = 1.
    ///
    UINT32  Excursion_To_Minimum:1;
    UINT32  Reserved2:29;
    UINT32  Reserved3:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_HWP_STATUS_REGISTER;


/**
  x2APIC ID Register (R/O) See x2APIC Specification. If CPUID.01H:ECX[21] = 1
  && IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_APICID (0x00000802)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_APICID);
  @endcode
**/
#define MSR_IA32_X2APIC_APICID                   0x00000802


/**
  x2APIC Version Register (R/O). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_VERSION (0x00000803)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_VERSION);
  @endcode
**/
#define MSR_IA32_X2APIC_VERSION                  0x00000803


/**
  x2APIC Task Priority Register (R/W). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_TPR (0x00000808)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_TPR);
  AsmWriteMsr64 (MSR_IA32_X2APIC_TPR, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_TPR                      0x00000808


/**
  x2APIC Processor Priority Register (R/O). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_PPR (0x0000080A)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_PPR);
  @endcode
**/
#define MSR_IA32_X2APIC_PPR                      0x0000080A


/**
  x2APIC EOI Register (W/O). If CPUID.01H:ECX.[21] = 1 && IA32_APIC_BASE.[10]
  = 1.

  @param  ECX  MSR_IA32_X2APIC_EOI (0x0000080B)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = 0;
  AsmWriteMsr64 (MSR_IA32_X2APIC_EOI, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_EOI                      0x0000080B


/**
  x2APIC Logical Destination Register (R/O). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_LDR (0x0000080D)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_LDR);
  @endcode
**/
#define MSR_IA32_X2APIC_LDR                      0x0000080D


/**
  x2APIC Spurious Interrupt Vector Register (R/W). If CPUID.01H:ECX.[21] = 1
  && IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_SIVR (0x0000080F)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_SIVR);
  AsmWriteMsr64 (MSR_IA32_X2APIC_SIVR, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_SIVR                     0x0000080F


/**
  x2APIC In-Service Register Bits (n * 32 + 31):(n * 32) (R/O).
  If CPUID.01H:ECX.[21] = 1 && IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_ISRn
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_ISR0);
  @endcode
  @{
**/
#define MSR_IA32_X2APIC_ISR0                     0x00000810
#define MSR_IA32_X2APIC_ISR1                     0x00000811
#define MSR_IA32_X2APIC_ISR2                     0x00000812
#define MSR_IA32_X2APIC_ISR3                     0x00000813
#define MSR_IA32_X2APIC_ISR4                     0x00000814
#define MSR_IA32_X2APIC_ISR5                     0x00000815
#define MSR_IA32_X2APIC_ISR6                     0x00000816
#define MSR_IA32_X2APIC_ISR7                     0x00000817
/// @}


/**
  x2APIC Trigger Mode Register Bits (n * 32 + ):(n * 32) (R/O).
  If CPUID.01H:ECX.[21] = 1 && IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_TMRn
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_TMR0);
  @endcode
  @{
**/
#define MSR_IA32_X2APIC_TMR0                     0x00000818
#define MSR_IA32_X2APIC_TMR1                     0x00000819
#define MSR_IA32_X2APIC_TMR2                     0x0000081A
#define MSR_IA32_X2APIC_TMR3                     0x0000081B
#define MSR_IA32_X2APIC_TMR4                     0x0000081C
#define MSR_IA32_X2APIC_TMR5                     0x0000081D
#define MSR_IA32_X2APIC_TMR6                     0x0000081E
#define MSR_IA32_X2APIC_TMR7                     0x0000081F
/// @}


/**
  x2APIC Interrupt Request Register Bits (n* 32 + 31):(n * 32) (R/O).
  If CPUID.01H:ECX.[21] = 1 && IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_IRRn
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_IRR0);
  @endcode
  @{
**/
#define MSR_IA32_X2APIC_IRR0                     0x00000820
#define MSR_IA32_X2APIC_IRR1                     0x00000821
#define MSR_IA32_X2APIC_IRR2                     0x00000822
#define MSR_IA32_X2APIC_IRR3                     0x00000823
#define MSR_IA32_X2APIC_IRR4                     0x00000824
#define MSR_IA32_X2APIC_IRR5                     0x00000825
#define MSR_IA32_X2APIC_IRR6                     0x00000826
#define MSR_IA32_X2APIC_IRR7                     0x00000827
/// @}


/**
  x2APIC Error Status Register (R/W). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_ESR (0x00000828)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_ESR);
  AsmWriteMsr64 (MSR_IA32_X2APIC_ESR, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_ESR                      0x00000828


/**
  x2APIC LVT Corrected Machine Check Interrupt Register (R/W). If
  CPUID.01H:ECX.[21] = 1 && IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_LVT_CMCI (0x0000082F)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_LVT_CMCI);
  AsmWriteMsr64 (MSR_IA32_X2APIC_LVT_CMCI, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_LVT_CMCI                 0x0000082F


/**
  x2APIC Interrupt Command Register (R/W). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_ICR (0x00000830)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_ICR);
  AsmWriteMsr64 (MSR_IA32_X2APIC_ICR, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_ICR                      0x00000830


/**
  x2APIC LVT Timer Interrupt Register (R/W). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_LVT_TIMER (0x00000832)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_LVT_TIMER);
  AsmWriteMsr64 (MSR_IA32_X2APIC_LVT_TIMER, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_LVT_TIMER                0x00000832


/**
  x2APIC LVT Thermal Sensor Interrupt Register (R/W). If CPUID.01H:ECX.[21] =
  1 && IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_LVT_THERMAL (0x00000833)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_LVT_THERMAL);
  AsmWriteMsr64 (MSR_IA32_X2APIC_LVT_THERMAL, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_LVT_THERMAL              0x00000833


/**
  x2APIC LVT Performance Monitor Interrupt Register (R/W). If
  CPUID.01H:ECX.[21] = 1 && IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_LVT_PMI (0x00000834)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_LVT_PMI);
  AsmWriteMsr64 (MSR_IA32_X2APIC_LVT_PMI, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_LVT_PMI                  0x00000834


/**
  x2APIC LVT LINT0 Register (R/W). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_LVT_LINT0 (0x00000835)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_LVT_LINT0);
  AsmWriteMsr64 (MSR_IA32_X2APIC_LVT_LINT0, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_LVT_LINT0                0x00000835


/**
  x2APIC LVT LINT1 Register (R/W). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_LVT_LINT1 (0x00000836)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_LVT_LINT1);
  AsmWriteMsr64 (MSR_IA32_X2APIC_LVT_LINT1, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_LVT_LINT1                0x00000836


/**
  x2APIC LVT Error Register (R/W). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_LVT_ERROR (0x00000837)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_LVT_ERROR);
  AsmWriteMsr64 (MSR_IA32_X2APIC_LVT_ERROR, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_LVT_ERROR                0x00000837


/**
  x2APIC Initial Count Register (R/W). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_INIT_COUNT (0x00000838)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_INIT_COUNT);
  AsmWriteMsr64 (MSR_IA32_X2APIC_INIT_COUNT, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_INIT_COUNT               0x00000838


/**
  x2APIC Current Count Register (R/O). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_CUR_COUNT (0x00000839)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_CUR_COUNT);
  @endcode
**/
#define MSR_IA32_X2APIC_CUR_COUNT                0x00000839


/**
  x2APIC Divide Configuration Register (R/W). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_DIV_CONF (0x0000083E)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_X2APIC_DIV_CONF);
  AsmWriteMsr64 (MSR_IA32_X2APIC_DIV_CONF, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_DIV_CONF                 0x0000083E


/**
  x2APIC Self IPI Register (W/O). If CPUID.01H:ECX.[21] = 1 &&
  IA32_APIC_BASE.[10] = 1.

  @param  ECX  MSR_IA32_X2APIC_SELF_IPI (0x0000083F)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = 0;
  AsmWriteMsr64 (MSR_IA32_X2APIC_SELF_IPI, Msr);
  @endcode
**/
#define MSR_IA32_X2APIC_SELF_IPI                 0x0000083F


/**
  Silicon Debug Feature Control (R/W). If CPUID.01H:ECX.[11] = 1.

  @param  ECX  MSR_IA32_DEBUG_INTERFACE (0x00000C80)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_DEBUG_INTERFACE_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_DEBUG_INTERFACE_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_DEBUG_INTERFACE_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_DEBUG_INTERFACE);
  AsmWriteMsr64 (MSR_IA32_DEBUG_INTERFACE, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_DEBUG_INTERFACE                 0x00000C80

/**
  MSR information returned for MSR index #MSR_IA32_DEBUG_INTERFACE
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Enable (R/W) BIOS set 1 to enable Silicon debug features.
    /// Default is 0. If CPUID.01H:ECX.[11] = 1.
    ///
    UINT32  Enable:1;
    UINT32  Reserved1:29;
    ///
    /// [Bit 30] Lock (R/W): If 1, locks any further change to the MSR. The
    /// lock bit is set automatically on the first SMI assertion even if not
    /// explicitly set by BIOS. Default is 0. If CPUID.01H:ECX.[11] = 1.
    ///
    UINT32  Lock:1;
    ///
    /// [Bit 31] Debug Occurred (R/O): This "sticky bit" is set by hardware to
    /// indicate the status of bit 0. Default is 0. If CPUID.01H:ECX.[11] = 1.
    ///
    UINT32  DebugOccurred:1;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_DEBUG_INTERFACE_REGISTER;


/**
  L3 QOS Configuration (R/W). If ( CPUID.(EAX=10H, ECX=1):ECX.[2] = 1 ).

  @param  ECX  MSR_IA32_L3_QOS_CFG (0x00000C81)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_L3_QOS_CFG_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_L3_QOS_CFG_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_L3_QOS_CFG_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_L3_QOS_CFG);
  AsmWriteMsr64 (MSR_IA32_L3_QOS_CFG, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_L3_QOS_CFG                      0x00000C81

/**
  MSR information returned for MSR index #MSR_IA32_L3_QOS_CFG
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] Enable (R/W) Set 1 to enable L3 CAT masks and COS to operate
    /// in Code and Data Prioritization (CDP) mode.
    ///
    UINT32  Enable:1;
    UINT32  Reserved1:31;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_L3_QOS_CFG_REGISTER;


/**
  Monitoring Event Select Register (R/W). If ( CPUID.(EAX=07H, ECX=0):EBX.[12]
  = 1 ).

  @param  ECX  MSR_IA32_QM_EVTSEL (0x00000C8D)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_QM_EVTSEL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_QM_EVTSEL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_QM_EVTSEL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_QM_EVTSEL);
  AsmWriteMsr64 (MSR_IA32_QM_EVTSEL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_QM_EVTSEL                       0x00000C8D

/**
  MSR information returned for MSR index #MSR_IA32_QM_EVTSEL
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 7:0] Event ID: ID of a supported monitoring event to report via
    /// IA32_QM_CTR.
    ///
    UINT32  EventID:8;
    UINT32  Reserved:24;
    ///
    /// [Bits 63:32] Resource Monitoring ID: ID for monitoring hardware to
    /// report monitored data via IA32_QM_CTR. N = Ceil (Log:sub:`2` (
    /// CPUID.(EAX= 0FH, ECX=0H).EBX[31:0] +1)).
    ///
    UINT32  ResourceMonitoringID:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_QM_EVTSEL_REGISTER;


/**
  Monitoring Counter Register (R/O). If ( CPUID.(EAX=07H, ECX=0):EBX.[12] = 1
  ).

  @param  ECX  MSR_IA32_QM_CTR (0x00000C8E)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_QM_CTR_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_QM_CTR_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_QM_CTR_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_QM_CTR);
  @endcode
**/
#define MSR_IA32_QM_CTR                          0x00000C8E

/**
  MSR information returned for MSR index #MSR_IA32_QM_CTR
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 31:0] Resource Monitored Data.
    ///
    UINT32  ResourceMonitoredData:32;
    ///
    /// [Bits 61:32] Resource Monitored Data.
    ///
    UINT32  ResourceMonitoredDataHi:30;
    ///
    /// [Bit 62] Unavailable: If 1, indicates data for this RMID is not
    /// available or not monitored for this resource or RMID.
    ///
    UINT32  Unavailable:1;
    ///
    /// [Bit 63] Error: If 1, indicates and unsupported RMID or event type was
    /// written to IA32_PQR_QM_EVTSEL.
    ///
    UINT32  Error:1;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_QM_CTR_REGISTER;


/**
  Resource Association Register (R/W). If ( CPUID.(EAX=07H, ECX=0):EBX.[12] =
  1 ).

  @param  ECX  MSR_IA32_PQR_ASSOC (0x00000C8F)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PQR_ASSOC_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PQR_ASSOC_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PQR_ASSOC_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PQR_ASSOC);
  AsmWriteMsr64 (MSR_IA32_PQR_ASSOC, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PQR_ASSOC                       0x00000C8F

/**
  MSR information returned for MSR index #MSR_IA32_PQR_ASSOC
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 31:0] Resource Monitoring ID (R/W): ID for monitoring hardware
    /// to track internal operation, e.g. memory access. N = Ceil (Log:sub:`2`
    /// ( CPUID.(EAX= 0FH, ECX=0H).EBX[31:0] +1)).
    ///
    UINT32  ResourceMonitoringID:32;
    ///
    /// [Bits 63:32] COS (R/W). The class of service (COS) to enforce (on
    /// writes); returns the current COS when read. If ( CPUID.(EAX=07H,
    /// ECX=0):EBX.[15] = 1 ).
    ///
    UINT32  COS:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PQR_ASSOC_REGISTER;


/**
  Supervisor State of MPX Configuration. (R/W). If (CPUID.(EAX=07H,
  ECX=0H):EBX[14] = 1).

  @param  ECX  MSR_IA32_BNDCFGS (0x00000D90)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_BNDCFGS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_BNDCFGS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_BNDCFGS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_BNDCFGS);
  AsmWriteMsr64 (MSR_IA32_BNDCFGS, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_BNDCFGS                         0x00000D90

/**
  MSR information returned for MSR index #MSR_IA32_BNDCFGS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] EN: Enable Intel MPX in supervisor mode.
    ///
    UINT32  EN:1;
    ///
    /// [Bit 1] BNDPRESERVE: Preserve the bounds registers for near branch
    /// instructions in the absence of the BND prefix.
    ///
    UINT32  BNDPRESERVE:1;
    UINT32  Reserved:10;
    ///
    /// [Bits 31:12] Base Address of Bound Directory.
    ///
    UINT32  Base:20;
    ///
    /// [Bits 63:32] Base Address of Bound Directory.
    ///
    UINT32  BaseHi:32;
  } Bits;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_BNDCFGS_REGISTER;


/**
  Extended Supervisor State Mask (R/W). If( CPUID.(0DH, 1):EAX.[3] = 1.

  @param  ECX  MSR_IA32_XSS (0x00000DA0)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_XSS_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_XSS_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_XSS_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_XSS);
  AsmWriteMsr64 (MSR_IA32_XSS, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_XSS                             0x00000DA0

/**
  MSR information returned for MSR index #MSR_IA32_XSS
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    UINT32  Reserved1:8;
    ///
    /// [Bit 8] Trace Packet Configuration State (R/W).
    ///
    UINT32  TracePacketConfigurationState:1;
    UINT32  Reserved2:23;
    UINT32  Reserved3:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_XSS_REGISTER;


/**
  Package Level Enable/disable HDC (R/W). If CPUID.06H:EAX.[13] = 1.

  @param  ECX  MSR_IA32_PKG_HDC_CTL (0x00000DB0)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PKG_HDC_CTL_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PKG_HDC_CTL_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PKG_HDC_CTL_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PKG_HDC_CTL);
  AsmWriteMsr64 (MSR_IA32_PKG_HDC_CTL, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PKG_HDC_CTL                     0x00000DB0

/**
  MSR information returned for MSR index #MSR_IA32_PKG_HDC_CTL
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] HDC_Pkg_Enable (R/W) Force HDC idling or wake up HDC-idled
    /// logical processors in the package. See Section 14.5.2, "Package level
    /// Enabling HDC". If CPUID.06H:EAX.[13] = 1.
    ///
    UINT32  HDC_Pkg_Enable:1;
    UINT32  Reserved1:31;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PKG_HDC_CTL_REGISTER;


/**
  Enable/disable HWP (R/W). If CPUID.06H:EAX.[13] = 1.

  @param  ECX  MSR_IA32_PM_CTL1 (0x00000DB1)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_PM_CTL1_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_PM_CTL1_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_PM_CTL1_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PM_CTL1);
  AsmWriteMsr64 (MSR_IA32_PM_CTL1, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_PM_CTL1                         0x00000DB1

/**
  MSR information returned for MSR index #MSR_IA32_PM_CTL1
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] HDC_Allow_Block (R/W) Allow/Block this logical processor for
    /// package level HDC control. See Section 14.5.3.
    /// If CPUID.06H:EAX.[13] = 1.
    ///
    UINT32  HDC_Allow_Block:1;
    UINT32  Reserved1:31;
    UINT32  Reserved2:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_PM_CTL1_REGISTER;


/**
  Per-Logical_Processor HDC Idle Residency (R/0). If CPUID.06H:EAX.[13] = 1.
  Stall_Cycle_Cnt (R/W) Stalled cycles due to HDC forced idle on this logical
  processor. See Section 14.5.4.1. If CPUID.06H:EAX.[13] = 1.

  @param  ECX  MSR_IA32_THREAD_STALL (0x00000DB2)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_THREAD_STALL);
  @endcode
**/
#define MSR_IA32_THREAD_STALL                    0x00000DB2


/**
  Extended Feature Enables. If ( CPUID.80000001H:EDX.[2 0]
  CPUID.80000001H:EDX.[2 9]).

  @param  ECX  MSR_IA32_EFER (0xC0000080)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_EFER_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_EFER_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_EFER_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_EFER);
  AsmWriteMsr64 (MSR_IA32_EFER, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_EFER                            0xC0000080

/**
  MSR information returned for MSR index #MSR_IA32_EFER
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bit 0] SYSCALL Enable: IA32_EFER.SCE (R/W) Enables SYSCALL/SYSRET
    /// instructions in 64-bit mode.
    ///
    UINT32  SCE:1;
    UINT32  Reserved1:7;
    ///
    /// [Bit 8] IA-32e Mode Enable: IA32_EFER.LME (R/W) Enables IA-32e mode
    /// operation.
    ///
    UINT32  LME:1;
    UINT32  Reserved2:1;
    ///
    /// [Bit 10] IA-32e Mode Active: IA32_EFER.LMA (R) Indicates IA-32e mode
    /// is active when set.
    ///
    UINT32  LMA:1;
    ///
    /// [Bit 11] Execute Disable Bit Enable: IA32_EFER.NXE (R/W).
    ///
    UINT32  NXE:1;
    UINT32  Reserved3:20;
    UINT32  Reserved4:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_EFER_REGISTER;


/**
  System Call Target Address (R/W). If CPUID.80000001:EDX.[29] = 1.

  @param  ECX  MSR_IA32_STAR (0xC0000081)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_STAR);
  AsmWriteMsr64 (MSR_IA32_STAR, Msr);
  @endcode
**/
#define MSR_IA32_STAR                            0xC0000081


/**
  IA-32e Mode System Call Target Address (R/W). If CPUID.80000001:EDX.[29] = 1.

  @param  ECX  MSR_IA32_LSTAR (0xC0000082)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_LSTAR);
  AsmWriteMsr64 (MSR_IA32_LSTAR, Msr);
  @endcode
**/
#define MSR_IA32_LSTAR                           0xC0000082


/**
  System Call Flag Mask (R/W). If CPUID.80000001:EDX.[29] = 1.

  @param  ECX  MSR_IA32_FMASK (0xC0000084)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_FMASK);
  AsmWriteMsr64 (MSR_IA32_FMASK, Msr);
  @endcode
**/
#define MSR_IA32_FMASK                           0xC0000084


/**
  Map of BASE Address of FS (R/W). If CPUID.80000001:EDX.[29] = 1.

  @param  ECX  MSR_IA32_FS_BASE (0xC0000100)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_FS_BASE);
  AsmWriteMsr64 (MSR_IA32_FS_BASE, Msr);
  @endcode
**/
#define MSR_IA32_FS_BASE                         0xC0000100


/**
  Map of BASE Address of GS (R/W). If CPUID.80000001:EDX.[29] = 1.

  @param  ECX  MSR_IA32_GS_BASE (0xC0000101)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_GS_BASE);
  AsmWriteMsr64 (MSR_IA32_GS_BASE, Msr);
  @endcode
**/
#define MSR_IA32_GS_BASE                         0xC0000101


/**
  Swap Target of BASE Address of GS (R/W). If CPUID.80000001:EDX.[29] = 1.

  @param  ECX  MSR_IA32_KERNEL_GS_BASE (0xC0000102)
  @param  EAX  Lower 32-bits of MSR value.
  @param  EDX  Upper 32-bits of MSR value.

  <b>Example usage</b>
  @code
  UINT64  Msr;

  Msr = AsmReadMsr64 (MSR_IA32_KERNEL_GS_BASE);
  AsmWriteMsr64 (MSR_IA32_KERNEL_GS_BASE, Msr);
  @endcode
**/
#define MSR_IA32_KERNEL_GS_BASE                  0xC0000102


/**
  Auxiliary TSC (RW). If CPUID.80000001H: EDX[27] = 1.

  @param  ECX  MSR_IA32_TSC_AUX (0xC0000103)
  @param  EAX  Lower 32-bits of MSR value.
               Described by the type MSR_IA32_TSC_AUX_REGISTER.
  @param  EDX  Upper 32-bits of MSR value.
               Described by the type MSR_IA32_TSC_AUX_REGISTER.

  <b>Example usage</b>
  @code
  MSR_IA32_TSC_AUX_REGISTER  Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_TSC_AUX);
  AsmWriteMsr64 (MSR_IA32_TSC_AUX, Msr.Uint64);
  @endcode
**/
#define MSR_IA32_TSC_AUX                         0xC0000103

/**
  MSR information returned for MSR index #MSR_IA32_TSC_AUX
**/
typedef union {
  ///
  /// Individual bit fields
  ///
  struct {
    ///
    /// [Bits 31:0] AUX: Auxiliary signature of TSC.
    ///
    UINT32  AUX:32;
    UINT32  Reserved:32;
  } Bits;
  ///
  /// All bit fields as a 32-bit value
  ///
  UINT32  Uint32;
  ///
  /// All bit fields as a 64-bit value
  ///
  UINT64  Uint64;
} MSR_IA32_TSC_AUX_REGISTER;

#endif
