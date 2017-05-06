// $Change: 403439 $
//-----------------------------------------------------------------------------
//
// Copyright (c) Mstar Semiconductor Inc. All rights reserved.
//
//-----------------------------------------------------------------------------
// FILE
//      columbus_mpif.h
//
// DESCRIPTION
//      Define the MPIF registers
//
// HISTORY
//      2008.5.9        CC Wen        Initial Version
//      2008.10.30      CC Wen        Merge the definition in mpif.h
//      2009.01.08      Chun.Fan      add Columbus U02 DMA_RX_DONE flag code
//      2009.02.13      Chun.Fan      add get GET_MPIF_DMA_PA_ADDR ioctl
//      2009.02.17      Chun.Fan      add get GET_REG_ARM_PA_ADDR ioctl
//                                    add get GET_MPIF_DMA_OFFSET ioctl
//      2009.02.23      Chun.Fan      modify PIF API for Hydra only
//      2009.08.21      Chun.Fan      add Declaration Description
//-----------------------------------------------------------------------------

#ifndef __COLUMBUS_MPIF_H
#define __COLUMBUS_MPIF_H

#include "ceddk.h"

#if __cplusplus
extern "C" {
#endif  //__cplusplus

/* for new General API */
#define MIU_MODE_MAX_DATA_BYTES_3A 128*1024    //64k bytes are verified
#define MIU_MODE_MAX_DATA_BYTES_4A 64                //HW limit
#define MAX_RX_LIMIT 0x3


typedef struct {
    U32 REG_LC1A_SETTINGS;                   // h00
    U32 REG_LC2A_REG_CONTROL;                // h01
    U32 REG_LC2A_REG_ADDRESS;                // h02
    U32 REG_LC2A_REG_DATA;                   // h03
    U32 REG_LC2B_REG_CONTROL;                // h04
    U32 REG_LC2B_REG_ADDRESS;                // h05
    U32 REG_LC2B_REG_DATA;                   // h06
    U32 REG_LC3A_PACKET_CONTROL;             // h07
    U32 REG_LC3A_PACKET_LENGTH;              // h08
    U32 REG_LC3A_PACKET_DATA[8];             // h09-h10
    U32 REG_LC3A_PACKET_MIU_BASE_ADDRESS_L;  // h11
    U32 REG_LC3A_PACKET_MIU_BASE_ADDRESS_H;  // h12
    U32 REG_LC3B_PACKET_CONTROL;             // h13
    U32 REG_LC3B_PACKET_LENGTH;              // h14
    U32 REG_LC3B_PACKET_DATA[8];             // h15-h1C
    U32 REG_LC3B_PACKET_MIU_BASE_ADDRESS_L;  // h1D
    U32 REG_LC3B_PACKET_MIU_BASE_ADDRESS_H;  // h1E
    U32 REG_LC4A_STREAM_CONTROL;             // h1F
    U32 REG_LC4A_STREAM_LENGTH;              // h20
    U32 REG_LC4A_STREAM_MIU_BASE_ADDRESS_L;  // h21
    U32 REG_LC4A_STREAM_MIU_BASE_ADDRESS_H;  // h22
    U32 REG_4WIRE_SPI_CONTROL;               // h23
    U32 REG_3WIRE_SPI_CONTROL;               // h24
    U32 REG_SPI_CONTROL;                     // h25
    U32 REG_SPI_COMMAND_VALUE_L;             // h26
    U32 REG_SPI_COMMAND_VALUE_H;             // h27
    U32 REG_SPI_DATA_LENGTH;                 // h28
    U32 REG_SPI_MIU_BASE_ADDRESS_L;          // h29
    U32 REG_SPI_MIU_BASE_ADDRESS_H;          // h2A
    U32 REG_CHANNEL_BUSY_STATUS;             // h2B
    U32 REG_INTERRUPT_EVENT_MASK;            // h2C
    U32 REG_INTERRUPT_EVENT_STATUS;          // h2D
    U32 REG_BUSY_TIMEOUT_COUNTER;            // h2E
    U32 REG_BUSY_TIMEOUT_TO;                 // h2F
    U32 REG_MISC1;                           // h30
    U32 REG_MISC2;                           // h31
    U32 REG_MISC3;                           // h32
    U32 REG_SPI_SYNC_CONTROL;                // h33
    U32 REG_SPI_UCPLT_LENGTH;                // h34
    U32 REG_MIU_WRITE_PROTECT_CONTROL;       // h35
    U32 REG_MIU_WRITE_PROTECT_ADDR_LO_L;     // h36
    U32 REG_MIU_WRITE_PROTECT_ADDR_LO_H;     // h37
    U32 REG_MIU_WRITE_PROTECT_ADDR_HI_L;     // h38
    U32 REG_MIU_WRITE_PROTECT_ADDR_HI_H;     // h39
    U32 REG_COMMAND_WITH_NEGOTIATION;        // h3A
    U32 REG_THROTTLE_CONTROL;                // h3B
    U32 REG_RESERVED_3C;                     // h3C
    U32 REG_RESERVED_3D;                     // h3D
    U32 REG_SPARE_L;                         // h3E
    U32 REG_SPARE_H;                         // h3F
    U32 REG_DEBUG_SEL;                       // h40
    U32 REG_DEBUG_CHECKSUM_L;                // h41
    U32 REG_DEBUG_CHECKSUM_H;                // h42
    U32 REG_DEBUG_STS_IF_CTRL0;              // h43
    U32 REG_DEBUG_STS_IF_CTRL1;              // h44
    U32 REG_DEBUG_STS_PKT3A_CNT;             // h45
    U32 REG_DEBUG_STS_PKT3B_CNT;             // h46
    U32 REG_DEBUG_STS_STM0_CNT;              // h47
    U32 REG_DEBUG_STS_LC2X_CNT;              // h48
    U32 REG_DEBUG_STS_LC3X_CNT;              // h49
    U32 REG_DEBUG_STS_LC4X_CNT;              // h4A
    U32 REG_DEBUG_M2I_BIST_FAIL;             // h4B
    U32 REG_DEBUG_MIU_MASK_RD;               // h4C
    U32 REG_DEBUG_MPIF2MI_SEL_BIT;           // h4D
} REG_MPIF_st, *PREG_MPIF_st;


typedef struct {
    U32 REG_CHIPTOP_00;
    U32 REG_CHIPTOP_01;
    U32 REG_CHIPTOP_02;
    U32 REG_CHIPTOP_03;
    U32 REG_CHIPTOP_04;
    U32 REG_CHIPTOP_05;
    U32 REG_CHIPTOP_06;
    U32 REG_CHIPTOP_07;
    U32 REG_CHIPTOP_08;
    U32 REG_CHIPTOP_09;
    U32 REG_CHIPTOP_0A;
    U32 REG_CHIPTOP_0B;
    U32 REG_CHIPTOP_0C;
    U32 REG_CHIPTOP_0D;
    U32 REG_CHIPTOP_0E;
    U32 REG_CHIPTOP_0F;
    U32 REG_CHIPTOP_10;
    U32 REG_CHIPTOP_11;
    U32 REG_CHIPTOP_12;
    U32 REG_CHIPTOP_13;
    U32 REG_CHIPTOP_14;
    U32 REG_CHIPTOP_15;
    U32 REG_CHIPTOP_16;
    U32 REG_CHIPTOP_17;
    U32 REG_CHIPTOP_18;
    U32 REG_CHIPTOP_19;
    U32 REG_CHIPTOP_1A;
    U32 REG_CHIPTOP_1B;
    U32 REG_CHIPTOP_1C;
    U32 REG_CHIPTOP_1D;
    U32 REG_CHIPTOP_1E;
    U32 REG_CHIPTOP_1F;
    U32 REG_CHIPTOP_20;
    U32 REG_CHIPTOP_21;
    U32 REG_CHIPTOP_22;
    U32 REG_CHIPTOP_23;
    U32 REG_CHIPTOP_24;
    U32 REG_CHIPTOP_25;
    U32 REG_CHIPTOP_26;
    U32 REG_CHIPTOP_27;
    U32 REG_CHIPTOP_28;
    U32 REG_CHIPTOP_29;
    U32 REG_CHIPTOP_2A;
    U32 REG_CHIPTOP_2B;
    U32 REG_CHIPTOP_2C;
    U32 REG_CHIPTOP_2D;
    U32 REG_CHIPTOP_2E;
    U32 REG_CHIPTOP_2F;
    U32 REG_CHIPTOP_30;
    U32 REG_CHIPTOP_31;
    U32 REG_CHIPTOP_32;
    U32 REG_CHIPTOP_33;
    U32 REG_CHIPTOP_34;
    U32 REG_CHIPTOP_35;
    U32 REG_CHIPTOP_36;
    U32 REG_CHIPTOP_37;
    U32 REG_CHIPTOP_38;
    U32 REG_CHIPTOP_39;
    U32 REG_CHIPTOP_3A;
    U32 REG_CHIPTOP_3B;
    U32 REG_CHIPTOP_3C;
    U32 REG_CHIPTOP_3D;
    U32 REG_CHIPTOP_3E;
    U32 REG_CHIPTOP_3F;
    U32 REG_CHIPTOP_40;
    U32 REG_CHIPTOP_41;
    U32 REG_CHIPTOP_42;
    U32 REG_CHIPTOP_43;
    U32 REG_CHIPTOP_44;
    U32 REG_CHIPTOP_45;
    U32 REG_CHIPTOP_46;
    U32 REG_CHIPTOP_47;
    U32 REG_CHIPTOP_48;
    U32 REG_CHIPTOP_49;
    U32 REG_CHIPTOP_4A;
    U32 REG_CHIPTOP_4B;
    U32 REG_CHIPTOP_4C;
    U32 REG_CHIPTOP_4D;
    U32 REG_CHIPTOP_4E;
    U32 REG_CHIPTOP_4F;
    U32 REG_CHIPTOP_50;
    U32 REG_CHIPTOP_51;
    U32 REG_CHIPTOP_52;
    U32 REG_CHIPTOP_53;
    U32 REG_CHIPTOP_54;
    U32 REG_CHIPTOP_55;
    U32 REG_CHIPTOP_56;
    U32 REG_CHIPTOP_57;
    U32 REG_CHIPTOP_58;
    U32 REG_CHIPTOP_59;
    U32 REG_CHIPTOP_5A;
    U32 REG_CHIPTOP_5B;
    U32 REG_CHIPTOP_5C;
    U32 REG_CHIPTOP_5D;
    U32 REG_CHIPTOP_5E;
    U32 REG_CHIPTOP_5F;
    U32 REG_CHIPTOP_60;
    U32 REG_CHIPTOP_61;
    U32 REG_CHIPTOP_62;
    U32 REG_CHIPTOP_63;
    U32 REG_CHIPTOP_64;
    U32 REG_CHIPTOP_65;
    U32 REG_CHIPTOP_66;
    U32 REG_CHIPTOP_67;
    U32 REG_CHIPTOP_68;
    U32 REG_CHIPTOP_69;
    U32 REG_CHIPTOP_6A;
    U32 REG_CHIPTOP_6B;
    U32 REG_CHIPTOP_6C;
    U32 REG_CHIPTOP_6D;
    U32 REG_CHIPTOP_6E;
    U32 REG_CHIPTOP_6F;
    U32 REG_CHIPTOP_70;
    U32 REG_CHIPTOP_71;
    U32 REG_CHIPTOP_72;
    U32 REG_CHIPTOP_73;
    U32 REG_CHIPTOP_74;
    U32 REG_CHIPTOP_75;
    U32 REG_CHIPTOP_76;
    U32 REG_CHIPTOP_77;
    U32 REG_CHIPTOP_78;
    U32 REG_CHIPTOP_79;
    U32 REG_CHIPTOP_7A;
    U32 REG_CHIPTOP_7B;
    U32 REG_CHIPTOP_7C;
    U32 REG_CHIPTOP_7D;
    U32 REG_CHIPTOP_7E;
    U32 REG_CHIPTOP_7F;
} REG_CHIP_st, *PREG_CHIP_st;


typedef struct { // temporary definition for opening the USB MPLL
    U32 REG_UTMI_00;
    U32 REG_UTMI_01;
    U32 REG_UTMI_02;
    U32 REG_UTMI_03;
    U32 REG_UTMI_04;
} REG_UTMI_st, *PREG_UTMI_st;

 //declare in pioneer_armmcu.h, only [0x35]
typedef struct {
     U32 REG_ARM[0x40];
} REG_ARM_DUMMY_st, *PREG_ARM_DUMMY_st;


/* for new General API */
typedef union {
    U8 u8Data[0x20000];   // 128KB, the maximum data length
    U32 u32Data[0x8000];
} REG_MPIFDMA_st, *PREG_MPIFDMA_st;


typedef enum
{
    MPIF_SUCCESS = 0,
    MPIF_CHANNEL_BUSY,
    MPIF_INTERRUPT_NOT_TRIGGERED,
    MPIF_INTERRUPT_NOT_CLEAR,
    MPIF_TRANSMISSION_FAILED,
    MPIF_BUSYWAIT_SUCCESS,
    MPIF_UNKNOWN_EVENT_TYPE,
    MPIF_UNKNOWN_BUS_TYPE,
    MPIF_UNKNOWN_SLAVE_ID,
    MPIF_FETAL_ERROR,
    MPIF_INVALID_PARAMETER,
    MPIF_MMIOSPACE_ERROR,
    MPIF_OVER_64BYTES_MPIF_BUG,
    MPIF_SPI_WRITE_LAST_BYTE_ERROR,
    MPIF_UNKNOWN_ERROR,
    MPIF_UNKNOWN_MPIF_ID
} MPIF_ERROR_CODE_et;

/* for new General API */
typedef enum
{
    SPI = 0,
    PIF
} MPIF_BUS_TYPE_et;

typedef enum
{
    MPIF1 = 0,
    MPIF2,
    MPIF_NUM
} MPIF_DEVICE_ID_et;

typedef enum
{
    POLLING = 0,
    MPIF_M3_INTERRUPT
} MPIF_WAIT_TYPE_et;

typedef enum
{
    ONE_BIT = 0,
    TWO_BIT,
    FOUR_BITS
} MPIF_BIT_MODE_et;

typedef enum
{
    RIU_MODE = 0,
    MIU_MODE,
    MODE_NUM
} MPIF_RW_MODE_et;

typedef enum
{
    SLAVE_0 = 0,
    SLAVE_1,
    SLAVE_2,
    SLAVE_3,
    SLAVE_NUM
} MPIF_SLAVE_ID_et;



typedef enum
{
    INDEX_0 = 0,
    INDEX_1,
    INDEX_2,
    INDEX_3,
    INDEX_4,
    INDEX_5,
    INDEX_6,
    INDEX_7,
    INDEX_NUM
} MPIF_LC1A_INDEX_et;


typedef enum
{
    LC1A = 0,
    LC2A,
    LC2B,
    LC3A,
    LC3B,
    LC4A,
    LC_SPI,
    LC_NUM
} MPIF_LOGICAL_CHANNEL_ID_et;


typedef enum
{
    NOOP = 0,
    HYDRA_INIT,
    HYDRA_RESET,
    MPIF_RESET,
    SLAVE0_CHANGE_TO_1BIT_MODE,
    SLAVE1_CHANGE_TO_1BIT_MODE,
    SLAVE2_CHANGE_TO_1BIT_MODE,
    SLAVE3_CHANGE_TO_1BIT_MODE,
    SLAVE0_CHANGE_TO_2BIT_MODE,
    SLAVE1_CHANGE_TO_2BIT_MODE,
    SLAVE2_CHANGE_TO_2BIT_MODE,
    SLAVE3_CHANGE_TO_2BIT_MODE,
    SLAVE0_CHANGE_TO_4BIT_MODE,
    SLAVE1_CHANGE_TO_4BIT_MODE,
    SLAVE2_CHANGE_TO_4BIT_MODE,
    SLAVE3_CHANGE_TO_4BIT_MODE,
    SLAVE0_CHANGE_TO_8BIT_MODE,
    SLAVE1_CHANGE_TO_8BIT_MODE,
    SLAVE2_CHANGE_TO_8BIT_MODE,
    SLAVE3_CHANGE_TO_8BIT_MODE,
    SET_MPIF_CLOCK_120M,
    SET_MPIF_CLOCK_108M,
    SET_MPIF_CLOCK_86_5M,
    SET_MPIF_CLOCK_72M,
    SET_MPIF_CLOCK_60M,
    SET_MPIF_CLOCK_40M,
    SET_MPIF_CLOCK_24M,
    SET_MPIF_CLOCK_12M,
    ENHANCE_DRIVING_ALL,
    GET_MPIF_DMA_PA_ADDR,
    GET_REG_ARM_PA_ADDR,
    GET_MPIF_DMA_OFFSET,
    SET_MPIF_CLOCK_27M,  //Dummy clock rate, not supported in Columbus
    SPIF_RESET,
    SLAVE_SPI_HW_RESET,
    SLAVE_CHANGE_TO_3A_MIU_MODE,
    SLAVE_CHANGE_TO_3B_MIU_MODE,
    ARGO_POWER_INIT,
    ARGO_RUN_SAVE_REG,
    ARGO_EXIT_REWRITE_REG,
} MPIF_IOCONTROL_CODE_et;


/* for new General API */
typedef enum
{
    FAST_CLK_78M = 0,
    FAST_CLK_26M,
    FAST_CLK_62M,
    FAST_CLK_52M,
    FAST_CLK_104M,
    FAST_CLK_39M,
    FAST_CLK_130M,
    FAST_CLK_DIV
} FAST_CLOCK_et;

typedef enum
{
    DIV_CLK_312M = 0,
    DIV_CLK_300M,
    DIV_CLK_156M,
    DIV_CLK_130M,
    DIV_CLK_104M,
    DIV_CLK_78M,
    DIV_CLK_62M,
    DIV_CLK_52M
} FAST_CLOCK_DIV_et;

typedef struct{
    U32 handler;
}pmstar_multi_pif_reset_st;


typedef enum
{
    FAST_MODE_DISABLE = 0,
    FAST_MODE_ENABLE
} MPIF_FAST_MODE_et;


typedef enum
{
    PAGE_SIZE_0 = 0,    // 1*256 bytes
    PAGE_SIZE_1,        // 2*256 bytes
    PAGE_SIZE_2,        // 3*256 bytes
    PAGE_SIZE_3            // 4*256 bytes
} MPIF_LC4A_PAGE_SIZE_et;


typedef enum
{
    SPI4W = 0,
    SPI3W
} MPIF_SPI_WIRES_et;


typedef enum
{
    CLOCK_PHASE_LOW = 0,
    CLOCK_PHASE_HIGH
} SPI_CLOCK_PHASE_et;


typedef enum
{
    CLOCK_POLARITY_LOW = 0,
    CLOCK_POLARITY_HIGH
} SPI_CLOCK_POLARITY_et;


typedef enum
{
    CHANNEL_FREE = 0,
    INTERRUPT_ACTIVE,
    INTERRUPT_CLEAR,
} EVENT_TYPE_et;


typedef struct
{
    BOOL bCheckSum;
    BOOL bReTX;
    U8 u8ReTXTimes;
    U8 u8TurnAroundCycle;
    U8 u8WaitCycle;
    MPIF_RW_MODE_et eMode;      /* for new General API, reservered for old API */
    MPIF_FAST_MODE_et eFastMode;
    U8 u8MaxWait;
    U32 u32MIUAddr;
    U8 u8PageSize;
    U32 u32DMAGuardingTime;
} MPIF_OPTIONS_st, *pMPIF_OPTIONS_st; // Note that some settings in MPIF_OPTIONS_st may be meaningless in different PIF channel modes.


typedef struct
{
    MPIF_SPI_WIRES_et eWire;
    SPI_CLOCK_PHASE_et ePhase;
    SPI_CLOCK_POLARITY_et ePolarity;
    U8 u8LeadingCycle;
    U8 u8TailingCycle;
    U8 u8TurnAroundCycle;
    U8 u8WaitCycle;
    MPIF_RW_MODE_et eMode;        /* for new General API, reservered for old API */
    U32 u32MIUAddr;
    U32 u32DMAGuardingTime; // Note that the transmission done interrupt doesn't mean the data from SPIF is ready in MIU. Now, we use a guarding time to prevent the CPU to get the data too early.
    U32 u32SpiSlaveId;
} SPI_OPTIONS_st, *pSPI_OPTIONS_st;


typedef struct{
    U8  g_DMABuffer[MIU_MODE_MAX_DATA_BYTES_3A]; //[0x100];
    U8 Flag;
    MPIF_DEVICE_ID_et        device_id;                    //[IN] MPIF1, MPIF2
    MPIF_SLAVE_ID_et         slave_id;                    //[IN] SLAVE_0, SLAVE_1, SLAVE_2, SLAVE_3
    MPIF_WAIT_TYPE_et        wait_type;                    //[IN] POLLING, MPIF_M3_INTERRUPT
    void                    (*pservice_routine)(U32 status);            //[IN] For wait_type is MPIF_M3_INTERRUPT, if the transmission done, call the service_routine
    //MPIF_BIT_MODE_et        bit_mode;                    //[IN] ONE_BIT, TWO_BIT, FOUR_BITS
    MPIF_OPTIONS_st            sOption;                    //[IN] sOption for 3A/4A setting.
    SPI_OPTIONS_st             SPI_sOption;                //[IN] sOption for SPI setting.
    U8 *DMAAddr;
    BOOL int_valid;
}MPIF_DB_st;

typedef struct{
    MPIF_DEVICE_ID_et        device_id;                    //[IN] MPIF1, MPIF2
    MPIF_SLAVE_ID_et         slave_id;                    //[IN] SLAVE_0, SLAVE_1, SLAVE_2, SLAVE_3
    MPIF_WAIT_TYPE_et        wait_type;                    //[IN] POLLING, MPIF_M3_INTERRUPT
    void                    (*pservice_routine)(U32 status);            //[IN] For wait_type is MPIF_M3_INTERRUPT, if the transmission done, call the service_routine
    //MPIF_BIT_MODE_et        bit_mode;                    //[IN] ONE_BIT, TWO_BIT, FOUR_BITS
    MPIF_OPTIONS_st            sOption;                    //[IN] sOption for 3A/4A setting.
    U32                    DMA_buffer_size;
    U32                    handler;                                    //[OUT]
    U8                    *pput_data_buffer;                //[OUT]
    //U32                    put_max_length;                        //[OUT]
}pmstar_multi_pif_register_st;

typedef struct{
    U32 handler;
    void *pget_data_buffer;
    U32 get_length;
    MPIF_LOGICAL_CHANNEL_ID_et eChannel;
    MPIF_RW_MODE_et mode;
    U16 slave_address;
    U32 cmd;
    U8 cmdlength;
}pmstar_multi_pif_get_st;

typedef struct{
    U32 handler;
    void *pput_data_buffer;
    U32 put_length;
    MPIF_LOGICAL_CHANNEL_ID_et eChannel;
    MPIF_RW_MODE_et mode;
    U16 slave_address;
    U32 cmd;
    U8 cmdlength;
}pmstar_multi_pif_put_st;

typedef struct {
    U32 handler;
    SPI_OPTIONS_st sOption;
} MPIF_CONFIG_SPI_INFO;

typedef struct {
    U32 handler;
    MPIF_OPTIONS_st sOption;
} MPIF_CONFIG_PIF_INFO;

typedef struct {
    U32 handler;
    MPIF_IOCONTROL_CODE_et byIoControlCode;
} MPIF_CONTROL_INFO;

//------------------------------------------------------------------------------
// Macro definition
//------------------------------------------------------------------------------

// MPIF global part
#define MCR_MPIF_PAD_ENABLE_ALL()   \
 SETREG32( &g_pCHIPRegs->REG_CHIPTOP_09, 0x03FC )

#if 1//defined(NEW_MPIF_API) || defined(BSP_MPIF_IOCTL)     /* New_MPIF_API */
// MPIF part - LC1A
#define MCR_MPIF_LC1A_SETTINGS_VALID(mpid)    \
        SETREG32( &g_pMPIFRegs[mpid]->REG_LC1A_SETTINGS, BIT00 )
#define MCR_MPIF_LC1A_SETTINGS_READ(mpid)    \
        CLRREG32( &g_pMPIFRegs[mpid]->REG_LC1A_SETTINGS, BIT01 )
#define MCR_MPIF_LC1A_SETTINGS_WRITE(mpid)    \
        SETREG32( &g_pMPIFRegs[mpid]->REG_LC1A_SETTINGS, BIT01 )
#define MCR_MPIF_LC1A_SETTINGS_SLAVEID(x, mpid)    \
        CLRREG32( &g_pMPIFRegs[mpid]->REG_LC1A_SETTINGS, ( 0x03 << 2 ) ); \
        SETREG32( &g_pMPIFRegs[mpid]->REG_LC1A_SETTINGS, ( (x&0x03) << 2 ) )
#define MCR_MPIF_LC1A_SETTINGS_INDEX(x, mpid)    \
        CLRREG32( &g_pMPIFRegs[mpid]->REG_LC1A_SETTINGS, ( 0x07 << 4 ) );    \
        SETREG32( &g_pMPIFRegs[mpid]->REG_LC1A_SETTINGS, ( (x&0x07) << 4 ) )
#define MCR_MPIF_LC1A_SETTINGS_RDATA(mpid)    \
        ((INREG32( &g_pMPIFRegs[mpid]->REG_LC1A_SETTINGS ) >> 8 ) & 0xFF )
#define MCR_MPIF_LC1A_SETTINGS_WDATA(x, mpid)    \
        CLRREG32( &g_pMPIFRegs[mpid]->REG_LC1A_SETTINGS, ( 0xFF << 8 ) );    \
        SETREG32( &g_pMPIFRegs[mpid]->REG_LC1A_SETTINGS, ( (x&0xFF) << 8 ) )


// MPIF part - LC2A
#define MCR_MPIF_LC2A_REG_CONTROL_VALID(mpid)    \
        SETREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_CONTROL, BIT00 )
