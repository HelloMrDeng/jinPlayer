// $Change: 482920 $
//-----------------------------------------------------------------------------
//
// Copyright (c) 2008 MStar Semiconductor, Inc.  All rights reserved.
//
//-----------------------------------------------------------------------------
// FILE
//      columbus_pmu.h
//
// DESCRIPTION
//      Define the PMU controller registers
//
// HISTORY
//      2008.2.29       Bryan Fan      Initial Version
//      2008.6.27       Bryan Fan      Revised for new Code Review
//      2008.07.02      Bryan Fan      Revised for new Code Review
//
//-----------------------------------------------------------------------------

#ifndef __COLUMBUS_PMU_H
#define __COLUMBUS_PMU_H

#if __cplusplus
extern "C" {
#endif //__cplusplus
//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------
#include "columbus_typedefs.h"


#define READ_REGISTER_ULONG(reg) \
    (*(volatile unsigned long * const)(reg))

#define WRITE_REGISTER_ULONG(reg, val) \
    (*(volatile unsigned long * const)(reg)) = (val)

#define READ_REGISTER_USHORT(reg) \
    (*(volatile unsigned short * const)(reg))

#define WRITE_REGISTER_USHORT(reg, val) \
    (*(volatile unsigned short * const)(reg)) = (val)

#define READ_REGISTER_UCHAR(reg) \
    (*(volatile unsigned char * const)(reg))

#define WRITE_REGISTER_UCHAR(reg, val) \
    (*(volatile unsigned char * const)(reg)) = (val)


#define INREG8(x)           READ_REGISTER_UCHAR(x)
#define OUTREG8(x, y)       WRITE_REGISTER_UCHAR(x, (UCHAR)(y))
#define SETREG8(x, y)       OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)       OUTREG8(x, INREG8(x)&~(y))

#define INREG16(x)          READ_REGISTER_USHORT(x)
#define OUTREG16(x, y)      WRITE_REGISTER_USHORT(x,(USHORT)(y))
#define SETREG16(x, y)      OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)      OUTREG16(x, INREG16(x)&~(y))

#define INREG32(x)          READ_REGISTER_ULONG(x)
#define OUTREG32(x, y)      WRITE_REGISTER_ULONG(x, (ULONG)(y))
#define SETREG32(x, y)      OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)      OUTREG32(x, INREG32(x)&~(y))

//------------------------------------------------------------------------------
//  Data Types
//------------------------------------------------------------------------------
typedef struct
{
    U16 u16REG_LDO_PG_STATUS;                   // PA: 0xA0007E00, VA: 0xA4007E00
    U16 u16REG_REG00;

    U16 u16REG_LDO_VMEM_VLCM;
    U16 u16REG_REG01;

    U16 u16REG_LDO_VWIFI;
    U16 u16REG_REG02;

    U16 u16REG_SUSPEND_PWD;
    U16 u16REG_REG03;

    U16 u16REG_GPIO_G_IN;
    U16 u16REG_REG04;

    U16 u16REG_LDO_PERR_INTR_EN;
    U16 u16REG_REG05;

    U16 u16REG_BAT_DETECT;
    U16 u16REG_REG06;

    U16 u16REG_REFGEN_EN;
    U16 u16REG_REG07;

    U16 u16REG_REFGEN_CTL;
    U16 u16REG_REG08;

    U16 u16REG_SPACE09;
    U16 u16REG_REG09;

    U16 u16REG_TRIM;
    U16 u16REG_REG0A;

    U16 u16REG_PMU_FSM;
    U16 u16REG_REG0B;

    U16 u16REG_WAIT_TIME;
    U16 u16REG_REG0C;

    U16 u16REG_WAKEUP_SRC;
    U16 u16REG_REG0D;

    U16 u16REG_PMU_STATE;
    U16 u16REG_REG0E;

    U16 u16REG_PMU_TOP;
    U16 u16REG_REG0F;

    U16 u16REG_CKSUM_OFF0;//key to off riu_cksum_prot (h51685168)
    U16 u16REG_REG10;

    U16 u16REG_CKSUM_OFF1;
    U16 u16REG_REG11;

    U16 u16REG_SPACE12;
    U16 u16REG_REG12;

    U16 u16REG_BOOST_CTL;
    U16 u16REG_REG13;

    U16 u16REG_BOOST_MISC;
    U16 u16REG_REG14;

    U16 u16REG_SPACE15;
    U16 u16REG_REG15;

    U16 u16REG_SPACE16;
    U16 u16REG_REG16;

    U16 u16REG_SPACE17;
    U16 u16REG_REG17;

    U16 u16REG_SW_SPARE0;
    U16 u16REG_REG18;

    U16 u16REG_SW_SPARE1;
    U16 u16REG_REG19;

    U16 u16REG_SW_SPARE2;
    U16 u16REG_REG1A;

    U16 u16REG_SW_SPARE3;
    U16 u16REG_REG1B;

    U16 u16REG_PMU_SPARE0;
    U16 u16REG_REG1C;

    U16 u16REG_PMU_SPARE1;
    U16 u16REG_REG1D;

    U16 u16REG_PMU_SPARE2;
    U16 u16REG_REG1E;

    U16 u16REG_PMU_SPARE3;
    U16 u16REG_REG1F;

    U16 u16REG_SPACE20;
    U16 u16REG_REG20;

    U16 u16REG_SPACE21;
    U16 u16REG_REG21;

    U16 u16REG_SPACE22;
    U16 u16REG_REG22;

    U16 u16REG_SPACE23;
    U16 u16REG_REG23;

    U16 u16REG_SPACE24;
    U16 u16REG_REG24;

    U16 u16REG_SPACE25;
    U16 u16REG_REG25;

    U16 u16REG_SPACE26;
    U16 u16REG_REG26;

    U16 u16REG_CLK32K_FLG;
    U16 u16REG_REG27;

    U16 u16REG_SPACE28;
    U16 u16REG_REG28;

    U16 u16REG_SPACE29;
    U16 u16REG_REG29;

    U16 u16REG_SPACE2A;
    U16 u16REG_REG2A;

    U16 u16REG_SPACE2B;
    U16 u16REG_REG2B;

    U16 u16REG_SPACE2C;
    U16 u16REG_REG2C;

    U16 u16REG_SPACE2D;
    U16 u16REG_REG2D;

    U16 u16REG_SPACE2E;
    U16 u16REG_REG2E;

    U16 u16REG_SPACE2F;
    U16 u16REG_REG2F;

    U16 u16REG_SPACE30;                   // PA: 0xA0007E00, VA: 0xA4007E00
    U16 u16REG_REG30;

    U16 u16REG_SPACE31;
    U16 u16REG_REG31;

    U16 u16REG_SPACE32;
    U16 u16REG_REG32;

    U16 u16REG_SPACE33;
    U16 u16REG_REG33;

    U16 u16REG_VMEM_RAMP_CTL;
    U16 u16REG_REG34;

    U16 u16REG_SPACE35;
    U16 u16REG_REG35;

    U16 u16REG_SPACE36;
    U16 u16REG_REG36;

    U16 u16REG_SPACE37;
    U16 u16REG_REG37;

    U16 u16REG_SPACE38;
    U16 u16REG_REG38;

    U16 u16REG_SPACE39;
    U16 u16REG_REG39;

    U16 u16REG_SPACE3A;
    U16 u16REG_REG3A;

    U16 u16REG_SPACE3B;
    U16 u16REG_REG3B;

    U16 u16REG_SPACE3C;
    U16 u16REG_REG3C;

    U16 u16REG_SPACE3D;
    U16 u16REG_REG3D;

    U16 u16REG_SPACE3E;
    U16 u16REG_REG3E;

    U16 u16REG_SPACE3F;
    U16 u16REG_REG3F;

    U16 u16REG_SPACE40;
    U16 u16REG_REG40;

    U16 u16REG_SPACE41;
    U16 u16REG_REG41;

    U16 u16REG_LDO_VL_EN;
    U16 u16REG_REG42;

    U16 u16REG_SPACE43;
    U16 u16REG_REG43;

    U16 u16REG_FRO_CTL;
    U16 u16REG_REG44;

    U16 u16REG_LDO_VPER12_CTL;
    U16 u16REG_REG45;

    U16 u16REG_LDO_VPER3_CTL;
    U16 u16REG_REG46;

    U16 u16REG_BUCK12_MISC;
    U16 u16REG_REG47;

    U16 u16REG_BUCK1_CTL;
    U16 u16REG_REG48;

    U16 u16REG_PM_LDO;
    U16 u16REG_REG49;

    U16 u16REG_BUCK2_CTL;
    U16 u16REG_REG4A;

    U16 u16REG_SPACE4B;
    U16 u16REG_REG4B;

    U16 u16REG_SPACE4C;
    U16 u16REG_REG4C;

    U16 u16REG_SPACE4D;
    U16 u16REG_REG4D;

    U16 u16REG_SPACE4E;
    U16 u16REG_REG4E;

    U16 u16REG_SPACE4F;
    U16 u16REG_REG4F;

    U16 u16REG_SPACE50;
    U16 u16REG_REG50;

    U16 u16REG_SPACE51;
    U16 u16REG_REG51;

    U16 u16REG_SPACE52;
    U16 u16REG_REG52;

    U16 u16REG_SPACE53;
    U16 u16REG_REG53;

    U16 u16REG_SPACE54;
    U16 u16REG_REG54;

    U16 u16REG_SPACE55;
    U16 u16REG_REG55;

    U16 u16REG_SPACE56;
    U16 u16REG_REG56;

    U16 u16REG_SPACE57;
    U16 u16REG_REG57;

    U16 u16REG_SPACE58;
    U16 u16REG_REG58;

    U16 u16REG_SPACE59;
    U16 u16REG_REG59;

    U16 u16REG_SPACE5A;
    U16 u16REG_REG5A;

    U16 u16REG_SPACE5B;
    U16 u16REG_REG5B;

    U16 u16REG_SPACE5C;
    U16 u16REG_REG5C;

    U16 u16REG_PMU_TESTBUS;
    U16 u16REG_REG5D;

    U16 u16REG_SPACE5E;
    U16 u16REG_REG5E;

    U16 u16REG_RESV_STATUS;
    U16 u16REG_REG5F;

    U16 u16REG_CHARGER_CTL;                   // PA: 0xA0007E00, VA: 0xA4007E00
    U16 u16REG_REG60;

    U16 u16REG_CHARGER_MISC;
    U16 u16REG_REG61;

    U16 u16REG_ABBRESET_PWD;
    U16 u16REG_REG62;

    U16 u16REG_PHONE_OFF;
    U16 u16REG_REG63;

    U16 u16REG_PMU_MODE_CTL;
    U16 u16REG_REG64;

    U16 u16REG_PMU_MISC;
    U16 u16REG_REG65;

    U16 u16REG_PMU_FSMSTU0;
    U16 u16REG_REG66;

    U16 u16REG_PMU_FSMSTU1;
    U16 u16REG_REG67;

    U16 u16REG_SUSPEN_STU;
    U16 u16REG_REG68;

    U16 u16REG_KEYOFF_TIME;
    U16 u16REG_REG69;

    U16 u16REG_SEL_TMUX;
    U16 u16REG_REG6A;

    U16 u16REG_CLASSD_TEST;
    U16 u16REG_REG6B;

    U16 u16REG_CLASSD_CTL;
    U16 u16REG_REG6C;

    U16 u16REG_CLASSD_MISC;
    U16 u16REG_REG6D;

    U16 u16REG_ASPARE01;
    U16 u16REG_REG6E;

    U16 u16REG_SPACE6F;
    U16 u16REG_REG6F;

    U16 u16REG_IRQ_MASK0;
    U16 u16REG_REG70;

    U16 u16REG_IRQ_MASK1;
    U16 u16REG_REG71;

    U16 u16REG_IRQ_FORCE0;
    U16 u16REG_REG72;

    U16 u16REG_IRQ_FORCE1;
    U16 u16REG_REG73;

    U16 u16REG_IRQ_CLR0;
    U16 u16REG_REG74;

    U16 u16REG_IRQ_CLR1;
    U16 u16REG_REG75;

    U16 u16REG_IRQ_RAW_STU0;
    U16 u16REG_REG76;

    U16 u16REG_IRQ_RAW_STU1;
    U16 u16REG_REG77;

    U16 u16REG_IRQ_FINAL_STU0;
    U16 u16REG_REG78;

    U16 u16REG_IRQ_FINAL_STU1;
    U16 u16REG_REG79;

    U16 u16REG_IRQ_POLARITY0;
    U16 u16REG_REG7A;

    U16 u16REG_IRQ_POLARITY1;
    U16 u16REG_REG7B;

    U16 u16REG_RIF_MODE;
    U16 u16REG_REG7C;

    U16 u16REG_GPIO_G_OUT;
    U16 u16REG_REG7D;

    U16 u16REG_GPIO_G_OEN;
    U16 u16REG_REG7E;
} REG_PMU_st, *PREG_PMU_st;

//------------------------------------------------------------------------------
//  Constants
//------------------------------------------------------------------------------
#define PMU_FREQ_DIV            32
#define PMU_PRESSED_PERIOD      128//68+3 //ms
#define PMU_TP_PERIOD           256
#define PMU_POR_READY_PERIOD    64
#define PMU_CPU_AWAIT_TIME      464
#define PMU_READY_DELAY         30
#define PMU_INT_DELAY           5
#define PMU_PK_PRESSED_PERIOD   6000 // 6 secs

#define PMU_FSM_NUM             5
#define PMU_DEFAULT_IRQ_MASK    0x0E30
#define PMU_KEY_PRESSED_READY   1000 //msec
#define PMU_KEY_PERIOD_SLEEP    128

#define ITCM_PA                 (0x00000000)
#define ITCM_UA                 (COLUMBUS_BASE_REG_RIU_UA + 0x00100000)

//PMU_FLAG
#define PMU_KEY_ONOFF_FLAG           0x00000002
#define PMU_CHARGDET_FLAG             0x00000008

// PMU_IRQ_STATUS
#define PMU_IRQ_CHARGDET              0x00000200//[9] : adaptor_plug_irq; charger plug-in

#define PMU_IRQ_STATUS_MASK        0x000007FF

#define PMU_IRQ_BUCK1OCP_DEBOUNCE 0x00000001//[0] : buck1ocp_debounce; buck1 over current protect"
#define PMU_IRQ_BUCK2OCP_DEBOUNCE 0x00000002//[1] : buck2ocp_debounce; buck2 over current protect
#define PMU_IRQ_OVER_VOLT             0x00000004//[2] : over_voltage_irq; charger over voltage
#define PMU_IRQ_OTP_DEBOUNCE      0x00000008//[3] : otp_debounce; refgen over temperature protect
#define PMU_IRQ_LDO_ERR                 0x00000010//[4] : LDO_ERR_IRQ; LDO ERROR
#define PMU_IRQ_PGA_MUTE              0x00000020 //[5] : irq_pga_mute; AUDIO receiver PGA mute
#define PMU_IRQ_OCP_CLASSD           0x00000040//[6] : irq_ocp_classd; CLASS-D over current protect
#define PMU_IRQ_BAT_NONE              0x00000080 //[7] : no_bat_int; no battery
#define PMU_IRQ_BAT_LOW                0x00000100//[8] : bat_low_irq; battery <3.0V
#define PMU_IRQ_ADPT_PLUG             0x00000200//[9] : adaptor_plug_irq; charger plug-in
#define PMU_IRQ_ADPT_UNPLUG        0x00000400//[10] : adaptor_unplug_irq; charger plug-out

#define PMU_IRQ_KEY_ONOFF_RELEASED 0x00000800//[11] : KEYOFF_PULSE; ONOFF KEY release
#define PMU_IRQ_KEY_ONOFF_PRESSED   0x00001000//[12] : ONOFF_PULSE; ONOFF KEY push
#define PMU_IRQ_RTC_ALARM             0x00002000//[13] : RTC alarm
#define PMU_IRQ_TP_INT                    0x00004000//[14] : touch panel
#define PMU_IRQ_SAR_DET                 0x00008000//[15] : SAR key detection
//[26:16] : GPIO interrupt
#define PMU_IRQ_SD_INT                    0x08000000//[27] : SD card insertion
#define PMU_IRQ_MS_INT                    0x10000000//[28] : MS insertion
#define PMU_IRQ_USB_CID                  0x20000000//[29] : USB insertion
#define PMU_IRQ_CHKSUM_PRT           0x40000000//[30] : Checksum protect
//[31] : reserve

// PMU_IRQ_RAW_STATUS
#define PMU_RAW_PWR_BUTTON          0x00000001
#define PMU_RAW_USB_CID_OUT         0x00000002
#define PMU_RAW_CHARGDET_OUT        0x00000004
#define PMU_RAW_ATHENA_OUT          0x00000008
#define PMU_RAW_TP_OUT              0x00000010
#define PMU_RAW_SD_CDZ_OUT          0x00000020
#define PMU_RAW_MS_INS_OUT          0x00000040
#define PMU_RAW_PMIC_BATON_C_PMU    0x00000080
#define PMU_RAW_SC_CDZ_C            0x00000100
#define PMU_RAW_MS_INS_C            0x00000200

// PMU_SW_PK_THRESHOLD
#define SW_PK_THRESHOLD_MASK        0x00000007
#define SW_PK_500                   0x00000000
#define SW_PK_1000                  0x00000001
#define SW_PK_2000                  0x00000002
#define SW_PK_3000                  0x00000003
#define SW_PK_4000                  0x00000004
#define SW_PK_5000                  0x00000005
#define SW_PK_6000                  0x00000006
#define SW_PK_7000                  0x00000007

// PMU_TEST_MODE1
#define TEST_MODE1_MASK             0x00000391
#define TEST_MODE1_KEY_PRESS        0x00000001
#define TEST_MODE1_USB_CID          0x00000010
#define TEST_MODE1_CHARGDET         0x00000080
#define TEST_MODE1_ATHENA_INT       0x00000100
#define TEST_MODE1_TP_INT           0x00000200

#define TEST_MODE1_PF_WORKING1      0x00000002
#define TEST_MODE1_PF_WORKING2      0x00000004

// PMU_TEST_MODE2
#define TEST_MODE2_FSM_0            0x00000001
#define TEST_MODE2_FSM_1            0x00000002
#define TEST_MODE2_FSM_2            0x00000004
#define TEST_MODE2_FSM_3            0x00000008
#define TEST_MODE2_FSM_4            0x00000010
#define TEST_MODE2_FSM_5            0x00000020

#define TEST_MODE2_POR              0x00000080
#define TEST_MODE2_BUCKEN_DE        0x00000200
#define TEST_MODE2_CHIPEN_DE        0x00000800
#define TEST_MODE2_PF_WORKING       0x00001000
#define TEST_MODE2_SD_CDZ_C         0x00004000

#define TEST_MODE2_FSM_MASK         0x0000003F
#define TEST_MODE2_BUCKEN_MASK      0x00000200
#define TEST_MODE2_CHIPEN_MASK      0x00000800

#define PMU_IRQ_MASK_SLEEP_U01      0xA10//MS/TP/USB MASK_ON
//If we need SD card wakeup in Columbus U02, we shall set MS/SD MASK OFF!
//Turn off USB wakepup : set USB&Chargdet mask ON
#define PMU_IRQ_MASK_SLEEP_U02      0xA90//0x200 for SD detection
#define PMU_IRQ_MASK_SHUTDOWN      0xEDFF//0xE5FF
#define PMU_IRQ_MASK_SLEEP_C2        0xFFFF

#define FORCE_SHUTDOWN_SIGN_ADDR    (COLUMBUS_BASE_REG_RIU_UA+0x6940) //(0xA4006940)

#define FORCE_SHUTDOWN_SIGN         (0xFF)
#define FORCE_SOFTWARE_RESET_SIGN   (0x88)
#define SUSPEND_CHARGING_SIGN       (0x0F)
#define NON_SUSPEND_CHARGING_SIGN   (0xF0)

#define OFF_CHARGING_SIGN           (0x00FF)
#define POWER_ON_SIGN               (0xFF00)
#define NON_OFF_CHARGING_SIGN       (0x0FF0)
#define POWER_RESET_SIGN       (0x0000)
#define POWER_NO_RESET_SIGN       (0xAAAA)

//------------------------------------------------------------------------------
//  Export Functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#if __cplusplus
}
#endif//__cplusplus

#endif  //__COLUMBUS_PMU_H
