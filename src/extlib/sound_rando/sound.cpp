#include "sound.hpp"
#include <array>

// Sound slots
// Details from https://docs.google.com/spreadsheets/d/1YVJ7GdzNZUese6H8d40lzpxp8ZfWlt75c-lzBzKngmo/edit?gid=1343879920#gid=1343879920
// (Credit to ZoeyZolotova/isghj for linking to the above in MMR)
std::array<SoundEffect, 0x200> EnvSounds
{
    SoundEffect(0x2000,     "WoodenDoorOpen",       SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2001,     "WoodenDoorClose",      SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2002,     "BigExplosion",         SND_NONE,               SND_NONE            ), // Not bombs
    SoundEffect(0x2003,     "OOTEponaWalk",         SND_NONE,               SND_NONE            ),
    SoundEffect(0x2004,     "OOTEponaRun",          SND_NONE,               SND_NONE            ),
    SoundEffect(0x2005,     "OOTEponaNeigh",        SND_NONE,               SND_NONE            ),  // Duplicate, per MMR
    SoundEffect(0x2006,     "RiverFlow",            SND_LOOP,               SND_NONE            ), 
    SoundEffect(0x2007,     "WaterfallBig",         SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2008,     "FishOutOfWater",       SND_SHORT | SND_BEEP,   SND_NONE            ),
    SoundEffect(0x2009,     "DiveIntoWater",        SND_NONE,               SND_NONE            ),
    SoundEffect(0x200A,     "BlockSlide",           SND_NONE,               SND_NONE            ),
    SoundEffect(0x200B,     "MagmaBubble",          SND_NONE,               SND_NONE            ), // MMR has this as 0x2008?
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
    SoundEffect(0x2021,     "TimerTicking",         SND_LOOP,               SND_NONE            ),  // Pea-nut but-ter pea-nut but-ter (Wrong bank)
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
    SoundEffect(0x2030,     "WaterDripping",        SND_NONE,               SND_NONE            ),
    SoundEffect(0x2031,     "TorchCrackle",         SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2032,     "MagmaBubble2",         SND_LOOP,               SND_NONE            ),
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
    SoundEffect(0x203F,     "WaterWallBigSilent",   SND_NONE,               SND_NONE            ), // Not, in fact, silent
    SoundEffect(0x2040,     "KoumeKotakeWarpHole",  SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2041,     "FountainGush",         SND_NONE,               SND_NONE            ),
    SoundEffect(0x2042,     "EponaWalk",            SND_NONE,               SND_NONE            ),
    SoundEffect(0x2043,     "EponaRun",             SND_NONE,               SND_NONE            ),
    SoundEffect(0x2044,     "EponaNeigh",           SND_SHORT | SND_LONG,   SND_SHORT | SND_LONG), // MMR has this as beep, but... yeesh
    SoundEffect(0x2045,     "EponaSnort",           SND_SHORT,              SND_SHORT           ),
    SoundEffect(0x2046,     "GiantRoomLeave",       SND_NONE,               SND_NONE            ), // Zelda 64 loves its high-pitched whistles
    SoundEffect(0x2047,     "BossRemainsGoToWall",  SND_LOOP,               SND_LOOP            ),
    SoundEffect(0x2048,     "TreeCut",              SND_NONE,               SND_NONE            ), // Not sure what this is. Tree hit w/ sword?
    SoundEffect(0x2049,     "DeathMountainErupt",   SND_LOOP,               SND_NONE            ),
    SoundEffect(0x204A,     "PostmanWalk",          SND_BEEP,               SND_NONE            ), // bwooip bwooip bwooip bwooip
    SoundEffect(0x204B,     "PostmanSlapPostbox",   SND_NONE,               SND_NONE            ),
    SoundEffect(0x204C,     "PostboxOpen",          SND_SHORT,              SND_SHORT           ), // Ka-ching
    SoundEffect(0x204D,     "ChainTrapDown",        SND_NONE,               SND_NONE            ), // Idk what this is
    SoundEffect(0x204E,     "GrassCut",             SND_BEEP,               SND_NONE            ),
    SoundEffect(0x204F,     "StoneSwitchDown",      SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2050,     "FlagFlap",             SND_NONE,               SND_NONE            ),
    SoundEffect(0x2051,     "TrapBounce",           SND_NONE,               SND_NONE            ),
    SoundEffect(0x2052,     "RockBreak",            SND_NONE,               SND_NONE            ), // Also used for guy digging at rock
    SoundEffect(0x2053,     "PhantomGanonWarp",     SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2054,     "PhantomGanonWarpLoop", SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2055,     "CoffinLidOpen",        SND_NONE,               SND_NONE            ),
    SoundEffect(0x2056,     "ChestBounce",          SND_NONE,               SND_NONE            ),
    SoundEffect(0x2057,     "OoTFanBlow",           SND_NONE,               SND_NONE            ),
    SoundEffect(0x2058,     "SpikeWallSlide",       SND_NONE,               SND_NONE            ),
    SoundEffect(0x2059,     "PortcullisRise",       SND_NONE,               SND_NONE            ),
    SoundEffect(0x205A,     "PortcullisFall",       SND_NONE,               SND_NONE            ),
    SoundEffect(0x205B,     "MajoraBurn",           SND_NONE,               SND_NONE            ), // Majora burning after first phase?
    SoundEffect(0x205C,     "GanondorfBlockFall",   SND_NONE,               SND_NONE            ),
    SoundEffect(0x205D,     "OoTMarketChatter",     SND_LOOP,               SND_NONE            ),
    SoundEffect(0x205E,     "WaterLevelRise",       SND_NONE,               SND_NONE            ), // Ikana well water rising
    SoundEffect(0x205F,     "TatlHide",             SND_BEEP | SND_SHORT,   SND_NONE            ),
    SoundEffect(0x2060,     "StoneSwitchUp",        SND_NONE,               SND_NONE            ),
    SoundEffect(0x2061,     "DekuTreeWebShake",     SND_NONE,               SND_NONE            ),
    SoundEffect(0x2062,     "IcePlatformAppear",    SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2063,     "RollStandAppear",      SND_NONE,               SND_NONE            ), // Holy shit first contact year also idk what this is
    SoundEffect(0x2064,     "WaterSeesawBounce",    SND_NONE,               SND_NONE            ), // GBT scales?
    SoundEffect(0x2065,     "ConveyorBeltMove",     SND_NONE,               SND_NONE            ),
    SoundEffect(0x2066,     "ClockTowerDoorOpen",   SND_LONG,               SND_NONE            ),
    SoundEffect(0x2067,     "OoTMetalGateOpen",     SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2068,     "BottleScoopWater",     SND_NONE,               SND_NONE            ), // Wrong bank
    SoundEffect(0x2069,     "FlopLikeAFish",        SND_BEEP,               SND_NONE            ),
    SoundEffect(0x206A,     "ScarecrowWave",        SND_SHORT,              SND_NONE            ),
    SoundEffect(0x206B,     "ScarecrowSpin",        SND_SHORT,              SND_NONE            ),
    SoundEffect(0x206C,     "BottleUncap",          SND_BEEP,               SND_NONE            ), // Wrong bank? Pop is funny for beep though
    SoundEffect(0x206D,     "GossipStoneFlatten",   SND_NONE,               SND_NONE            ), // I feel like this is used for something else in OoT
    SoundEffect(0x206E,     "SkarecrowShake",       SND_SHORT,              SND_NONE            ),
    SoundEffect(0x206F,     "SunSwitchActivate",    SND_NONE,               SND_NONE            ),
    SoundEffect(0x2070,     "RutoFallTroughHernia", SND_NONE,               SND_NONE            ),
    SoundEffect(0x2071,     "SeesawBounce",         SND_NONE,               SND_NONE            ),
    SoundEffect(0x2072,     "TatlAttackDoor",       SND_SHORT | SND_BEEP,   SND_NONE            ),
    SoundEffect(0x2073,     "WoodFall",             SND_BEEP,               SND_NONE            ),
    SoundEffect(0x2074,     "ScoopUpWater",         SND_NONE,               SND_NONE            ), // No idea what's supposed to be scooping up water here
    SoundEffect(0x2075,     "WaterDripLoop",        SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2076,     "WindmillGearSpin",     SND_NONE,               SND_NONE            ),
    SoundEffect(0x2077,     "BushSway",             SND_BEEP,               SND_NONE            ),
    SoundEffect(0x2078,     "AutoDoorClose",        SND_NONE,               SND_NONE            ), // idk
    SoundEffect(0x2079,     "TatlFly",              SND_BEEP,               SND_NONE            ), // Called "Rebirth"?
    SoundEffect(0x207A,     "ChainLiftStop",        SND_NONE,               SND_NONE            ),
    SoundEffect(0x207B,     "ChestAppear",          SND_LONG,               SND_NONE            ),
    SoundEffect(0x207C,     "SmallKeyUse",          SND_SHORT,              SND_NONE            ),
    SoundEffect(0x207D,     "ScytheMove",           SND_SHORT,              SND_NONE            ), // Also poker chip thingies
    SoundEffect(0x207E,     "HealingAura",          SND_NONE,               SND_NONE            ),
    SoundEffect(0x207F,     "GreatFairyAppear",     SND_NONE,               SND_NONE            ),
    SoundEffect(0x2080,     "GreatFairyVanish",     SND_NONE,               SND_NONE            ),
    SoundEffect(0x2081,     "FakeEyeSwitchFire",    SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2082,     "RollStand2",           SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2083,     "WallSlide",            SND_LOOP,               SND_NONE            ),
    SoundEffect(0x2084,     "ChestGlow",            SND_SHORT,              SND_NONE            ), // During opening animation
    SoundEffect(0x2085,     "ClockTowerGear",       SND_LOOP,               SND_NONE            ), // I think this is right, but listed as only OoT Windmill
    SoundEffect(0x2086,     "GoToHeaven",           SND_NONE,               SND_NONE            ), // Lol? Another generic whistle
    SoundEffect(0x2087,     "PotShatter",           SND_SHORT | SND_BEEP,   SND_NONE            ),
    SoundEffect(0x2088,     "PutPotDown",           SND_NONE,               SND_NONE            ),
    SoundEffect(0x2089,     "DiveIntoWater",        SND_LONG,               SND_NONE            ),
    SoundEffect(0x208A,     "ClimbOutOfWater",      SND_SHORT,              SND_NONE            ),
    SoundEffect(0x208B,     "IcicleShatter",        SND_NONE,               SND_NONE            ),
    SoundEffect(0x208C,     "TriforceShine",        SND_NONE,               SND_NONE            ), // OoT Cutscenes
    SoundEffect(0x208D,     "LinkAura",             SND_NONE,               SND_NONE            ), // Yet another whistle
    SoundEffect(0x208E,     "StrayFairyAppear",     SND_SHORT,              SND_SHORT           ),
    SoundEffect(0x208F,     "JabuJabuPlatformRise", SND_NONE,               SND_NONE            ),
    SoundEffect(0x2090,     "JabuJabuPlatformSink", SND_NONE,               SND_NONE            ),
    SoundEffect(0x2091,     "MilkBottleShatter",    SND_SHORT,              SND_NONE            ),
    SoundEffect(0x2092,     "ChairRoll",            SND_NONE,               SND_NONE            ), // Idk
    SoundEffect(0x2093,     "BigStoneDoorThud",     SND_SHORT | SND_BEEP,   SND_NONE            ),
    SoundEffect(0x2094,     "SpiritualStoneAppear", SND_NONE,               SND_NONE            ), // At least the whistle is interesting this time
    SoundEffect(0x2095,     "WaterBubbleLoop",      SND_NONE,               SND_NONE            ),
    SoundEffect(0x2096,     "BomberKidSit",         SND_NONE,               SND_NONE            ),
    SoundEffect(0x2097,     "ToiletFlush",          SND_LONG,               SND_SHORT | SND_LONG),
    SoundEffect(0x2098,     "Earthquake",           SND_NONE,               SND_NONE            ),
    SoundEffect(0x2099,     "Sweep",                SND_NONE,               SND_NONE            ), // Mr. Barten?
    SoundEffect(0x209A,     "SagesBecomeLight",     SND_NONE,               SND_NONE            ),
    SoundEffect(0x209B,     "SagesFlyAround",       SND_NONE,               SND_NONE            ),
    SoundEffect(0x209C,     "SagesTrapGanon",       SND_NONE,               SND_NONE            ), // Duplicate?
    SoundEffect(0x209D,     "OoTTimeTravel",        SND_NONE,               SND_NONE            ),
    SoundEffect(0x209E,     "KafeiDoorbell",        SND_SHORT,              SND_SHORT | SND_LONG),
    SoundEffect(0x209F,     "BounceOnMagma",        SND_NONE,               SND_NONE            ),
};