#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "modtrackdefs.h"
#include "logging.h"

RECOMP_IMPORT(".", bool read_oot_audiobin());
RECOMP_IMPORT(".", void get_oot_audiobin_headers(AudioTableHeader* soundTableHeader, AudioTableHeader* bankTableHeader));
RECOMP_IMPORT(".", void get_oot_audiobin_entries(AudioTableEntry* soundTableEntries, AudioTableEntry* bankTableEntries));
RECOMP_IMPORT(".", void get_oot_audiobin_data(void* dataAddr));

extern Logger logger;

bool prepare_oot_audiotables();

AudioTable* OoTSoundTable;
void* OoTSoundData;
AudioTable* OoTBankTable;
void* OoTBankData;

int OoTBanksAddedIdx[0x26];