#pragma once

// This board implements two channels
#define AFR_CHANNELS 2
#define EGT_CHANNELS 2

// UART used for debug
#define UART_DEBUG

// Fundamental board constants
#define VCC_VOLTS (3.3f)
#define HALF_VCC (VCC_VOLTS / 2)
#define ADC_MAX_COUNT (4095)
#define ADC_OVERSAMPLE 24

// *******************************
//    Nernst voltage & ESR sense
// *******************************
#define NERNST_INPUT_GAIN (1 / 3.0f)

// *******************************
//        Battery Sensing
// *******************************
// 100K + 10K divider
#define BATTERY_INPUT_DIVIDER (10.0 / (10.0 + 100.0))
#define BATTERY_FILTER_ALPHA (0.1f)

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
//#define TS_PRIMARY_UART_PORT	UARTD1
//#define TS_PRIMARY_BAUDRATE		38400
#define TS_PRIMARY_SERIAL_PORT	SD1
#define TS_PRIMARY_BAUDRATE		115200

// *******************************
//   MAX31855 spi port
// *******************************
#define EGT_SPI_PORT			SPID1