#define MCR_MPIF_LC2A_REG_CONTROL_READ(mpid)     \
        CLRREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_CONTROL, BIT01 )
#define MCR_MPIF_LC2A_REG_CONTROL_WRITE(mpid)    \
        SETREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_CONTROL, BIT01 )
#define MCR_MPIF_LC2A_REG_CONTROL_SLAVEID(x, mpid)  \
        CLRREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_CONTROL, ( 0x03 << 2 ) );    \
        SETREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_CONTROL, ( (x & 0x03) << 2 ) )
#define MCR_MPIF_LC2A_REG_CONTROL_CKSUM_ENABLE(mpid)    \
        SETREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_CONTROL, BIT04 )
#define MCR_MPIF_LC2A_REG_CONTROL_CKSUM_DISABLE(mpid)   \
        CLRREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_CONTROL, BIT04 )
#define MCR_MPIF_LC2A_REG_CONTROL_MAX_RTX(x, mpid)  \
        CLRREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_CONTROL, ( MAX_RX_LIMIT << 6 ) );    \
        SETREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_CONTROL, ( (x & MAX_RX_LIMIT) << 6 ) )
#define MCR_MPIF_LC2A_REG_ADDRESS_SET(addr, mpid)   \
        OUTREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_ADDRESS, ( addr ) )
