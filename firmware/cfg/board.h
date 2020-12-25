/*
    ChibiOS - Copyright (C) 2006..2020 Giovanni Di Sirio

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

/*
 * This file has been automatically generated using ChibiStudio board
 * generator plugin. Do not edit manually.
 */

#ifndef BOARD_H
#define BOARD_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/


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

#define STM32_LSEDRV                (3U << 3U)

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                0U
#endif

/*
 * MCU type as defined in the ST header.
 */
#define STM32F042x6

/*
 * IO pins assignments.
 */
#define GPIOA_PIN0                  0U
#define GPIOA_PIN1                  1U
#define GPIOA_PIN2                  2U
#define GPIOA_PIN3                  3U
#define GPIOA_PIN4                  4U
#define GPIOA_PIN5                  5U
#define GPIOA_PIN6                  6U
#define GPIOA_PIN7                  7U
#define GPIOA_PIN8                  8U
#define GPIOA_PIN9                  9U
#define GPIOA_PIN10                 10U
#define GPIOA_PIN11                 11U
#define GPIOA_PIN12                 12U
#define GPIOA_SWDIO                 13U
#define GPIOA_SWCLK                 14U
#define GPIOA_PIN15                 15U

#define GPIOB_PIN0                  0U
#define GPIOB_PIN1                  1U
#define GPIOB_PIN2                  2U
#define GPIOB_PIN3                  3U
#define GPIOB_PIN4                  4U
#define GPIOB_PIN5                  5U
#define GPIOB_PIN6                  6U
#define GPIOB_PIN7                  7U
#define GPIOB_PIN8                  8U
#define GPIOB_PIN9                  9U
#define GPIOB_PIN10                 10U
#define GPIOB_PIN11                 11U
#define GPIOB_PIN12                 12U
#define GPIOB_PIN13                 13U
#define GPIOB_PIN14                 14U
#define GPIOB_PIN15                 15U

#define GPIOC_PIN0                  0U
#define GPIOC_PIN1                  1U
#define GPIOC_PIN2                  2U
#define GPIOC_PIN3                  3U
#define GPIOC_PIN4                  4U
#define GPIOC_PIN5                  5U
#define GPIOC_PIN6                  6U
#define GPIOC_PIN7                  7U
#define GPIOC_PIN8                  8U
#define GPIOC_PIN9                  9U
#define GPIOC_PIN10                 10U
#define GPIOC_PIN11                 11U
#define GPIOC_PIN12                 12U
#define GPIOC_PIN13                 13U
#define GPIOC_PIN14                 14U
#define GPIOC_PIN15                 15U

#define GPIOF_PIN0                0U
#define GPIOF_PIN1                1U
#define GPIOF_PIN2                  2U
#define GPIOF_PIN3                  3U
#define GPIOF_PIN4                  4U
#define GPIOF_PIN5                  5U
#define GPIOF_PIN6                  6U
#define GPIOF_PIN7                  7U
#define GPIOF_PIN8                  8U
#define GPIOF_PIN9                  9U
#define GPIOF_PIN10                 10U
#define GPIOF_PIN11                 11U
#define GPIOF_PIN12                 12U
#define GPIOF_PIN13                 13U
#define GPIOF_PIN14                 14U
#define GPIOF_PIN15                 15U

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2U))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2U))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2U))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2U))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_VERYLOW(n)       (0U << ((n) * 2U))
#define PIN_OSPEED_LOW(n)           (1U << ((n) * 2U))
#define PIN_OSPEED_MEDIUM(n)        (2U << ((n) * 2U))
#define PIN_OSPEED_HIGH(n)          (3U << ((n) * 2U))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2U))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2U))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2U))
#define PIN_AFIO_AF(n, v)           ((v) << (((n) % 8U) * 4U))

