#pragma once

#define LED_BLUE_PORT GPIOB
#define LED_BLUE_PIN 13
#define LL_LED_BLUE_PIN LL_GPIO_PIN_13

#define LED_GREEN_PORT GPIOA
#define LED_GREEN_PIN 8

// Communication - UART
#define UART_GPIO_PORT				GPIOA
#define LL_UART_TX_PIN				LL_GPIO_PIN_9
#define LL_UART_RX_PIN				LL_GPIO_PIN_10

// Communication - CAN1
#define CAN_GPIO_PORT				GPIOA
#define LL_CAN_TX_PIN				LL_GPIO_PIN_12
#define LL_CAN_RX_PIN				LL_GPIO_PIN_11

// LSU 4.2 - 6.8K
#define NERNST_42_ESR_DRIVER_PORT   GPIOB
#define NERNST_42_ESR_DRIVER_PIN    12

// LSU 4.9 - 22K
#define NERNST_ESR_DRIVER_PORT      GPIOB
#define NERNST_ESR_DRIVER_PIN       11

// LSU ADV - 47K
#define NERNST_ADV_ESR_DRIVER_PORT  GPIOB
#define NERNST_ADV_ESR_DRIVER_PIN   10

// PB6 TIM4_CH1
#define HEATER_PWM_DEVICE PWMD4
#define HEATER_PWM_CHANNEL_0 0

// PA1 TIM2_CH2
#define PUMP_DAC_PWM_DEVICE PWMD2
#define PUMP_DAC_PWM_CHANNEL_0 1

// TIM1 - DAC for AUX outputs
#define AUXOUT_DAC_PWM_DEVICE       PWMD1
// PB14 - TIM1_CH2N
#define AUXOUT_DAC_PWM_CHANNEL_0    1
// PB15 - TIM1_CH3N
#define AUXOUT_DAC_PWM_CHANNEL_1    2

#define ID_SEL1_PORT                GPIOC
#define ID_SEL1_PIN                 13

#define ID_SEL2_PORT                GPIOC
#define ID_SEL2_PIN                 14
