# Menu Loop Randomizer Changelog
## v1.12.0
*<c-aaaaaa>Friendly reminder that</c> <cl>some</c> <cj>features</c> <c-aaaaaa>of this mod (including features from this update and features related to bugfixes in this update) require [Better Touch Prio](mod:alk.better-touch-prio) to be installed. These decisions are still here to stay.</c>*
- <cg>Added</c> playback increment/decrement controls and playback progress to both the Control Panel and the Song List menus.
  - You can toggle this option off for either menu in the mod settings.
  - You can also adjust how many milliseconds to increment/decrement by in the mod settings. Changes apply to both menus.
  - [Better Touch Prio](mod:alk.better-touch-prio) is <cl>requ</c><cj>ired</c> for this feature.
- <cg>Added</c> song name indicators for the song name text in the Control Panel.
  - You can toggle this option off in the mod settings.
  - Song name indicators for song name label are the same as those seen in the Song List menu.
  - For a number of reasons, the green "current song" indicator and the small dark gradient indicator for Jukebox songs do not apply to the Control Panel.
- <co>Fixed</c> an issue where menu loop positions wouldn't be restored when leaving a level whose most recently played song was also the most recently selected menu loop.
- <co>Fixed</c> several UI-related issues related to setting/unsetting a Song Override.
- <co>"Fixed"</c> a long-standing bug where song names and artist names modified by Jukebox would spill over to menu loop display names.
  - The quotation marks are there because the approach involved manually parsing Jukebox's own JSON data to fetch the original song name/artist name.
  - <cr>Please don't use this changelog item as a justification to harass any of the developers of the Jukebox mod, *especially* if you don't know what's going on beneath the hood in either mod.</c>
## v1.11.3, v1.11.4, and v1.11.5
*<c-aaaaaa>Friendly reminder that</c> <cl>some</c> <cj>features</c> <c-aaaaaa>of this mod (including features related to bugfixes in this update) require [Better Touch Prio](mod:alk.better-touch-prio) to be installed. These decisions are still here to stay.</c>*
- Fix massive oversights.
- Reduce log spam when Advanced Logs are enabled.
- Fix a visual error when trying to favorite a song twice.
- Fix changes not being reflected in the Song List menu when favoriting/blacklisting a song.
## v1.11.2
*<c-aaaaaa>Friendly reminder that</c> <cl>some</c> <cj>features</c> <c-aaaaaa>of this mod (including features from this update) require [Better Touch Prio](mod:alk.better-touch-prio) to be installed. These decisions are still here to stay.</c>*
- Added the <cj>File Extension sorting option</c> to the Song List.
  - Sort files based on their file extension alphabetically.
  - [Better Touch Prio](mod:alk.better-touch-prio) is <cl>requ</c><cj>ired</c> to use this <cj>sorting</c> option in the Song List menu.
- Attempted to offload Song Length calculations to another thread.
  - <cj>Sorting</c> songs by <cj>Song Length</c> should not freeze your game for as long now. <c-aaaaaa>*(Unless if you change your settings too frequently, but **__why__** would you want to do that?)*</c>
  - The mod setting to disable <cj>sorting</c> by <cj>song length</c> is still available. However, this is now a purely cosmetic setting, since song length calculations happen for everyone unless if [Better Touch Prio](mod:alk.better-touch-prio) is not loaded.
  - [Better Touch Prio](mod:alk.better-touch-prio) is still <cl>requ</c><cj>ired</c> to use the Song Length <cj>sorting</c> option in the Song List menu.
- Added an option to show extra information about a song when Compact Mode is enabled in the Song List menu.
  - You can toggle this option off in the mod settings.
  - [Better Touch Prio](mod:alk.better-touch-prio) is <cl>requ</c><cj>ired</c> to use this feature, as this feature relies on Compact Mode being enabled.
