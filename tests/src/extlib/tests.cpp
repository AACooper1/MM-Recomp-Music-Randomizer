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

std::shared_ptr<Track> create_dummy_track(bool hasSeq, bool hasBank, int noSounds)
{
    std::chrono::system_clock::time_point time = std::chrono::system_clock::now();

    std::shared_ptr<Track> track = std::make_shared<Track>();
    std::ostringstream oss;
    oss << "Dummy Track " << time;

    track->name = oss.str();
    track->timestamp = 0;
    track->path = "DummyTrack";
    track->type = TrackType::UNKNOWN;
    (*track->categories)[1] = true;
    (*track->categories)[3] = true;
    (*track->categories)[5] = true;
    (*track->categories)[7] = true;
    (*track->categories)[9] = true;

    return track;
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
        fs::path sectionPath = create_path(testCasePath / "3. Can add to track table");

        std::shared_ptr<Database> db = std::make_shared<Database>(sectionPath);
        db->init();
        std::shared_ptr<Track> dummyTrack = create_dummy_track(false, false, 0);
        db->add_song(dummyTrack);

        REQUIRE(db->tables->track->check_exists(1));
    }
    SECTION("Selecting from track table yields accurate track", "[database]")
    {
        fs::path sectionPath = create_path(testCasePath / "4. Select from track table accurate");

        std::shared_ptr<Database> db = std::make_shared<Database>(sectionPath);
        db->init();
        std::shared_ptr<Track> dummyTrack = create_dummy_track(false, false, 0);
        db->add_song(dummyTrack);

        REQUIRE(db->tables->track->check_exists(1));
        
        std::shared_ptr<Track> fromDb = db->tables->track->select(1);

        REQUIRE(fromDb->name == dummyTrack->name);
        REQUIRE(fromDb->timestamp == dummyTrack->timestamp);
        REQUIRE(fromDb->path == dummyTrack->path);
        REQUIRE(*fromDb->categories == *dummyTrack->categories);
        REQUIRE(fromDb->bankNo == dummyTrack->bankNo);
        REQUIRE(*fromDb->formmask.states == *dummyTrack->formmask.states);
        REQUIRE(fromDb->formmask.cumulativeStates == dummyTrack->formmask.cumulativeStates);
    }
}