/*
 * GPIOA setup:
 *
 * PA0  - Un_sense                  (analog in).
 * PA1  - Vm_sense                  (analog in).
 * PA2  - Ip_sense                  (analog in).
 * PA6  - Ip_dac (PWM)              (output pushpull, alternate 1).
 * PA7  - Heater PWM                (output pushpull, alternate 2).
 * PA9  - UART TX                   (alternate 1).
 * PA10 - UART RX                   (alternate 1).
 * PA11 - CAN RX                    (alternate 4).
 * PA12 - CAN TX                    (alternate 4).
 * PA13 - SWDIO                     (alternate 0).
 * PA14 - SWCLK                     (alternate 0).
 * PA15 - SPI NSS                   (alternate 0).
 */
#define VAL_GPIOA_MODER             (PIN_MODE_ANALOG(GPIOA_PIN0) |         \
                                     PIN_MODE_ANALOG(GPIOA_PIN1) |         \
                                     PIN_MODE_ANALOG(GPIOA_PIN2) |     \
                                     PIN_MODE_INPUT(GPIOA_PIN3) |         \
                                     PIN_MODE_INPUT(GPIOA_PIN4) |         \
                                     PIN_MODE_INPUT(GPIOA_PIN5) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_PIN6) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_PIN7) |         \
                                     PIN_MODE_INPUT(GPIOA_PIN8) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_PIN9) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_PIN10) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_PIN11) |        \
                                     PIN_MODE_ALTERNATE(GPIOA_PIN12) |         \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDIO) |      \
                                     PIN_MODE_ALTERNATE(GPIOA_SWCLK) |      \
                                     PIN_MODE_ALTERNATE(GPIOA_PIN15))
#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_PIN0) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN1) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN2) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN3) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN4) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN5) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN6) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN7) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN8) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN9) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN10) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN11) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN12) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWDIO) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWCLK) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN15))
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_LOW(GPIOA_PIN0) |        \
                                     PIN_OSPEED_LOW(GPIOA_PIN1) |        \
                                     PIN_OSPEED_LOW(GPIOA_PIN2) |         \
                                     PIN_OSPEED_LOW(GPIOA_PIN3) |         \
                                     PIN_OSPEED_LOW(GPIOA_PIN4) |        \
                                     PIN_OSPEED_LOW(GPIOA_PIN5) |         \
                                     PIN_OSPEED_LOW(GPIOA_PIN6) |        \
                                     PIN_OSPEED_LOW(GPIOA_PIN7) |        \
                                     PIN_OSPEED_LOW(GPIOA_PIN8) |        \
                                     PIN_OSPEED_HIGH(GPIOA_PIN9) |        \
                                     PIN_OSPEED_HIGH(GPIOA_PIN10) |        \
                                     PIN_OSPEED_HIGH(GPIOA_PIN11) |       \
                                     PIN_OSPEED_HIGH(GPIOA_PIN12) |        \
                                     PIN_OSPEED_HIGH(GPIOA_SWDIO) |         \
                                     PIN_OSPEED_HIGH(GPIOA_SWCLK) |         \
                                     PIN_OSPEED_LOW(GPIOA_PIN15))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_PULLUP(GPIOA_PIN0) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN1) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN2) |     \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN3) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN4) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN5) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN6) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN7) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN8) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN9) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN10) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN11) |      \
                                     PIN_PUPDR_PULLUP(GPIOA_PIN12) |       \
                                     PIN_PUPDR_PULLUP(GPIOA_SWDIO) |        \
                                     PIN_PUPDR_PULLDOWN(GPIOA_SWCLK) |      \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN15))
