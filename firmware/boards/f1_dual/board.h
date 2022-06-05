/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Board identifier.
 */
#define BOARD_RUSEFI_WIDEBAND_DUAL
/*
 * Board identifier.
 */
#define BOARD_NAME                  "rusEfi Wideband Controller Dual"

/*
 * Board oscillators-related settings.
 * NOTE: LSE not fitted.
 * NOTE: HSE not fitted.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                0U
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                0U
#endif

/*
 * MCU type, supported types are defined in ./os/hal/platforms/hal_lld.h.
 */
#define STM32F103xB

/*
 * IO pins assignments.
 */

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 *
 * The digits have the following meaning:
 *   0 - Analog input.
 *   1 - Push Pull output 10MHz.
 *   2 - Push Pull output 2MHz.
 *   3 - Push Pull output 50MHz.
 *   4 - Digital input.
 *   5 - Open Drain output 10MHz.
 *   6 - Open Drain output 2MHz.
 *   7 - Open Drain output 50MHz.
 *   8 - Digital input with PullUp or PullDown resistor depending on ODR.
 *   9 - Alternate Push Pull output 10MHz.
 *   A - Alternate Push Pull output 2MHz.
 *   B - Alternate Push Pull output 50MHz.
 *   C - Reserved.
 *   D - Alternate Open Drain output 10MHz.
 *   E - Alternate Open Drain output 2MHz.
 *   F - Alternate Open Drain output 50MHz.
 * Please refer to the STM32 Reference Manual for details.
 */

/*
 * Port A setup.
 * PA0  - R_Ip_sense                (analog in)
 * PA1  - R_Ip_dac (PWM)            (output pushpull, alternate, 2 MHz)
 * PA2  - L_Ip_dac (PWM)            (output pushpull, alternate, 2 MHz)
 * PA3  - R_Un_3x_sense             (analog in)
 * PA4  - L_Ip_sense                (analog in)
 * PA5  - L_Un_3x_sense             (analog in)
 * PA6  - R_AUX_ADC                 (analog in)
 * PA7  - L_AUX_ADC                 (analog in)
 * PA8  - LED_GREEN                 (output pushpull, 2 MHz)
 * PA9  - UART_TX                   (output pushpull, alternate, 50 MHz)
 * PA10 - UART_RX                   (digital input, alternate)
 * PA11 - CAN_RX                    (digital input, alternate)
 * PA12 - CAN_TX                    (output pushpull, alternate, 50 MHz)
 * PA13 - SWDIO                     (digital input)
 * PA14 - SWCLK                     (digital input)
 * PA15 - SPI_CS2                   (output pushpull, 2 MHz)
 */
#define VAL_GPIOACRL            0x00000BB0      /*  PA7...PA0 */
#define VAL_GPIOACRH            0x288B88B2      /* PA15...PA8 */
#define VAL_GPIOAODR            0x0000FFFF

/*
 * Port B setup.
 * PB0  - L_Heater_sense            (analog in).
 * PB1  - R_Heater_sense            (analog in).
 * PB2  - Nernsr_4.9_bias           (digital output, 2 Mhz)
 * PB3  - SPI1_SCK                  (output puspull, alternate, 50 MHz)
 * PB4  - SPI1_MISO                 (digital input, alternate)
 * PB5  - SPI1_MOSI                 (output puspull, alternate, 50 MHz)
 * PB6  - R_heater_pwm              (output pushpull, alternate, 2 MHz)
 * PB7  - L_heater_pwm              (output pushpull, alternate, 2 MHz)
 * PB8  - I2C1_SCL                  (output pushpull, alternate, 50 MHz)
 * PB9  - I2C1_SDA                  (output OD, alternate, 50 MHz)
 * PB10 - Nernsr_ADV_esr_drive      (digital input, no pull) - keep high-Z after power on
 * PB11 - Nernsr_4.9_esr_drive      (output pushpull, 50 Mhz)
 * PB12 - Nernsr_4.2_esr_drive      (digital input, no pull) - keep high-Z after power on
 * PB13 - Blue LED                  (output pushpull, 2 MHz)
 * PB14 - PWMout2                   (output pushpull, alternate, 2 Mhz).
 * PB15 - PWMout1                   (output pushpull, alternate, 2 Mhz).
 */
#define VAL_GPIOBCRL            0xAAB8B200      /*  PB7...PB0 */
#define VAL_GPIOBCRH            0xAA24348B      /* PB15...PB8 */
#define VAL_GPIOBODR            0x0000FFFF

/*
 * Port C setup. (only PC13..PC15 exist on 48-pin package)
 * PC13 - config0                   (digital input, no pull)
 * PC14 - SPI_CS1                   (output pushpull, 2 MHz)
 * PC15 - SPI_CS0                   (output pushpull, 2 MHz)
 */
#define VAL_GPIOCCRL            0x88888888      /*  PC7...PC0 */
#define VAL_GPIOCCRH            0x22488888      /* PC15...PC8 */
#define VAL_GPIOCODR            0x0000FFFF

/*
 * Port D setup.
 * Everything input with pull-up except:
 * PD0  - R_OUT_en                 (digital output, 2 Mhz) - keep low after power on
 * PD1  - L_OUT_en                 (digital output, 2 Mhz) - keep low after power on
 */
#define VAL_GPIODCRL            0x88888822      /*  PD7...PD0 */
#define VAL_GPIODCRH            0x88888888      /* PD15...PD8 */
#define VAL_GPIODODR            0x0000FFFC

/*
 * Port E setup.
 * Everything input with pull-up except:
 */
#define VAL_GPIOECRL            0x88888888      /*  PE7...PE0 */
#define VAL_GPIOECRH            0x88888888      /* PE15...PE8 */
#define VAL_GPIOEODR            0x0000FFFF

/*
 * USB bus activation macro, required by the USB driver.
 */
#define usb_lld_connect_bus(usbp)

/*
 * USB bus de-activation macro, required by the USB driver.
 */
#define usb_lld_disconnect_bus(usbp)

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