#define MCR_MPIF_LC2A_REG_DATA_RDATA(mpid)  \
        ( INREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_DATA ) )
#define MCR_MPIF_LC2A_REG_DATA_WDATA(data, mpid)    \
        OUTREG32( &g_pMPIFRegs[mpid]->REG_LC2A_REG_DATA, ( data ) )


// MPIF part - LC2B
#define MCR_MPIF_LC2B_REG_CONTROL_VALID(mpid)   \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_CONTROL, BIT00 )
#define MCR_MPIF_LC2B_REG_CONTROL_READ(mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_CONTROL, BIT01 )
#define MCR_MPIF_LC2B_REG_CONTROL_WRITE(mpid)   \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_CONTROL, BIT01 )
#define MCR_MPIF_LC2B_REG_CONTROL_SLAVEID(x, mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_CONTROL, ( 0x03 << 2 ) );    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_CONTROL, ( (x&0x03) << 2 ) )
#define MCR_MPIF_LC2B_REG_CONTROL_CKSUM_ENABLE(mpid)    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_CONTROL, BIT04 )
#define MCR_MPIF_LC2B_REG_CONTROL_CKSUM_DISABLE(mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_CONTROL, BIT04 )
#define MCR_MPIF_LC2B_REG_CONTROL_MAX_RTX(x, mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_CONTROL, ( MAX_RX_LIMIT << 6 ) );    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_CONTROL, ( (x & MAX_RX_LIMIT) << 6 ) )
#define MCR_MPIF_LC2B_REG_ADDRESS_SET(addr, mpid)   \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_ADDRESS, ( addr ) )
#define MCR_MPIF_LC2B_REG_DATA_RDATA(mpid)          \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_DATA ) )
#define MCR_MPIF_LC2B_REG_DATA_WDATA(data, mpid)    \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC2B_REG_DATA, ( data ) )


// MPIF part - LC3A
#define MCR_MPIF_LC3A_PACKET_CONTROL_VALID(mpid)    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT00 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_READ(mpid) \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT01 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_WRITE(mpid)    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT01 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_SLAVEID(x, mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, ( 0x03 << 2 ) ); \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, ( (x&0x03) << 2 ) )
#define MCR_MPIF_LC3A_PACKET_CONTROL_CKSUM_ENABLE(mpid) \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT04 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_CKSUM_DISABLE(mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT04 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_RTX_ENABLE(mpid)   \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT05 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_RTX_DISABLE(mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT05 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_MAX_RTX(x, mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, ( MAX_RX_LIMIT << 6 ) ); \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, ( (x & MAX_RX_LIMIT) << 6 ) )
#define MCR_MPIF_LC3A_PACKET_CONTROL_SET_SRCDST_RIU(mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT08 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_SET_SRCDST_MIU(mpid)   \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT08 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_SET_MIUID_0(mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT09 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_SET_MIUID_1(mpid)  \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT09 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_FASTMODE_ENABLE(mpid)  \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT10 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_FASTMODE_DISABLE(mpid) \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, BIT10 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_MAX_WAIT(t, mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, ( 0x0F << 12 ) );    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_CONTROL, ( (t&0x0F) << 12 ) )

#define MCR_MPIF_LC3A_PACKET_LENGTH_SET(len, mpid)  \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_LENGTH, ( len ) )

#define MCR_MPIF_LC3A_PACKET_DATA_CLEAR(mpid)   \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[0], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[1], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[2], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[3], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[4], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[5], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[6], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[7], 0x00000000 );

#define MCR_MPIF_LC3A_PACKET_DATA_SET(mpid, val)    \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[0], val );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[1], val );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[2], val );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[3], val );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[4], val );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[5], val );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[6], val );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_DATA[7], val );


#define MCR_MPIF_LC3A_PACKET_MIU_BASE_ADDRESS_SET(addr, mpid)   \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_MIU_BASE_ADDRESS_L, ( (addr) & 0xFFFF ) );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3A_PACKET_MIU_BASE_ADDRESS_H, ( ( (addr) >> 16 ) & 0xFFFF ) )


// MPIF part - LC3B
#define MCR_MPIF_LC3B_PACKET_CONTROL_VALID(mpid)        \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT00 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_READ(mpid)         \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT01 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_WRITE(mpid)        \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT01 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_SLAVEID(x, mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, ( 0x03 << 2 ) );     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, ( (x&0x03) << 2 ) )
#define MCR_MPIF_LC3B_PACKET_CONTROL_CKSUM_ENABLE(mpid) \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT04 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_CKSUM_DISABLE(mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT04 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_RTX_ENABLE(mpid)   \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT05 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_RTX_DISABLE(mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT05 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_MAX_RTX(x, mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, ( MAX_RX_LIMIT << 6 ) ); \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, ( (x & MAX_RX_LIMIT) << 6 ) )
#define MCR_MPIF_LC3B_PACKET_CONTROL_SET_SRCDST_RIU(mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT08 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_SET_SRCDST_MIU(mpid)   \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT08 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_SET_MIUID_0(mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT09 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_SET_MIUID_1(mpid)  \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT09 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_FASTMODE_ENABLE(mpid) \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT10 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_FASTMODE_DISABLE(mpid) \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, BIT10 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_MAX_WAIT(t, mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, ( 0x0F << 12 ) );    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_CONTROL, ( (t&0x0F) << 12 ) )

#define MCR_MPIF_LC3B_PACKET_LENGTH_SET(len, mpid)  \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_LENGTH, ( len ) )

#define MCR_MPIF_LC3B_PACKET_DATA_CLEAR(mpid)   \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_DATA[0], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_DATA[1], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_DATA[2], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_DATA[3], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_DATA[4], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_DATA[5], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_DATA[6], 0x00000000 );\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_DATA[7], 0x00000000 );



#define MCR_MPIF_LC3B_PACKET_MIU_BASE_ADDRESS_SET(addr, mpid)   \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_MIU_BASE_ADDRESS_L, ( (addr) & 0xFFFF ) ); \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC3B_PACKET_MIU_BASE_ADDRESS_H, ( ( (addr) >> 16 ) & 0xFFFF ) )


// MPIF part - LC4A
#define MCR_MPIF_LC4A_STREAM_CONTROL_VALID(mpid)    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, BIT00 )
#define MCR_MPIF_LC4A_STREAM_CONTROL_READ(mpid) \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, BIT01 )
#define MCR_MPIF_LC4A_STREAM_CONTROL_WRITE(mpid)    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, BIT01 )
#define MCR_MPIF_LC4A_STREAM_CONTROL_SLAVEID(x, mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, ( 0x03 << 2 ) ); \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, ( (x&0x03) << 2 ) )
#define MCR_MPIF_LC4A_STREAM_CONTROL_MAX_RTX(x, mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, ( MAX_RX_LIMIT << 6 ) ); \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, ( (x & MAX_RX_LIMIT) << 6 ) )
#define MCR_MPIF_LC4A_STREAM_CONTROL_SET_MIUID_0(mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, BIT09 )
#define MCR_MPIF_LC4A_STREAM_CONTROL_SET_MIUID_1(mpid)  \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, BIT09 )
#define MCR_MPIF_LC4A_STREAM_PAGE_SIZE_SET(x, mpid) \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, ( 0x03 << 10 ) );    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, ( (x&0x03) << 10 ) )
#define MCR_MPIF_LC4A_STREAM_CONTROL_MAX_WAIT(t, mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, ( 0x0F << 12 ) );    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_CONTROL, ( (t&0x0F) << 12 ) )

#define MCR_MPIF_LC4A_STREAM_LENGTH_SET(len, mpid)  \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_LENGTH, ( len ) )

#define MCR_MPIF_LC4A_STREAM_MIU_BASE_ADDRESS_SET(addr, mpid)   \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_MIU_BASE_ADDRESS_L, ( addr & 0xFFFF ) );  \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_LC4A_STREAM_MIU_BASE_ADDRESS_H, ( ( addr >> 16 ) & 0xFFFF ) )

// SPI part
#define MCR_MPIF_4WIRE_SPI_CONTROL_VALID(mpid)  \
    SETREG32( &g_pMPIFRegs[mpid]->REG_4WIRE_SPI_CONTROL, BIT00 )
#define MCR_MPIF_4WIRE_SPI_CONTROL_READ(mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_4WIRE_SPI_CONTROL, BIT01 )
#define MCR_MPIF_4WIRE_SPI_CONTROL_WRITE(mpid)  \
    SETREG32( &g_pMPIFRegs[mpid]->REG_4WIRE_SPI_CONTROL, BIT01 )

