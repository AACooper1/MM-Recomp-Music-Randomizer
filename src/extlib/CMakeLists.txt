target_sources(${TARGET_NAME} PUBLIC
    "lib/sqlite3.c"
    "lib/miniz.c"

    "lib_main.cpp"

    "util/logging.cpp"
    "util/util.cpp"
    "util/thread.cpp"

    "database/database.cpp"
    "database/table.cpp"
    "database/tracktable.cpp"
    "database/sequencetable.cpp"
    "database/banktable.cpp"
    "database/soundtable.cpp"
    "database/relationtable.cpp"
    
    "fileparse/audiofile.cpp"
    "fileparse/track.cpp"
    "fileparse/formmask.cpp"
    
    "seed/seed.cpp"
    "seed/songforce_songtest.cpp"
    
    "ootrs/ootrs.cpp"
    "ootrs/decompress_rom.cpp"
)
