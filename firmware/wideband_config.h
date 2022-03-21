#pragma once

#include "io_pins.h"
#include "wideband_board_config.h"

// *******************************
//    Nernst voltage & ESR sense
// *******************************

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
#define HEATER_WARMUP_TIMEOUT 60000

#define HEATER_BATTERY_STAB_TIME	3000
/* minimal battery voltage to start heating without CAN command */
#define HEATER_BATTERY_ON_VOLTAGE	9.5
/* mininal battery voltage to continue heating */
#define HEATER_BATTETY_OFF_VOLTAGE  8.5

#define HEATER_CLOSED_LOOP_THRESHOLD_ESR 500
#define HEATER_TARGET_ESR 300
#define HEATER_OVERHEAT_ESR 150
#define HEATER_UNDERHEAT_ESR 700
