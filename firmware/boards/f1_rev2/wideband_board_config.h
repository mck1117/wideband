#pragma once

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

#define AFR_CHANNELS 2