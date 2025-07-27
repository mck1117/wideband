#pragma once

// TS settings
#define TS_SIGNATURE "rusEFI 2023.05.10.wideband_f1"

// Fundamental board constants
#define VCC_VOLTS (3.3f)
#define HALF_VCC (VCC_VOLTS / 2)
#define ADC_MAX_COUNT (4095)
#define ADC_OVERSAMPLE 24

// *******************************
//    Nernst voltage & ESR sense
// *******************************
#define NERNST_INPUT_GAIN (3.0f)

// *******************************
//        Battery Sensing
// *******************************
// 100K + 10K divider
#define BATTERY_INPUT_DIVIDER (10.0 / (10.0 + 100.0))
#define BOARD_HAS_VOLTAGE_SENSE

// *******************************
//     Heater low side Sensing
// *******************************
// 100K + 10K divider
#define HEATER_INPUT_DIVIDER (10.0 / (10.0 + 100.0))

// *******************************
//        Vm output Sensing
// *******************************
// 100K + 10K divider
#define VM_INPUT_DIVIDER (10.0 / (10.0 + 100.0))

#define AFR_CHANNELS 1

// *******************************
//    Nernst voltage & ESR sense
// *******************************
#define VM_RESISTOR_VALUE (0)

// *******************************
//          AUX outputs
// *******************************
// OpAmp with 82K + 160K
#define AUXOUT_GAIN         ((82.0 + 160.0) / 160.0)

// *******************************
//   TunerStudio Primary Port
// *******************************
#define TS_PRIMARY_SERIAL_PORT	SD1
#define TS_PRIMARY_BAUDRATE		115200
