# v1.0.1
* Fixed a crash related to custom tracks with ID `0x1D`.
* Removed links to weird chess website in changelog.
* Updated README.

# v1.0.0
* Total rewrite of entire mod. Too many changes to list here
* Updated README.

# v0.2.0
* **Changed music directory to `mod_data/music`.**
* Added a progress counter to the debug mode when reading the `music` directory.
* Fixed a bug where an improperly initialized audiotable would overwrite data from other mods, resulting in many unpredictable behaviors.
* Fixed a bug where individual song slots would not be considered for randomization.
* Implemented a temporary workaround for a bug with soundfonts above id 255.
* Fixed a bug where custom sounds were not deleted from the database properly.
* Removed ProxyMM-KV as a dependency.
* Updated README.

# v0.1.3
* Fixed issue with category string not being null-terminated, resulting in final categories not being read correctly in some circumstances.
* Added a log-level mask to a certain print_bytes call.
* Updated README.

# v0.1.2
* Fixed handling of empty or missing music folder.
* Updated Audio API dependency to require version 0.5.2.
* Restored some debug log messages.
* Updated README.

# v0.1.1
* Added Recomp Rando as a dependency.
* Updated README.

# v0.1.0
* Initial release.
* Updated README.