#pragma once

#include <stdint.h>

#include <rusefi/fragments.h>

#include "wideband_config.h"

/* +0 offset */
struct livedata_common_s {
	union {
		struct {
			float vbatt;
		};
		uint8_t pad0[32];
	};
};

/* +32 offset */
struct livedata_afr_s {
	union {
		struct {
			// lambda also displayed by TS as AFR, same data with different scale factor
			float lambda;
			float temperature;
			float nernstVoltage;
			float pumpCurrentTarget;
			float pumpCurrentMeasured;
			float heaterDuty;
			uint8_t fault; // See wbo::Fault
			uint8_t heaterState;
		} __attribute__((packed));
		uint8_t pad[32];
	};
};

/* update functions */
void SamplingUpdateLiveData();
