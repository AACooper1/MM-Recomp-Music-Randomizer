#include "track.h"
#include "category.h"
#include "seed.h"
#include <string>

#define CAT(...) std::vector<int>({__VA_ARGS__})

std::array<std::shared_ptr<Track>, 0x80> Seed::vanillaTracks
(
    {
        /* Field */
        std::make_shared<Track>("Termina Field", CAT(FIELD, ACTION), 0x03, 0x02),
        std::make_shared<Track>("Southern Swamp", CAT(FIELD, TOWN, CALM), 0x1C, 0x0C),
        std::make_shared<Track>("Mountain Village", CAT(FIELD, TOWN, CALM), 0x15, 0x13),
        std::make_shared<Track>("Great Bay Coast", CAT(FIELD, TOWN, CALM), 0x1D, 0x10),
        std::make_shared<Track>("Ikana Valley", CAT(FIELD, TOWN, CALM), 0x1E, 0x11),

        /* Towns */
        std::make_shared<Track>("Clock Town (Day 1)", CAT(TOWN), 0x19, 0x15),
        std::make_shared<Track>("Clock Town (Day 2)", CAT(TOWN), 0x19, 0x16),
        std::make_shared<Track>("Clock Town (Day 3)", CAT(TOWN), 0x19, 0x17),
        std::make_shared<Track>("Goron Village", CAT(TOWN, BUILDING, CALM), 0x26, 0x30),
        std::make_shared<Track>("Romani Ranch", CAT(FIELD, TOWN, CALM), 0x07, 0x2F),
        std::make_shared<Track>("Zora Hall", CAT(TOWN, BUILDING, CALM), 0x0B, 0x36),
        std::make_shared<Track>("Deku Palace", CAT(TOWN, FIELD), 0x19, 0x12),

        /* Dungeons */
        std::make_shared<Track>("Cavern", CAT(DUNGEON, CALM), 0x1A, 0x3B),
        std::make_shared<Track>("Woodfall Temple", CAT(DUNGEON, CALM), 0x14, 0x1C),
        std::make_shared<Track>("Snowhead Temple", CAT(DUNGEON, CALM), 0x15, 0x65),
        std::make_shared<Track>("Great Bay Temple", CAT(DUNGEON, CALM), 0x16, 0x66),
        std::make_shared<Track>("Pirates' Fortress", CAT(TOWN, DUNGEON, CALM), 0x13, 0x14),
        std::make_shared<Track>("Ikana Castle", CAT(TOWN, DUNGEON, ACTION), 0x21, 0x6F),
        std::make_shared<Track>("Stone Tower Temple", CAT(TOWN, DUNGEON, CALM), 0x19, 0x06),
        std::make_shared<Track>("Stone Tower Temple Upside Down", CAT(TOWN, DUNGEON, CALM), 0x19, 0x07),

        /* Indoors */
        std::make_shared<Track>("Clock Tower", CAT(BUILDING, CALM), 0x17, 0x05),
        std::make_shared<Track>("Guru-Guru's Song", CAT(BUILDING, CALM, MINIGAME), 0x05, 0x2E),
        std::make_shared<Track>("Milk Bar", CAT(TOWN, BUILDING, MINIGAME), 0x20, 0x3C),
        std::make_shared<Track>("House", CAT(BUILDING, CALM), 0x03, 0x1F),
        std::make_shared<Track>("Shop", CAT(BUILDING, CALM), 0x0C, 0x44),
        std::make_shared<Track>("Shooting Gallery", CAT(BUILDING, MINIGAME, CALM), 0x0A, 0x46),
        std::make_shared<Track>("Marine Research Laboratory", CAT(BUILDING, CALM), 0x0D, 0x2C),
        std::make_shared<Track>("Astral Observatory", CAT(TOWN, BUILDING, DUNGEON, CALM), 0x17, 0x3A),
        std::make_shared<Track>("Music Box House", CAT(BUILDING, MINIGAME, CALM), 0x05, 0x27),
        
        /* Minigames */
        std::make_shared<Track>("Goron Race", CAT(MINIGAME, ACTION), 0x26, 0x26),
        std::make_shared<Track>("Mini Game", CAT(MINIGAME, ACTION, CALM), 0x03, 0x25),
        std::make_shared<Track>("Cremia's Carriage", CAT(TOWN, BUILDING, MINIGAME, CALM), 0x07, 0x72),
        std::make_shared<Track>("Swamp Cruise", CAT(BUILDING, MINIGAME, CALM), 0x05, 0x0E),
        std::make_shared<Track>("Horse Race", CAT(MINIGAME, ACTION), 0x08, 0x40),

        /* Scenes */
        std::make_shared<Track>("Mayor's Meeting", CAT(BUILDING, ACTION, CALM, BOSS, SPECIAL), 0x03, 0x31),
        std::make_shared<Track>("Ghost Attack", CAT(MINIGAME, ACTION, CALM), 0x16, 0x0D),
        std::make_shared<Track>("Swordman's School", CAT(BUILDING, MINIGAME, ACTION, CALM), 0x18, 0x50),
        std::make_shared<Track>("Sharp's Curse", CAT(BUILDING, ACTION, CALM, SPECIAL), 0x03, 0x0F),
        std::make_shared<Track>("Chase", CAT(MINIGAME, ACTION, BOSS), 0x03, 0x03),
        std::make_shared<Track>("Majora's Theme", CAT(CALM, SPECIAL), 0x11, 0x04),
        std::make_shared<Track>("To The Moon", CAT(MINIGAME, ACTION, BOSS), 0x11, 0x7B),
        std::make_shared<Track>("Fairy's Fountain/File Select", CAT(TOWN, BUILDING, CALM), 0x06, 0x28),
        std::make_shared<Track>("File Select/Fairy's Fountain", CAT(TOWN, BUILDING, CALM), 0x06, 0x18),
        std::make_shared<Track>("Keaton's Quiz", CAT(CALM), 0x27, 0x73),
        std::make_shared<Track>("Owl", CAT(BUILDING, CALM, SPECIAL), 0x10, 0x45),
        std::make_shared<Track>("Witches' Potion Shop", CAT(BUILDING, CALM), 0x0E, 0x43),
        std::make_shared<Track>("Gorman Track", CAT(TOWN, CALM), 0x08, 0x42),
        std::make_shared<Track>("Woods of Mystery", CAT(FIELD, TOWN, BUILDING, MINIGAME, ACTION, CALM, SPECIAL), 0x04, 0x3E),
        std::make_shared<Track>("Zelda's Lullaby", CAT(TOWN, BUILDING, CALM), 0x06, 0x29),
        std::make_shared<Track>("Tatl & Tael", CAT(CALM), 0x21, 0x7D),
        std::make_shared<Track>("Song of Healing", CAT(TOWN, BUILDING, CALM), 0x17, 0x0B),
        std::make_shared<Track>("Giants' Theme", CAT(TOWN, BUILDING, CALM), 0x12, 0x2D),
        std::make_shared<Track>("Happy Mask Salesman's Theme", CAT(MINIGAME, CALM), 0x1B, 0x0A),

        
        /* Bosses */
        std::make_shared<Track>("Middle Boss Battle", CAT(ACTION, BOSS), 0x03, 0x38),
        std::make_shared<Track>("Boss Battle", CAT(ACTION, BOSS), 0x03, 0x1B),
        std::make_shared<Track>("Majora's Mask Battle", CAT(ACTION, BOSS), 0x11, 0x6B),
        std::make_shared<Track>("Majora's Incarnation Battle", CAT(MINIGAME, ACTION, BOSS), 0x11, 0x6A),
        std::make_shared<Track>("Majora's Wrath Battle", CAT(ACTION, BOSS), 0x11, 0x69),
        
        /* Fanfares */
        std::make_shared<Track>("Missed Event 1", CAT(GAME_OVER), 0x03, 0x08),
        std::make_shared<Track>("Missed Event 2", CAT(GAME_OVER, AREA_CLEAR), 0x03, 0x09),
        std::make_shared<Track>("Event Clear", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x03, 0x19),
        std::make_shared<Track>("Game Over", CAT(GAME_OVER, AREA_CLEAR), 0x0F, 0x20),
        std::make_shared<Track>("Boss Clear", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x03, 0x21),
        std::make_shared<Track>("Item Catch", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x0F, 0x22),
        std::make_shared<Track>("Get a Heart Container!", CAT(FANFARE, AREA_CLEAR), 0x0F, 0x24),
        std::make_shared<Track>("Get a Mask!", CAT(FANFARE, AREA_CLEAR), 0x0F, 0x37),
        std::make_shared<Track>("Small Item Catch", CAT(FANFARE, AREA_CLEAR), 0x0F, 0x39),
        std::make_shared<Track>("Zelda Appears", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x03, 0x3D),
        std::make_shared<Track>("Goron Race Goal", CAT(FANFARE, GAME_OVER), 0x26, 0x3F),
        std::make_shared<Track>("Horse Race Goal", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x08, 0x41),
        std::make_shared<Track>("Get the Ocarina!", CAT(FANFARE, AREA_CLEAR), 0x06, 0x52),
        std::make_shared<Track>("Song of Soaring", CAT(FANFARE), 0x09, 0x55),

        std::make_shared<Track>("Woodfall Rises", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x0F, 0x77),
        std::make_shared<Track>("Southern Swamp Clears", CAT(AREA_CLEAR), 0x0F, 0x78),
        std::make_shared<Track>("Snowhead Clear", CAT(AREA_CLEAR), 0x0F, 0x79),
        std::make_shared<Track>("Moon's Destruction", CAT(AREA_CLEAR), 0x21, 0x7E),
        std::make_shared<Track>("The Giants' Exit", CAT(AREA_CLEAR), 0x21, 0x7C),

        std::make_shared<Track>("Sonata of Awakening", CAT(AREA_CLEAR), 0x12, 0x4B),
        std::make_shared<Track>("Goron Lullaby", CAT(AREA_CLEAR), 0x12, 0x4C),
        std::make_shared<Track>("New Wave Bossa Nova", CAT(AREA_CLEAR), 0x13, 0x4D),
        std::make_shared<Track>("Elegy of Emptiness", CAT(AREA_CLEAR), 0x12, 0x4E),
        std::make_shared<Track>("Oath to Order", CAT(AREA_CLEAR), 0x12, 0x4F),
        
        std::make_shared<Track>("Open Treasure Box", CAT(AREA_CLEAR), 0x03, 0x2B),
        std::make_shared<Track>("Ballad of the Wind Fish", CAT(AREA_CLEAR), 0x12, 0x54),

        /* Other */
        std::make_shared<Track>("Calling the Four Giants", CAT(SPECIAL), 0x21, 0x70),
        std::make_shared<Track>("Title Theme", CAT(SPECIAL), 0x23, 0x76),
        std::make_shared<Track>("Battle", CAT(MINIGAME, ACTION), 0x03, 0x1A),
        std::make_shared<Track>("Sun's Song (Morning Sequence)", CAT(AREA_CLEAR), 0x03, 0x1D),
        std::make_shared<Track>("Opening", CAT(AREA_CLEAR, SPECIAL), 0x11, 0x1E),
        std::make_shared<Track>("Opening (Loop)", CAT(SPECIAL), 0x03, 0x75),
        std::make_shared<Track>("The End (Credits 1)", CAT(SPECIAL, FINAL_HOURS), 0x24, 0x74),
        std::make_shared<Track>("Credits 2", CAT(SPECIAL, FINAL_HOURS), 0x25, 0x7F),

        /* No Categories */
        std::make_shared<Track>("Last Day", CAT(), 0x17, 0x57),
        std::make_shared<Track>("Mikau", CAT(), 0x13, 0x58),
        std::make_shared<Track>("Bass Practice", CAT(), 0x13, 0x6C),
        std::make_shared<Track>("Drum Practice", CAT(), 0x13, 0x6D),
        std::make_shared<Track>("Piano Practice", CAT(), 0x13, 0x6E),
        std::make_shared<Track>("Jam Session", CAT(), 0x13, 0x62),
        std::make_shared<Track>("Intellectual Property Theft", CAT(), 0x13, 0x63),
        std::make_shared<Track>("The Indigo-Go's", CAT(), 0x13, 0x64),
        std::make_shared<Track>("Rosa Sisters", CAT(), 0x1F, 0x2A),
        std::make_shared<Track>("Concert for You", CAT(), 0x13, 0x59),
        std::make_shared<Track>("Bremen Mask", CAT(), 0x09, 0x53),
        std::make_shared<Track>("Kamaro's Dance", CAT(), 0x0A, 0x71),

        /* Do Not Randomize */
        std::make_shared<Track>("Frog Song", CAT(), 0x00, 0x5A),
        std::make_shared<Track>("New Wave Bossa Nova (Saxophone)", CAT(), 0x13, 0x67),
        std::make_shared<Track>("New Wave Bossa Nova (Vocals)", CAT(), 0x13, 0x68),
        std::make_shared<Track>("General SFX", CAT(), 0x00, 0x00),
        std::make_shared<Track>("Ambience SFX", CAT(), 0x02, 0x01),

        /* Ocarina Songs */
        std::make_shared<Track>("Ocarina - Epona's Song", CAT(), 0x00, 0x32),
        std::make_shared<Track>("Ocarina - Sun's Song", CAT(), 0x00, 0x33),
        std::make_shared<Track>("Ocarina - Song of Time", CAT(), 0x00, 0x34),
        std::make_shared<Track>("Ocarina - Song of Storms", CAT(), 0x00, 0x35),

        std::make_shared<Track>("Ocarina - Song of Soaring", CAT(), 0x00, 0x47),
        std::make_shared<Track>("Ocarina - Song of Healing", CAT(), 0x00, 0x48),
        std::make_shared<Track>("Ocarina - Inverted Song of Time", CAT(), 0x00, 0x49),
        std::make_shared<Track>("Ocarina - Song of Double Time", CAT(), 0x00, 0x4A),
        
        std::make_shared<Track>("Ocarina - Sonata of Awakening", CAT(), 0x00, 0x5B),
        std::make_shared<Track>("Ocarina - Goron Lullaby Intro", CAT(), 0x00, 0x51),
        std::make_shared<Track>("Ocarina - Goron Lullaby", CAT(), 0x00, 0x5C),
        std::make_shared<Track>("Ocarina - New Wave Bossa Nova", CAT(), 0x00, 0x5D),
        std::make_shared<Track>("Ocarina - Elegy of Emptiness", CAT(), 0x00, 0x5E),
        std::make_shared<Track>("Ocarina - Oath to Order", CAT(), 0x00, 0x5F),

        /* Annoying Pointers */
        std::make_shared<Track>("Pointer to Clock Town (Day 2)", CAT(), 0x19, 0x23),
        std::make_shared<Track>("Pointer to Milk Bar", CAT(), 0x20, 0x56),
        std::make_shared<Track>("Pointer to Last Day", CAT(), 0x17, 0x60),
        std::make_shared<Track>("Pointer to Ocarina - Goron Lullaby", CAT(), 0x00, 0x61),

        /* Unknown */
        std::make_shared<Track>("Unk_00 (Listed as Pamela's Music Box House?", CAT(), 0x00, 0x7A)
    }
);