#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "modtrackdefs.h"
#include "logging.h"

#define OOT_AUDIOBANK_SIZE 0x1C5A0
#define OOT_AUDIOTABLE_SIZE 0x460AD0

RECOMP_IMPORT(".", bool read_oot_audiobin());
RECOMP_IMPORT(".", void get_oot_audiobin_headers(AudioTableHeader* soundTableHeader, AudioTableHeader* bankTableHeader));
RECOMP_IMPORT(".", void get_oot_audiobin_entries(AudioTableEntry* soundTableEntries, AudioTableEntry* bankTableEntries));
RECOMP_IMPORT(".", bool get_oot_bank_data(void* dest, int blob_index, int size));
RECOMP_IMPORT(".", bool get_oot_sound_data(void* dest, int blob_index, int size));

extern Logger logger;

bool prepare_oot_audiotables();
int prepare_oot_bank(cTrack* track);

AudioTable* OoTSoundTable;
AudioTable* OoTBankTable;

int OoTBanksAddedIdx[0x26];