#define MCR_MPIF_3WIRE_SPI_CONTROL_VALID(mpid)  \
    SETREG32( &g_pMPIFRegs[mpid]->REG_3WIRE_SPI_CONTROL, BIT00 )
#define MCR_MPIF_3WIRE_SPI_CONTROL_READ(mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_3WIRE_SPI_CONTROL, BIT01 )
#define MCR_MPIF_3WIRE_SPI_CONTROL_WRITE(mpid)  \
    SETREG32( &g_pMPIFRegs[mpid]->REG_3WIRE_SPI_CONTROL, BIT01 )

#define MCR_MPIF_SPI_CONTROL_SLAVEID(x, mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, 0x03 );\
    SETREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, ( (x)&0x03 ) )
#define MCR_MPIF_SPI_CONTROL_SET_CLOCK_PHASE_H(mpid)    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, BIT02 )
#define MCR_MPIF_SPI_CONTROL_SET_CLOCK_PHASE_L(mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, BIT02 )
#define MCR_MPIF_SPI_CONTROL_SET_CLOCK_POLARITY_H(mpid)     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, BIT03 )
#define MCR_MPIF_SPI_CONTROL_SET_CLOCK_POLARITY_L(mpid) \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, BIT03 )
#define MCR_MPIF_SPI_CONTROL_SET_LEADING(x, mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, ( 0x03 << 4 ) );     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, ( (x&0x03) << 4 ) )
#define MCR_MPIF_SPI_CONTROL_SET_TAILING(x, mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, ( 0x03 << 6 ) );     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, ( (x&0x03) << 6 ) )
#define MCR_MPIF_SPI_CONTROL_SET_CMDLEN(x, mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, ( 0x07 << 8 ) );     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, ( (x&0x07) << 8 ) )
#define MCR_MPIF_SPI_CONTROL_SET_SRCDST_RIU(mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, BIT11 )
#define MCR_MPIF_SPI_CONTROL_SET_SRCDST_MIU(mpid)   \
    SETREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, BIT11 )
#define MCR_MPIF_SPI_CONTROL_SET_MIUID_0(mpid)  \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, BIT12 )
#define MCR_MPIF_SPI_CONTROL_SET_MIUID_1(mpid)  \
    SETREG32( &g_pMPIFRegs[mpid]->REG_SPI_CONTROL, BIT12 )

#define MCR_MPIF_SPI_COMMAND_SET(cmd, mpid)     \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_SPI_COMMAND_VALUE_L, ( cmd & 0xFFFF ) );  \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_SPI_COMMAND_VALUE_H, ( ( cmd >> 16 ) & 0xFFFF ) )

#define MCR_MPIF_SPI_DATA_LENGTH_SET(len, mpid)     \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_SPI_DATA_LENGTH, ( len ) )

#define MCR_MPIF_SPI_MIU_BASE_ADDRESS_SET(addr, mpid)   \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_SPI_MIU_BASE_ADDRESS_L, ( addr & 0xFFFF ) );  \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_SPI_MIU_BASE_ADDRESS_H, ( ( addr >> 16 ) & 0xFFFF ) )

// IP-wide status
#define MCR_MPIF_CHANNEL_BUSY_STATUS_4WSPI(mpid)    \
	( INREG32( &g_pMPIFRegs[mpid]->REG_CHANNEL_BUSY_STATUS ) & BIT00 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_3WSPI(mpid)    \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_CHANNEL_BUSY_STATUS ) & BIT01 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC1A(mpid)     \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_CHANNEL_BUSY_STATUS ) & BIT02 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC2A(mpid)     \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_CHANNEL_BUSY_STATUS ) & BIT03 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC2B(mpid)     \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_CHANNEL_BUSY_STATUS ) & BIT04 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC3A(mpid)     \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_CHANNEL_BUSY_STATUS ) & BIT05 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC3B(mpid)     \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_CHANNEL_BUSY_STATUS ) & BIT06 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC4A(mpid)     \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_CHANNEL_BUSY_STATUS ) & BIT07 )

#define MCR_MPIF_INTERRUPT_EVENT_MASK_4WSPITX_ENABLE(mpid)      \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT00 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_3WSPITX_ENABLE(mpid)      \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT01 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC1ATX_ENABLE(mpid)       \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT02 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATX_ENABLE(mpid)       \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT03 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTX_ENABLE(mpid)       \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT04 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATX_ENABLE(mpid)       \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT05 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTX_ENABLE(mpid)       \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT06 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATX_ENABLE(mpid)       \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT07 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATXER_ENABLE(mpid)     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT08 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTXER_ENABLE(mpid)     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT09 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATXER_ENABLE(mpid)     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT10 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTXER_ENABLE(mpid)     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT11 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATXER_ENABLE(mpid)     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT12 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_BUSYTIMEOUT_ENABLE(mpid)  \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT13 )    // note that the #13 and #14 in the register table may be wrong, compared to the interrupt Event Status register
#define MCR_MPIF_INTERRUPT_EVENT_MASK_SLAVEREQ_ENABLE(mpid)     \
    SETREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT14 )

#define MCR_MPIF_INTERRUPT_EVENT_MASK_4WSPITX_DISABLE(mpid)     \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT00 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_3WSPITX_DISABLE(mpid)     \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT01 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC1ATX_DISABLE(mpid)      \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT02 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATX_DISABLE(mpid)      \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT03 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTX_DISABLE(mpid)      \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT04 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATX_DISABLE(mpid)      \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT05 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTX_DISABLE(mpid)      \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT06 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATX_DISABLE(mpid)      \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT07 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATXER_DISABLE(mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT08 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTXER_DISABLE(mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT09 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATXER_DISABLE(mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT10 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTXER_DISABLE(mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT11 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATXER_DISABLE(mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT12 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_BUSYTIMEOUT_DISABLE(mpid) \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT13 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_SLAVEREQ_DISABLE(mpid)    \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK, BIT14 )

#define MCR_MPIF_INTERRUPT_EVENT_MASK_4WSPITX_ISEN(mpid)        \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT00 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_3WSPITX_ISEN(mpid)        \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT01 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC1ATX_ISEN(mpid)         \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT02 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATX_ISEN(mpid)         \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT03 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTX_ISEN(mpid)         \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT04 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATX_ISEN(mpid)         \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT05 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTX_ISEN(mpid)         \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT06 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATX_ISEN(mpid)         \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT07 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATXER_ISEN(mpid)       \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT08 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTXER_ISEN(mpid)       \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT09 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATXER_ISEN(mpid)       \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT10 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTXER_ISEN(mpid)       \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT11 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATXER_ISEN(mpid)       \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT12 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_BUSYTIMEOUT_ISEN(mpid)    \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT13 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_SLAVEREQ_ISEN(mpid)       \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_MASK ) & BIT14 )

#define MCR_MPIF_INTERRUPT_EVENT_STATUS_ALL(mpid)               \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ))
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_4WSPITX_GET(mpid)       \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT00 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_3WSPITX_GET(mpid)       \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT01 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC1ATX_GET(mpid)        \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT02 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2ATX_GET(mpid)        \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT03 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2BTX_GET(mpid)        \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT04 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3ATX_GET(mpid)        \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT05 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3BTX_GET(mpid)        \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT06 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC4ATX_GET(mpid)        \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT07 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2ATXER_GET(mpid)      \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT08 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2BTXER_GET(mpid)      \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT09 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3ATXER_GET(mpid)      \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT10 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3BTXER_GET(mpid)      \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT11 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC4ATXER_GET(mpid)      \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT12 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_BUSYTIMEOUT_GET(mpid)   \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT13 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_SLAVEREQ_GET(mpid)      \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS ) & BIT14 )

#define MCR_MPIF_INTERRUPT_EVENT_STATUS_4WSPITX_CLEAR(mpid)     \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT00 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_3WSPITX_CLEAR(mpid)     \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT01 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC1ATX_CLEAR(mpid)      \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT02 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2ATX_CLEAR(mpid)      \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT03 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2BTX_CLEAR(mpid)      \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT04 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3ATX_CLEAR(mpid)      \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT05 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3BTX_CLEAR(mpid)      \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT06 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC4ATX_CLEAR(mpid)      \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT07 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_ALLTX_CLEAR(mpid)       \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, 0xFF )

#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2ATXER_CLEAR(mpid)    \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT08 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2BTXER_CLEAR(mpid)    \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT09 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3ATXER_CLEAR(mpid)    \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT10 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3BTXER_CLEAR(mpid)    \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT11 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC4ATXER_CLEAR(mpid)    \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT12 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_BUSYTIMEOUT_CLEAR(mpid)	\
    OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT13 )
//#define MCR_MPIF_INTERRUPT_EVENT_STATUS_SLAVEREQ_CLEAR(mpid)      OUTREG32( &g_pMPIFRegs[mpid]->REG_INTERRUPT_EVENT_STATUS, BIT14 )    // read-only?

#define MCR_MPIF_BUSY_TIMEOUT_COUNTER_SET(t, mpid) \
    OUTREG32( &g_pMPIFRegs[mpid]->REG_BUSY_TIMEOUT_COUNTER, ( t & 0xFFFF ) )
#define MCR_MPIF_BUSY_TIMEOUT_COUNTER_GET(t, mpid) \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_BUSY_TIMEOUT_COUNTER ) & 0xFFFF )

#define MCR_REG_MPIF_BUSY_TIMEOUT_TO_GET(mpid)      \
    ( INREG32( &g_pMPIFRegs[mpid]->REG_BUSY_TIMEOUT_TO )  )

#define MCR_MPIF_MISC1_SWRESET(mpid)          \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC1, BIT00 ); SETREG32( &g_pMPIFRegs[mpid]->REG_MISC1, BIT00 )
#define MCR_MPIF_MISC1_GATING_DISABLE(mpid)   \
    CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC1, BIT01 )
#define MCR_MPIF_MISC1_GATING_ENABLE(mpid)    \
    SETREG32( &g_pMPIFRegs[mpid]->REG_MISC1, BIT01 )
    // Set the turn-around cycle, default value is 1. If the slave doesn't need any turn-around cycle, i.e., returning the values immediately, set this value to zero
#define MCR_MPIF_MISC1_SET_TURNAROUND_CYCLE(x, mpid) \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( 0x03 << 2 ) ); \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( (x&0x03) << 2 ) )
#define MCR_MPIF_MISC1_SET_WAIT_CYCLE(x, mpid)       \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( 0x03 << 4 ) );  \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( (x&0x03) << 4 ) )
#define MCR_MPIF_MISC1_SLAVE0_DATAWIDTH(x, mpid)     \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( 0x03 << 6 ) ); \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( (x&0x03) << 6 ) )
#define MCR_MPIF_MISC1_SLAVE1_DATAWIDTH(x, mpid)     \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( 0x03 << 8 ) );\
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( (x&0x03) << 8 ) )
#define MCR_MPIF_MISC1_SLAVE2_DATAWIDTH(x, mpid)     \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( 0x03 << 10 ) ); \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( (x&0x03) << 10 ) )
#define MCR_MPIF_MISC1_SLAVE3_DATAWIDTH(x, mpid)     \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( 0x03 << 12 ) ); \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( (x&0x03) << 12 ) )
#define MCR_MPIF_MISC1_CMDPH_DATAWIDTH(x, mpid)      \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( 0x03 << 14 ) ); \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC1, ( (x&0x03) << 14 ) )

//#define MCR_MPIF_MISC2_WWTR_CYCLE(x, mpid)   CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC2, ( 0x07 ) ); SETREG32( &g_pMPIFRegs[mpid]->REG_MISC2, (x&0x07) )

    // Set the turn-around cycle, default value is 1. If the slave doesn't need any turn-around cycle, i.e., returning the values immediately, set this value to zero
#define MCR_MPIF_MISC2_SET_TURNAROUND_CYCLE(x, mpid) \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC2, ( 0x03 ) ); \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC2, ( (x&0x03)) )
#define MCR_MPIF_MISC2_TR_CYCLE_DISABLE(mpid)         \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC2, BIT03 )
#define MCR_MPIF_MISC2_TR_CYCLE_ENABLE(mpid)          \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC2, BIT03 )

