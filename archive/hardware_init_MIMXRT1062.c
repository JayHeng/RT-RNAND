/*
 * Copyright 2017 NXP
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "bootloader/bl_context.h"
#include "bootloader_common.h"
#include "fsl_device_registers.h"
#include "lpuart/fsl_lpuart.h"
#include "utilities/fsl_assert.h"
#if BL_ENABLE_CRC_CHECK
#include "bootloader/bl_app_crc_check.h"
#endif
#if BL_FEATURE_FLEXSPI_NOR_MODULE || BL_FEATURE_SPINAND_MODULE
#include "flexspi/fsl_flexspi.h"
#include "flexspi_nor/flexspi_nor_flash.h"
#endif // #if BL_FEATURE_FLEXSPI_NOR_MODULE || BL_FEATURE_SPINAND_MODULE
#if BL_FEATURE_SPI_NOR_EEPROM_MODULE
#include "microseconds/microseconds.h"
#include "memory/src/spi_nor_eeprom_memory.h"
#endif // BL_FEATURE_SPI_NOR_EEPROM_MODULE
#if BL_FEATURE_SEMC_NAND_MODULE || BL_FEATURE_SEMC_NOR_MODULE
#include "semc/fsl_semc.h"
#endif // #if BL_FEATURE_SEMC_NAND_MODULE || BL_FEATURE_SEMC_NOR_MODULE
#include "fusemap.h"
#include "peripherals_pinmux.h"
#include "bl_api.h"
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FREQ_396MHz (396000000U)
#define FREQ_480MHz (480000000U)
#define FREQ_528MHz (528000000U)
#define FREQ_24MHz (24000000U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Codes
 ******************************************************************************/

#if BL_FEATURE_SEMC_NAND_MODULE || BL_FEATURE_SEMC_NOR_MODULE
//!@brief Configure IOMUX for SEMC Peripheral
void semc_iomux_config(semc_mem_config_t *config)
{
    uint32_t dataInoutPadCtlValue = SEMC_SW_PAD_CTL_VAL;
    uint32_t addrInputPadCtlValue = SEMC_SW_PAD_CTL_VAL;
    uint32_t rdyOutputPadCtlValue = SEMC_RDY_SW_PAD_CTL_VAL;
    uint32_t ctlInputPadCtlValue = SEMC_SW_PAD_CTL_VAL;
    uint8_t cePortOutputSelection;

    // Pinmux configuration for SEMC DA[15:0] Port (NOR)
    // Pinmux configuration for SEMC D[15:0] Port (NAND)
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA0_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA0_IDX] = dataInoutPadCtlValue;
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA1_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA1_IDX] = dataInoutPadCtlValue;
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA2_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA2_IDX] = dataInoutPadCtlValue;
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA3_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA3_IDX] = dataInoutPadCtlValue;
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA4_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA4_IDX] = dataInoutPadCtlValue;
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA5_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA5_IDX] = dataInoutPadCtlValue;
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA6_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA6_IDX] = dataInoutPadCtlValue;
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA7_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA7_IDX] = dataInoutPadCtlValue;
    if ((config->deviceMemType == kSemcDeviceMemType_NOR) ||
        ((config->deviceMemType == kSemcDeviceMemType_NAND) && (config->nandMemConfig.ioPortWidth == 16u)))
    {
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA8_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA8_IDX] = dataInoutPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA9_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA9_IDX] = dataInoutPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA10_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA10_IDX] = dataInoutPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA11_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA11_IDX] = dataInoutPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA12_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA12_IDX] = dataInoutPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA13_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA13_IDX] = dataInoutPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA14_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA14_IDX] = dataInoutPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DATA15_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DATA15_IDX] = dataInoutPadCtlValue;
    }

    // Pinmux configuration for SEMC WE,OE,ADV Port (NOR)
    // Pinmux configuration for SEMC WE,RE,ALE Port (NAND)
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR11_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR11_IDX] = ctlInputPadCtlValue;
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR12_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR12_IDX] = ctlInputPadCtlValue;
    IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_BA1_IDX] = SEMC_MUX_VAL;
    IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_BA1_IDX] = ctlInputPadCtlValue;

    // Configure DQS pad
    /*
    if (config->readStrobeMode == kSemcDqsMode_LoopbackFromDqsPad)
    {
        // SEMC_DQS
        uint32_t dqsPadCtlValue = SEMC_DQS_SW_PAD_CTL_VAL;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_DQS_IDX] = SEMC_MUX_VAL | IOMUXC_SW_MUX_CTL_PAD_SION(1);
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_DQS_IDX] = dqsPadCtlValue;
    }
    */

    if (config->deviceMemType == kSemcDeviceMemType_NOR)
    {
        // Pinmux configuration for SEMC A[23:16], WAIT Port (NOR)
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_BA0_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_BA0_IDX] = ctlInputPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR0_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR0_IDX] = addrInputPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR1_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR1_IDX] = addrInputPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR2_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR2_IDX] = addrInputPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR3_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR3_IDX] = addrInputPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR4_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR4_IDX] = addrInputPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR5_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR5_IDX] = addrInputPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR6_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR6_IDX] = addrInputPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR7_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR7_IDX] = addrInputPadCtlValue;

        cePortOutputSelection = config->norMemConfig.cePortOutputSelection;
    }
    else if (config->deviceMemType == kSemcDeviceMemType_NAND)
    {
        // Pinmux configuration for SEMC CLE,R/B Port (NAND)
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_RDY_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_RDY_IDX] = rdyOutputPadCtlValue;
        IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR9_IDX] = SEMC_MUX_VAL;
        IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR9_IDX] = ctlInputPadCtlValue;

        cePortOutputSelection = config->nandMemConfig.cePortOutputSelection;
    }

    // Pinmux configuration for SEMC CE Port (NAND/NOR)
    switch (cePortOutputSelection)
    {
        case kSemcCeOutputSelection_MUX_CSX1:
            IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_CSX1_IDX] = SEMC_CSX123_MUX_VAL;
            IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_CSX1_IDX] = ctlInputPadCtlValue;
            break;
        case kSemcCeOutputSelection_MUX_CSX2:
            IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_CSX2_IDX] = SEMC_CSX123_MUX_VAL;
            IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_CSX2_IDX] = ctlInputPadCtlValue;
            break;
        case kSemcCeOutputSelection_MUX_CSX3:
            IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_CSX3_IDX] = SEMC_CSX123_MUX_VAL;
            IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_CSX3_IDX] = ctlInputPadCtlValue;
            break;
        case kSemcCeOutputSelection_MUX_A8:
            IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_ADDR8_IDX] = SEMC_MUX_VAL;
            IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_ADDR8_IDX] = ctlInputPadCtlValue;
            break;
        case kSemcCeOutputSelection_MUX_RDY:
        case kSemcCeOutputSelection_MUX_CSX0:
        default:
            IOMUXC->SW_MUX_CTL_PAD[SW_MUX_CTL_PAD_SEMC_CSX0_IDX] = SEMC_CSX0_MUX_VAL;
            IOMUXC->SW_PAD_CTL_PAD[SW_PAD_CTL_PAD_SEMC_CSX0_IDX] = ctlInputPadCtlValue;
            break;
    }
}
#endif // #if BL_FEATURE_SEMC_NAND_MODULE || BL_FEATURE_SEMC_NOR_MODULE

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
