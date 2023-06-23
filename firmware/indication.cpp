#include "ch.h"
#include "hal.h"

#include "fault.h"
#include "heater_control.h"

#include "indication.h"

#include "wideband_config.h"

using namespace wbo;

#ifdef ADVANCED_INDICATION

#define LED_BLINK_FAST			(50)
#define LED_BLINK_MEDIUM		(300)
#define LED_BLINK_SLOW			(700)
#define LED_OFF_TIME			(2000)

struct indicationThreadData {
	uint32_t idx;
	ioline_t line;
};

indicationThreadData indData[] = {
	{0, PAL_LINE(LED_GREEN_PORT, LED_GREEN_PIN)},
#ifdef LED_R_GREEN_PORT
	{1, PAL_LINE(LED_R_GREEN_PORT, LED_R_GREEN_PIN)},
#endif
};

static THD_WORKING_AREA(waIndicationThread, 128);
#ifdef LED_R_GREEN_PORT
static THD_WORKING_AREA(waIndicationThread2, 128);
#endif

static void IndicationThread(void *ptr)
{
    chRegSetThreadName("Indication");
    indicationThreadData *data = (indicationThreadData *)ptr;

    while(true)
    {
        auto fault = GetCurrentFault(data->idx);

        if (fault == Fault::None)
        {
            // Green is blinking
            palToggleLine(data->line);

            // Slow blink if closed loop, fast if not
            chThdSleepMilliseconds(GetHeaterController(data->idx).IsRunningClosedLoop() ? LED_BLINK_SLOW : LED_BLINK_FAST);
        }
        else
        {
            // Start from off state
            palClearLine(data->line);

            // Blink out the error code
            for (int i = 0; i < 2 * static_cast<int>(fault); i++)
            {
                // Blue is blinking
                palToggleLine(data->line);

                // fast blink
                chThdSleepMilliseconds(LED_BLINK_MEDIUM);
            }

            chThdSleepMilliseconds(LED_OFF_TIME);
        }
    }
}

void InitIndication()
{
    chThdCreateStatic(waIndicationThread, sizeof(waIndicationThread), NORMALPRIO, IndicationThread, &indData[0]);
#ifdef LED_R_GREEN_PORT
    chThdCreateStatic(waIndicationThread2, sizeof(waIndicationThread2), NORMALPRIO, IndicationThread, &indData[1]);
#endif
}

/* Can be calles from two TS channels. */
void onDataArrived(bool status)
{
	/* to avoid blinking when two TS channels available and only one is communicating
	 * another one will call this function with status = false every timeout */
	/* TODO: total crap, rework */
	static int filter = 0;

	if (status) {
		if (filter < 3) {
			filter = 3;
		}
	} else {
		if (filter > 0) {
			filter--;
		}
	}

	if (filter) {
		palSetPad(LED_BLUE_PORT, LED_BLUE_PIN);
	} else {
		palClearPad(LED_BLUE_PORT, LED_BLUE_PIN);
	}
}

#else

void InitIndication()
{
}

void onDataArrived(bool)
{
}

#endif