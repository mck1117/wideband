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
#define BOARD_ST_NUCLEO64_F103RB
/*
 * Board identifier.
 */
#define BOARD_NAME                  "rusEfi Wideband Controller"

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
 * PA0  - Ip_sense                  (analog in).
 * PA1  - Ip_dac (PWM)              (output pushpull, alternate, 2 MHz).
 * PA2  - Vm_sense                  (analog in).
 * PA3  - Un_sense                  (analog in).
 * PA4  - Vm                        (analog in, unused).
 * PA5  - Battery sense             (analog in).
 * PA6  - unused
 * PA7  - unused
 * PA8  - LED_GREEN                 (output pushpull, 2 MHz).
 * PA9  - UART_TX                   (output pushpull, alternate, 50 Mhz).
 * PA10 - UART_RX                   (digital input, alternate).
 * PA11 - CAN_RX                    (digital input, alternate).
 * PA12 - CAN_TX                    (output pushpull, alternate, 50 Mhz).
 * PA13 - SWDIO                     (digital input)
 * PA14 - SWCLK                     (digital input)
 * PA15 - DISP0 - unused
 */
#define VAL_GPIOACRL            0x880000B0      /*  PA7...PA0 */
#define VAL_GPIOACRH            0x888B88B2      /* PA15...PA8 */
#define VAL_GPIOAODR            0x0000FFFF

/*
 * Port B setup.
 * PB0..PB2 - config0..config2 - unused
 * PB3  - SWO                       (digital input)
 * PB4..PB5 - DISP1..DISP2 - unused
 * PB6  - heater_pwm                (output pushpull, alternate, 2 MHz).
 * PB7..PB9 - DISP4..DISP6 - unused
 * PB10 - config3 - unused
 * PB12 - Nernst ESR driver (GPIO)  (output pushpull, 50 Mhz)
 * PB13 - Blue LED                  (output pushpull, 2 MHz)
 * PB14 - PWMout2                   (output pushpull, alternate, 50 Mhz).
 * PB15 - PWMout1                   (output pushpull, alternate, 50 Mhz).
 */
#define VAL_GPIOBCRL            0x8A888888      /*  PB7...PB0 */
#define VAL_GPIOBCRH            0xBB238888      /* PB15...PB8 */
#define VAL_GPIOBODR            0x0000FFFF

/*
 * Port C setup.
 * Everything input with pull-up
 * PC13 - heater_pwm on old rev, should not conflict in input mode
 */
#define VAL_GPIOCCRL            0x88888888      /*  PC7...PC0 */
#define VAL_GPIOCCRH            0x88888888      /* PC15...PC8 */
#define VAL_GPIOCODR            0x0000FFFF

/*
 * Port D setup.
 * Everything input with pull-up except:
 */
#define VAL_GPIODCRL            0x88888888      /*  PD7...PD0 */
#define VAL_GPIODCRH            0x88888888      /* PD15...PD8 */
#define VAL_GPIODODR            0x0000FFFF

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
