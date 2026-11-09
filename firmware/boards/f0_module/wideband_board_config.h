#pragma once

// Fundamental board constants
#define VCC_VOLTS (3.3f)
#define HALF_VCC (VCC_VOLTS / 2)
#define ADC_MAX_COUNT (4095)
#define ADC_OVERSAMPLE 24

// *******************************
//    Nernst voltage & ESR sense
// *******************************
#define NERNST_INPUT_GAIN (2.7f)

#define AFR_CHANNELS 1

// Fixed sampling loop rate, the ESR driver toggles at half this (3khz)
// Must evenly divide CH_CFG_ST_FREQUENCY
#define SAMPLE_RATE_HZ 6000

// *******************************
//    Nernst voltage & ESR sense
// *******************************
#define VM_RESISTOR_VALUE (10)
