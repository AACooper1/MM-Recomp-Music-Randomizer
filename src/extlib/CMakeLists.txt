target_sources(${TARGET_NAME} PUBLIC
    "sqlite3.c"
    "miniz.c"

    "lib_main.cpp"

    "database/database.cpp"
    "database/tracktable.cpp"
    "database/sequencetable.cpp"
    "database/banktable.cpp"
    "database/soundtable.cpp"
    
    "audiofile.cpp"
    "track.cpp"
    "logging.cpp"
    "util.cpp"
    "formmask.cpp"
)