#define MCR_MPIF_MISC2_SRAM_CLOCK_GATING_DISABLE(mpid)  \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MISC2, BIT04 )
#define MCR_MPIF_MISC2_SRAM_CLOCK_GATING_ENABLE(mpid)   \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC2, BIT04 )

#define MCR_MPIF_MISC2_CLEAR_LC2X_ER(mpid) \
    SETREG32( &g_pMPIFRegs[mpid]->REG_MISC2, BIT05 )
#define MCR_MPIF_MISC2_CLEAR_LC3X_ER(mpid) \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC2, BIT06 )
#define MCR_MPIF_MISC2_CLEAR_LC4A_ER(mpid) \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MISC2, BIT07 )

#define MCR_MPIF_SPI_UCPLT_LENGTH_GET(mpid)   \
	  ( INREG32( &g_pMPIFRegs[mpid]->REG_SPI_UCPLT_LENGTH ) )

#define MCR_MPIF_DUMMY_WRITE_ENABLE(mpid)     \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MIU_WRITE_PROTECT_CONTROL, BIT02);
#define MCR_MPIF_DUMMY_WRITE_DISABLE(mpid)    \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MIU_WRITE_PROTECT_CONTROL, BIT02);
#define MCR_MPIF_DUMMY_WRITE_LENGTH_2(mpid)   \
		CLRREG32( &g_pMPIFRegs[mpid]->REG_MIU_WRITE_PROTECT_CONTROL, BIT03);
#define MCR_MPIF_DUMMY_WRITE_LENGTH_4(mpid)   \
		SETREG32( &g_pMPIFRegs[mpid]->REG_MIU_WRITE_PROTECT_CONTROL, BIT03);

#else /* use Old MPIF API*/

// MPIF part - LC1A
#define MCR_MPIF_LC1A_SETTINGS_VALID()                    SETREG32( &g_pMPIFRegs->REG_LC1A_SETTINGS, BIT00 )
#define MCR_MPIF_LC1A_SETTINGS_READ()                    CLRREG32( &g_pMPIFRegs->REG_LC1A_SETTINGS, BIT01 )
#define MCR_MPIF_LC1A_SETTINGS_WRITE()                    SETREG32( &g_pMPIFRegs->REG_LC1A_SETTINGS, BIT01 )
#define MCR_MPIF_LC1A_SETTINGS_SLAVEID(x)                CLRREG32( &g_pMPIFRegs->REG_LC1A_SETTINGS, ( 0x03 << 2 ) ); SETREG32( &g_pMPIFRegs->REG_LC1A_SETTINGS, ( ((x)&0x03) << 2 ) )
#define MCR_MPIF_LC1A_SETTINGS_INDEX(x)                    CLRREG32( &g_pMPIFRegs->REG_LC1A_SETTINGS, ( 0x07 << 4 ) ); SETREG32( &g_pMPIFRegs->REG_LC1A_SETTINGS, ( ((x)&0x07) << 4 ) )
#define MCR_MPIF_LC1A_SETTINGS_RDATA()                    ( ( INREG32( &g_pMPIFRegs->REG_LC1A_SETTINGS ) >> 8 ) & 0xFF )
#define MCR_MPIF_LC1A_SETTINGS_WDATA(x)                    CLRREG32( &g_pMPIFRegs->REG_LC1A_SETTINGS, ( 0xFF << 8 ) ); SETREG32( &g_pMPIFRegs->REG_LC1A_SETTINGS, ( ((x)&0xFF) << 8 ) )


// MPIF part - LC2A
#define MCR_MPIF_LC2A_REG_CONTROL_VALID()                SETREG32( &g_pMPIFRegs->REG_LC2A_REG_CONTROL, BIT00 )
#define MCR_MPIF_LC2A_REG_CONTROL_READ()                CLRREG32( &g_pMPIFRegs->REG_LC2A_REG_CONTROL, BIT01 )
#define MCR_MPIF_LC2A_REG_CONTROL_WRITE()                SETREG32( &g_pMPIFRegs->REG_LC2A_REG_CONTROL, BIT01 )
#define MCR_MPIF_LC2A_REG_CONTROL_SLAVEID(x)            CLRREG32( &g_pMPIFRegs->REG_LC2A_REG_CONTROL, ( 0x03 << 2 ) ); SETREG32( &g_pMPIFRegs->REG_LC2A_REG_CONTROL, ( ((x)&0x03) << 2 ) )
#define MCR_MPIF_LC2A_REG_CONTROL_CKSUM_ENABLE()        SETREG32( &g_pMPIFRegs->REG_LC2A_REG_CONTROL, BIT04 )
#define MCR_MPIF_LC2A_REG_CONTROL_CKSUM_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_LC2A_REG_CONTROL, BIT04 )
#define MCR_MPIF_LC2A_REG_CONTROL_MAX_RTX(x)            CLRREG32( &g_pMPIFRegs->REG_LC2A_REG_CONTROL, ( 0x03 << 6 ) ); SETREG32( &g_pMPIFRegs->REG_LC2A_REG_CONTROL, ( ((x)&0x03) << 6 ) )

#define MCR_MPIF_LC2A_REG_ADDRESS_SET(addr)                OUTREG32( &g_pMPIFRegs->REG_LC2A_REG_ADDRESS, ( addr ) )

#define MCR_MPIF_LC2A_REG_DATA_RDATA()                    ( INREG32( &g_pMPIFRegs->REG_LC2A_REG_DATA ) )
#define MCR_MPIF_LC2A_REG_DATA_WDATA(data)                OUTREG32( &g_pMPIFRegs->REG_LC2A_REG_DATA, ( data ) )


// MPIF part - LC2B
#define MCR_MPIF_LC2B_REG_CONTROL_VALID()                SETREG32( &g_pMPIFRegs->REG_LC2B_REG_CONTROL, BIT00 )
#define MCR_MPIF_LC2B_REG_CONTROL_READ()                CLRREG32( &g_pMPIFRegs->REG_LC2B_REG_CONTROL, BIT01 )
#define MCR_MPIF_LC2B_REG_CONTROL_WRITE()                SETREG32( &g_pMPIFRegs->REG_LC2B_REG_CONTROL, BIT01 )
#define MCR_MPIF_LC2B_REG_CONTROL_SLAVEID(x)            CLRREG32( &g_pMPIFRegs->REG_LC2B_REG_CONTROL, ( 0x03 << 2 ) ); SETREG32( &g_pMPIFRegs->REG_LC2B_REG_CONTROL, ( ((x)&0x03) << 2 ) )
#define MCR_MPIF_LC2B_REG_CONTROL_CKSUM_ENABLE()        SETREG32( &g_pMPIFRegs->REG_LC2B_REG_CONTROL, BIT04 )
#define MCR_MPIF_LC2B_REG_CONTROL_CKSUM_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_LC2B_REG_CONTROL, BIT04 )
#define MCR_MPIF_LC2B_REG_CONTROL_MAX_RTX(x)            CLRREG32( &g_pMPIFRegs->REG_LC2B_REG_CONTROL, ( 0x03 << 6 ) ); SETREG32( &g_pMPIFRegs->REG_LC2B_REG_CONTROL, ( ((x)&0x03) << 6 ) )

#define MCR_MPIF_LC2B_REG_ADDRESS_SET(addr)                OUTREG32( &g_pMPIFRegs->REG_LC2B_REG_ADDRESS, ( addr ) )

#define MCR_MPIF_LC2B_REG_DATA_RDATA()                    ( INREG32( &g_pMPIFRegs->REG_LC2B_REG_DATA ) )
#define MCR_MPIF_LC2B_REG_DATA_WDATA(data)                OUTREG32( &g_pMPIFRegs->REG_LC2B_REG_DATA, ( data ) )


// MPIF part - LC3A
#define MCR_MPIF_LC3A_PACKET_CONTROL_VALID()            SETREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT00 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_READ()                CLRREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT01 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_WRITE()            SETREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT01 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_SLAVEID(x)            CLRREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, ( 0x03 << 2 ) ); SETREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, ( ((x)&0x03) << 2 ) )
#define MCR_MPIF_LC3A_PACKET_CONTROL_CKSUM_ENABLE()        SETREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT04 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_CKSUM_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT04 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_RTX_ENABLE()        SETREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT05 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_RTX_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT05 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_MAX_RTX(x)            CLRREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, ( 0x03 << 6 ) ); SETREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, ( ((x)&0x03) << 6 ) )
#define MCR_MPIF_LC3A_PACKET_CONTROL_SET_SRCDST_RIU()    CLRREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT08 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_SET_SRCDST_MIU()    SETREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT08 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_SET_MIUID_0()        CLRREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT09 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_SET_MIUID_1()        SETREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT09 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_FASTMODE_ENABLE()    SETREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT10 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_FASTMODE_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, BIT10 )
#define MCR_MPIF_LC3A_PACKET_CONTROL_MAX_WAIT(t)        CLRREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, ( 0x0F << 12 ) ); SETREG32( &g_pMPIFRegs->REG_LC3A_PACKET_CONTROL, ( ((t)&0x0F) << 12 ) )

#define MCR_MPIF_LC3A_PACKET_LENGTH_SET(len)            OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_LENGTH, ( len ) )

#define MCR_MPIF_LC3A_PACKET_DATA_CLEAR()               OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_DATA[0], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_DATA[1], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_DATA[2], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_DATA[3], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_DATA[4], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_DATA[5], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_DATA[6], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_DATA[7], 0x00000000 );


#define MCR_MPIF_LC3A_PACKET_MIU_BASE_ADDRESS_SET(addr)    OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_MIU_BASE_ADDRESS_L, ( (addr) & 0xFFFF ) );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3A_PACKET_MIU_BASE_ADDRESS_H, ( ( (addr) >> 16 ) & 0xFFFF ) )


// MPIF part - LC3B
#define MCR_MPIF_LC3B_PACKET_CONTROL_VALID()            SETREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT00 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_READ()                CLRREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT01 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_WRITE()            SETREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT01 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_SLAVEID(x)            CLRREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, ( 0x03 << 2 ) ); SETREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, ( ((x)&0x03) << 2 ) )
#define MCR_MPIF_LC3B_PACKET_CONTROL_CKSUM_ENABLE()        SETREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT04 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_CKSUM_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT04 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_RTX_ENABLE()        SETREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT05 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_RTX_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT05 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_MAX_RTX(x)            CLRREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, ( 0x03 << 6 ) ); SETREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, ( ((x)&0x03) << 6 ) )
#define MCR_MPIF_LC3B_PACKET_CONTROL_SET_SRCDST_RIU()    CLRREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT08 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_SET_SRCDST_MIU()    SETREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT08 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_SET_MIUID_0()        CLRREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT09 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_SET_MIUID_1()        SETREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT09 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_FASTMODE_ENABLE()    SETREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT10 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_FASTMODE_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, BIT10 )
#define MCR_MPIF_LC3B_PACKET_CONTROL_MAX_WAIT(t)        CLRREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, ( 0x0F << 12 ) ); SETREG32( &g_pMPIFRegs->REG_LC3B_PACKET_CONTROL, ( ((t)&0x0F) << 12 ) )

#define MCR_MPIF_LC3B_PACKET_LENGTH_SET(len)            OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_LENGTH, ( len ) )

#define MCR_MPIF_LC3B_PACKET_DATA_CLEAR()               OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_DATA[0], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_DATA[1], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_DATA[2], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_DATA[3], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_DATA[4], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_DATA[5], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_DATA[6], 0x00000000 );\
                                                        OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_DATA[7], 0x00000000 );

#define MCR_MPIF_LC3B_PACKET_MIU_BASE_ADDRESS_SET(addr)    OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_MIU_BASE_ADDRESS_L, ( (addr) & 0xFFFF ) ); OUTREG32( &g_pMPIFRegs->REG_LC3B_PACKET_MIU_BASE_ADDRESS_H, ( ( (addr) >> 16 ) & 0xFFFF ) )


