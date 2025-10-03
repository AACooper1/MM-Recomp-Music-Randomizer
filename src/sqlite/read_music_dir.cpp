#include "read_music_dir.h"
#include "sql_mmrs.h"

namespace fs = std::filesystem;

RECOMP_DLL_FUNC(read_music_directory)
{
    fs::path modDataPath = fs::path(RECOMP_ARG_STR(0));

    fs::path dbPath = modDataPath / "musicDB.db";
    sql_init(dbPath);


    int numTracks = 0;

    RECOMP_RETURN(int, numTracks);
}