/*
 * Copyright 2016-2017 NXP
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "bootloader/bl_context.h"
#include "bootloader_common.h"
#include "fsl_device_registers.h"
#include "microseconds/microseconds.h"
#include "property/property.h"
#include "target_config.h"
#include "flexspi/fsl_flexspi.h"
#if BL_FEATURE_SEMC_NAND_MODULE || BL_FEATURE_SEMC_NOR_MODULE
#include "semc/fsl_semc.h"
#endif // #if BL_FEATURE_SEMC_NAND_MODULE || BL_FEATURE_SEMC_NOR_MODULE
#include "utilities/fsl_assert.h"
#include "ocotp/fsl_ocotp.h"
#include "fusemap.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define FREQ_396MHz (396UL * 1000 * 1000)
#define FREQ_528MHz (528UL * 1000 * 1000)
#define FREQ_24MHz (24UL * 1000 * 1000)
#define FREQ_480MHz (480UL * 1000 * 1000)

enum
{
    kMaxAHBClock = 144000000UL,
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////
void clock_setup(void);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

// See bootloader_common for documentation on this function.
void configure_clocks(bootloader_clock_option_t option)
{
    if (option == kClockOption_EnterBootloader)
    {
        clock_setup();
    }
}

void clock_setup(void)
{
    uint32_t clock_divider = 1;
    uint32_t fuse_div = 0;
    uint32_t clock_freq = 0;

    CLOCK_SetXtal0Freq(CPU_XTAL_CLK_HZ);
    // Get the Boot Up CPU Clock Divider
    // 00b = divide by 1
    // 01b = divide by 2
    // 10b = divide by 4
    // 11b = divide by 8
    fuse_div = ROM_OCOTP_LPB_BOOT_VALUE();
    clock_divider = 1 << fuse_div;

    // Get the Boot up frequency
    // 0 = 396Mhz
    // 1 = 528Mhz
    clock_freq = ROM_OCOTP_BOOT_FREQ_VALUE();

    CCM_ANALOG->PLL_ARM |= CCM_ANALOG_PLL_ARM_BYPASS_MASK;
    CCM_ANALOG->PLL_SYS |= CCM_ANALOG_PLL_SYS_BYPASS_MASK;
    CCM_ANALOG->PLL_USB1 |= CCM_ANALOG_PLL_USB1_BYPASS_MASK;
    // If clock is not configured, configure clock first, otherwise, just update SystemCoreClock

    /* Configure PLL_ARM: Reference clock = 24MHz
    * PLL_ARM = 24MHz * div / 2
    *  Core = PLL_ARM / 2 / clock_divider
    * To get 396MHz clock, PLL_ARM = 24 * 66 / 2 = 792, 792 / 2 = 396
    * To get 528MHz clock, PLL_ARM = 24 * 88 / 2 = 1056, 1056 / 2 = 528
    */
    uint32_t div = (clock_freq == 1) ? 88 : 66;
    CCM_ANALOG->PLL_ARM =
        CCM_ANALOG_PLL_ARM_BYPASS(1) | CCM_ANALOG_PLL_ARM_ENABLE(1) | CCM_ANALOG_PLL_ARM_DIV_SELECT(div);
    // Wait Until clock is locked
    while ((CCM_ANALOG->PLL_ARM & CCM_ANALOG_PLL_ARM_LOCK_MASK) == 0)
    {
    }

    /* Configure PLL_SYS */
    CCM_ANALOG->PLL_SYS &= ~CCM_ANALOG_PLL_SYS_POWERDOWN_MASK;
    // Wait Until clock is locked
    while ((CCM_ANALOG->PLL_SYS & CCM_ANALOG_PLL_SYS_LOCK_MASK) == 0)
    {
    }

    // Configure SYS_PLL PFD
    // PFD0 = 396MHz  - uSDHC CLOCK Source
    // PFD1 = 396MHz
    // PFD2 = 500MHz  - SEMC CLOCK Source
    // PFD3 = 396MHz
    CCM_ANALOG->PFD_528 =
        (CCM_ANALOG->PFD_528 & (~(CCM_ANALOG_PFD_528_PFD0_FRAC_MASK | CCM_ANALOG_PFD_528_PFD1_FRAC_MASK |
                                  CCM_ANALOG_PFD_528_PFD2_FRAC_MASK | CCM_ANALOG_PFD_528_PFD3_FRAC_MASK))) |
        CCM_ANALOG_PFD_528_PFD0_FRAC(24) | CCM_ANALOG_PFD_528_PFD1_FRAC(24) | CCM_ANALOG_PFD_528_PFD2_FRAC(19) |
        CCM_ANALOG_PFD_528_PFD3_FRAC(24);

    // Always configure USB1_PLL
    CCM_ANALOG->PLL_USB1 =
        CCM_ANALOG_PLL_USB1_DIV_SELECT(0) | CCM_ANALOG_PLL_USB1_POWER(1) | CCM_ANALOG_PLL_USB1_ENABLE(1);
    while ((CCM_ANALOG->PLL_USB1 & CCM_ANALOG_PLL_USB1_LOCK_MASK) == 0)
    {
    }
    CCM_ANALOG->PLL_USB1 &= ~CCM_ANALOG_PLL_USB1_BYPASS_MASK;

    // Configure USB_PLL PFD
    // PFD0 = 247MHz  - FLEXSPI CLOCK Source
    // PFD1 = 247MHz  - LPSPI CLOCK Source
    // PFD2 = 247MHz  - SAI CLOCK Source
    // PFD3 = 576MHz
    CCM_ANALOG->PFD_480 =
        (CCM_ANALOG->PFD_480 & (~(CCM_ANALOG_PFD_480_PFD0_FRAC_MASK | CCM_ANALOG_PFD_480_PFD1_FRAC_MASK |
                                  CCM_ANALOG_PFD_480_PFD2_FRAC_MASK | CCM_ANALOG_PFD_480_PFD3_FRAC_MASK))) |
        CCM_ANALOG_PFD_480_PFD0_FRAC(35) | CCM_ANALOG_PFD_480_PFD1_FRAC(35) | CCM_ANALOG_PFD_480_PFD2_FRAC(35) |
        CCM_ANALOG_PFD_480_PFD3_FRAC(15);

    // Set up CPU_PODF
    CCM->CACRR = CCM_CACRR_ARM_PODF(1);

    // Calculate the Final System Core Clock, it will be used to calculate the AHB / ARM Core divider later.
    SystemCoreClock = ((clock_freq == 0) ? FREQ_396MHz : FREQ_528MHz) / clock_divider;

    // Calculate the AHB clock divider
    uint32_t ahb_divider = 1;
    while ((SystemCoreClock / ahb_divider) > kMaxAHBClock)
    {
        ++ahb_divider;
    }

    // Set up AXI_PODF - SEMC clock root
    // Set up AHB_PODF - CORE clock
    // Set up IPG_PODF - BUS clock
    CCM->CBCDR = (CCM->CBCDR & (~(CCM_CBCDR_SEMC_PODF_MASK | CCM_CBCDR_AHB_PODF_MASK | CCM_CBCDR_IPG_PODF_MASK))) |
                 CCM_CBCDR_SEMC_PODF(ahb_divider - 1) | CCM_CBCDR_AHB_PODF(clock_divider - 1) |
                 CCM_CBCDR_IPG_PODF(ahb_divider - 1);

    // NOTE: SEMC clock configuration needs handshake, so it will be handled by SEMC driver itself

    // Finally, Enable PLL_ARM, PLL_SYS and PLL_USB1
    CCM_ANALOG->PLL_ARM &= ~CCM_ANALOG_PLL_ARM_BYPASS_MASK;
    CCM_ANALOG->PLL_SYS &= ~CCM_ANALOG_PLL_SYS_BYPASS_MASK;
    CCM_ANALOG->PLL_USB1 &= ~CCM_ANALOG_PLL_USB1_BYPASS_MASK;
}

