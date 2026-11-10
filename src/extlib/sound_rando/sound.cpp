#include "sound.hpp"
#include <array>

// Sound slots
// Details from https://docs.google.com/spreadsheets/d/1YVJ7GdzNZUese6H8d40lzpxp8ZfWlt75c-lzBzKngmo/edit?gid=1343879920#gid=1343879920
// (Credit to ZoeyZolotova/isghj for linking to the above in MMR)
std::array<SoundEffect, 0x200> EnvSounds
{
    SoundEffect(0x2000,     "WoodenDoorOpen",       SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2001,     "WoodenDoorClose",      SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2002,     "GenericExplosion",     SND_NONE,               SND_NONE            ),
    SoundEffect(0x2003,     "OOTEponaWalk",         SND_NONE,               SND_NONE            ),
    SoundEffect(0x2004,     "OOTEponaRun",          SND_NONE,               SND_NONE            ),
    SoundEffect(0x2005,     "OOTEponaNeigh",        SND_NONE,               SND_NONE            ),  // Duplicate, per MMR
    SoundEffect(0x2006,     "RiverFlow",            SND_LOOP,               SND_NONE            ), 
    SoundEffect(0x2007,     "WaterfallBig",         SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2008,     "FishOutOfWater",       SND_SHORT | SND_BEEP,   SND_NONE            ),
    SoundEffect(0x2009,     "DiveIntoWater",        SND_NONE,               SND_NONE            ),
    SoundEffect(0x200A,     "BlockSlide",           SND_NONE,               SND_NONE            ),
    SoundEffect(0x200B,     "MagmaBubble",          SND_SHORT | SND_BEEP,   SND_NONE            ), // MMR has this as 0x2008?
    SoundEffect(0x200C,     "DrawbridgeOpening",    SND_LOOP,               SND_NONE            ),
    SoundEffect(0x200D,     "DrawbridgeClosing",    SND_LOOP,               SND_NONE            ),
    SoundEffect(0x200E,     "DrawbridgeOpened",     SND_SHORT | SND_LONG,   SND_NONE            ),
    SoundEffect(0x200F,     "DrawbridgeClosed",     SND_SHORT | SND_LONG,   SND_NONE            ),
    SoundEffect(0x2010,     "WallExplode",          SND_LONG,               SND_NONE            ),
    SoundEffect(0x2011,     "CuccoCluck",           SND_LONG,               SND_NONE            ),
    SoundEffect(0x2012,     "CuccoAttacked",        SND_LONG,               SND_NONE            ),
    SoundEffect(0x2013,     "CuccoMorningCrow",     SND_LONG,               SND_LONG            ),
    SoundEffect(0x2014,     "WoodDoorSlideOpen",    SND_NONE,               SND_NONE            ),
    SoundEffect(0x2015,     "FloorSwitchPress",     SND_NONE,               SND_NONE            ),
    SoundEffect(0x2016,     "OOTEponaSnort",        SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2017,     "BombFizzle",           SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2018,     "EponaJump",            SND_NONE,               SND_NONE            ), // OoT Epona?
    SoundEffect(0x2019,     "EponaLand",            SND_NONE,               SND_NONE            ), // OoT Epona?
    SoundEffect(0x201A,     "EponaSlide",           SND_NONE,               SND_NONE            ),
    SoundEffect(0x201B,     "TatlFly",              SND_SHORT,              SND_NONE            ),
    SoundEffect(0x201C,     "WoodDoorSlideClosing", SND_SHORT,              SND_NONE            ),
    SoundEffect(0x201D,     "WoodDoorSlideClosed",  SND_SHORT,              SND_NONE            ),
    SoundEffect(0x201E,     "DoorOfTimeOpen",       SND_LONG,               SND_NONE            ),
    SoundEffect(0x201F,     "ChestUnlock",          SND_NONE,               SND_NONE            ),
    SoundEffect(0x2020,     "ChestCreakOpen",       SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2021,     "SysTimerTicking",      SND_LOOP,               SND_NONE            ),  // Wrong bank?
    SoundEffect(0x2022,     "TorchIgnite",          SND_NONE,               SND_NONE            ),
    SoundEffect(0x2023,     "GuardSpearHit",        SND_NONE,               SND_NONE            ),
    SoundEffect(0x2024,     "ElevatorMove",         SND_NONE,               SND_NONE            ),
    SoundEffect(0x2025,     "BlueWarpWait",         SND_LOOP,               SND_LOOP            ),
    SoundEffect(0x2026,     "BlueWarpActivate",     SND_LONG,               SND_NONE            ), // Wtf that's the year that it is
    SoundEffect(0x2027,     "PillarSink",           SND_NONE,               SND_NONE            ), // Wtf that's the year this hopefully releases
    SoundEffect(0x2028,     "WaterfallSmall",       SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2029,     "RiverSmall",           SND_LOOP,               SND_NONE            ),
    SoundEffect(0x202A,     "RiverBig",             SND_LOOP,               SND_NONE            ),
    SoundEffect(0x202B,     "EponaLandDirt",        SND_NONE,               SND_NONE            ), // OoT Epona?
    SoundEffect(0x202C,     "EponaLandSand",        SND_NONE,               SND_NONE            ), // OoT Epona?
    SoundEffect(0x202D,     "DUMMY_202D",           SND_NONE,               SND_NONE            ),
    SoundEffect(0x202E,     "TwinmoldHeadLand",     SND_SHORT,              SND_NONE            ),
    SoundEffect(0x202F,     "BombBounce",           SND_NONE,               SND_NONE            ),
    SoundEffect(0x2030,     "WaterDripping",        SND_NONE,               SND_NONE            ), // Loops
    SoundEffect(0x2031,     "TorchCrackle",         SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2032,     "MagmaBubble2",         SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2033,     "FirePillarBlaze",      SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2034,     "FireWallBlaze",        SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2035,     "BlockPushAgainstWall", SND_NONE,               SND_NONE            ),
    SoundEffect(0x2036,     "MetalDoorSlide",       SND_NONE,               SND_NONE            ),
    SoundEffect(0x2037,     "MetalDoorStop",        SND_NONE,               SND_NONE            ),
    SoundEffect(0x2038,     "FireTempleBlockVibe",  SND_NONE,               SND_NONE            ), // whar
    SoundEffect(0x2039,     "CrateDestroy",         SND_SHORT,              SND_NONE            ), // Also used for gears falling off clock tower, I think?
    SoundEffect(0x203A,     "HammerSwitch",         SND_NONE,               SND_NONE            ),
    SoundEffect(0x203B,     "MagmaBubble3",         SND_SHORT,              SND_NONE            ),
    SoundEffect(0x203C,     "PhantomGanonGate",     SND_SHORT,              SND_NONE            ),
    SoundEffect(0x203D,     "WaterDrip",            SND_SHORT | SND_BEEP,   SND_NONE            ),
    SoundEffect(0x203E,     "LongExplosion",        SND_LONG,               SND_NONE            ), // Maybe unused?
    SoundEffect(0x203F,     "WaterWallBigSilent",   SND_NONE,               SND_NONE            ), // idk what this is
    SoundEffect(0x2040,     "KoumeKotakeWarpHole",  SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2041,     "FountainGush",         SND_NONE,               SND_NONE            ), // No loop?
    SoundEffect(0x2042,     "EponaWalk",            SND_NONE,               SND_NONE            ),
    SoundEffect(0x2043,     "EponaRun",             SND_NONE,               SND_NONE            ),
    SoundEffect(0x2044,     "EponaNeigh",           SND_SHORT | SND_LONG,   SND_SHORT | SND_LONG), // MMR has this as beep, but... yeesh
    SoundEffect(0x2045,     "EponaSnort",           SND_SHORT,              SND_SHORT           ),
    SoundEffect(0x2046,     "GiantRoomLeave",       SND_NONE,               SND_NONE            ), // Zelda 64 loves its high-pitched whistles
    SoundEffect(0x2047,     "BossRemainsGoToWall",  SND_LOOP,               SND_LOOP            ),
    SoundEffect(0x2048,     "TreeCut",              SND_NONE,               SND_NONE            ), // Not sure what this is. Tree hit w/ sword?
    SoundEffect(0x2049,     "DeathMountainErupt",   SND_LONG,               SND_NONE            ),
    SoundEffect(0x204A,     "PostmanWalk",          SND_BEEP,               SND_NONE            ), // bwooip bwooip bwooip bwooip
    SoundEffect(0x204B,     "PostmanSlapPostbox",   SND_NONE,               SND_NONE            ),
    SoundEffect(0x204C,     "PostboxOpen",          SND_SHORT,              SND_SHORT           ), // Ka-ching
}