// MPIF part - LC4A
#define MCR_MPIF_LC4A_STREAM_CONTROL_VALID()            SETREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, BIT00 )
#define MCR_MPIF_LC4A_STREAM_CONTROL_READ()                CLRREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, BIT01 )
#define MCR_MPIF_LC4A_STREAM_CONTROL_WRITE()            SETREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, BIT01 )
#define MCR_MPIF_LC4A_STREAM_CONTROL_SLAVEID(x)            CLRREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, ( 0x03 << 2 ) ); SETREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, ( ((x)&0x03) << 2 ) )
#define MCR_MPIF_LC4A_STREAM_CONTROL_MAX_RTX(x)            CLRREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, ( 0x03 << 6 ) ); SETREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, ( ((x)&0x03) << 6 ) )
#define MCR_MPIF_LC4A_STREAM_CONTROL_SET_MIUID_0()        CLRREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, BIT09 )
#define MCR_MPIF_LC4A_STREAM_CONTROL_SET_MIUID_1()        SETREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, BIT09 )
#define MCR_MPIF_LC4A_STREAM_PAGE_SIZE_SET(x)            CLRREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, ( 0x03 << 10 ) ); SETREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, ( ((x)&0x03) << 10 ) )
#define MCR_MPIF_LC4A_STREAM_CONTROL_MAX_WAIT(t)        CLRREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, ( 0x0F << 12 ) ); SETREG32( &g_pMPIFRegs->REG_LC4A_STREAM_CONTROL, ( ((t)&0x0F) << 12 ) )
#define MCR_MPIF_LC4A_STREAM_LENGTH_SET(len)            OUTREG32( &g_pMPIFRegs->REG_LC4A_STREAM_LENGTH, ( len ) )

#define MCR_MPIF_LC4A_STREAM_MIU_BASE_ADDRESS_SET(addr)    OUTREG32( &g_pMPIFRegs->REG_LC4A_STREAM_MIU_BASE_ADDRESS_L, ( (addr) & 0xFFFF ) ); OUTREG32( &g_pMPIFRegs->REG_LC4A_STREAM_MIU_BASE_ADDRESS_H, ( ( addr >> 16 ) & 0xFFFF ) )


// SPI part
#define MCR_MPIF_4WIRE_SPI_CONTROL_VALID()                SETREG32( &g_pMPIFRegs->REG_4WIRE_SPI_CONTROL, BIT00 )
#define MCR_MPIF_4WIRE_SPI_CONTROL_READ()                CLRREG32( &g_pMPIFRegs->REG_4WIRE_SPI_CONTROL, BIT01 )
#define MCR_MPIF_4WIRE_SPI_CONTROL_WRITE()                SETREG32( &g_pMPIFRegs->REG_4WIRE_SPI_CONTROL, BIT01 )

#define MCR_MPIF_3WIRE_SPI_CONTROL_VALID()                SETREG32( &g_pMPIFRegs->REG_3WIRE_SPI_CONTROL, BIT00 )
#define MCR_MPIF_3WIRE_SPI_CONTROL_READ()                CLRREG32( &g_pMPIFRegs->REG_3WIRE_SPI_CONTROL, BIT01 )
#define MCR_MPIF_3WIRE_SPI_CONTROL_WRITE()                SETREG32( &g_pMPIFRegs->REG_3WIRE_SPI_CONTROL, BIT01 )

#define MCR_MPIF_SPI_CONTROL_SLAVEID(x)                    CLRREG32( &g_pMPIFRegs->REG_SPI_CONTROL, 0x03 ); SETREG32( &g_pMPIFRegs->REG_SPI_CONTROL, ( (x)&0x03 ) )
#define MCR_MPIF_SPI_CONTROL_SET_CLOCK_PHASE_H()        SETREG32( &g_pMPIFRegs->REG_SPI_CONTROL, BIT02 )
#define MCR_MPIF_SPI_CONTROL_SET_CLOCK_PHASE_L()        CLRREG32( &g_pMPIFRegs->REG_SPI_CONTROL, BIT02 )
#define MCR_MPIF_SPI_CONTROL_SET_CLOCK_POLARITY_H()        SETREG32( &g_pMPIFRegs->REG_SPI_CONTROL, BIT03 )
#define MCR_MPIF_SPI_CONTROL_SET_CLOCK_POLARITY_L()        CLRREG32( &g_pMPIFRegs->REG_SPI_CONTROL, BIT03 )
#define MCR_MPIF_SPI_CONTROL_SET_LEADING(x)                CLRREG32( &g_pMPIFRegs->REG_SPI_CONTROL, ( 0x03 << 4 ) ); SETREG32( &g_pMPIFRegs->REG_SPI_CONTROL, ( ((x)&0x03) << 4 ) )
#define MCR_MPIF_SPI_CONTROL_SET_TAILING(x)                CLRREG32( &g_pMPIFRegs->REG_SPI_CONTROL, ( 0x03 << 6 ) ); SETREG32( &g_pMPIFRegs->REG_SPI_CONTROL, ( ((x)&0x03) << 6 ) )
#define MCR_MPIF_SPI_CONTROL_SET_CMDLEN(x)                CLRREG32( &g_pMPIFRegs->REG_SPI_CONTROL, ( 0x07 << 8 ) ); SETREG32( &g_pMPIFRegs->REG_SPI_CONTROL, ( ((x)&0x07) << 8 ) )
#define MCR_MPIF_SPI_CONTROL_SET_SRCDST_RIU()            CLRREG32( &g_pMPIFRegs->REG_SPI_CONTROL, BIT11 )
#define MCR_MPIF_SPI_CONTROL_SET_SRCDST_MIU()            SETREG32( &g_pMPIFRegs->REG_SPI_CONTROL, BIT11 )
#define MCR_MPIF_SPI_CONTROL_SET_MIUID_0()                CLRREG32( &g_pMPIFRegs->REG_SPI_CONTROL, BIT12 )
#define MCR_MPIF_SPI_CONTROL_SET_MIUID_1()                SETREG32( &g_pMPIFRegs->REG_SPI_CONTROL, BIT12 )

#define MCR_MPIF_SPI_COMMAND_SET(cmd)                    OUTREG32( &g_pMPIFRegs->REG_SPI_COMMAND_VALUE_L, ( (cmd) & 0xFFFF ) ); OUTREG32( &g_pMPIFRegs->REG_SPI_COMMAND_VALUE_H, ( ( (cmd) >> 16 ) & 0xFFFF ) )

#define MCR_MPIF_SPI_DATA_LENGTH_SET(len)                OUTREG32( &g_pMPIFRegs->REG_SPI_DATA_LENGTH, ( len ) )

#define MCR_MPIF_SPI_MIU_BASE_ADDRESS_SET(addr)            OUTREG32( &g_pMPIFRegs->REG_SPI_MIU_BASE_ADDRESS_L, ( addr & 0xFFFF ) ); OUTREG32( &g_pMPIFRegs->REG_SPI_MIU_BASE_ADDRESS_H, ( ( (addr) >> 16 ) & 0xFFFF ) )


// IP-wide status
#define MCR_MPIF_CHANNEL_BUSY_STATUS_4WSPI()            ( INREG32( &g_pMPIFRegs->REG_CHANNEL_BUSY_STATUS ) & BIT00 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_3WSPI()            ( INREG32( &g_pMPIFRegs->REG_CHANNEL_BUSY_STATUS ) & BIT01 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC1A()                ( INREG32( &g_pMPIFRegs->REG_CHANNEL_BUSY_STATUS ) & BIT02 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC2A()                ( INREG32( &g_pMPIFRegs->REG_CHANNEL_BUSY_STATUS ) & BIT03 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC2B()                ( INREG32( &g_pMPIFRegs->REG_CHANNEL_BUSY_STATUS ) & BIT04 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC3A()                ( INREG32( &g_pMPIFRegs->REG_CHANNEL_BUSY_STATUS ) & BIT05 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC3B()                ( INREG32( &g_pMPIFRegs->REG_CHANNEL_BUSY_STATUS ) & BIT06 )
#define MCR_MPIF_CHANNEL_BUSY_STATUS_LC4A()                ( INREG32( &g_pMPIFRegs->REG_CHANNEL_BUSY_STATUS ) & BIT07 )

#define MCR_MPIF_INTERRUPT_EVENT_MASK_4WSPITX_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT00 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_3WSPITX_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT01 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC1ATX_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT02 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATX_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT03 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTX_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT04 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATX_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT05 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTX_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT06 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATX_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT07 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATXER_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT08 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTXER_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT09 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATXER_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT10 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTXER_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT11 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATXER_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT12 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_BUSYTIMEOUT_ENABLE()    SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT13 )    // note that the #13 and #14 in the register table may be wrong, compared to the Interrupt Event Status register
#define MCR_MPIF_INTERRUPT_EVENT_MASK_SLAVEREQ_ENABLE()        SETREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT14 )

#define MCR_MPIF_INTERRUPT_EVENT_MASK_4WSPITX_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT00 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_3WSPITX_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT01 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC1ATX_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT02 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATX_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT03 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTX_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT04 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATX_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT05 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTX_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT06 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATX_DISABLE()        CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT07 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATXER_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT08 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTXER_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT09 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATXER_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT10 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTXER_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT11 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATXER_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT12 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_BUSYTIMEOUT_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT13 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_SLAVEREQ_DISABLE()    CLRREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK, BIT14 )

#define MCR_MPIF_INTERRUPT_EVENT_MASK_4WSPITX_ISEN()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT00 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_3WSPITX_ISEN()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT01 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC1ATX_ISEN()            ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT02 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATX_ISEN()            ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT03 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTX_ISEN()            ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT04 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATX_ISEN()            ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT05 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTX_ISEN()            ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT06 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATX_ISEN()            ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT07 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2ATXER_ISEN()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT08 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC2BTXER_ISEN()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT09 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3ATXER_ISEN()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT10 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC3BTXER_ISEN()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT11 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_LC4ATXER_ISEN()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT12 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_BUSYTIMEOUT_ISEN()    ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT13 )
#define MCR_MPIF_INTERRUPT_EVENT_MASK_SLAVEREQ_ISEN()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_MASK ) & BIT14 )

#define MCR_MPIF_INTERRUPT_EVENT_STATUS_4WSPITX_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT00 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_3WSPITX_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT01 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC1ATX_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT02 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2ATX_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT03 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2BTX_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT04 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3ATX_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT05 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3BTX_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT06 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC4ATX_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT07 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2ATXER_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT08 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2BTXER_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT09 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3ATXER_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT10 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3BTXER_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT11 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC4ATXER_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT12 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_BUSYTIMEOUT_GET()    ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT13 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_SLAVEREQ_GET()        ( INREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS ) & BIT14 )

#define MCR_MPIF_INTERRUPT_EVENT_STATUS_4WSPITX_CLEAR()        OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT00 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_3WSPITX_CLEAR()        OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT01 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC1ATX_CLEAR()        OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT02 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2ATX_CLEAR()        OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT03 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2BTX_CLEAR()        OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT04 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3ATX_CLEAR()        OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT05 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3BTX_CLEAR()        OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT06 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC4ATX_CLEAR()        OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT07 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2ATXER_CLEAR()    OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT08 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC2BTXER_CLEAR()    OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT09 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3ATXER_CLEAR()    OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT10 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC3BTXER_CLEAR()    OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT11 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_LC4ATXER_CLEAR()    OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT12 )
#define MCR_MPIF_INTERRUPT_EVENT_STATUS_BUSYTIMEOUT_CLEAR()    OUTREG32( &g_pMPIFRegs->REG_INTERRUPT_EVENT_STATUS, BIT13 )

#define MCR_MPIF_BUSY_TIMEOUT_COUNTER_SET(t)                OUTREG32( &g_pMPIFRegs->REG_BUSY_TIMEOUT_COUNTER, ( (t) & 0xFFFF ) )
#define MCR_MPIF_BUSY_TIMEOUT_COUNTER_GET(t)                ( INREG32( &g_pMPIFRegs->REG_BUSY_TIMEOUT_COUNTER ) & 0xFFFF )

