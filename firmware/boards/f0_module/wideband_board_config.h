#pragma once

// Fundamental board constants
#define VCC_VOLTS (3.3f)
#define HALF_VCC (VCC_VOLTS / 2)
#define ADC_MAX_COUNT (4095)
#define ADC_OVERSAMPLE 24

// *******************************
//    Nernst voltage & ESR sense
// *******************************
#define NERNST_INPUT_GAIN (1 / 2.7f)

#define AFR_CHANNELS 1

// *******************************
//    Nernst voltage & ESR sense
// *******************************
#define VM_RESISTOR_VALUE (10)
