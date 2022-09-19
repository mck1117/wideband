#include "port.h"

#include "wideband_config.h"

#include "hal.h"
#include "hal_mfs.h"

// Settings

static struct {
    uint8_t        pad[128];
} config;

const MFSConfig mfscfg1 = {
    .flashp           = (BaseFlash *)&EFLD1,
    .erased           = 0xFFFFFFFFU,
    .bank_size        = 4096U,
    .bank0_start      = 120U,
    .bank0_sectors    = 4U,
    .bank1_start      = 124U,
    .bank1_sectors    = 4U
};

MFSDriver mfs1;

int InitConfiguration()
{
    /* Starting EFL driver.*/
    eflStart(&EFLD1, NULL);

    mfsObjectInit(&mfs1);

    mfsStart(&mfs1, &mfscfg1);

    return 0;
}

static Configuration c;

Configuration& GetConfiguration()
{
    // TODO: implement me!
    return c;
}

void SetConfiguration(const Configuration& newConfig)
{
    // TODO: implement me!
}

/* TS stuff */
void SaveConfiguration() {
    // TODO: implement me!
}

uint8_t *GetConfiguratiuonPtr()
{
    return (uint8_t *)&config;
}

size_t GetConfiguratiuonSize()
{
    return sizeof(config);
}