#define VAL_GPIOA_ODR               (PIN_ODR_LOW(GPIOA_PIN0) |           \
                                     PIN_ODR_LOW(GPIOA_PIN1) |           \
                                     PIN_ODR_LOW(GPIOA_PIN2) |           \
                                     PIN_ODR_LOW(GPIOA_PIN3) |           \
                                     PIN_ODR_LOW(GPIOA_PIN4) |           \
                                     PIN_ODR_LOW(GPIOA_PIN5) |            \
                                     PIN_ODR_LOW(GPIOA_PIN6) |           \
                                     PIN_ODR_LOW(GPIOA_PIN7) |           \
                                     PIN_ODR_LOW(GPIOA_PIN8) |           \
                                     PIN_ODR_LOW(GPIOA_PIN9) |           \
                                     PIN_ODR_LOW(GPIOA_PIN10) |           \
                                     PIN_ODR_LOW(GPIOA_PIN11) |          \
                                     PIN_ODR_LOW(GPIOA_PIN12) |           \
                                     PIN_ODR_LOW(GPIOA_SWDIO) |            \
                                     PIN_ODR_LOW(GPIOA_SWCLK) |            \
                                     PIN_ODR_LOW(GPIOA_PIN15))
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_PIN0, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN1, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN2, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN3, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN4, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN5, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN6, 1U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN7, 2U))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_PIN8, 1U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN9, 1U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN10, 0U) |        \
                                     PIN_AFIO_AF(GPIOA_PIN11, 4U) |       \
                                     PIN_AFIO_AF(GPIOA_PIN12, 4U) |        \
                                     PIN_AFIO_AF(GPIOA_SWDIO, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_SWCLK, 0U) |         \
                                     PIN_AFIO_AF(GPIOA_PIN15, 0U))

/*
 * GPIOB setup:
 *
 * PB5  - Blue LED                  (output pushpull)
 * PB6  - Green LED                 (output pushpull)
 * PB7  - Nernst ESR driver (GPIO)  (output pushpull)
 */
#define VAL_GPIOB_MODER             (PIN_MODE_INPUT(GPIOB_PIN0) |         \
                                     PIN_MODE_INPUT(GPIOB_PIN1) |         \
                                     PIN_MODE_INPUT(GPIOB_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN3) |       \
                                     PIN_MODE_INPUT(GPIOB_PIN4) |        \
                                     PIN_MODE_OUTPUT(GPIOB_PIN5) |        \
                                     PIN_MODE_OUTPUT(GPIOB_PIN6) |         \
                                     PIN_MODE_OUTPUT(GPIOB_PIN7) |         \
                                     PIN_MODE_INPUT(GPIOB_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN15))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOB_PIN0) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN1) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN3) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN4) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN5) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN6) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN7) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN15))
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_LOW(GPIOB_PIN0) |        \
                                     PIN_OSPEED_LOW(GPIOB_PIN1) |        \
                                     PIN_OSPEED_LOW(GPIOB_PIN2) |          \
                                     PIN_OSPEED_LOW(GPIOB_PIN3) |       \
                                     PIN_OSPEED_LOW(GPIOB_PIN4) |       \
                                     PIN_OSPEED_LOW(GPIOB_PIN5) |       \
                                     PIN_OSPEED_LOW(GPIOB_PIN6) |        \
                                     PIN_OSPEED_LOW(GPIOB_PIN7) |        \
                                     PIN_OSPEED_LOW(GPIOB_PIN8) |          \
                                     PIN_OSPEED_LOW(GPIOB_PIN9) |          \
                                     PIN_OSPEED_LOW(GPIOB_PIN10) |         \
                                     PIN_OSPEED_LOW(GPIOB_PIN11) |         \
                                     PIN_OSPEED_LOW(GPIOB_PIN12) |         \
                                     PIN_OSPEED_LOW(GPIOB_PIN13) |         \
                                     PIN_OSPEED_LOW(GPIOB_PIN14) |         \
                                     PIN_OSPEED_LOW(GPIOB_PIN15))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_PULLUP(GPIOB_PIN0) |       \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN1) |       \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN3) |    \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN4) |      \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN5) |      \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN6) |       \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN7) |       \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOB_PIN15))