#define MCR_REG_MPIF_BUSY_TIMEOUT_TO_GET()                    ( INREG32( &g_pMPIFRegs->REG_BUSY_TIMEOUT_TO )  )

#define MCR_MPIF_MISC1_SWRESET()                            CLRREG32( &g_pMPIFRegs->REG_MISC1, BIT00 ); SETREG32( &g_pMPIFRegs->REG_MISC1, BIT00 )
#define MCR_MPIF_MISC1_GATING_DISABLE()                        CLRREG32( &g_pMPIFRegs->REG_MISC1, BIT01 )
#define MCR_MPIF_MISC1_GATING_ENABLE()                        SETREG32( &g_pMPIFRegs->REG_MISC1, BIT01 )
#define MCR_MPIF_MISC1_SET_TURNAROUND_CYCLE(x)                CLRREG32( &g_pMPIFRegs->REG_MISC1, ( 0x03 << 2 ) ); SETREG32( &g_pMPIFRegs->REG_MISC1, ( ((x)&0x03) << 2 ) )    // Set the turn-around cycle, default value is 1. If the slave doesn't need any turn-around cycle, i.e., returning the values immediately, set this value to zero
#define MCR_MPIF_MISC1_SET_WAIT_CYCLE(x)                    CLRREG32( &g_pMPIFRegs->REG_MISC1, ( 0x03 << 4 ) ); SETREG32( &g_pMPIFRegs->REG_MISC1, ( ((x)&0x03) << 4 ) )
#define MCR_MPIF_MISC1_SLAVE0_DATAWIDTH(x)                    CLRREG32( &g_pMPIFRegs->REG_MISC1, ( 0x03 << 6 ) ); SETREG32( &g_pMPIFRegs->REG_MISC1, ( ((x)&0x03) << 6 ) )
#define MCR_MPIF_MISC1_SLAVE1_DATAWIDTH(x)                    CLRREG32( &g_pMPIFRegs->REG_MISC1, ( 0x03 << 8 ) ); SETREG32( &g_pMPIFRegs->REG_MISC1, ( ((x)&0x03) << 8 ) )
#define MCR_MPIF_MISC1_SLAVE2_DATAWIDTH(x)                    CLRREG32( &g_pMPIFRegs->REG_MISC1, ( 0x03 << 10 ) ); SETREG32( &g_pMPIFRegs->REG_MISC1, ( ((x)&0x03) << 10 ) )
#define MCR_MPIF_MISC1_SLAVE3_DATAWIDTH(x)                    CLRREG32( &g_pMPIFRegs->REG_MISC1, ( 0x03 << 12 ) ); SETREG32( &g_pMPIFRegs->REG_MISC1, ( ((x)&0x03) << 12 ) )
#define MCR_MPIF_MISC1_CMDPH_DATAWIDTH(x)                    CLRREG32( &g_pMPIFRegs->REG_MISC1, ( 0x03 << 14 ) ); SETREG32( &g_pMPIFRegs->REG_MISC1, ( ((x)&0x03) << 14 ) )

#define MCR_MPIF_MISC2_CLOCK_DELAY(x)                        CLRREG32( &g_pMPIFRegs->REG_MISC2, ( 0x0F ) ); SETREG32( &g_pMPIFRegs->REG_MISC2, ((x)&0x0F) )
#define MCR_MPIF_MISC2_CLOCK_INVERSE_DISABLE()                CLRREG32( &g_pMPIFRegs->REG_MISC2, BIT04 )
#define MCR_MPIF_MISC2_CLOCK_INVERSE_ENABLE()                SETREG32( &g_pMPIFRegs->REG_MISC2, BIT04 )
#define MCR_MPIF_MISC2_CLEAR_LC2X_ER()                        SETREG32( &g_pMPIFRegs->REG_MISC2, BIT05 )
#define MCR_MPIF_MISC2_CLEAR_LC3X_ER()                        SETREG32( &g_pMPIFRegs->REG_MISC2, BIT06 )
#define MCR_MPIF_MISC2_CLEAR_LC4A_ER()                        SETREG32( &g_pMPIFRegs->REG_MISC2, BIT07 )

#define MCR_MPIF_SPI_UCPLT_LENGTH_GET()                        ( INREG32( &g_pMPIFRegs->REG_SPI_UCPLT_LENGTH ) )

#endif  /* #ifdef NEW_MPIF_API */

#define MCR_MPIF_CLOCK_ENABLE_USB_MPLL()                    SETREG32( &g_pUTMIRegs->REG_UTMI_04, 0x0800 )
#define MCR_MPIF_CLOCK_SET_TO_120M()                        CLRREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x1C); SETREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x00 )
#define MCR_MPIF_CLOCK_SET_TO_108M()                        CLRREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x1C); SETREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x04 )
#define MCR_MPIF_CLOCK_SET_TO_86_5M()                       CLRREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x1C); SETREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x08 )
#define MCR_MPIF_CLOCK_SET_TO_72M()                         CLRREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x1C); SETREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x0C )
#define MCR_MPIF_CLOCK_SET_TO_60M()                         CLRREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x1C); SETREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x10 )
#define MCR_MPIF_CLOCK_SET_TO_40M()                         CLRREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x1C); SETREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x14 )
#define MCR_MPIF_CLOCK_SET_TO_24M()                         CLRREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x1C); SETREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x18 )
#define MCR_MPIF_CLOCK_SET_TO_12M()                         CLRREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x1C); SETREG32( &g_pCHIPRegs->REG_CHIPTOP_27, 0x1C )

#define MCR_MPIF_PAD_DRIVING_SET(x)                         OUTREG32( &g_pCHIPRegs->REG_CHIPTOP_3B, (x) )

#define MCR_MPIF_DISABLE_MPIFMIU_GATING()                   CLRREG32( &g_pCHIPRegs->REG_CHIPTOP_2E, BIT07 )

#define BUSYWAIT_TIMEOUT                        0x00FFFFFF
#define BUSYWAIT_TIMEOUT_3AB                    0xFFFF * 20   /* for new General API */
#define RIU_MODE_MAX_DATA_BYTES                 16
#define MIU_MODE_MAX_DATA_BYTES                 0x20000   /* sync with NEW API, modify to 128KB */

/* donot sync with NEW API, no used in NEW API */
#define MPIF_DMA_MIU_BASE_OFFSET                ( ( COLUMBUS_BASE_DMA_MPIF_PA - 0x20000000 ) / 8 ) // (0x0808D000-0x08000000)/8 = 0x011A00
//#define MPIF_DMADUMMY_MIU_BASE_OFFSET           ( ( COLUMBUS_BASE_DMADUMMY_MPIF_PA - 0x08000000 ) / 8 ) // (0x0808CF00-0x08000000)/8 = 0x0119E0


#define CHANNEL_BUSY_STATUS_BIT_4WSPI            BIT00
#define CHANNEL_BUSY_STATUS_BIT_3WSPI            BIT01
#define CHANNEL_BUSY_STATUS_BIT_LC1A            BIT02
#define CHANNEL_BUSY_STATUS_BIT_LC2A            BIT03
#define CHANNEL_BUSY_STATUS_BIT_LC2B            BIT04
#define CHANNEL_BUSY_STATUS_BIT_LC3A            BIT05
#define CHANNEL_BUSY_STATUS_BIT_LC3B            BIT06
#define CHANNEL_BUSY_STATUS_BIT_LC4A            BIT07

#define INTERRUPT_EVENT_STATUS_BIT_4WSPITX        BIT00
#define INTERRUPT_EVENT_STATUS_BIT_3WSPITX        BIT01
#define INTERRUPT_EVENT_STATUS_BIT_LC1ATX        BIT02
#define INTERRUPT_EVENT_STATUS_BIT_LC2ATX        BIT03
#define INTERRUPT_EVENT_STATUS_BIT_LC2BTX        BIT04
#define INTERRUPT_EVENT_STATUS_BIT_LC3ATX        BIT05
#define INTERRUPT_EVENT_STATUS_BIT_LC3BTX        BIT06
#define INTERRUPT_EVENT_STATUS_BIT_LC4ATX        BIT07
#define INTERRUPT_EVENT_STATUS_BIT_LC2ATXER        BIT08
#define INTERRUPT_EVENT_STATUS_BIT_LC2BTXER        BIT09
#define INTERRUPT_EVENT_STATUS_BIT_LC3ATXER        BIT10
#define INTERRUPT_EVENT_STATUS_BIT_LC3BTXER        BIT11
#define INTERRUPT_EVENT_STATUS_BIT_LC4ATXER        BIT12
#define INTERRUPT_EVENT_STATUS_BIT_BUSYTIMEOUT    BIT13
#define INTERRUPT_EVENT_STATUS_BIT_SLAVEREQ        BIT14     //chun 20090108 Columbus_u01
#define INTERRUPT_EVENT_STATUS_BIT_RXDMADONE    BIT14    //chun 20090108 Columbus_u02 is Done Flag

#define IOCTL_MPIF_READ                         CTL_CODE(FILE_DEVICE_SERIAL_PORT, 1,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_WRITE                        CTL_CODE(FILE_DEVICE_SERIAL_PORT, 2,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_REGISTER                     CTL_CODE(FILE_DEVICE_SERIAL_PORT, 3,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_CONTROL                      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 4,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_INIT                         CTL_CODE(FILE_DEVICE_SERIAL_PORT, 5,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_RESET                        CTL_CODE(FILE_DEVICE_SERIAL_PORT, 6,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_START_TIMER                  CTL_CODE(FILE_DEVICE_SERIAL_PORT, 7,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_STOP_TIMER                   CTL_CODE(FILE_DEVICE_SERIAL_PORT, 8,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_GET_TIMER_COUNT              CTL_CODE(FILE_DEVICE_SERIAL_PORT, 9,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_CONFIG_SPI                   CTL_CODE(FILE_DEVICE_SERIAL_PORT, 10, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_CONFIG_PIF                   CTL_CODE(FILE_DEVICE_SERIAL_PORT, 11, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_GET_DMA_BUFFER_PA            CTL_CODE(FILE_DEVICE_SERIAL_PORT, 12, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_GET_REG_ARM_PA               CTL_CODE(FILE_DEVICE_SERIAL_PORT, 13, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MPIF_GET_DMA_MIU_BASE_OFFSET      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 14, METHOD_NEITHER, FILE_ANY_ACCESS)



//------------------------------------------------------------------------------
// Function prototype
//------------------------------------------------------------------------------
#if 1//defined(NEW_MPIF_API) || defined(BSP_MPIF_IOCTL)     /* New_MPIF_API */
extern MPIF_ERROR_CODE_et ReadSPISlaveData      ( pmstar_multi_pif_get_st *pst/*SPI_OPTIONS_st sOption, U8 byCmdLength, U32 u32DataLength, U32 u32Cmd, U8 *pbyReadValue*/ );
extern MPIF_ERROR_CODE_et WriteSPISlaveData     ( pmstar_multi_pif_put_st *pst/*SPI_OPTIONS_st sOption, U8 byCmdLength, U32 u32DataLength, U32 u32Cmd, U8 abyWriteValue[RIU_MODE_MAX_DATA_BYTES]*/ );

extern MPIF_ERROR_CODE_et ConfigSPI         ( U32 handler, SPI_OPTIONS_st sOption );
extern MPIF_ERROR_CODE_et ConfigPIF         ( U32 handler, MPIF_OPTIONS_st sOption);

// General purpose API
MPIF_ERROR_CODE_et ReadSPIFStatus    ( pmstar_multi_pif_get_st *pst );
MPIF_ERROR_CODE_et ReadSPIFRegister  ( pmstar_multi_pif_get_st *pst );
MPIF_ERROR_CODE_et ReadSPIFData      ( pmstar_multi_pif_get_st *pst);
MPIF_ERROR_CODE_et ReadSPIFStream    ( pmstar_multi_pif_get_st *pst );