- Added a setting to use Miniaudio instead of FMOD to perform (most) song length calculations.
  - This setting is only available for non-iOS users.
  - This setting is enabled by default to preserve past behavior.
  - If this setting is disabled, FMOD will perform song length calculations for all of your songs instead.
  - This option is available for the minority of non-iOS users who prefer using FMOD to calculate song file lengths.
  - [Better Touch Prio](mod:alk.better-touch-prio) is <cl>requ</c><cj>ired</c> to use this feature, as calculating song file lengths without [Better Touch Prio](mod:alk.better-touch-prio) loaded would only waste resources.
- Fix a bug where some song length calculations would be inaccurate if Miniaudio failed to calculate the length.
  - Most calculations that used to fail in v1.11.0 or v1.11.1 will use FMOD instead.
- Adjust height limits for song name labels when Compact Mode is enabled in the Song List menu.
- Fix log spamming that would happen if  `Advanced Logging`  was enabled while inside the level editor.
- Fix a bug where you could play empty song files from the Song List menu if you tried hard enough.
- Even more various (attempts at) micro-optimizations.
## v1.11.1
- Hotfix.
## v1.11.0
***__This is a really lengthy changelog; be sure to read everything!__***
*(Changelog items/subitems containing <cl>blue</c> or <cj>cyan</c> text indicates that [Better Touch Prio](mod:alk.better-touch-prio) is required to use that feature. These decisions will not be reverted anytime soon.)*
- <cg>Added</c> `Continue Menu Loop on Level Exit` and `Continue Menu Loop on Editor Exit` (suggested by [Myron472/GlydeR](https://github.com/Myron472) and [Abagur900](https://github.com/Abagur900)).
  - <c-ff0000>***__READ:__*** **__These two toggles will__** ***__ONLY__*** **__work if you disable [Colon's Menu Loop Start Time mod](mod:colon.menu_loop_start_time)!!!__** [You can either disable it through its mod settings or through Geode's buttons, either is fine.]</c>
  - These toggles are mutually exclusive with their corresponding `Randomize Menu Loop on Level Exit` and `Randomize Menu Loop on Editor Exit` toggles.
  - Example: if `Randomize Menu Loop on Level Exit` is enabled, it will apply first, and ignore your preference for `Continue Menu Loop on Level Exit`.
  - The same logic applies to the `Randomize Menu Loop on Editor Exit` and `Continue Menu Loop on Editor Exit` toggles.
- <cg>Added</c> a Song List scrolling shortcut to the currently playing song. 
  - Shortcuts for the top and bottom of the song list also included, because why not.
  - Song list scrolling shortcuts are to the right of the scrollbar.
  - You can toggle it off in mod settings.
- <cg>Added</c> <cl>Compact Mode</c> to the Song List.
  - Your preferences for Compact Mode are saved between restarts.
  - If you want even smaller song entries in the Song List, check the mod settings.
  - [Better Touch Prio](mod:alk.better-touch-prio) is <cl>requ</c><cj>ired</c> to use this feature.
  - You can toggle it off in mod settings.
- <cg>Added</c> a few <cl>filtering</c> and <cj>sorting</c> options to the Song List.
  - <cl>Favorites Only</c> - show only favorited songs.
  - <cl>Reverse</c> - reverses the Song List's entries.
  - <cj>Alphabetical</c> - sorts the Song List's entries alphabetically.
  - <cj>Date Created</c> - sorts the Song List's entries by the date they were added to your device.
  - <cj>Song Length</c> - sorts the Song List's entries by their length. <c-ff0000>[Hidden by default--this one can freeze your game and take an awfully long amount of time!]</c>
  - <cj>Song Size</c> - sorts the Song List's entries by file size.
  - Your preferences for these <cl>filtering</c> and <cj>sorting</c> options are saved between restarts.
  - All of the <cj>sorting</c> options are mutually exclusive from each other.
  - [Better Touch Prio](mod:alk.better-touch-prio) is <cl>requ</c><cj>ired</c> to use this feature.
  - You can toggle it off in mod settings.
- <co>Changed</c> the Search Bar a bit so it <cl>requ</c><cj>ires</c> [Better Touch Prio](mod:alk.better-touch-prio) to be used.
- <co>Changed</c> the Search Bar a bit so it can be toggled off in mod settings.
- <co>Changed</c> the placeholder text in the Search Bar in the Song List to include the name of the currently playing song.
- <co>Fix</c> an incompatibility with Eclipse Menu's "Transparent Lists" feature.
- <co>Attempt to reduce</c> frame drops when viewing many songs at once in the Song List menu.
- Various (<co>attempts</c> at) micro-optimizations.
## v1.10.1
- Add a scrollbar.
- Move binary releases to Codeberg.
## v1.10.0
- <co>Rewrite</c> `Constant Shuffle Mode`.
  - It should shuffle songs more consistently than before.
  - It should also not override/break menu loops from other screens in Geometry Dash anymore, or menu loops from other Geode mods.
- Add JIT-less iOS support. (Begrudgingly.)
- Remove all support for custom songs from the levels in The Tower.
## v1.9.3
- Move releases to be hosted on Codeberg instead. Octocat is being a big meanie this week and that's probably the last straw.
## v1.9.2
- GitHub keeps marking releases as drafts for some reason.
## v1.9.1
- Against all odds, GitHub decided to mark 1.9.0 as a draft release.
  - Pretend I slapped a bunch of profanities in here. >:(
- Nothing in this binary has changed, this is purely a dummy update so everyone fetches the update properly this time.
## v1.9.0
- <cg>Added</c> "playlists" support (suggested by [TheTrueNyxa](https://discord.com/users/702243715812556863))!
  - Add songs to a playlist file, then activate `Load Playlist File Instead` to load your playlist file!
  - Favorite songs lists and song blacklists still apply.
  - Constant Shuffle Mode (formerly `Playlist Mode`) is also supported.
- <cg>Added</c> a new menu to see the names of all loaded songs.
  - <c-FFCC00>Gold</c> song names indicate a favorited song, and <cg>green</c> song names indicate a currently playing song.
  - *Italicized* song names indicate a NG/ML song that came from a different directory than expected (currently can happen with Custom Songs Folder by Sawblade).
  - Each song entry (besides the currently playing one) has a play button so you can change songs anytime.
  - <c-FF0000>No, I will not be adding a search bar.</c>
- <cg>Added</c> `Minimal` as an option for `Button Mode`.
  - Only one button appears in this Button Mode: the "Control Panel" button.
- <co>Fix</c> an issue where changing some settings didn't render a new song title card. Oops!
- <co>Fix</c> an issue where blacklisting songs didn't update the name of the song chosen as the new menu loop. Oops!
- <co>Attempted to improve</c> compatibility with song file path names that use non-UTF-8 encoding.
  - This is most noticeable on Windows. We just ***love*** and ***appreciate*** lack of UTF-8 support for nearly ***two decades***, don't we? :)
- <co>Retroactively rename</c> `Playlist Mode` to `Constant Shuffle Mode` across as many front-facing areas as possible to avoid confusion with more recent features (see first bullet point).
- <cr>Removed</c> elasticity from MLR menu opening animations. <c-FF0000>***This is a non-negotiable change.***</c>
- <c-aaaaaa>Moved</c> repository URL to Codeberg.
## v1.8.4
- Added `Advanced Logging` toggle.
- Added an easter egg.
## v1.8.3
- Allowed the name of the currently playing song inside the controls menu to update more often.
- Included Constant Shuffle Mode and Override Mode indicators in the controls menu.
- Refactor some additional code.
- Removed a controversial function hook.
## v1.8.2
- Added experimental support for iOS devices.
  - <c-ff0000>READ: ***EXPERIMENTAL***.</c> <c-ffff00>Getting this mod to compile for iOS is already enough of a pain to begin with.</c>
- Added experimental support for searching songs in nested folders (suggested by [RighteousRyan](https://www.youtube.com/channel/UCnz-trf-dF7j8mDVOuoy4Ig)).
  - <c-ff0000>READ: ***EXPERIMENTAL***.</c> <c-ffff00>This setting may not work as expected for those with non-English folder paths/file names, or those using MLR on mobile devices.</c>
- Added experimental support for loading secondary folders (suggested by [RighteousRyan](https://www.youtube.com/channel/UCnz-trf-dF7j8mDVOuoy4Ig)).
  - <c-ff0000>READ: ***EXPERIMENTAL***.</c> <c-ffff00>This setting may not work as expected for those with non-English folder paths/file names, or those using MLR on mobile devices.</c>
## v1.8.1
- Attempt to fix a bug where some filenames would be ignored.
  - The guy who reported the issue to me won't respond to my DMs to test if my bugfix worked. Shame on them, I guess.
  - Thankfully dankmeme01 was able to test those changes, so all's well! (I hope.)
- Created a separate menu for all buttons from this mod.
  - To revert to the original behavior, use the "Classic" `Button Mode` setting.
  - Existing mod settings for toggling the <cl>Blacklist Song</c>, <cl>Favorite Song</c>, <cl>Hold Song</c>, and <cl>Previous Song</c> buttons will only apply when using the "Classic" `Button Mode` setting.
## v1.8.0
- Added "Hold Song" button. Think of it like that "Hold" button from Tetris.
- Added "Previous Song" button (suggested by [DurexSensitive](https://github.com/DurexSensitive)). <cy>This only stores *one* song at a time. Don't expect this intentional limitation to go away.</c>
- Added "Favorite Song" button (suggested by [AlphaQuata](https://discord.com/users/766876726566846505)). <cy>Now your favorite songs are more likely to play!</c>
- Refactor a few things inside the code for readability and whatnot.
## v1.7.0
- New logo by [dasshu_](https://linktr.ee/jaydasshu)!
- Refactor how the song notification card is removed when switching scenes.
  - Constant Shuffle Mode may have been broken in the process if there is only one song available for the mod to choose from. But at that point, you might as well use a texture pack instead.
## v1.6.4
- Added option to replace unsupported characters with `?`s, enabled by default.
- Only display extended warnings for `Constant Shuffle Mode` when enabling it from mod settings.
- Preparing in advance for some naughty behavior from other mods.
- Changed logging level in anticipation of future changes to Geode.
- Removed Goro Akechi.
## v1.6.3
- Some cleanup from behind the scenes. Nothing noticeable for end users, though.
## v1.6.2
- Port to 2.2074.
- Remove the pink button config directory shortcut.
## v1.6.1
- Redo the button for blacklisting songs. Base icon shape provided by [FREAKINGDAN](https://github.com/freakingdan).
- Reduce confusion when copying song names/IDs while playing songs replaced by Jukebox/Auto Nong. {Reported by CyBlanc}
- Fix a bug where some songs downloaded from vanilla GD wouldn't be treated properly in Constant Shuffle Mode (potentially related to "No Songs Limit" being enabled). {Reported by ita_player}
- Make the setting description for Constant Shuffle Mode a scrollable setting description. {Reported by hiimjustin000}
- _Attempt_ to fix a bug where setting the music volume to 0 while on Constant Shuffle Mode would re-start a song while failing to end the original playback. {Reported by EmirNoob}
## v1.6.0
- Added "Copy Song ID" button (suggested by [Akashnil](https://discord.com/users/1068096599701590056)). <cy>Custom songs and NONGs will not return a song ID. They will fall back to its song name instead.</c> <cy>Button sprite adapted from [hiimjustin000](https://github.com/hiimjustin000) with permission.</c>
- Added "Blacklist Song" button (suggested by [AlphaQuata](https://discord.com/users/766876726566846505)). <cy>This button works regardless of your custom songs or Constant Shuffle Mode choices.</c> <cy>To manually un-blacklist a song, open `blacklist.txt` in the mod's config directory and edit it accordingly.</c>
- Fix a bug where Constant Shuffle Mode would ignore the in-game "Menu Music" toggle if that was disabled.
- Fix another bug where "Remember Last Menu Loop" would remember the original menu loop from GD's resources. 
## v1.5.0
- Added "Constant Shuffle Mode" (suggested by [TheKevineer](https://discord.com/users/633040761256017963)). <cy>This feature is incredibly unstable, especially outside of Windows. I have done everything I could to keep its behavior consistent between Windows, macOS, and Android, but any crashes/issues from this feature are either beyond my control or have been acknowledged during beta testing without an obvious/more immediate fix.</c>
- Added "Remember Last Menu Loop" (suggested by [HexCodesGMD](https://discord.com/users/1222327935315218506)).
- Renamed and reordered a few settings to reduce confusion.
- Added config directory shortcut button.
  - Incidentally, the pink config directory shortcut button from previous versions won't be added if QOLMod is loaded.
## v1.4.3
- Made sure the new "Regenerate Notification" sprite is actually part of the mod's resources.
- Also shrunk the mod file size down as there haven't been any new crashes from the changes introduced in v1.4.1.
## v1.4.2
- Redrew the "Regenerate Notification" sprite (courtesy of elnexreal).
## v1.4.1
- Add a new button to show the "Now Playing" notification again, per GitHub feedback. (The texture is incredibly scuffed. I will revise it later. The important thing is that it works as intended.)
- Attempted to fix a weird `stoi` crash. Not sure why it happens, but it did for the one person who reported it, and they didn't really provide further context beyond a crashlog file reading `C++ Exception: class std::out_of_range("stoi argument out of range")`. ¯\\\_(ツ)\_/¯
  - The mod now uses Geode's `geode::utils::numFromString<int>()` function.
- License the mod under LGPLv2.
## v1.4.0
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
## v1.3.0
- Added macOS support (thx [ninxout](https://github.com/ninXout), [raydeeux](https://github.com/RayDeeUx), and [hiimjustin000](https://github.com/hiimjustin000)).
- Added support for `.wav`, `.ogg`/`.oga`, and `.flac` files for custom songs.
- Included artist name for all "Now Playing" notifications.
- Set Z order of the "Now Playing" notification to 200 for easier readability against most MenuLayer mods (such as Overcharged Main Menu).
- Shuffling the song now generates a new "Now Playing" notification.
- Increase "Now Playing" notification maximum duration to 5 seconds.
- Add better slider control for adjusting notification duration.
- Added Node IDs to nodes added by this mod.
- Squash a few bugs, optimize a few things, minimize likelihood of potential crashes.
## v1.2.2
- Added an option to enable the shuffle button (thx [reinmar](https://github.com/Reinmmar)).
## v1.2.1
- Fixed crash caused by Newgrounds manager trying to gather info of default menuLoop.mp3.
## v1.2.0
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
## v1.1.0
- Fixed crash when opening the game (missing textures crash).
- Fixed random song picker being on a loop for every song downloaded.
- Moved song class to its own file.
- The mod should now check for the songs path when loaded, not when executed.
- Added setting to randomize song on level exit.
- Fixed Android crash caused by not able to access the memory (f*** pointers).
- Added `Utils.hpp`
## v1.1.0-alpha.5
- Updated Geode target version to `v3.0.0-beta.1`.
## v1.1.0-alpha.4
- Updated notification card & position.
- Added the property `name` to the `Song` class.
## v1.1.0-alpha.3
- Added a notification animation for the card.
- The position is now added in `MainLayer` instead of the layer itself.
## v1.1.0-alpha.2
- Fixed crash on startup caused by a missing dependency.
## v1.1.0-alpha.1
- Added a card showing which song is playing. (you can disable it in settings.)
- Changed a little bit how songs are stored in the vector.
- Removed MacOS support. (sorry, i don't wanna deal with MacOS bindings.)
#### Notes
- This version is for `Geode v3.0.0-alpha.2` only.
## v1.0.0
- Initial release.
