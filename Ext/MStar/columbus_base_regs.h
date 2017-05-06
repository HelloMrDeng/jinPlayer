// $Change: 398433 $
//------------------------------------------------------------------------------
//
// Copyright (c) 2008 MStar Semiconductor, Inc.  All rights reserved.
//
//------------------------------------------------------------------------------
// FILE
//      columbus_base_reg.h
//
// DESCRIPTION
//      This header file defines the Physical Addresses (PA) of the base
//      registers for the System on Chip (SoC) components.
//
// HISTORY
//      2008.1.7        Alec Lin      Initial Version
//      2008.6.10       Alec Lin      Add I2C register base address
//      2008.6.12       Bryan Fan     Add PMU and PWM definition
//      2008.6.12       Bryan Fan     Revised for code review
//      2008.6.23       Calvin Hung   Add UART0/1/2 register base address
//      2008.6.30       Alec Lin      Code refine for new coding convention
//      2008.7.9        Alec Lin      Add I2C/SharedReg base address
//      2008.7.31       Baker Chang   Add USB base address
//      2009.2.4        Chun Fan      64KB MPIF DMA RAM MAP change address
//      2009.06.26      Hata Tang     Add JPD base address
//
//------------------------------------------------------------------------------

#ifndef __COLUMBUS_BASE_REG_H
#define __COLUMBUS_BASE_REG_H