#if BL_FEATURE_SEMC_NAND_MODULE || BL_FEATURE_SEMC_NOR_MODULE
//!@brief Get Clock for SEMC peripheral
status_t semc_get_clock(semc_clock_type_t type, uint32_t *freq)
{
    uint32_t clockFrequency = 0;
    status_t status = kStatus_Success;

#ifndef BL_TARGET_FPGA
    uint32_t ipgBusDivider;
    uint32_t axiClkDivider;
    uint32_t pfd1Frac;
    uint32_t pfd1Clk;

    switch (type)
    {
        case kSemcClkType_IpgClock:
            ipgBusDivider = ((CCM->CBCDR & CCM_CBCDR_IPG_PODF_MASK) >> CCM_CBCDR_IPG_PODF_SHIFT) + 1;
            clockFrequency = SystemCoreClock / ipgBusDivider;
            break;
        case kSemcClkType_AxiClock:
            pfd1Frac = (CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD1_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD1_FRAC_SHIFT;
            axiClkDivider = ((CCM->CBCDR & CCM_CBCDR_SEMC_PODF_MASK) >> CCM_CBCDR_SEMC_PODF_SHIFT) + 1;
            pfd1Clk = FREQ_528MHz / pfd1Frac * 18;
            clockFrequency = pfd1Clk / axiClkDivider;
            break;
        default:
            status = kStatus_InvalidArgument;
            break;
    }
#else
#warning "Need to check with FPGA team"
    switch (type)
    {
        case kSemcClkType_IpgClock:
            clockFrequency = 24000000;
            break;
        case kSemcClkType_AxiClock:
            clockFrequency = 12000000;
            break;
        default:
            status = kStatus_InvalidArgument;
            break;
    }
#endif
    *freq = clockFrequency;

    return status;
}

//!@brief Configure axi clock for SEMC peripheral
void semc_axi_clock_config(semc_clk_freq_t freq)
{
#if !defined(BL_TARGET_FPGA)
    uint32_t pfd528 = 0;
    uint32_t cbcdr = 0;
    uint32_t frac = 0;
    uint32_t podf = 0;

    // Select the clk source
    /* Note1: There are total three kinds of clock source for SEMC AXI clk
    //  1. CCM_CBCDR[SEMC_CLK_SEL] = 1'b1 : SEMC alternative clock
    //     CCM_CBCDR[SEMC_ALT_CLK_SEL]:
    //                                  1'b0: PLL2 PFD2
    //                                  1'b1: PLL3 PFD1
    //  2. CCM_CBCDR[SEMC_CLK_SEL] = 1'b0 : Periph_clk
    //     CCM_CBCDR[PERIPH_CLK_SEL] = 1'b0: PLL2 (pll2_main_clk) and ARM PLL
    //     CCM_CBCMR[PRE_PERIPH_CLK_SEL]:
    //                                  2'b00: PLL2 Main
    //                                  2'b01: PLL2 PFD2
    //                                  2'b10: PLL2 PFD0
    //  3. CCM_CBCDR[SEMC_CLK_SEL] = 1'b0 : Periph_clk
    //     CCM_CBCDR[PERIPH_CLK_SEL] = 1'b1: periph_clk2
    //     CCM_CBCMR[PERIPH_CLK2_SEL]:
    //                                  2'b00: PLL3 Main
    //                                  2'b01: OSC

    // Note2: top clk is defined in hapi_clock_init()
    //    PLL2 PFD2: 500MHz (selected)
    //    PLL3 PFD1:
    */
    CCM->CBCDR |= CCM_CBCDR_SEMC_CLK_SEL_MASK;
    CCM->CBCDR &= ~CCM_CBCDR_SEMC_ALT_CLK_SEL_MASK;

    // Now set the divider
    pfd528 = CCM_ANALOG->PFD_528 & (~CCM_ANALOG_PFD_528_PFD2_FRAC_MASK);
    cbcdr = CCM->CBCDR & (~CCM_CBCDR_SEMC_PODF_MASK);

    switch (freq)
    {
        case kSemcClkFreq_33MHz:
            // FRAC = 35, 528 * 18 / 35 = 271.54Hz
            // PODF = 8, 271.54 / 8 = 33.9MHz
            frac = 35;
            podf = 8;
            break;
        case kSemcClkFreq_40MHz:
            // FRAC = 34, 528 * 18 / 34 = 279.53Hz
            // PODF = 7, 279.53 / 7 = 39.9MHz
            frac = 34;
            podf = 7;
            break;
        case kSemcClkFreq_50MHz:
            // FRAC = 27, 528 * 18 / 27 = 352Hz
            // PODF = 7, 352 / 7 = 50.29MHz
            frac = 27;
            podf = 7;
            break;
        case kSemcClkFreq_108MHz:
            // FRAC = 18, 528 * 18 / 29 = 327.72z
            // PODF = 8, 528 / 3 = 109.2MHz
            frac = 29;
            podf = 3;
            break;
        case kSemcClkFreq_133MHz:
            // FRAC = 18, 528 * 18 / 18 = 528MHz
            // PODF = 4, 528 / 4 = 132MHz
            frac = 18;
            podf = 4;
            break;
        case kSemcClkFreq_166MHz:
            // FRAC = 19, 528 * 18 / 19 = 500.21MHz
            // PODF = 3, 500.21 / 3 = 166.7MHz
            frac = 19;
            podf = 3;
            break;
        case kSemcClkFreq_66MHz:
        default:
            // FRAC = 18, 528 * 18 / 18 = 528Hz
            // PODF = 8, 528 / 8 = 66MHz
            frac = 18;
            podf = 8;
            break;
    }

    pfd528 |= CCM_ANALOG_PFD_528_PFD2_FRAC(frac);
    cbcdr |= CCM_CBCDR_SEMC_PODF(podf - 1);
    if (pfd528 != CCM_ANALOG->PFD_528)
    {
        CCM_ANALOG->PFD_528 = pfd528;
    }
    if (cbcdr != CCM->CBCDR)
    {
        // Any change of this divider might involve handshake with EMI.
        CCM->CBCDR = cbcdr;
        // Check busy indicator for semc_clk_podf.
        while (CCM->CDHIPR & CCM_CDHIPR_SEMC_PODF_BUSY_MASK)
            ;
    }

#endif
}

//!@brief Gate on the clock for the SEMC peripheral
void semc_clock_gate_enable(void)
{
    CCM->CCGR3 |= CCM_CCGR3_CG2_MASK;
}

//!@brief Gate off the clock the SEMC peripheral
void semc_clock_gate_disable(void)
{
    CCM->CCGR3 &= (uint32_t)~CCM_CCGR3_CG2_MASK;
}
#endif // #if BL_FEATURE_SEMC_NAND_MODULE || BL_FEATURE_SEMC_NOR_MODULE

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
