# MM Recomp Music Randomizer (v1.0.0)

Reimplements the music randomization features from [MMR](https://github.com/ZoeyZolotova/mm-rando).

The best place to find custom music tracks for MM is [the music-releases channel on the MMR Discord](https://discord.gg/pJdQMqGWXp). You can also find all of my own soundfont adaptations [on my GitHub](https://github.com/AACooper1/MM-Soundfont-Songs/tree/main).

If you want to report any, please contact me on the N64 Recomp Discord, N64 Recomp Modding Discord, DM me on Discord directly at @asticky, or open an issue on GitHub.

## Instructions:
Drag and drop all mod files (the `.nrm`, and the `.dll` for Windows, `.so` for Linux, or `.dylib` for MacOS) into your `mods` directory, or use the `Install Mods` button on the Zelda64 Recomp Launcher's mod menu.

To add custom music, create a `mod_data` folder in the same directory as `mods`, then create a `music` folder and drop your `.mmrs` files inside that. (i.e., on Windows, `AppData\Local\Zelda64Recompiled\mod_data\music`.) These directories will be created on launching the game if they do not exist. 

A database will be created in the `mod_data` folder, which will add or remove tracks based on filenames and modification dates. If you want to reset your music database, you can delete `mod_data\musicDB.db`.

Your folder structure should look like this:

```
⌞Zelda64Recompiled
  ⌞mods
    ⌞mm_recomp_music_randomizer.nrm
    ⌞mm_recomp_music_randomizer_extlib(.dll|.so|.dylib)
    ⌞magemods_audio_api.nrm
    ⌞magemods_audio_api(.dll|.so|.dylib)
  ⌞mod_data
    ⌞music.db
    ⌞music
      ⌞(.mmrs files)
```

## Features:
* Reads and interprets `.mmrs` files

* Shuffles both vanilla and custom music using the same categories as MMR

* Supports custom sounds, custom audiobanks, and `.formmask`

* Adds a toggle to enable or disable combat music

* Displays the name of the currently-playing song on load

## New in 1.0!
* Greatly improved stability. Probably won't crash for you now.
* Can now choose custom-only or vanilla-only randomization on generation of a seed.
* Music seeds are now saved to disk on a per-save-file basis.
* The Morning jingle can now be randomized and can appear as a random track. (this is worth a mention bc it was really hard to implement lmao)
* Can now run without rando.
* Implemented MMR's Songtest and Songforce features.
* Randomization now happens on the extlib side, meaning the pool of songs for any seed is no longer limited to 256 songs.
* Updated song title display to use RecompUI. Much prettier, works on the title screen, and allows for longer song name display.
* New option to log to an external file
* Many, many bugfixes
