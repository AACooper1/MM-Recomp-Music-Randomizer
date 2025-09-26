# MM Recomp Music Randomizer (v0.1.4)

Reimplements the music randomization features from [MMR](https://github.com/ZoeyZolotova/mm-rando).

**This mod is still in development. This version should be considered an initial beta release, and there will almost certainly be bugs.** If you want to report any, please contact me on the N64 Recomp Modding Discord, DM me on Discord directly at @asticky, or open an issue on GitHub.

## Instructions:
Drag and drop all mod files into your `mods` directory (usually `AppData\Local\Zelda64Recompiled\mods` on Windows).

Additionally, download the latest release of [MageMods' Audio API](https://github.com/magemods/mm-audio-api/tags) and install it in the same way. **This is required.**

To add custom music, create a `mod_data` folder in the same directory as `mods`, then create a `music` folder and drop your `.mmrs` files inside that (i.e., on Windows, `AppData\Local\Zelda64Recompiled\mod_data\music`). These directories will be created on launching the game if they do not exist.

A database will be created in the `mod_data` folder, which will add or remove tracks based on filenames and modification dates. If you want to reset your music database, you can delete `mod_data\musicDB.db`.

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