#include "global.h"
#include "ch.hpp" // chVTGetTimeStamp
#include <rusefi/rusefi_time_math.h>

efitick_t getTimeNowNt() {
    return chVTGetTimeStamp();
}

efitimeus_t getTimeNowUs() {
    return NT2US(getTimeNowNt());
}
