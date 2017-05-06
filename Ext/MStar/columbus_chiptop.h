// $Change: 410639 $
//------------------------------------------------------------------------------
//
// Copyright (c) 2008 MStar Semiconductor, Inc.  All rights reserved.
//
//------------------------------------------------------------------------------
// FILE
//      columbus_chiptop.h
//
// DESCRIPTION
//      Define the chiptop registers
//
// HISTORY
//      2008.7.23       Bryan Fan      Initial Version
//
//------------------------------------------------------------------------------

#ifndef __COLUMBUS_CHIPTOP_H
#define __COLUMBUS_CHIPTOP_H

#if __cplusplus
extern "C" {
#endif  //__cplusplus


//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------
#include "columbus_typedefs.h"

//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------
#define UART0_CLK_EN        (1<<11)
#define UART1_CLK_EN        (1<<12)
#define UART2_CLK_EN        (1<<13)

#define UART0_CLK_54MHZ     (0x4<<2)
#define UART1_CLK_54MHZ     (0x4<<7)
#define UART2_CLK_54MHZ     (0x4<<12)

#define UART0_PAD_EN        (1<<13)
#define UART1_PAD_EN        (1<<14)
#define UART2_PAD_EN        (1<<15)

#define GE_CLK_SRC_173 (0<<2)
#define GE_CLK_SRC_144 (1<<2)
#define GE_CLK_SRC_108 (2<<2)
#define GE_CLK_SRC_86_5 (3<<2)

#define OP2_EN 0x4220


//------------------------------------------------------------------------------
// LPLL
//------------------------------------------------------------------------------
#define CKG_LPLL_216    (0x00<<14)
#define CKG_LPLL_108    (0x01<<14)
#define CKG_LPLL_SEL    (CKG_LPLL_216)

#define CKG_LPLL_216_XTAL_24M   (216)
#define CKG_LPLL_108_XTAL_24M   (108)
#define CKG_LPLL_216_XTAL_16M   (220)
#define CKG_LPLL_108_XTAL_16M   (110)

#if(CKG_LPLL_SEL == CKG_LPLL_216)
#define CKG_LPLL_XTAL_24M   (CKG_LPLL_216_XTAL_24M)
#define CKG_LPLL_XTAL_16M   (CKG_LPLL_216_XTAL_16M)
#else
#define CKG_LPLL_XTAL_24M   (CKG_LPLL_108_XTAL_24M)
#define CKG_LPLL_XTAL_16M   (CKG_LPLL_108_XTAL_16M)
#endif

//------------------------------------------------------------------------------
// ODCLK
//------------------------------------------------------------------------------
#define CKG_ODCLK_SRC_TMP0 (0x10<<8)
#define CKG_ODCLK_SRC_TMP1 (0x14<<8)
#define CKG_ODCLK_SRC_TMP2 (0x18<<8)
#define CKG_ODCLK_SRC_SCALAR (0x1C<<8)

//------------------------------------------------------------------------------
// FIODCLK
//------------------------------------------------------------------------------
#define FIODCLK_FCLK  (0x00)
#define FIODCLK_IDCLK (0x40)
#define FIODCLK_LCDC  (0x80)


#define PWR_SWITCH_EN_LCD 0x0001

#define COLUMBUS_U01 (0x0<<8)
#define COLUMBUS_U02 (0x1<<8)

#define JPD_CLK_10MHZ   (0<<2)
#define JPD_CLK_20MHZ   (1<<2)
#define JPD_CLK_30MHZ   (2<<2)
#define JPD_CLK_40MHZ   (3<<2)
#define JPD_CLK_60MHZ   (4<<2)
#define JPD_CLK_80MHZ   (5<<2)
#define JPD_CLK_108MHZ  (6<<2)
#define JPD_CLK_120MHZ  (7<<2)

//------------------------------------------------------------------------------
//  Data Types
//------------------------------------------------------------------------------
//for C2 chip top regisetr

// renable later
#define  C2_chip_top_register  1

typedef struct
{
    U16 u16REG_CONFIG_OVERWRITE;
    U16 u16REG_RESERVED_00;

    U16 u16REG_DT_C_DRV;
    U16 u16REG_RESERVED_01;

    U16 u16REG_DT_R_DRV;
    U16 u16REG_RESERVED_02;

    U16 u16REG_DT_G_DRV;
    U16 u16REG_RESERVED_03;

    U16 u16REG_DT_B_DRV;
    U16 u16REG_RESERVED_04;

    U16 u16REG_EFUSE;
    U16 u16REG_RESERVED_05;

    U16 u16REG_TOP_SW_RST;
    U16 u16REG_RESERVED_06;

    U16 u16REG_RESET_CPU0;
    U16 u16REG_RESERVED_07;

    U16 u16REG_IP_EN1;
    U16 u16REG_RESERVED_08;

    U16 u16REG_IP_EN2;
    U16 u16REG_RESERVED_09;

    U16 u16REG_IP_EN3;
    U16 u16REG_RESERVED_0A;

    U16 u16REG_TRAP_STS;
    U16 u16REG_RESERVED_0B;

    U16 u16REG_TRAP_OV;
    U16 u16REG_RESERVED_0C;

    U16 u16REG_TRAP_OVEN;
    U16 u16REG_RESERVED_0D;

    U16 u16REG_IR_EN;
    U16 u16REG_RESERVED_0E;

    U16 u16REG_RIU;
    U16 u16REG_RESERVED_0F;

    U16 u16REG_MCU_RST;
    U16 u16REG_RESERVED_10;

    U16 u16REG_SW_MCU_CLK;
    U16 u16REG_RESERVED_11;

    U32 u32REG_RESERVED_12;
    U32 u32REG_RESERVED_13;
    U32 u32REG_RESERVED_14;
    U32 u32REG_RESERVED_15;
    U32 u32REG_RESERVED_16;
    U32 u32REG_RESERVED_17;
    U32 u32REG_RESERVED_18;
    U32 u32REG_RESERVED_19;

    U16 u16REG_CKG_FCIE_CKGEN;
    U16 u16REG_RESERVED_1A;

    U16 u16REG_CKG_ODCLK;
    U16 u16REG_RESERVED_1B;

    U16 u16REG_CKG_IDCLK_FMCLK_LPLL;
    U16 u16REG_RESERVED_1C;

    U16 u16REG_CKG_FICLK_FCLK_SDM;
    U16 u16REG_RESERVED_1D;

    U16 u16REG_CKG_FM_SAR_TPL_ARM;
    U16 u16REG_RESERVED_1E;

    U16 u16REG_CKG_CPUIF_VEDAC;
    U16 u16REG_RESERVED_1F;

    U32 u32REG_RESERVED_20;
    U32 u32REG_RESERVED_21;

    U16 u16REG_CKG_MCU;
    U16 u16REG_RESERVED_22;

    U16 u16REG_CKG_HAYDN2_MIU;
    U16 u16REG_RESERVED_23;

    U16 u16REG_CKG_JPD;
    U16 u16REG_RESERVED_24;

    U16 u16REG_CKG_FCIE_NFIE;
    U16 u16REG_RESERVED_25;

    U16 u16REG_CKG_OP;
    U16 u16REG_RESERVED_26;

    U16 u16REG_CKG_MPIF;
    U16 u16REG_RESERVED_27;

    U16 u16REG_CKG_GE;
    U16 u16REG_RESERVED_28;

    U16 u16REG_CKG_SDR_TCK;
    U16 u16REG_RESERVED_29;

    U16 u16REG_CKG_SPDIF_GOP_DAC_UARTCLK;
    U16 u16REG_RESERVED_2A;

    U16 u16REG_CKG_UART;
    U16 u16REG_RESERVED_2B;

    U16 u16REG_CKG_G3D;
    U16 u16REG_RESERVED_2C;

    U16 u16REG_CKG_OTG_UHC;
    U16 u16REG_RESERVED_2D;

    U16 u16REG_CKG_MCLK_EN;
    U16 u16REG_RESERVED_2E;

    U16 u16REG_CKG_CPUPLL_MPLL_SYN;
    U16 u16REG_RESERVED_2F;

    U16 u16REG_CKG_SDIO_CKGEN;
    U16 u16REG_RESERVED_30;

    U16 u16REG_SD20_DRV;
    U16 u16REG_RESERVED_31;

    U16 u16REG_SD25_DRV;
    U16 u16REG_RESERVED_32;

    U16 u16REG_SD2A_DRV;
    U16 u16REG_RESERVED_33;

    U16 u16REG_SD2F_DRV;
    U16 u16REG_RESERVED_34;

    U16 u16REG_SD34_DRV;
    U16 u16REG_RESERVED_35;


    U16 u16REG_SD39_DRV;
    U16 u16REG_RESERVED_36;

    U16 u16REG_SD13_DRV;
    U16 u16REG_RESERVED_37;

    U16 u16REG_SD18_DRV;
    U16 u16REG_RESERVED_38;


    U32 u32REG_RESERVED_39;
    U32 u32REG_RESERVED_3A;

    U16 u16REG_MPIF_DRV;
    U16 u16REG_RESERVED_3B;

    U16 u16REG_GPIO_C;
    U16 u16REG_RESERVED_3C;

    U16 u16REG_GPIO_C_PD;
    U16 u16REG_RESERVED_3D;

    U16 u16REG_GPIO_C_DRV;
    U16 u16REG_RESERVED_3E;

    U16 u16REG_SPI_DRV;
    U16 u16REG_RESERVED_3F;

    U16 u16REG_GPIO_UART_IN;
    U16 u16REG_RESERVED_40;

    U16 u16REG_GPIO_UART_OEN;
    U16 u16REG_RESERVED_41;

    U16 u16REG_GPIO_CCIR_IN;
    U16 u16REG_RESERVED_42;

    U16 u16REG_GPIO_CCIR_OUT;
    U16 u16REG_RESERVED_43;

    U16 u16REG_GPIO_CCIR_OEN;
    U16 u16REG_RESERVED_44;

    U16 u16REG_IIS_INOUT_EN;
    U16 u16REG_RESERVED_45;

    U16 u16REG_GPIO_MPIF_INOUT;
    U16 u16REG_RESERVED_46;

    U16 u16REG_GPIO_MPIF_OEN;
    U16 u16REG_RESERVED_47;

    U16 u16REG_GPIO_SD_IN;
    U16 u16REG_RESERVED_48;

    U16 u16REG_GPIO_SD_OUT;
    U16 u16REG_RESERVED_49;

    U16 u16REG_GPIO_SD_OEN;
    U16 u16REG_RESERVED_4A;

    U16 u16REG_GPIO_GPS_IN;
    U16 u16REG_RESERVED_4B;

    U16 u16REG_GPIO_GPS_OEN;
    U16 u16REG_RESERVED_4C;

    U16 u16REG_GPIO_NF_IN;
    U16 u16REG_RESERVED_4D;

    U16 u16REG_GPIO_NF_OUT;
    U16 u16REG_RESERVED_4E;

    U16 u16REG_GPIO_NF_OEN;
    U16 u16REG_RESERVED_4F;

    U16 u16REG_GPIO_SPI_IN;
    U16 u16REG_RESERVED_50;

    U16 u16REG_GPIO_G_IN;
    U16 u16REG_RESERVED_51;

    U16 u16REG_GPIO_G_OUT;
    U16 u16REG_RESERVED_52;

    U16 u16REG_GPIO_G_OEN;
    U16 u16REG_RESERVED_53;


    U32 u32REG_RESERVED_54;
    U32 u32REG_RESERVED_55;
    U32 u32REG_RESERVED_56;

    U16 u16REG_DEV_PWREN;
    U16 u16REG_RESERVED_57;

    U16 u16REG_PWR_SWITCH_EN;
    U16 u16REG_RESERVED_58;

    U16 u16REG_32K_OUT_EN;
    U16 u16REG_RESERVED_59;

    U16 u16REG_GPIO_TTL_IN0;
    U16 u32REG_RESERVED_5A;

    U16 u16REG_GPIO_TTL_IN1;
    U16 u16REG_RESERVED_5B;


    U32 u32REG_RESERVED_5C;

    U16 u16REG_GPIO_TTL_OUT;
    U16 u16REG_RESERVED_5D;

    U32 u32REG_RESERVED_5E;

    U16 u16REG_GPIO_TTL_OEN;
    U16 u16REG_RESERVED_5F;


    U16 u16REG_BOND_OV;
    U16 u16REG_RESERVED_60;

    U16 u16REG_STAT_BOND;
    U16 u16REG_RESERVED_61;

    U16 u16REG_BOND_AUDIO_OV;
    U16 u16REG_RESERVED_62;

    U16 u16REG_BOND_OV_KEY;
    U16 u16REG_RESERVED_63;

    U16 u16REG_ALL_PAD_IN;
    U16 u16REG_RESERVED_64;

    U16 u16REG_CHIP_CONFIG_STAT;
    U16 u16REG_RESERVED_65;

    U16 u16REG_DEVICE_ID;
    U16 u16REG_RESERVED_66;

    U16 u16REG_CHIP_VERSION_REVISION;
    U16 u16REG_RESERVED_67;

    U16 u16REG_DEBUG;
    U16 u16REG_RESERVED_68;

    U16 u16REG_SPARE0;
    U16 u16REG_RESERVED_69;

    U16 u16REG_SPARE1;
    U16 u16REG_RESERVED_6A;


    U16 u16REG_CKG_SAR;
    U16 u16REG_RESERVED_6B;

    U32 u32REG_RESERVED_6C;
    U32 u32REG_RESERVED_6D;
    U32 u32REG_RESERVED_6E;

    U16 u16REG_PMU_CKGEN;
    U16 u16REG_RESERVED_6F;

    U16 u16REG_ROSC_OUT_SEL;
    U16 u16REG_RESERVED_70;

    U32 u32REG_RESERVED_71;

    U16 u16REG_CKG_ALLDFT;
    U16 u16REG_RESERVED_72;

    U32 u32REG_RESERVED_73;
    U32 u32REG_RESERVED_74;

    U16 u16REG_CLK_OUT_SEL;
    U16 u16REG_RESERVED_75;

    U16 u16REG_SINGLE_CLK_OUT_SEL;
    U16 u16REG_RESERVED_76;

    U16 u16REG_TEST_BUS24B_SEL;
    U16 u16REG_RESERVED_77;

    U16 u16REG_TEST_PLL;
    U16 u16REG_RESERVED_78;

    U16 u16REG_TSTCLK;
    U16 u16REG_RESERVED_79;

    U16 u16REG_IIC_DEST_SEL;
    U16 u16REG_RESERVED_7A;

    U16 u16REG_DTON_MODE;
    U16 u16REG_RESERVED_7B;

    U16 u16REG_MOD_SELVCM;
    U16 u16REG_RESERVED_7C;

    U16 u16REG_TEST_OUT_SEL;
    U16 u16REG_RESERVED_7D;

    U16 u16REG_DIGMIC_MODE;
    U16 u16REG_RESERVED_7E;

    U16 u16REG_PWM_MODE;
    U16 u16REG_RESERVED_7F;
} REG_CHIPTOP_st, *PREG_CHIPTOP_st;


#if __cplusplus
}
#endif  //__cplusplus

#endif  //__COLUMBUS_CHIPTOP_H
