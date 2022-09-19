#include "port.h"

#include "wideband_config.h"

#include "hal.h"
#include "hal_mfs.h"

// Storage

static const MFSConfig mfscfg1 = {
    .flashp           = (BaseFlash *)&EFLD1,
    .erased           = 0xFFFFFFFFU,
    .bank_size        = 4096U,
    .bank0_start      = 120U,
    .bank0_sectors    = 4U,
    .bank1_start      = 124U,
    .bank1_sectors    = 4U
};

static MFSDriver mfs1;

// Settings
static Configuration cfg;
#define MFS_CONFIGURATION_RECORD_ID     1

// Configuration defaults
void Configuration::LoadDefaults()
{
    CanIndexOffset = 0;

    /* Finaly */
    Tag = ExpectedTag;
}

int InitConfiguration()
{
    size_t size = GetConfiguratiuonSize();

    /* Starting EFL driver.*/
    eflStart(&EFLD1, NULL);

    mfsObjectInit(&mfs1);

    mfsStart(&mfs1, &mfscfg1);

    mfs_error_t err = mfsReadRecord(&mfs1, MFS_CONFIGURATION_RECORD_ID, &size, GetConfiguratiuonPtr());
    if ((err != MFS_NO_ERROR) || (size != GetConfiguratiuonSize() || !cfg.IsValid())) {
        /* load defaults */
        cfg.LoadDefaults();
    }

    return 0;
}

static Configuration c;

Configuration& GetConfiguration()
{
    return cfg;
}

void SetConfiguration(const Configuration& newConfig)
{
    cfg = newConfig;

    SaveConfiguration();
}

/* TS stuff */
void SaveConfiguration() {
    /* TODO: handle error */
    mfsWriteRecord(&mfs1, MFS_CONFIGURATION_RECORD_ID, GetConfiguratiuonSize(), GetConfiguratiuonPtr());
}

uint8_t *GetConfiguratiuonPtr()
{
    return (uint8_t *)&cfg;
}

size_t GetConfiguratiuonSize()
{
    return sizeof(cfg);
}

const char *getTsSignature() {
    return TS_SIGNATURE;
}
