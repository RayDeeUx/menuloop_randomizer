# v1.4.1
- Attempted to fix a weird `stoi` crash. Not sure why it happens, but it did for the one person who reported it, and they didn't really provide further context beyond a crashlog file reading `C++ Exception: class std::out_of_range("stoi argument out of range")`.
- License the mod under LGPLv2.
# v1.4.0
- Added setting to choose font for "Now Playing" notifications.
- Added an extra child folder in the mod's config directory for enabling/disabling menuloops easily. The mod itself doesn't do anything to it beyond creating the child folder.
- Hopefully fix the mistake where macOS ARM support wasn't added the first time in v1.3.0.
- Fix rolling the same song twice in a row. (Unless if you have only one song loaded for the mod.)
- Fix that one godforsaken Android crash. (It was caused by ninxout being a bit silly and failing to check if certain directories existed on Android.)
  - Incidentally, this also fixes potential issues where songs from RobTop's Music Library *could* be added twice on certain platforms, skewing the probability distribution.
- Fix arrow steps in settings being wonky. (Geode has yet to support floating point increments in arrow steps.)
- Notifications now move off-screen when switching menus to minimize chances of the song title in the "Now Playing" notification from being incorrect.
- Add text formatting options for songs from Newgrounds or RobTop's Music Library.
  - Song Name
  - Song Name + Artist
  - Song Name + Song ID
  - Song Name, Artist, Song ID
- Added punctuation to the changelog.
# v1.3.0
- Added macOS support (thx [ninxout](https://github.com/ninXout), [raydeeux](https://github.com/RayDeeUx), and [hiimjustin000](https://github.com/hiimjustin000)).
- Added support for `.wav`, `.ogg`/`.oga`, and `.flac` files for custom songs.
- Included artist name for all "Now Playing" notifications.
- Set Z order of the "Now Playing" notification to 200 for easier readability against most MenuLayer mods (such as Overcharged Main Menu).
- Shuffling the song now generates a new "Now Playing" notification.
- Increase "Now Playing" notification maximum duration to 5 seconds.
- Add better slider control for adjusting notification duration.
- Added Node IDs to nodes added by this mod.
- Squash a few bugs, optimize a few things, minimize likelihood of potential crashes.
# v1.2.2
- Added an option to enable the shuffle button (thx [reinmar](https://github.com/Reinmmar)).
# v1.2.1
- Fixed crash caused by Newgrounds manager trying to gather info of default menuLoop.mp3.
# v1.2.0
- Added `Randomize on editor exit` setting.
- Added `Notification time` setting.
- Added `Use custom songs path` setting.
- Added `SongManager.hpp`.
- Added custom songs support.
- Added `stoi` to `Utils` (prolly it crashed because of Autonong).
- Added a button to add custom songs (present in the settings tab).
- Added a button for randomizing the song manually.
- Most of the functions related to the `m_songs` vector are now in `SongManager`.
- Fixed crash when there are no songs (original menuLoop plays instead).
- Fixed crash caused by switching the songs location.
- Modified `PlayingCard` to fit the new system.
- Now the song randomizes every time you change `Use custom songs path`.
# v1.1.0
- Fixed crash when opening the game (missing textures crash).
- Fixed random song picker being on a loop for every song downloaded.
- Moved song class to its own file.
- The mod should now check for the songs path when loaded, not when executed.
- Added setting to randomize song on level exit.
- Fixed Android crash caused by not able to access the memory (f*** pointers).
- Added `Utils.hpp`
# v1.1.0-alpha.5
- Updated Geode target version to `v3.0.0-beta.1`.
# v1.1.0-alpha.4
- Updated notification card & position.
- Added the property `name` to the `Song` class.
# v1.1.0-alpha.3
- Added a notification animation for the card.
- The position is now added in `MainLayer` instead of the layer itself.
# v1.1.0-alpha.2
- Fixed crash on startup caused by a missing dependency.
# v1.1.0-alpha.1
- Added a card showing which song is playing. (you can disable it in settings.)
- Changed a little bit how songs are stored in the vector.
- Removed MacOS support. (sorry, i don't wanna deal with MacOS bindings.)
### Notes
- This version is for `Geode v3.0.0-alpha.2` only.
# v1.0.0
- Initial release