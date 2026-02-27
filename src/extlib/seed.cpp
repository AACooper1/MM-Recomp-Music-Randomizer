#include "category.h"
#include "vanillatracks.h"
#include "songslot.h"
#include "seed.h"

std::vector<int> Seed::get_available_tracks(SongSlotID slotId)
{
    return songSlots[(int)slotId].availableTracksNoRemove;
}

std::string Seed::get_slot_name(SongSlotID slotId)
{
    return songSlots[(int)slotId].name;
}

void Seed::randomize()
{
    std::vector<int> randomOrder;
    for (int i = 2; i < songSlots.size(); i++)
    {
        randomOrder.push_back(i);
    }

    std::ranges::shuffle(randomOrder, rng);

    for (int s = 0; s < randomOrder.size(); s++)
    {
        int i = randomOrder[s];
        randomize_slot(i); 
    }
    
    prepare_tracks();
}

void Seed::randomize_slot(int i)
{
    if (songSlots[i].availableTracks.size() == 0)
    {
        songSlots[i].availableTracks = songSlots[i].availableTracksNoRemove;
    }
    // If there are really no tracks available at all, just choose from everything available. 
    // Only distinguishes between fanfare and BGM
    if (songSlots[i].availableTracks.size() == 0)
    {
        for (const auto & [ id, track ] : tracks)
        {
            if (!(track->is_fanfare() ^ songSlots[i].is_fanfare()))
                songSlots[i].availableTracks.push_back(id);
        }
    }
    // If we don't have anything available at all after accounting for fanfare and BGM,
    // Just don't randomize the slot (i.e. choose the vanilla song)
    if (songSlots[i].availableTracks.size() == 0)
    {
        randomized.emplace(i, songSlots[i].vanillaTrack);
        return;
    }

    std::ranges::shuffle(songSlots[i].availableTracks, rng);

    int trackId = songSlots[i].availableTracks[0];

    randomized.emplace(i, tracks[trackId]);
    for (int i = 2; i < songSlots.size(); i++)
    {
        std::erase(songSlots[i].availableTracks, trackId);
    }
}

void Seed::populate_track_table()
{
    int idx = 0;
    if (use_vanilla)
    {
        for (int i = 0; i < vanillaTracks.size(); i++)
        {
            tracks.emplace(vanillaTracks[i]->id * -1, vanillaTracks[i]);
            vanillaTracks[i]->seedIdx = vanillaTracks[i]->id * -1;
            idx++;
        }
    }
    if (use_custom)
    {
        for (const auto & [ id, track ] : db->tables->track->entries)
        {
            tracks.emplace(track->databaseIndex, track);
            track->seedIdx = track->databaseIndex;
            idx++;
        }
    }
}

void Seed::link_slots_to_vanilla()
{
    for (int i = 0; i < vanillaTracks.size(); i++)
    {
        songSlots[vanillaTracks[i]->id - 0x100].vanillaTrack = vanillaTracks[i];
    }
}

void Seed::prepare_song_slots()
{
    link_slots_to_vanilla();

    for (const auto & [ id, track ] : tracks)
    {
        for (int j = 0; j < 0x200; j++)
        {
            if ((*track->categories)[j])
            {
                categories[j].trackList.push_back(id);
            }
        }
    }

    for (int i = 2; i < songSlots.size(); i++)
    {
        SongSlot& slot = songSlots[i];
        std::set<int> availTracks;
        for (int j = 0; j < songSlots[i].categories.size(); j++)
        {
            int category = songSlots[i].categories[j];

            for(int k = 0; k < categories[category].trackList.size(); k++)
            {
                availTracks.insert(categories[category].trackList[k]);
            }
        }

        slot.availableTracks.assign(availTracks.begin(), availTracks.end());
        slot.availableTracksNoRemove = slot.availableTracks;
    }

}

void Seed::prepare_tracks()
{
    for (const auto & [id, track] : randomized)
    {
        if (track->type != TrackType::VANILLA)
            db->prepare_track(track->databaseIndex);
    }
}

int Seed::save_seed()
{
    std::shared_ptr<Database> seedDb = std::make_shared<Database>(this->savePath, true);
    seedDb->seedTable = std::make_unique<SlotToTrackTable>(seedDb, "slot_to_track");

    for (const auto & [slotId, track] : randomized)
    {
        if (track->type != TrackType::VANILLA)
        {
            seedDb->seedTable->insert(slotId, track->databaseIndex);
        }
        else
        {
            seedDb->seedTable->insert(slotId, track->id * -1);
        }
    }

    return 1;
}

