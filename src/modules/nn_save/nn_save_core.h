#pragma once
#include "systemtypes.h"
#include "modules/coreinit/coreinit_fs.h"

using SaveStatus = FSStatus;

SaveStatus
SAVEInit();

void
SAVEShutdown();

SaveStatus
SAVEInitSaveDir(uint8_t userID);

SaveStatus
SAVEGetSharedDataTitlePath(uint64_t titleID, const char *dir, char *buffer, uint32_t bufferSize);
