/****************************************************************************************
* Include files
****************************************************************************************/
#include "boot.h"                                /* bootloader generic header          */
#include "led.h"                                 /* module header                      */
#include "stm32f1xx.h"                           /* STM32 registers and drivers        */
#include "stm32f1xx_ll_gpio.h"                   /* STM32 LL GPIO header               */

#include "../io/io_pins.h"

/****************************************************************************************
* Local data declarations
****************************************************************************************/
/** \brief Holds the desired LED blink interval time. */
static blt_int16u ledBlinkIntervalMs;


/************************************************************************************//**
** \brief     Initializes the LED blink driver.
** \param     interval_ms Specifies the desired LED blink interval time in milliseconds.
** \return    none.
**
****************************************************************************************/
void LedBlinkInit(blt_int16u interval_ms)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct;

  /* Configure GPIO pin for the LED. */
  GPIO_InitStruct.Pin = LL_LED_BLUE_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(LED_BLUE_PORT, &GPIO_InitStruct);
  LL_GPIO_ResetOutputPin(LED_BLUE_PORT, LL_LED_BLUE_PIN);

  /* store the interval time between LED toggles */
  ledBlinkIntervalMs = interval_ms;
} /*** end of LedBlinkInit ***/


/************************************************************************************//**
** \brief     Task function for blinking the LED as a fixed timer interval.
** \return    none.
**
****************************************************************************************/
void LedBlinkTask(void)
{
  static blt_bool ledOn = BLT_FALSE;
  static blt_int32u nextBlinkEvent = 0;

  /* check for blink event */
  if (TimerGet() >= nextBlinkEvent)
  {
    /* toggle the LED state */
    if (ledOn == BLT_FALSE)
    {
      ledOn = BLT_TRUE;
      LL_GPIO_SetOutputPin(LED_BLUE_PORT,LL_LED_BLUE_PIN);
    }
    else
    {
      ledOn = BLT_FALSE;
      LL_GPIO_ResetOutputPin(LED_BLUE_PORT, LL_LED_BLUE_PIN);
    }
    /* schedule the next blink event */
    nextBlinkEvent = TimerGet() + ledBlinkIntervalMs;
  }
} /*** end of LedBlinkTask ***/


/************************************************************************************//**
** \brief     Cleans up the LED blink driver. This is intended to be used upon program
**            exit.
** \return    none.
**
****************************************************************************************/
void LedBlinkExit(void)
{
  /* turn the LED off */
  LL_GPIO_ResetOutputPin(LED_BLUE_PORT, LL_LED_BLUE_PIN);
} /*** end of LedBlinkExit ***/


/*********************************** end of led.c **************************************/