#if __cplusplus
extern "C" {
#endif  //__cplusplus

//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_RIU_PA / COLUMBUS_BASE_REG_RIU_CA / COLUMBUS_BASE_REG_RIU_UA
//
//  Locates the RIU register base.
//
#ifdef RAM_64M
#define COLUMBUS_BASE_REG_RIU_PA              			(0xA0000000)
#define COLUMBUS_BASE_REG_RIU_CA              			(0x84000000)
#define COLUMBUS_BASE_REG_RIU_UA              			(0xA4000000)
#endif
#ifdef RAM_128M
#define COLUMBUS_BASE_REG_RIU_PA              			(0xA0000000)
#define COLUMBUS_BASE_REG_RIU_CA              			(0x88000000)
#define COLUMBUS_BASE_REG_RIU_UA              			(0xA8000000)
#endif


//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_ARMPFET_PA
//
//
//
#define COLUMBUS_BASE_REG_ARMPFET_PA              (0xA0002000)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_OP2DISPMISC_PA
//
//  Locates the OP2 Disp Misc register base.
//
#define COLUMBUS_BASE_REG_OP2DISPMISC_PA              (0xA0003200)


//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_DISP_PA
//
//  Locates the Disp register base.
//
#define COLUMBUS_BASE_REG_DISP_PA                   (0xA0003200)


//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_HVSP_PA
//
//  Locates the HVSP register base.
//
#define COLUMBUS_BASE_REG_HVSP_PA                   (0xA0005C00)

//------------------------------------------------------------------------------
//  Define:  COLUMBUS_BASE_REG_IPM_PA
//
//  Locates the IPM register base.
//
#define COLUMBUS_BASE_REG_IPM_PA                    (0xA0005E00)

//------------------------------------------------------------------------------
//  Define:  COLUMBUS_BASE_REG_IPW_PA
//
//  Locates the IPW register base.
//
#define COLUMBUS_BASE_REG_IPW_PA                    (0xA0005F00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_OP2LPLL_PA
//
//  Locates the OP2 LPLL register base.
//
#define COLUMBUS_BASE_REG_OP2LPLL_PA              (0xA0003400)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_TOUCH_PA
//
//  Locates the Touch register base.
//
#define COLUMBUS_BASE_REG_TOUCH_PA              (0xA0003600)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_AUX_PA
//
//  Locates the AUX register base.
//
#define COLUMBUS_BASE_REG_AUX_PA              (0xA0003600)


//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_CPUIF_PA
//
//  Locates the CPUIF register base.
//
#define COLUMBUS_BASE_REG_CPUIF_PA              (0xA0003780)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_SC0_PA
//
//  Locates the OP2 Dith register base.
//
#define COLUMBUS_BASE_REG_SC0_PA                 (0xA0003800)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_OP2PIP_PA
//
//  Locates the OP2 PIP register base.
//
#define COLUMBUS_BASE_REG_OP2PIP_PA             (0xA0003A00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_CHIPTOP_PA
//
//  Locates the chiptop register base.
//
#define COLUMBUS_BASE_REG_CHIPTOP_PA            (0xA0003C00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_USBC_PA
//
//  Locates the usb Controller register base.
//
#define COLUMBUS_BASE_REG_USBC_PA               (0xA0004A00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_USBOTG_PA
//
//  Locates the usb OTG register base.
//
#define COLUMBUS_BASE_REG_USBOTG_PA             (0xA0004C00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_USBUTMI_PA
//
//  Locates the usb UTMI register base.
//
#define COLUMBUS_BASE_REG_USBUTMI_PA            (0xA0003E00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_UHC_PA
//
//  Locates the usb UHC register base.
//
#define COLUMBUS_BASE_REG_UHC_PA                (0xA0005400)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_INTR_PA
//
//  Locates the interrupt controller register base.
//
#define COLUMBUS_BASE_REG_INTR_PA               (0xA0005600)
#define COLUMBUS_BASE_REG_INTR1_PA              (0xA0007100)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_IPM_PA
//
//  Locates the ipm register base.
//
#define COLUMBUS_BASE_REG_IPM_PA            (0xA0005E00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_IPM2_PA
//
//  Locates the ipm2 register base.
//
#define COLUMBUS_BASE_REG_IPM2_PA            (0xA0005E80)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_IPW_PA
//
//  Locates the ipw register base.
//
#define COLUMBUS_BASE_REG_IPW_PA            (0xA0005F00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_VIP_PA
//
//  Locates the vip register base.
//
#define COLUMBUS_BASE_REG_VIP_PA            (0xA0006000)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_DLC_PA
//
//  Locates the dlc register base.
//
#define COLUMBUS_BASE_REG_DLC_PA            (0xA0006200)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_TCON_PA
//
//  Locates the Tcon register base.
//
#define COLUMBUS_BASE_REG_TCON_PA            (0xA0006400)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_MBX_PA
//
//  Locates the mailbox register base.
//
#define COLUMBUS_BASE_REG_MBX_PA            (0xA0006600)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_SEMAPH_PA
//
//  Locates the semaphore register base.
//
#define COLUMBUS_BASE_REG_SEMAPH_PA            (0xA0006800)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_SCVOP_PA
//
//  Locates the Scalar VOP register base.
//
#define COLUMBUS_BASE_REG_SCVOP_PA              (0xA0006200)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_I2C_PA
//
//  Locates the I2C register base.
//
#define COLUMBUS_BASE_REG_I2C_PA                (0xA0006840)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_RTC_PA
//
//  Locates the RTC register base.
//
#define COLUMBUS_BASE_REG_RTC_PA                (0xA0004800)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_GOP_PA
//
//  Locates the GOP register base.
//
#define COLUMBUS_BASE_REG_GOP_PA                (0xA0006A00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_GE_PA
//
//  Locates the GE register base.
//
#define COLUMBUS_BASE_REG_GE_PA                    (0xA0006C00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_JPD_PA
//
//  Locates the JPD register base.
//
#define COLUMBUS_BASE_REG_JPD_PA                (0xA0006E00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_SARADC
//
//  Locates the SAR ADC register base.
//
#define COLUMBUS_BASE_REG_SARADC_PA             (0xA0007400)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_UART0~2_PA
//
//  Locates the uart 0~2 register base.
//
#define COLUMBUS_BASE_REG_UART0_PA              (0xA0007600)
#define COLUMBUS_BASE_REG_UART1_PA              (0xA0007680)
#define COLUMBUS_BASE_REG_UART2_PA              (0xA0007700)
#define COLUMBUS_BASE_REG_UR2DMA_PA             (0xA0007A00)
#define COLUMBUS_UART_ENABLE_REG_PA             (COLUMBUS_BASE_REG_CHIPTOP_PA+(8<<2))

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_WATCHDOG_PA
//
//  Locates the watchdog register base.
//
#define COLUMBUS_BASE_REG_WATCHDOG_PA           (0xA0006980)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_TIMER_PA
//
//  Locates the timer register base.
//
#define COLUMBUS_BASE_REG_TIMER0_PA              (0xA00069C0)
#define COLUMBUS_BASE_REG_TIMER1_PA              (0xA0007780)
#define COLUMBUS_BASE_REG_TIMER2_PA              (0xA00077C0)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_G3D_PA
//
//  Locates the G3D register base.
//
#define COLUMBUS_BASE_REG_G3D_PA                (0xA0007C00)

//-----------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_PMU_PA
//
//  Locates the PMU register base.
//
#define COLUMBUS_BASE_REG_PMU_PA                (0x1F007E00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_PWM_PA
//
//  Locates the PWM register base.
//
#define COLUMBUS_BASE_REG_PWM_PA                (0xA0006880)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_MPIF_PA
//
//  Locates the MPIF register base.
//
#define COLUMBUS_BASE_REG_MPIF_PA                (0xA0000C00)
#define COLUMBUS_BASE_DMA_MPIF_PA                (0x20086000)   // 2009/0204 Chun 64KB MPIF DMA supported from V1.0015
#define COLUMBUS_BASE_REG_UTMI_PA                (0xA0003E00)   // temporary definition for opening the USB MPLL

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_ARM_PA
//
//  Locates the ARM register base.
//
#define COLUMBUS_BASE_REG_ARM_PA                 (0xA0001E00)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_REG_GPIO_PA
//
//  Locates the GPIO register base.
//
#define COLUMBUS_BASE_REG_GPIO_PA                (0xA0003D00)

//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_RMA_PA
//
//  Locates the RAM base.
//
#define COLUMBUS_BASE_RAM_PA             	     (0x20000000)
#define COLUMBUS_BASE_RAM_CA             	     (0x80000000)
#define COLUMBUS_BASE_RAM_UA             	     (0xA0000000)

//------------------------------------------------------------------------------
//
//  Define:  COLUMBUS_BASE_ITCM_PA
//
//  Locates the I/TCM base.
//
#define COLUMBUS_BASE_ITCM_PA             	     (0x00000000)
#define COLUMBUS_BASE_ITCM_CA             	     (COLUMBUS_BASE_REG_RIU_CA+0x100000)
#define COLUMBUS_BASE_ITCM_UA             	     (COLUMBUS_BASE_REG_RIU_UA+0x100000)

#if __cplusplus
}
#endif  //__cplusplus

#endif  //__COLUMBUS_BASE_REG_H

