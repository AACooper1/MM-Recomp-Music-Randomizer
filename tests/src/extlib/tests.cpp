#include <iostream>

#include "lib_recomp.hpp"

#include "tests.h"

extern Log logger;

fs::path testDataPath;

RECOMP_DLL_FUNC(launch_tests) 
{
    logger.set_log_level(LogLevel::LOG_DEV);

    sqlite3_config(SQLITE_CONFIG_SERIALIZED);

    testDataPath = RECOMP_ARG_STR(0);
    testDataPath = testDataPath.parent_path();
    testDataPath /= "test_data";

    fs::remove_all(testDataPath / "tests");

    Catch::Session session;
    Catch::ConfigData config;

    const char* argv[] = {"--order=decl"};
    const int argc = 1;

    session.applyCommandLine(argc, argv);

    int initializationSucceeded = session.run();

    RECOMP_RETURN(int, 0);
}

fs::path create_path(fs::path p)
{
    fs::create_directories(p);
    return p;
}


TEST_CASE("Catch2 Tests", "[meta]")
{
    int i = 0;
    REQUIRE( i == 0);
}

TEST_CASE("Database Initialization", "[Database]")
{
    fs::path testCasePath = create_path(testDataPath / "tests" / "2 - Database Initialization");
    SECTION("Connecting to nonexistent database creates a new one")
    {
        fs::path sectionPath = create_path(testCasePath / "1 - Not Exists");

        fs::remove(sectionPath / "music.db");
        fs::remove_all(sectionPath / "music");

        REQUIRE_FALSE(fs::exists(sectionPath / "music.db"));
        REQUIRE_FALSE(fs::exists(sectionPath / "music"));
        std::shared_ptr<Database> db = std::make_shared<Database>(sectionPath);
        db->init();
        
        REQUIRE(fs::exists(sectionPath / "music.db"));
        REQUIRE(fs::exists(sectionPath / "music"));
    }
    SECTION("Can connect to existing database path")
    {
        fs::path sectionPath = create_path(testCasePath / "2 - Exists");

        std::ofstream emptyDb(sectionPath / "music.db");
        emptyDb.close();

        REQUIRE(fs::exists(sectionPath / "music.db"));
        std::shared_ptr<Database> db = std::make_shared<Database>(sectionPath);
        db->init();

        REQUIRE(db->get_last_rc() == SQLITE_OK);
    }
}

TEST_CASE("Database Operations", "[Database]")
{
    fs::path testCasePath = create_path(testDataPath / "tests" / "3 - Database Operations");
    SECTION("Can add to track table")
    {
        fs::path sectionPath = create_path(testCasePath / "3. Can add");

        std::shared_ptr<Database> db = std::make_shared<Database>(sectionPath);
        db->init();
        std::shared_ptr<Track> track = std::make_shared<Track>("test_data/persistent/Mario Kart Wii - Moonview Highway.mmrs");
        db->add_song(track);

        REQUIRE(db->tables->track->check_exists(1));
    }
}