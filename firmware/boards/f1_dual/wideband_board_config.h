#pragma once

// TS settings
#define TS_SIGNATURE "rusEFI 2023.05.10.wideband_dual"

// This board implements two channels
#define AFR_CHANNELS 2
#define EGT_CHANNELS 2

// Fundamental board constants
#define VCC_VOLTS (3.3f)
#define HALF_VCC (VCC_VOLTS / 2)
#define ADC_MAX_COUNT (4095)
#define ADC_OVERSAMPLE 16

// *******************************
//    Nernst voltage & ESR sense
// *******************************
#define NERNST_INPUT_GAIN (3.0f)

// *******************************
//        Battery Sensing
// *******************************
// 100K + 10K divider
#define BATTERY_INPUT_DIVIDER (10.0 / (10.0 + 100.0))

// *******************************
//     Heater low side Sensing
// *******************************
// 100K + 10K divider
#define HEATER_INPUT_DIVIDER (10.0 / (10.0 + 100.0))
#define HEATER_FILTER_ALPHA (0.1f)

// *******************************
//        Vm output Sensing
// *******************************
// 100K + 10K divider
#define VM_INPUT_DIVIDER (10.0 / (10.0 + 100.0))

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
// With heater constantly on we
// are not able to measure Vbatt
// through Heater-
// *******************************
#define HEATER_MAX_DUTY		(0.85)

// *******************************
//   TunerStudio Primary Port
// *******************************
#define TS_PRIMARY_SERIAL_PORT	SD1
#define TS_PRIMARY_BAUDRATE		115200

// *******************************
//   MAX31855 spi port
// *******************************
#define EGT_SPI_DRIVER			(&SPID1)

// Serial connection over BT module
// enable BT module setup attempt
#define BT_SERIAL_OVER_JDY33	TRUE
#define BT_BROADCAST_NAME		"RusEFI WBO x2"
