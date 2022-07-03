/****************************************************************************************
* Include files
****************************************************************************************/
#include "boot.h"                                /* bootloader generic header          */
#include "led.h"                                 /* module header                      */
#include "stm32f1xx.h"                           /* STM32 registers and drivers        */
#include "stm32f1xx_ll_gpio.h"                   /* STM32 LL GPIO header               */


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
      LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
    }
    else
    {
      ledOn = BLT_FALSE;
      LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
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
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
} /*** end of LedBlinkExit ***/


/*********************************** end of led.c **************************************/