MPIF_ERROR_CODE_et WriteSPIFStatus   ( pmstar_multi_pif_put_st *pst );
MPIF_ERROR_CODE_et WriteSPIFRegister ( pmstar_multi_pif_put_st *pst );
MPIF_ERROR_CODE_et WriteSPIFData     ( pmstar_multi_pif_put_st *pst);
MPIF_ERROR_CODE_et WriteSPIFStream   ( pmstar_multi_pif_put_st *pst);

extern MPIF_ERROR_CODE_et mstar_multi_pif_register ( pmstar_multi_pif_register_st *pst);
extern MPIF_ERROR_CODE_et mstar_multi_pif_get_data ( pmstar_multi_pif_get_st *pst);
extern MPIF_ERROR_CODE_et mstar_multi_pif_put_data ( pmstar_multi_pif_put_st *pst);
extern void mstar_multi_pif_reset         ( void);
extern void mstar_multi_pif_init         ( void );
extern MPIF_ERROR_CODE_et mstar_multi_pif_IOControl(U32 handler, MPIF_IOCONTROL_CODE_et byIoControlCode);
void clock_pif_set(U16 mode, FAST_CLOCK_et fast, FAST_CLOCK_DIV_et fast_div, U16 divisor);
//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------
MPIF_ERROR_CODE_et DDK_BusyWait_ChannelIsFree( MPIF_DEVICE_ID_et mpid, U8 byBit, U32 u32Timeout);
MPIF_ERROR_CODE_et DDK_BusyWait_InterruptEvent(MPIF_DEVICE_ID_et mpid, U32 u32Bits, U32 u32Timeout);

//MPIF_ERROR_CODE_et DDK_MmMapIoSpace_MPIF ( void );
//MPIF_ERROR_CODE_et DDK_UnMapIoSpace_MPIF ( void );
MPIF_ERROR_CODE_et DDK_ConfigSPI( U32 handler, SPI_OPTIONS_st sOption );
MPIF_ERROR_CODE_et DDK_ReadSPISlaveData  ( MPIF_DEVICE_ID_et mpid, SPI_OPTIONS_st sOption, U16 mode, U8 byCmdLength, U32 u32DataLength, U32 u32Cmd, U8 *DMAAddr, U8 *pbyReadValue );
MPIF_ERROR_CODE_et DDK_WriteSPISlaveData ( MPIF_DEVICE_ID_et mpid, SPI_OPTIONS_st sOption, U16 mode, U8 byCmdLength, U32 u32DataLength, U32 u32Cmd, U8 *DMAAddr, U8 abyWriteValue[RIU_MODE_MAX_DATA_BYTES] );

MPIF_ERROR_CODE_et DDK_ConfigPIF        ( U32 handler, MPIF_OPTIONS_st sOption);
MPIF_ERROR_CODE_et DDK_ReadSPIFStatus   ( MPIF_DEVICE_ID_et mpid, MPIF_SLAVE_ID_et eSlaveId, U16 eRegisterIndex, U32 *pu32ReadValue);
MPIF_ERROR_CODE_et DDK_WriteSPIFStatus  ( MPIF_DEVICE_ID_et mpid, MPIF_SLAVE_ID_et eSlaveId, U16 eRegisterIndex, U32 u32WriteValue );
MPIF_ERROR_CODE_et DDK_ReadSPIFRegister ( MPIF_DEVICE_ID_et mpid, MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U16 u16RegAddr, U32 *pu32ReadValue );
MPIF_ERROR_CODE_et DDK_WriteSPIFRegister( MPIF_DEVICE_ID_et mpid, MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U16 u16RegAddr, U32 u32WriteValue );
MPIF_ERROR_CODE_et DDK_ReadSPIFData     ( MPIF_DEVICE_ID_et mpid, MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U16 mode, U8 *DMAAddr, U8 *pu8WriteValue, U32 u32DataLength,MPIF_WAIT_TYPE_et wait_type);
MPIF_ERROR_CODE_et DDK_WriteSPIFData    ( MPIF_DEVICE_ID_et mpid, MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U16 mode, U8 *DMAAddr, U8 *pu8WriteValue, U32 u32DataLength,MPIF_WAIT_TYPE_et wait_type);
MPIF_ERROR_CODE_et DDK_ReadSPIFStream   ( MPIF_DEVICE_ID_et mpid, MPIF_SLAVE_ID_et eSlaveId, MPIF_OPTIONS_st sOption, U8 *DMAAddr, U8 *pu8WriteValue, U32 u32DataLength);
MPIF_ERROR_CODE_et DDK_WriteSPIFStream  ( MPIF_DEVICE_ID_et mpid, MPIF_SLAVE_ID_et eSlaveId, MPIF_OPTIONS_st sOption, U8 *DMAAddr, U8 *pu8WriteValue, U32 u32DataLength );

#else

// Exposed API
MPIF_ERROR_CODE_et MPIFIOControl         ( MPIF_IOCONTROL_CODE_et eIoControlCode );
MPIF_ERROR_CODE_et ConfigSPI             ( SPI_OPTIONS_st sOption );
MPIF_ERROR_CODE_et ReadSPISlaveData      ( SPI_OPTIONS_st sOption, U8 u8CmdLength, U32 u32DataLength, U32 u32Cmd, U8 *pu8ReadValue );
MPIF_ERROR_CODE_et WriteSPISlaveData     ( SPI_OPTIONS_st sOption, U8 u8CmdLength, U32 u32DataLength, U32 u32Cmd, U8 au8WriteValue[RIU_MODE_MAX_DATA_BYTES] );

// General purpose API
#if 1
MPIF_ERROR_CODE_et ReadSPIFStatus        ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LC1A_INDEX_et eRegisterIndex, U32 *pu32ReadValue );
MPIF_ERROR_CODE_et WriteSPIFStatus       ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LC1A_INDEX_et eRegisterIndex, U32 u8WriteValue );
MPIF_ERROR_CODE_et ReadSPIFRegister      ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32RegAddr, U32 *pu32ReadValue );
MPIF_ERROR_CODE_et WriteSPIFRegister     ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32RegAddr, U32 u32WriteValue );
MPIF_ERROR_CODE_et ReadSPIFData          ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 *pu8ReadValue );
MPIF_ERROR_CODE_et WriteSPIFData         ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 au8WriteValue[RIU_MODE_MAX_DATA_BYTES] );
MPIF_ERROR_CODE_et ReadSPIFStream        ( MPIF_SLAVE_ID_et eSlaveId, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 *pu8ReadValue );
MPIF_ERROR_CODE_et WriteSPIFStream       ( MPIF_SLAVE_ID_et eSlaveId, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 *pu8WriteValue );
#else

// Special purpose API for Hydra only
MPIF_ERROR_CODE_et ReadSPIFStatus       ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LC1A_INDEX_et eRegisterIndex, U32 *pu32ReadValue );
MPIF_ERROR_CODE_et WriteSPIFStatus      ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LC1A_INDEX_et eRegisterIndex, U32 u32WriteValue );
MPIF_ERROR_CODE_et ReadSPIFRegister     ( U32 u32RegAddr, U32 *pu32ReadValue );
MPIF_ERROR_CODE_et WriteSPIFRegister    ( U32 u32RegAddr, U32 u32WriteValue );
MPIF_ERROR_CODE_et ReadSPIFData         ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 *pu8ReadValue );
MPIF_ERROR_CODE_et WriteSPIFData        ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 *pu8WriteValue );
MPIF_ERROR_CODE_et ReadSPIFStream       ( U32 u32DataLength );
MPIF_ERROR_CODE_et WriteSPIFStream      ( U32 u32DataLength );
#endif

MPIF_ERROR_CODE_et ConfigPIF            ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption);

//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------
MPIF_ERROR_CODE_et DDK_BusyWait_ChannelIsFree ( U8 u8Bit, U32 u32Timeout );
MPIF_ERROR_CODE_et DDK_BusyWait_InterruptEvent( U32 u32Bits, U32 u32Timeout );
MPIF_ERROR_CODE_et DDK_MmMapIoSpace_MPIF ( void );
MPIF_ERROR_CODE_et DDK_UnMapIoSpace_MPIF ( void );
MPIF_ERROR_CODE_et DDK_ConfigSPI         ( SPI_OPTIONS_st sOption );
MPIF_ERROR_CODE_et DDK_ReadSPISlaveData  ( SPI_OPTIONS_st sOption, U8 u8CmdLength, U32 u32DataLength, U32 u32Cmd, U8 *pu8ReadValue );
MPIF_ERROR_CODE_et DDK_WriteSPISlaveData ( SPI_OPTIONS_st sOption, U8 u8CmdLength, U32 u32DataLength, U32 u32Cmd, U8 au8WriteValue[RIU_MODE_MAX_DATA_BYTES] );
// General purpose API
#if 1
MPIF_ERROR_CODE_et DDK_ReadSPIFStatus    ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LC1A_INDEX_et eRegisterIndex, U32 *pu32ReadValue );
MPIF_ERROR_CODE_et DDK_WriteSPIFStatus   ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LC1A_INDEX_et eRegisterIndex, U32 u8WriteValue );
MPIF_ERROR_CODE_et DDK_ReadSPIFRegister  ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32RegAddr, U32 *pu32ReadValue );
MPIF_ERROR_CODE_et DDK_WriteSPIFRegister ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32RegAddr, U32 u32WriteValue );
MPIF_ERROR_CODE_et DDK_ReadSPIFData      ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 *pu8ReadValue );
MPIF_ERROR_CODE_et DDK_WriteSPIFData     ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 au8WriteValue[RIU_MODE_MAX_DATA_BYTES] );
MPIF_ERROR_CODE_et DDK_ReadSPIFStream    ( MPIF_SLAVE_ID_et eSlaveId, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 *pu8ReadValue );
MPIF_ERROR_CODE_et DDK_WriteSPIFStream   ( MPIF_SLAVE_ID_et eSlaveId, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 *pu8WriteValue );
#else
// Special purpose API for Hydra only
MPIF_ERROR_CODE_et DDK_ReadSPIFStatus   ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LC1A_INDEX_et eRegisterIndex, U32 *pu32ReadValue);
MPIF_ERROR_CODE_et DDK_WriteSPIFStatus  ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LC1A_INDEX_et eRegisterIndex, U32 u32WriteValue );
MPIF_ERROR_CODE_et DDK_ReadSPIFRegister ( U32 u32RegAddr, U32 *pu32ReadValue);
MPIF_ERROR_CODE_et DDK_WriteSPIFRegister( U32 u32RegAddr, U32 u32WriteValue );
MPIF_ERROR_CODE_et DDK_ReadSPIFData     ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32DataLength,U8 *pu8ReadValue );
MPIF_ERROR_CODE_et DDK_WriteSPIFData    ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel, MPIF_OPTIONS_st sOption, U32 u32DataLength, U8 *pu8WriteValue );
MPIF_ERROR_CODE_et DDK_ReadSPIFStream   ( U32 u32DataLength);
MPIF_ERROR_CODE_et DDK_WriteSPIFStream  ( U32 u32DataLength );
#endif
MPIF_ERROR_CODE_et DDK_ConfigPIF        ( MPIF_SLAVE_ID_et eSlaveId, MPIF_LOGICAL_CHANNEL_ID_et eChannel,MPIF_OPTIONS_st sOption);
#endif      /* #ifdef NEW_MPIF_API  */

// Called by DllMain() in ceddk.c
MPIF_ERROR_CODE_et DDK_MPIF_Init         ( void );
MPIF_ERROR_CODE_et DDK_MPIF_Deinit       ( void );
volatile LPVOID DDK_MPIF_GetVirtualAddr(PHYSICAL_ADDRESS pa, DWORD size, BOOL cacheEnable);
volatile LPVOID DDK_MPIF_DMA_BUFFER_VA(void);

U32 DDK_MPIF_DMA_BUFFER_PA(void);
U32 DDK_MPIF_REG_ARM_PA(void);
U32 DDK_MPIF_DMA_MIU_BASE_OFFSET(void);
#if __cplusplus
}
#endif  //#if __cplusplus
#endif  //__COLUMBUS_MPIF_H