#define VAL_GPIOB_ODR               (PIN_ODR_LOW(GPIOB_PIN0) |           \
                                     PIN_ODR_LOW(GPIOB_PIN1) |           \
                                     PIN_ODR_LOW(GPIOB_PIN2) |             \
                                     PIN_ODR_LOW(GPIOB_PIN3) |           \
                                     PIN_ODR_LOW(GPIOB_PIN4) |          \
                                     PIN_ODR_LOW(GPIOB_PIN5) |          \
                                     PIN_ODR_LOW(GPIOB_PIN6) |           \
                                     PIN_ODR_LOW(GPIOB_PIN7) |           \
                                     PIN_ODR_LOW(GPIOB_PIN8) |             \
                                     PIN_ODR_LOW(GPIOB_PIN9) |             \
                                     PIN_ODR_LOW(GPIOB_PIN10) |            \
                                     PIN_ODR_LOW(GPIOB_PIN11) |            \
                                     PIN_ODR_LOW(GPIOB_PIN12) |            \
                                     PIN_ODR_LOW(GPIOB_PIN13) |            \
                                     PIN_ODR_LOW(GPIOB_PIN14) |            \
                                     PIN_ODR_LOW(GPIOB_PIN15))
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_PIN0, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_PIN1, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_PIN3, 0U) |       \
                                     PIN_AFIO_AF(GPIOB_PIN4, 0U) |       \
                                     PIN_AFIO_AF(GPIOB_PIN5, 0U) |       \
                                     PIN_AFIO_AF(GPIOB_PIN6, 0U) |        \
                                     PIN_AFIO_AF(GPIOB_PIN7, 0U))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOB_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOB_PIN15, 0U))

/*
 * GPIOC setup:
 *
 * PC0  - PIN0                      (input pullup).
 * PC1  - PIN1                      (input pullup).
 * PC2  - PIN2                      (input pullup).
 * PC3  - PIN3                      (input pullup).
 * PC4  - PIN4                      (input pullup).
 * PC5  - PIN5                      (input pullup).
 * PC6  - PIN6                      (input pullup).
 * PC7  - PIN7                      (input pullup).
 * PC8  - PIN8                      (input pullup).
 * PC9  - PIN9                      (input pullup).
 * PC10 - PIN10                     (input pullup).
 * PC11 - PIN11                     (input pullup).
 * PC12 - PIN12                     (input pullup).
 * PC13 - PIN13                     (input pullup).
 * PC14 - PIN14                     (input pullup).
 * PC15 - PIN15                     (input pullup).
 */
#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_PIN0) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN1) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOC_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOC_PIN15))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_PIN0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN15))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_LOW(GPIOC_PIN0) |          \
                                     PIN_OSPEED_LOW(GPIOC_PIN1) |          \
                                     PIN_OSPEED_LOW(GPIOC_PIN2) |          \
                                     PIN_OSPEED_LOW(GPIOC_PIN3) |          \
                                     PIN_OSPEED_LOW(GPIOC_PIN4) |          \
                                     PIN_OSPEED_LOW(GPIOC_PIN5) |          \
                                     PIN_OSPEED_LOW(GPIOC_PIN6) |          \
                                     PIN_OSPEED_LOW(GPIOC_PIN7) |          \
                                     PIN_OSPEED_LOW(GPIOC_PIN8) |          \
                                     PIN_OSPEED_LOW(GPIOC_PIN9) |          \
                                     PIN_OSPEED_LOW(GPIOC_PIN10) |         \
                                     PIN_OSPEED_LOW(GPIOC_PIN11) |         \
                                     PIN_OSPEED_LOW(GPIOC_PIN12) |         \
                                     PIN_OSPEED_LOW(GPIOC_PIN13) |         \
                                     PIN_OSPEED_LOW(GPIOC_PIN14) |         \
                                     PIN_OSPEED_LOW(GPIOC_PIN15))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLUP(GPIOC_PIN0) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN1) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOC_PIN15))
