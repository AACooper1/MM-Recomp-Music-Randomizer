#include "track.h"
#include "category.h"
#include "seed.h"
#include <string>

#define CAT(...) std::vector<int>({__VA_ARGS__})

std::array<std::shared_ptr<Track>, 0x80> Seed::vanillaTracks
(
    {
        /* Field */
        std::make_shared<Track>("Termina Field", CAT(FIELD), 0x03, 0x102),
        std::make_shared<Track>("Southern Swamp", CAT(FIELD, TOWN, CALM), 0x1C, 0x10C),
        std::make_shared<Track>("Mountain Village", CAT(FIELD, TOWN, CALM), 0x15, 0x113),
        std::make_shared<Track>("Great Bay Coast", CAT(FIELD, TOWN, CALM), 0x1D, 0x110),
        std::make_shared<Track>("Ikana Valley", CAT(FIELD, TOWN, CALM), 0x1E, 0x111),

        /* Towns */
        std::make_shared<Track>("Clock Town (Day 1)", CAT(TOWN), 0x19, 0x115),
        std::make_shared<Track>("Clock Town (Day 2)", CAT(TOWN), 0x19, 0x116),
        std::make_shared<Track>("Clock Town (Day 3)", CAT(TOWN), 0x19, 0x117),
        std::make_shared<Track>("Goron Village", CAT(TOWN, BUILDING, CALM), 0x26, 0x130),
        std::make_shared<Track>("Romani Ranch", CAT(FIELD, TOWN, CALM), 0x07, 0x12F),
        std::make_shared<Track>("Zora Hall", CAT(TOWN, BUILDING, CALM), 0x0B, 0x136),
        std::make_shared<Track>("Deku Palace", CAT(TOWN, FIELD), 0x19, 0x112),

        /* Dungeons */
        std::make_shared<Track>("Cavern", CAT(DUNGEON, CALM), 0x1A, 0x13B),
        std::make_shared<Track>("Woodfall Temple", CAT(DUNGEON, CALM), 0x14, 0x11C),
        std::make_shared<Track>("Snowhead Temple", CAT(DUNGEON, CALM), 0x15, 0x165),
        std::make_shared<Track>("Great Bay Temple", CAT(DUNGEON, CALM), 0x16, 0x166),
        std::make_shared<Track>("Pirates' Fortress", CAT(TOWN, DUNGEON, CALM), 0x13, 0x114),
        std::make_shared<Track>("Ikana Castle", CAT(TOWN, DUNGEON, ACTION), 0x21, 0x16F),
        std::make_shared<Track>("Stone Tower Temple", CAT(TOWN, DUNGEON, CALM), 0x19, 0x106),
        std::make_shared<Track>("Stone Tower Temple Upside Down", CAT(TOWN, DUNGEON, CALM), 0x19, 0x107),

        /* Indoors */
        std::make_shared<Track>("Clock Tower", CAT(BUILDING, CALM), 0x17, 0x105),
        std::make_shared<Track>("Guru-Guru's Song", CAT(BUILDING, CALM, MINIGAME), 0x05, 0x12E),
        std::make_shared<Track>("Milk Bar", CAT(TOWN, BUILDING, MINIGAME), 0x20, 0x13C),
        std::make_shared<Track>("House", CAT(BUILDING, CALM), 0x03, 0x11F),
        std::make_shared<Track>("Shop", CAT(BUILDING, CALM), 0x0C, 0x144),
        std::make_shared<Track>("Shooting Gallery", CAT(BUILDING, MINIGAME, CALM), 0x0A, 0x146),
        std::make_shared<Track>("Marine Research Laboratory", CAT(BUILDING, CALM), 0x0D, 0x12C),
        std::make_shared<Track>("Astral Observatory", CAT(TOWN, BUILDING, DUNGEON, CALM), 0x17, 0x13A),
        std::make_shared<Track>("Music Box House", CAT(BUILDING, MINIGAME, CALM), 0x05, 0x127),
        
        /* Minigames */
        std::make_shared<Track>("Goron Race", CAT(MINIGAME, ACTION), 0x26, 0x126),
        std::make_shared<Track>("Mini Game", CAT(MINIGAME, ACTION), 0x03, 0x125),
        std::make_shared<Track>("Cremia's Carriage", CAT(TOWN, BUILDING, MINIGAME, CALM), 0x08, 0x172),
        std::make_shared<Track>("Swamp Cruise", CAT(BUILDING, MINIGAME, CALM), 0x05, 0x10E),
        std::make_shared<Track>("Horse Race", CAT(MINIGAME, ACTION), 0x08, 0x140),

        /* Scenes */
        std::make_shared<Track>("Mayor's Meeting", CAT(BUILDING, ACTION, CALM, BOSS, SPECIAL), 0x03, 0x131),
        std::make_shared<Track>("Ghost Attack", CAT(MINIGAME, ACTION, CALM), 0x16, 0x10D),
        std::make_shared<Track>("Swordman's School", CAT(BUILDING, MINIGAME, ACTION, CALM), 0x18, 0x150),
        std::make_shared<Track>("Sharp's Curse", CAT(BUILDING, ACTION, CALM, SPECIAL), 0x03, 0x10F),
        std::make_shared<Track>("Chase", CAT(MINIGAME, ACTION, BOSS), 0x03, 0x103),
        std::make_shared<Track>("Majora's Theme", CAT(CALM, SPECIAL), 0x11, 0x104),
        std::make_shared<Track>("To The Moon", CAT(MINIGAME, ACTION, BOSS), 0x11, 0x17B),
        std::make_shared<Track>("Fairy's Fountain/File Select", CAT(TOWN, BUILDING, CALM), 0x06, 0x128),
        std::make_shared<Track>("File Select/Fairy's Fountain", CAT(TOWN, BUILDING, CALM), 0x06, 0x118),
        std::make_shared<Track>("Keaton's Quiz", CAT(CALM), 0x27, 0x173),
        std::make_shared<Track>("Owl", CAT(BUILDING, CALM, SPECIAL), 0x10, 0x145),
        std::make_shared<Track>("Witches' Potion Shop", CAT(BUILDING, CALM), 0x0E, 0x143),
        std::make_shared<Track>("Gorman Track", CAT(TOWN, CALM), 0x08, 0x142),
        std::make_shared<Track>("Woods of Mystery", CAT(FIELD, TOWN, BUILDING, MINIGAME, ACTION, CALM, SPECIAL), 0x04, 0x13E),
        std::make_shared<Track>("Zelda's Lullaby", CAT(TOWN, BUILDING, CALM), 0x06, 0x129),
        std::make_shared<Track>("Tatl & Tael", CAT(CALM), 0x21, 0x17D),
        std::make_shared<Track>("Song of Healing", CAT(TOWN, BUILDING, CALM), 0x17, 0x10B),
        std::make_shared<Track>("Giants' Theme", CAT(TOWN, BUILDING, CALM), 0x12, 0x12D),
        std::make_shared<Track>("Happy Mask Salesman's Theme", CAT(MINIGAME, CALM), 0x1B, 0x10A),

        
        /* Bosses */
        std::make_shared<Track>("Middle Boss Battle", CAT(ACTION, BOSS), 0x03, 0x138),
        std::make_shared<Track>("Boss Battle", CAT(ACTION, BOSS), 0x03, 0x11B),
        std::make_shared<Track>("Majora's Mask Battle", CAT(ACTION, BOSS), 0x11, 0x16B),
        std::make_shared<Track>("Majora's Incarnation Battle", CAT(MINIGAME, ACTION, BOSS), 0x11, 0x16A),
        std::make_shared<Track>("Majora's Wrath Battle", CAT(ACTION, BOSS), 0x11, 0x169),
        
        /* Fanfares */
        std::make_shared<Track>("Missed Event 1", CAT(GAME_OVER), 0x03, 0x108),
        std::make_shared<Track>("Missed Event 2", CAT(GAME_OVER, AREA_CLEAR), 0x03, 0x109),
        std::make_shared<Track>("Event Clear", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x03, 0x119),
        std::make_shared<Track>("Game Over", CAT(GAME_OVER, AREA_CLEAR), 0x0F, 0x120),
        std::make_shared<Track>("Boss Clear", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x03, 0x121),
        std::make_shared<Track>("Item Catch", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x0F, 0x122),
        std::make_shared<Track>("Get a Heart Container!", CAT(FANFARE, AREA_CLEAR), 0x0F, 0x124),
        std::make_shared<Track>("Get a Mask!", CAT(FANFARE, AREA_CLEAR), 0x0F, 0x137),
        std::make_shared<Track>("Small Item Catch", CAT(FANFARE, AREA_CLEAR), 0x0F, 0x139),
        std::make_shared<Track>("Zelda Appears", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x03, 0x13D),
        std::make_shared<Track>("Goron Race Goal", CAT(FANFARE, GAME_OVER), 0x26, 0x13F),
        std::make_shared<Track>("Horse Race Goal", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x08, 0x141),
        std::make_shared<Track>("Get the Ocarina!", CAT(FANFARE, AREA_CLEAR), 0x06, 0x152),
        std::make_shared<Track>("Song of Soaring", CAT(FANFARE), 0x09, 0x155),

        std::make_shared<Track>("Woodfall Rises", CAT(FANFARE, GAME_OVER, AREA_CLEAR), 0x0F, 0x177),
        std::make_shared<Track>("Southern Swamp Clears", CAT(AREA_CLEAR), 0x0F, 0x178),
        std::make_shared<Track>("Snowhead Clear", CAT(AREA_CLEAR), 0x0F, 0x179),
        std::make_shared<Track>("Moon's Destruction", CAT(AREA_CLEAR), 0x21, 0x17E),
        std::make_shared<Track>("The Giants' Exit", CAT(AREA_CLEAR), 0x21, 0x17C),

        std::make_shared<Track>("Sonata of Awakening", CAT(AREA_CLEAR), 0x12, 0x14B),
        std::make_shared<Track>("Goron Lullaby", CAT(AREA_CLEAR), 0x12, 0x14C),
        std::make_shared<Track>("New Wave Bossa Nova", CAT(AREA_CLEAR), 0x13, 0x14D),
        std::make_shared<Track>("Elegy of Emptiness", CAT(AREA_CLEAR), 0x12, 0x14E),
        std::make_shared<Track>("Oath to Order", CAT(AREA_CLEAR), 0x12, 0x14F),
        
        std::make_shared<Track>("Open Treasure Box", CAT(AREA_CLEAR), 0x03, 0x12B),
        std::make_shared<Track>("Ballad of the Wind Fish", CAT(AREA_CLEAR), 0x12, 0x154),

        /* Other */
        std::make_shared<Track>("Calling the Four Giants", CAT(SPECIAL), 0x21, 0x170),
        std::make_shared<Track>("Title Theme", CAT(SPECIAL), 0x23, 0x176),
        std::make_shared<Track>("Battle", CAT(MINIGAME, ACTION), 0x03, 0x11A),
        std::make_shared<Track>("Sun's Song (Morning Sequence)", CAT(AREA_CLEAR), 0x03, 0x11D),
        std::make_shared<Track>("Opening", CAT(AREA_CLEAR, SPECIAL), 0x11, 0x11E),
        std::make_shared<Track>("Opening (Loop)", CAT(SPECIAL), 0x03, 0x175),
        std::make_shared<Track>("The End (Credits 1)", CAT(SPECIAL, FINAL_HOURS), 0x24, 0x174),
        std::make_shared<Track>("Credits 2", CAT(SPECIAL, FINAL_HOURS), 0x25, 0x17F),

        /* No Categories */
        std::make_shared<Track>("Last Day", CAT(), 0x17, 0x157),
        std::make_shared<Track>("Mikau", CAT(), 0x13, 0x158),
        std::make_shared<Track>("Bass Practice", CAT(), 0x13, 0x16C),
        std::make_shared<Track>("Drum Practice", CAT(), 0x13, 0x16D),
        std::make_shared<Track>("Piano Practice", CAT(), 0x13, 0x16E),
        std::make_shared<Track>("Jam Session", CAT(), 0x13, 0x162),
        std::make_shared<Track>("Intellectual Property Theft", CAT(), 0x13, 0x163),
        std::make_shared<Track>("The Indigo-Go's", CAT(), 0x13, 0x164),
        std::make_shared<Track>("Rosa Sisters", CAT(), 0x1F, 0x12A),
        std::make_shared<Track>("Concert for You", CAT(), 0x13, 0x159),
        std::make_shared<Track>("Bremen Mask", CAT(), 0x09, 0x153),
        std::make_shared<Track>("Kamaro's Dance", CAT(), 0x0A, 0x171),

        /* Do Not Randomize */
        std::make_shared<Track>("Frog Song", CAT(), 0x00, 0x15A),
        std::make_shared<Track>("New Wave Bossa Nova (Saxophone)", CAT(), 0x13, 0x167),
        std::make_shared<Track>("New Wave Bossa Nova (Vocals)", CAT(), 0x13, 0x168),
        std::make_shared<Track>("General SFX", CAT(), 0x00, 0x100),
        std::make_shared<Track>("Ambience SFX", CAT(), 0x02, 0x101),

        /* Ocarina Songs */
        std::make_shared<Track>("Ocarina - Epona's Song", CAT(), 0x00, 0x132),
        std::make_shared<Track>("Ocarina - Sun's Song", CAT(), 0x00, 0x133),
        std::make_shared<Track>("Ocarina - Song of Time", CAT(), 0x00, 0x134),
        std::make_shared<Track>("Ocarina - Song of Storms", CAT(), 0x00, 0x135),

        std::make_shared<Track>("Ocarina - Song of Soaring", CAT(), 0x00, 0x147),
        std::make_shared<Track>("Ocarina - Song of Healing", CAT(), 0x00, 0x148),
        std::make_shared<Track>("Ocarina - Inverted Song of Time", CAT(), 0x00, 0x149),
        std::make_shared<Track>("Ocarina - Song of Double Time", CAT(), 0x00, 0x14A),
        
        std::make_shared<Track>("Ocarina - Sonata of Awakening", CAT(), 0x00, 0x15B),
        std::make_shared<Track>("Ocarina - Goron Lullaby Intro", CAT(), 0x00, 0x151),
        std::make_shared<Track>("Ocarina - Goron Lullaby", CAT(), 0x00, 0x15C),
        std::make_shared<Track>("Ocarina - New Wave Bossa Nova", CAT(), 0x00, 0x15D),
        std::make_shared<Track>("Ocarina - Elegy of Emptiness", CAT(), 0x00, 0x15E),
        std::make_shared<Track>("Ocarina - Oath to Order", CAT(), 0x00, 0x15F),

        /* Annoying Pointers */
        std::make_shared<Track>("Pointer to Clock Town (Day 2)", CAT(), 0x19, 0x123),
        std::make_shared<Track>("Pointer to Milk Bar", CAT(), 0x20, 0x156),
        std::make_shared<Track>("Pointer to Last Day", CAT(), 0x17, 0x160),
        std::make_shared<Track>("Pointer to Ocarina - Goron Lullaby", CAT(), 0x00, 0x161),

        /* Unknown */
        std::make_shared<Track>("Unk_00 (Listed as Pamela's Music Box House?)", CAT(), 0x00, 0x17A)
    }
);