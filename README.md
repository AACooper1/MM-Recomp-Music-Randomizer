# MM Recomp Music Randomizer (v0.1.0)

Reimplements the music randomization features from [MMR](https://github.com/ZoeyZolotova/mm-rando).

**This mod is still in development. This version should be considered an initial beta release, and there will almost certainly be bugs.** If you want to report any, please contact me on the N64 Recomp Modding Discord, DM me on Discord directly at @asticky, or open an issue on GitHub.

## Instructions:
Extract all mod files into your `mods` directory (usually `AppData\Local\Zelda64Recompiled\mods` on Windows).

Additionally, download the latest release of [MageMods' Audio API](https://github.com/magemods/mm-audio-api/tags) and install it in the same way. **This is required.**

To add custom music, create a `music` folder in the same directory as your `Zelda64Recompiled.exe` and drop `.mmrs` files inside that folder. 

A database will be created in the `assets` folder in the same parent directory, which will add or remove tracks based on filenames and modification dates. If you want to reset your music database, you can delete `assets/musicDB.db`.

## Features:
* Reads and interprets `.mmrs` files

* Shuffles both vanilla and custom music using the same categories as MMR

* Supports custom sounds, custom audiobanks, and `.formmask`

* Adds a toggle to enable or disable combat music

* Displays the name of the currently-playing song on load (max 40 characters)

## Planned features:

* Randomization of certain sounds, as in MMR

* Support for OOTRS and streamed music tracks

* Music menu, with support for randomization changes, song testing, and volume control

Requires [MageMods' custom audio API](https://github.com/magemods/mm-audio-api). Made using [LT_Schmiddy's extlib template](https://github.com/LT-Schmiddy/SchmiddysMMRecompModTemplate).