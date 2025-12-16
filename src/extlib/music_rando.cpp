// Main functions of mod/native interaction.

#include "lib_recomp.hpp"

#include "database.h"

RECOMP_DLL_FUNC(music_rando_init)
{
    std::u8string modPath = RECOMP_ARG_U8STR(0);
    fs::path dbPath = fs::path(modPath).parent_path();

    fs::create_directory(dbPath / "mod_data");
    dbPath /= "mod_data";
    dbPath /= "musicDB.db";
    
    Database* db = Database::get_db(dbPath);
}