#define VAL_GPIOC_ODR               (PIN_ODR_HIGH(GPIOC_PIN0) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN1) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOC_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOC_PIN15))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_PIN0, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN1, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN7, 0U))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOC_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOC_PIN15, 0U))

/*
 * GPIOF setup:
 *
 * none
 */
#define VAL_GPIOF_MODER             (PIN_MODE_INPUT(GPIOF_PIN0) |         \
                                     PIN_MODE_INPUT(GPIOF_PIN1) |         \
                                     PIN_MODE_INPUT(GPIOF_PIN2) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN3) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN4) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN5) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN6) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN7) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN9) |           \
                                     PIN_MODE_INPUT(GPIOF_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN13) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN14) |          \
                                     PIN_MODE_INPUT(GPIOF_PIN15))
#define VAL_GPIOF_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOF_PIN0) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN1) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN13) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN14) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOF_PIN15))
#define VAL_GPIOF_OSPEEDR           (PIN_OSPEED_LOW(GPIOF_PIN0) |        \
                                     PIN_OSPEED_LOW(GPIOF_PIN1) |        \
                                     PIN_OSPEED_LOW(GPIOF_PIN2) |          \
                                     PIN_OSPEED_LOW(GPIOF_PIN3) |          \
                                     PIN_OSPEED_LOW(GPIOF_PIN4) |          \
                                     PIN_OSPEED_LOW(GPIOF_PIN5) |          \
                                     PIN_OSPEED_LOW(GPIOF_PIN6) |          \
                                     PIN_OSPEED_LOW(GPIOF_PIN7) |          \
                                     PIN_OSPEED_LOW(GPIOF_PIN8) |          \
                                     PIN_OSPEED_LOW(GPIOF_PIN9) |          \
                                     PIN_OSPEED_LOW(GPIOF_PIN10) |         \
                                     PIN_OSPEED_LOW(GPIOF_PIN11) |         \
                                     PIN_OSPEED_LOW(GPIOF_PIN12) |         \
                                     PIN_OSPEED_LOW(GPIOF_PIN13) |         \
                                     PIN_OSPEED_LOW(GPIOF_PIN14) |         \
                                     PIN_OSPEED_LOW(GPIOF_PIN15))
#define VAL_GPIOF_PUPDR             (PIN_PUPDR_PULLUP(GPIOF_PIN0) |       \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN1) |       \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN2) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN3) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN4) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN5) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN6) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN7) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN8) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN9) |         \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN10) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN11) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN12) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN13) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN14) |        \
                                     PIN_PUPDR_PULLUP(GPIOF_PIN15))
#define VAL_GPIOF_ODR               (PIN_ODR_HIGH(GPIOF_PIN0) |           \
                                     PIN_ODR_HIGH(GPIOF_PIN1) |           \
                                     PIN_ODR_HIGH(GPIOF_PIN2) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN3) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN4) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN5) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN6) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN7) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN8) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN9) |             \
                                     PIN_ODR_HIGH(GPIOF_PIN10) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN11) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN12) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN13) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN14) |            \
                                     PIN_ODR_HIGH(GPIOF_PIN15))
#define VAL_GPIOF_AFRL              (PIN_AFIO_AF(GPIOF_PIN0, 0U) |        \
                                     PIN_AFIO_AF(GPIOF_PIN1, 0U) |        \
                                     PIN_AFIO_AF(GPIOF_PIN2, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN3, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN4, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN5, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN6, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN7, 0U))
#define VAL_GPIOF_AFRH              (PIN_AFIO_AF(GPIOF_PIN8, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN9, 0U) |          \
                                     PIN_AFIO_AF(GPIOF_PIN10, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN11, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN12, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN13, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN14, 0U) |         \
                                     PIN_AFIO_AF(GPIOF_PIN15, 0U))

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* BOARD_H */