int Seed::load_seed(fs::path savePath)
{
    std::shared_ptr<Database> seedDb = std::make_shared<Database>(this->savePath, true);
    seedDb->seedTable = std::make_unique<SlotToTrackTable>(seedDb, "slot_to_track");
    for (int i = 2; i < songSlots.size(); i++)
    {
        int trackId = seedDb->seedTable->select(i);

        if (trackId >= 0)
        {
            if (tracks.contains(trackId))
            {
                bool can_go_in_slot = false;
                for (int j = 0; j < songSlots[i].categories.size(); j++)
                {
                    if ((*tracks[trackId]->categories)[j])
                    {
                        can_go_in_slot = true;
                        break;
                    }
                }
                if (can_go_in_slot)
                {
                    randomized[i] = tracks[trackId];
                }
                else
                {
                    randomize_slot(i);
                }
            }
            else
            {
                randomize_slot(i);
            }
        }
        else if (trackId > -0x200)
        {
            if (use_vanilla)
            {
                randomized[i] = tracks[trackId];
            }
            else
            {
                randomize_slot(i);
            }
        }
        else
        {
            randomize_slot(i);
        }
    }

    prepare_tracks();

    return true;
}



std::array<SongSlot, 0x80> Seed::songSlots
(
    {
        // Do not touch these two.
        SongSlot(0x00, "General SFX", {}),
        SongSlot(0x01, "Ambience", {}),

        SongSlot(0x02, "Termina Field", {FIELD}),
        SongSlot(0x03, "Chase", {ACTION, BOSS}),
        SongSlot(0x04, "Majora's Theme", {CALM}),
        SongSlot(0x05, "Clock Tower", {BUILDING, CALM}),
        SongSlot(0x06, "Stone Tower Temple", {DUNGEON}),
        SongSlot(0x07, "Stone Tower Temple Upside-Down", {DUNGEON}),
        SongSlot(0x08, "Missed Event 1", {GAME_OVER}),
        SongSlot(0x09, "Missed Event 2", {GAME_OVER}),
        SongSlot(0x0A, "Happy Mask Salesman's Theme", {}),  // Not randomized bc you never hear it in the rando
        SongSlot(0x0B, "Song of Healing", {BUILDING, CALM}),
        SongSlot(0x0C, "Southern Swamp", {FIELD, TOWN}),
        SongSlot(0x0D, "Ghost Attack", {ACTION, CALM}),
        SongSlot(0x0E, "Swamp Cruise", {BUILDING, MINIGAME, CALM}),
        SongSlot(0x0F, "Sharp's Curse", {BUILDING, ACTION, CALM}),
        SongSlot(0x10, "Great Bay Coast", {FIELD, TOWN}),
        SongSlot(0x11, "Ikana Valley", {FIELD, TOWN}),
        SongSlot(0x12, "Deku Palace", {FIELD, TOWN}),
        SongSlot(0x13, "Mountain Village", {FIELD, TOWN}),
        SongSlot(0x14, "Pirates' Fortress", {DUNGEON}),
        SongSlot(0x15, "Clock Town (Day 1)", {TOWN}),
        SongSlot(0x16, "Clock Town (Day 2)", {TOWN}),
        SongSlot(0x17, "Clock Town (Day 3)", {TOWN}),
        SongSlot(0x18, "File Select", {TOWN, BUILDING, CALM}),
        SongSlot(0x19, "Event Clear", {FANFARE}),
        SongSlot(0x1A, "Battle", {MINIGAME, BOSS, ACTION}),
        SongSlot(0x1B, "Boss Battle", {BOSS}),
        SongSlot(0x1C, "Woodfall Temple", {DUNGEON}),
        SongSlot(0x1D, "Sun's Song (Morning Sequence)", {}), // Look into this - might be able to use the IO thing
        SongSlot(0x1E, "Opening", {}),
        SongSlot(0x1F, "House", {BUILDING}),
        SongSlot(0x20, "Game Over", {GAME_OVER}),
        SongSlot(0x21, "Boss Clear", {GAME_OVER, AREA_CLEAR}),
        SongSlot(0x22, "Item Catch", {FANFARE}),
        SongSlot(0x23, "Pointer to Clock Town (Day 2)", {}), // Pointer
        SongSlot(0x24, "Get a Heart Container!", {FANFARE}),
        SongSlot(0x25, "Mini Game", {MINIGAME}),
        SongSlot(0x26, "Goron Race", {MINIGAME}),
        SongSlot(0x27, "Music Box House", {BUILDING, MINIGAME, CALM}),
        SongSlot(0x28, "Fairy's Fountain", {TOWN, BUILDING, CALM}),
        SongSlot(0x29, "Zelda's Theme", {TOWN, BUILDING, CALM}),
        SongSlot(0x2A, "Rosa Sisters", {FIELD, TOWN, DUNGEON, BUILDING, MINIGAME, ACTION, CALM, BOSS, SPECIAL, FINAL_HOURS}),
        SongSlot(0x2B, "Open Treasure Box", {}), // Not randomized
        SongSlot(0x2C, "Marine Research Laboratory", {BUILDING, CALM}),
        SongSlot(0x2D, "Giants' Theme", {BUILDING, CALM}),
        SongSlot(0x2E, "Guru-Guru's Song", {FIELD, TOWN, DUNGEON, BUILDING, MINIGAME, ACTION, CALM, BOSS, SPECIAL, FINAL_HOURS}),
        SongSlot(0x2F, "Romani Ranch", {FIELD, TOWN}),
        SongSlot(0x30, "Goron Village", {TOWN}),
        SongSlot(0x31, "Mayor's Meeting", {BUILDING, CALM, ACTION}),
        SongSlot(0x32, "Ocarina - Epona's Song", {}), // Ocarina song
        SongSlot(0x33, "Ocarina - Sun's Song", {}), // Ocarina song
        SongSlot(0x34, "Ocarina - Song of Time", {}), // Ocarina song 
        SongSlot(0x35, "Ocarina - Song of Storms", {}),  // Ocarina song
        SongSlot(0x36, "Zora Hall", {TOWN}),
        SongSlot(0x37, "Get a Mask!", {FANFARE}),
        SongSlot(0x38, "Middle Boss Battle", {BOSS}),
        SongSlot(0x39, "Small Item Catch", {FANFARE}),
        SongSlot(0x3A, "Astral Observatory", {BUILDING}),
        SongSlot(0x3B, "Cavern", {DUNGEON}),
        SongSlot(0x3C, "Milk Bar", {BUILDING}),
        SongSlot(0x3D, "Zelda Appears", {FANFARE}),
        SongSlot(0x3E, "Woods of Mystery", {FIELD, TOWN, BUILDING, MINIGAME, CALM}),
        SongSlot(0x3F, "Goron Race Goal", {FANFARE}),
        SongSlot(0x40, "Horse Race", {MINIGAME}),
        SongSlot(0x41, "Horse Race Goal", {FANFARE}),
        SongSlot(0x42, "Gorman Track", {TOWN}),
        SongSlot(0x43, "Witches' Potion Shop", {BUILDING, CALM}),
        SongSlot(0x44, "Shop", {BUILDING}),
        SongSlot(0x45, "Owl", {CALM}),
        SongSlot(0x46, "Shooting Gallery", {BUILDING, MINIGAME}),
        SongSlot(0x47, "Ocarina - Song of Soaring", {}), // Ocarina song
        SongSlot(0x48, "Ocarina - Song of Healing", {}), // Ocarina song
        SongSlot(0x49, "Ocarina - Inverted Song of Time", {}), // Ocarina song
        SongSlot(0x4A, "Ocarina - Song of Double Time", {}), // Ocarina song
        SongSlot(0x4B, "Sonata of Awakening", {}), // Ocarina song
        SongSlot(0x4C, "Goron Lullaby", {}), // Ocarina song
        SongSlot(0x4D, "New Wave Bossa Nova", {}), // Ocarina song
        SongSlot(0x4E, "Elegy of Emptiness", {}), // Ocarina song
        SongSlot(0x4F, "Oath to Order", {}), // Ocarina song
        SongSlot(0x50, "Swordsman's School", {BUILDING, ACTION, MINIGAME}),
        SongSlot(0x51, "Ocarina - Goron Lullaby Intro", {}), // Ocarina song
        SongSlot(0x52, "Get the Ocarina!", {FANFARE}), 
        SongSlot(0x53, "Bremen March", {}), // Not randomized - look later into adding tempo & dog thing
        SongSlot(0x54, "Ballad of the Wind Fish", {}), // // Ocarina song
        SongSlot(0x55, "Song of Soaring", {}), // Ocarina song
        SongSlot(0x56, "Milk Bar (Pointer)", {}), // Pointer
        SongSlot(0x57, "Last Day", {FINAL_HOURS}),
        SongSlot(0x58, "Mikau", {}), // Not randomized bc you never hear it in the rando
        SongSlot(0x59, "Concert For You", {}), // Not randomized bc you never hear it in the rando
        SongSlot(0x5A, "Frog Song", {}), // Ocarina song, sorta
        SongSlot(0x5B, "Ocarina - Sonata of Awakening", {}), // Ocarina song
        SongSlot(0x5C, "Ocarina - Goron Lullaby", {}),  // Ocarina song
        SongSlot(0x5D, "Ocarina - New Wave Bossa Nova", {}),  // Ocarina song
        SongSlot(0x5E, "Ocarina - Elegy of Emptiness", {}), // Ocarina song
        SongSlot(0x5F, "Ocarina - Oath to Order", {}),  // Ocarina song
        SongSlot(0x60, "Pointer to Last Day", {}), // Pointer
        SongSlot(0x61, "A Pointer to Ocarina - Goron Lullaby Intro For Some Reason", {}), // Pointer
        SongSlot(0x62, "Bass & Guitar Session", {}),  // Ocarina song
        SongSlot(0x63, "Piano Solo", {}), // Not randomized bc you never hear it in the rando
        SongSlot(0x64, "The Indigo-Go's", {FIELD, TOWN, DUNGEON, BUILDING, MINIGAME, ACTION, CALM, BOSS, SPECIAL, FINAL_HOURS}),  // You never hear this in the rando unless you go out of your way to but having them just play whatever is a funny bit
        SongSlot(0x65, "Snowhead Temple", {DUNGEON}),
        SongSlot(0x66, "Great Bay Temple", {DUNGEON}),
        SongSlot(0x67, "New Wave Bossa Nova (Saxophone)", {}), // Not randomized bc you never hear it in the rando
        SongSlot(0x68, "New Wave Bossa Nova (Vocals)", {}), // Not randomized bc you never hear it in the rando
        SongSlot(0x69, "Majora's Wrath Battle", {BOSS}),
        SongSlot(0x6A, "Majora's Incarnation Battle", {BOSS}),
        SongSlot(0x6B, "Majora's Mask Battle", {BOSS}),
        SongSlot(0x6C, "Bass Practice", {}), // Unique
        SongSlot(0x6D, "Drums Practice", {}), // Unique
        SongSlot(0x6E, "Piano Practice", {}), // Unique
        SongSlot(0x6F, "Ikana Castle", {DUNGEON}),
        SongSlot(0x70, "Calling the Four Giants", {SPECIAL}),
        SongSlot(0x71, "Kamaro's Dance", {FIELD, TOWN, DUNGEON, BUILDING, MINIGAME, ACTION, CALM, BOSS, SPECIAL, FINAL_HOURS}),
        SongSlot(0x72, "Cremia's Carriage", {BUILDING, MINIGAME, CALM}),
        SongSlot(0x73, "Keaton's Quiz", {CALM}),
        SongSlot(0x74, "The End (Credits 1)", {}),  // Not randomized bc you never hear it in the rando
        SongSlot(0x75, "Opening (Loop)", {}),  // Not randomized bc you never hear it in the rando, or in the game for that matter
        SongSlot(0x76, "Title Theme", {SPECIAL}),
        SongSlot(0x77, "Woodfall Rises", {FANFARE, GAME_OVER}),
        SongSlot(0x78, "Southern Swamp Clears", {AREA_CLEAR}),
        SongSlot(0x79, "Snowhead Clear", {AREA_CLEAR}),
        SongSlot(0x7A, "Unk_00", {}), // I have no idea what this is
        SongSlot(0x7B, "To the Moon", {ACTION}),
        SongSlot(0x7C, "The Giants' Exit", {SPECIAL}),
        SongSlot(0x7D, "Tatl & Tael", {CALM}),
        SongSlot(0x7E, "Moon's Destruction", {AREA_CLEAR}),
        SongSlot(0x7F, "Credits 2", {})  // Not randomized bc you never hear it in the rando
    }
);