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

// Nernst AC injection resistor value
#define ESR_SUPPLY_R (22000)
#define VM_RESISTOR_VALUE (10)

// Heater low pass filter
#define ESR_SENSE_ALPHA (0.002f)

// *******************************
//       Pump current sense
// *******************************

#define PUMP_CURRENT_SENSE_GAIN (10)

// LSU sense resistor - 61.9 ohms
#define LSU_SENSE_R (61.9f)

// Pump low pass filter alpha
// sampling at 2.5khz, alpha of 0.01 gives about 50hz bandwidth
#define PUMP_FILTER_ALPHA (0.02f)

// *******************************
//        Pump controller
// *******************************
#define NERNST_TARGET (0.45f)

// *******************************
//    Heater controller config
// *******************************
#define HEATER_CONTROL_PERIOD 50

#define HEATER_PREHEAT_TIME 5000
#define HEATER_WARMUP_TIMEOUT 45000

#define HEATER_CLOSED_LOOP_THRESHOLD_ESR 400
#define HEATER_TARGET_ESR 300
#define HEATER_OVERHEAT_ESR 150
#define HEATER_UNDERHEAT_ESR 600
