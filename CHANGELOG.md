# v0.1.4
* **Changed music directory to `mod_data/music`.**
* Added a progress counter to the debug mode when reading the `music` directory.
* Fixed a bug where custom sounds were not deleted from the database properly.
* Removed ProxyMM-KV as a dependency.
* Updated README.md.

# v0.1.3
* Fixed issue with category string not being null-terminated, resulting in final categories not being read correctly in some circumstances.
* Added a log-level mask to a certain print_bytes call.

# v0.1.2
* Fixed handling of empty or missing music folder.
* Updated Audio API dependency to require version 0.5.2.
* Restored some debug log messages.
* Updated README.md.

# v0.1.1
* Added Recomp Rando as a dependency.

# v0.1.0
* Initial release.