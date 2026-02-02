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

std::shared_ptr<Bank> create_dummy_bank()
{
    std::vector<char> dataBuf;
    for (char i = 99; i >= 0; i--) { dataBuf.push_back(i); }
    std::shared_ptr<Bank> dummyBank = std::make_shared<Bank>(dataBuf, false);
    dummyBank->header = std::make_shared<std::vector<char>>(0x08, 0x02);

    return dummyBank;
}

std::shared_ptr<Sequence> create_dummy_sequence()
{
    std::vector<char> dataBuf;
    for (char i = 0; i < 99; i++) { dataBuf.push_back(i); }
    std::shared_ptr<Sequence> dummySeq = std::make_shared<Sequence>(dataBuf);

    return dummySeq;
}

std::shared_ptr<Sound> create_dummy_sound(u32 uniqueId)
{
    std::vector<char> dataBuf = std::vector<char>(0x99, 99);
    std::shared_ptr<Sound> dummySound = std::make_shared<Sound>(dataBuf, uniqueId);

    return dummySound;
}

std::shared_ptr<Track> create_dummy_track(bool hasSeq, bool hasBank, int noSounds)
{
    std::chrono::system_clock::time_point time = std::chrono::system_clock::now();

    std::shared_ptr<Track> track = std::make_shared<Track>();
    std::ostringstream oss;
    oss << "Dummy Track " << time;

    track->name = oss.str();
    track->timestamp = 0;
    track->path = oss.str();
    track->type = TrackType::UNKNOWN;
    (*track->categories)[1] = true;
    (*track->categories)[3] = true;
    (*track->categories)[5] = true;
    (*track->categories)[7] = true;
    (*track->categories)[9] = true;

    if (hasSeq)
    {
        track->sequence = create_dummy_sequence();
    }
    if (hasBank)
    {
        track->bank = create_dummy_bank();
    }
    for (int i = 0; i < noSounds; i++)
    {
        track->sounds.push_back(create_dummy_sound(i));
    }

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
    SECTION("Adding", "[Database]")
    {
        fs::path sectionPath = create_path(testCasePath / "3. Can add to track table");

        std::shared_ptr<Database> db = std::make_shared<Database>(sectionPath);
        db->init();
        

        SECTION("Can add to track table")
        {
            std::shared_ptr<Track> dummyTrack = create_dummy_track(false, false, 0);
            db->add_song(dummyTrack);

            REQUIRE(db->tables->track->check_exists(1));
            REQUIRE_FALSE(db->tables->track->check_exists(2));
        }
        SECTION("Selecting from track table yields accurate track", "[database]")
        {
            std::shared_ptr<Track> dummyTrack = create_dummy_track(false, false, 0);
            db->add_song(dummyTrack);

            std::shared_ptr<Track> fromDb = db->tables->track->select(dummyTrack->databaseIndex);

            REQUIRE(fromDb->name == dummyTrack->name);
            REQUIRE(fromDb->timestamp == dummyTrack->timestamp);
            REQUIRE(fromDb->path == dummyTrack->path);
            REQUIRE(*fromDb->categories == *dummyTrack->categories);
            REQUIRE(fromDb->bankNo == dummyTrack->bankNo);
            REQUIRE(*fromDb->formmask.states == *dummyTrack->formmask.states);
            REQUIRE(fromDb->formmask.cumulativeStates == dummyTrack->formmask.cumulativeStates);
        }
        
        SECTION("Adding track with seq adds to seq table")
        {
            std::shared_ptr<Track> seqBankDummyTrack = create_dummy_track(true, true, 0);
            db->add_song(seqBankDummyTrack);
            REQUIRE(db->tables->track->check_exists(seqBankDummyTrack->databaseIndex));

            std::shared_ptr<Track> dbTrack = db->tables->track->select(seqBankDummyTrack->databaseIndex);
            REQUIRE(dbTrack->name == seqBankDummyTrack->name);

            REQUIRE(db->tables->seq->check_exists(1));

            std::shared_ptr<Sequence> dbSeq = db->tables->seq->select(seqBankDummyTrack->sequence->databaseIndex);
            
            REQUIRE(dbSeq->size == seqBankDummyTrack->sequence->size);
            REQUIRE(*dbSeq->data == *seqBankDummyTrack->sequence->data);
        }
        SECTION("track_to_seq properly relates")
        {
            std::shared_ptr<Track> seqBankDummyTrack = create_dummy_track(true, true, 0);
            db->add_song(seqBankDummyTrack);
            REQUIRE(db->tables->relation.track_to_seq->select(seqBankDummyTrack->databaseIndex) == seqBankDummyTrack->sequence->databaseIndex);
        }
        SECTION("Adding track with bank adds to bank table")
        {
            std::shared_ptr<Track> seqBankDummyTrack = create_dummy_track(true, true, 0);
            db->add_song(seqBankDummyTrack);
            REQUIRE(db->tables->track->check_exists(seqBankDummyTrack->databaseIndex));

            std::shared_ptr<Track> dbTrack = db->tables->track->select(seqBankDummyTrack->databaseIndex);
            REQUIRE(dbTrack->name == seqBankDummyTrack->name);

            REQUIRE(db->tables->bank->check_exists(seqBankDummyTrack->bank->databaseIndex));

            std::shared_ptr<Bank> dbBank = db->tables->bank->select(seqBankDummyTrack->bank->databaseIndex);
            
            REQUIRE(*dbBank->header == *seqBankDummyTrack->bank->header);
            REQUIRE(*dbBank->data == *seqBankDummyTrack->bank->data);
            REQUIRE(dbBank->size == seqBankDummyTrack->bank->size);
        }
        SECTION("track_to_bank properly relates")
        {
            std::shared_ptr<Track> seqBankDummyTrack = create_dummy_track(true, true, 0);
            db->add_song(seqBankDummyTrack);
            REQUIRE(db->tables->relation.track_to_bank->select(seqBankDummyTrack->databaseIndex) == seqBankDummyTrack->sequence->databaseIndex);
        }
        SECTION("Adding track with sounds adds to sound table")
        {
            std::shared_ptr<Track> soundDummyTrack = create_dummy_track(false, false, 2);
            db->add_song(soundDummyTrack);
            REQUIRE(db->tables->track->check_exists(soundDummyTrack->databaseIndex));

            std::shared_ptr<Track> dbTrack = db->tables->track->select(soundDummyTrack->databaseIndex);
            REQUIRE(dbTrack->name == soundDummyTrack->name);

            REQUIRE(db->tables->sound->check_exists(soundDummyTrack->sounds[0]->databaseIndex));
            REQUIRE(db->tables->sound->check_exists(soundDummyTrack->sounds[1]->databaseIndex));

            std::shared_ptr<Sound> dbSound = db->tables->sound->select(soundDummyTrack->sounds[0]->databaseIndex);
            
            REQUIRE(dbSound->sampleAddr == soundDummyTrack->sounds[0]->sampleAddr);
            REQUIRE(*dbSound->data == *soundDummyTrack->sounds[0]->data);
            REQUIRE(dbSound->size == soundDummyTrack->sounds[0]->size);

            dbSound = db->tables->sound->select(soundDummyTrack->sounds[1]->databaseIndex);
            
            REQUIRE(dbSound->sampleAddr == soundDummyTrack->sounds[1]->sampleAddr);
            REQUIRE(*dbSound->data == *soundDummyTrack->sounds[1]->data);
            REQUIRE(dbSound->size == soundDummyTrack->sounds[1]->size);
        }
        SECTION("track_to_sound properly relates")
        {
            std::shared_ptr<Track> soundDummyTrack = create_dummy_track(true, true, 2);
            db->add_song(soundDummyTrack);
            
            Statement statement = db->tables->relation.track_to_sound->select_iter(soundDummyTrack->databaseIndex);
            for (int i = 0; statement.step() == SQLITE_ROW; i++)
            {
                REQUIRE(statement.column_int(1) == soundDummyTrack->sounds[i]->databaseIndex);
            }
        }
    }
}