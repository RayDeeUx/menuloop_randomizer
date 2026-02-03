#include "ConstantShuffleModeWarning.hpp"
#include "SongManager.hpp"
#include "Utils.hpp"
#include "Settings.hpp"
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

SongManager& songManager = SongManager::get();
const std::filesystem::path& configDir = Mod::get()->getConfigDir();
bool originalOverrideWasEmpty = false;

#define VIBECODED_RADIO Loader::get()->getInstalledMod("joseii.ventilla")
#define BTP Loader::get()->getInstalledMod("alk.better-touch-prio")
#define USE_BETTER_TOUCH_PRIO_DAMMIT "# <c-ff0000>***__Please install \"Better Touch Prio\" to use this feature.__***</c>\n\n<c-ff0000>Your setting was saved, but nothing more will happen unless if you install the \"Better Touch Prio\" mod.</c>"

$on_mod(Loaded) {
	(void) Mod::get()->registerCustomSettingType("configdir", &MyButtonSettingV3::parse);
	songManager.setConstantShuffleMode();
	songManager.setLastMenuLoopPosition(0);
	songManager.setShouldRestoreMenuLoopPoint(true);
	songManager.setFinishedCalculatingSongLengths(false);
	songManager.setAdvancedLogs(Mod::get()->getSettingValue<bool>("advancedLogs"));
	songManager.setShowPlaybackProgressAndControls(Mod::get()->getSettingValue<bool>("showPlaybackProgressAndControls"));
	songManager.setShowPlaybackControlsSongList(Mod::get()->getSettingValue<bool>("showPlaybackProgressControlsSongList"));
	GEODE_DESKTOP(songManager.setYoutubeAndVLCKeyboardShortcutsSongList(Mod::get()->getSettingValue<bool>("youtubeAndVLCKeyboardShortcutsSongList"));)
	GEODE_DESKTOP(songManager.setYoutubeAndVLCKeyboardShortcutsControlPanel(Mod::get()->getSettingValue<bool>("youtubeAndVLCKeyboardShortcutsControlPanel"));)
	songManager.setIncrementDecrementByMilliseconds(Mod::get()->getSettingValue<int64_t>("incrementDecrementByMilliseconds"));
	songManager.setVibecodedVentilla(VIBECODED_RADIO && (VIBECODED_RADIO->isEnabled() || VIBECODED_RADIO->shouldLoad()));
	songManager.setUndefined0Alk1m123TouchPrio(BTP && (BTP->isEnabled() || BTP->shouldLoad()) && !BTP->hasUnresolvedDependencies() && !BTP->hasUnresolvedIncompatibilities());
	if (!std::filesystem::exists(configDir / "playlistOne.txt")) Utils::writeToFile("# This file was generated automatically as it hadn't existed previously.", configDir / "playlistOne.txt");
	if (!std::filesystem::exists(configDir / "playlistTwo.txt")) Utils::writeToFile("# This file was generated automatically as it hadn't existed previously.", configDir / "playlistTwo.txt");
	if (!std::filesystem::exists(configDir / "playlistThree.txt")) Utils::writeToFile("# This file was generated automatically as it hadn't existed previously.", configDir / "playlistThree.txt");

	auto blacklistTxt = configDir / R"(blacklist.txt)";
	if (!std::filesystem::exists(blacklistTxt)) {
		const std::string& blacklistFileIntro = R"(# Welcome to the Menu Loop Randomizer song blacklist!
# Each line that doesn't start with a "#" will be treated as a blacklisted song file.
# All lines that start with a "#" are ignored. This means you can un-blacklist a song by adding "#" next to it.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# --RayDeeUx)";
		auto result = utils::file::writeString(blacklistTxt, blacklistFileIntro);
		if (result.isErr()) {
			log::error("Error writing to blacklist.txt");
		}
	}

	auto favoriteTxt = configDir / R"(favorites.txt)";
	if (!std::filesystem::exists(favoriteTxt)) {
		const std::string& favoritesFileIntro = R"(# Welcome to the Menu Loop Randomizer favorites list!
# Each line that doesn't start with a "#" will be treated as a favorited song file.
# All lines that start with a "#" are ignored. This means you can un-favorite a song by adding "#" next to it.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# --RayDeeUx)";
		auto result = utils::file::writeString(favoriteTxt, favoritesFileIntro);
		if (result.isErr()) {
			log::error("Error writing to favorites.txt");
		}
	}

	if (SongManager::get().getAdvancedLogs()) geode::log::info("repopulating vector from on_mod(Loaded)");
	Utils::refreshTheVector();

	const std::string& override = Utils::toNormalizedString(Mod::get()->getSettingValue<std::filesystem::path>("specificSongOverride"));
	originalOverrideWasEmpty = override.empty();
	songManager.setOverride(override);

	const std::string& lastMenuLoop = Mod::get()->getSavedValue<std::string>("lastMenuLoop");
	const std::filesystem::path& lastMenuLoopPath = Mod::get()->getSavedValue<std::filesystem::path>("lastMenuLoopPath");
	const bool saveSongOnGameClose = Utils::getBool("saveSongOnGameClose");
	const bool loopExists = std::filesystem::exists(Utils::toProblematicString(lastMenuLoop)) || std::filesystem::exists(lastMenuLoopPath);
	log::info("\n=== 'REMEMBER LAST MENU LOOP' DEBUG INFO ===\nlast menu loop: {}\nlast menu loop as path: {}\n'saveSongOnGameClose' setting: {}\nloopExists: {}\noverride: {}", lastMenuLoop, lastMenuLoopPath, saveSongOnGameClose, loopExists, override);
	if (!override.empty() && Utils::isSupportedFile(override)) {
		if (SongManager::get().getAdvancedLogs()) log::info("setting songManager's current song to overridden song from settings");
		songManager.setCurrentSongToOverride();
	} else if (!lastMenuLoop.empty() && Utils::isSupportedFile(lastMenuLoop) && loopExists && saveSongOnGameClose) {
		if (SongManager::get().getAdvancedLogs()) log::info("setting songManager's current song to saved song from on_mod(Loaded)");
		songManager.setCurrentSongToSavedSong();
	} else {
		log::info("randomizing songManager's current song through on_mod(Loaded)");
		songManager.pickRandomSong();
	}
	Utils::composeAndSetCurrentSongDisplayNameOnlyOnLoadOrWhenBlacklistingSongs();

	if (!std::filesystem::exists(configDir / R"(store_your_disabled_menuloops_here)")) {
		std::filesystem::create_directory(configDir / R"(store_your_disabled_menuloops_here)");
	}
	listenForSettingChanges<bool>("useCustomSongs", [](const bool) {
		Utils::resetSongManagerRefreshVectorSetNewSongBecause("useCustomSongs");
	});
	listenForSettingChanges<bool>("searchDeeper", [](const bool) {
		Utils::resetSongManagerRefreshVectorSetNewSongBecause("searchDeeper");
	});
	listenForSettingChanges<std::filesystem::path>("additionalFolder", [](const std::filesystem::path&) {
		Utils::resetSongManagerRefreshVectorSetNewSongBecause("additionalFolder");
	});
	listenForSettingChanges<bool>("loadPlaylistFile", [](const bool loadPlaylistFile) {
		Utils::resetSongManagerRefreshVectorSetNewSongBecause("loadPlaylistFile");
		if (!loadPlaylistFile) return;
		if (CCScene* scene = CCScene::get(); scene && (scene->getChildByID("playlist-files-warning"_spr) || scene->getChildByTag(7302025))) return;
		Utils::showMDPopup("Playlist Files", "## ***<c-FF0000>MLR PLAYLIST FILES ARE __FOR PERSONAL USE ONLY__. MLR PLAYLIST FILES SHOULD __NOT__ BE SHARED BETWEEN DEVICES OR USERS.</c>***\n\n\n\n<cy>Problems created by, or as a result of, ignoring this basic advice are your sole responsibility.</c>", 7302025, "playlist-files");
	});
	listenForSettingChanges<std::filesystem::path>("playlistFile", [](const std::filesystem::path&) {
		Utils::resetSongManagerRefreshVectorSetNewSongBecause("playlistFile");
		if (CCScene* scene = CCScene::get(); scene && (scene->getChildByID("playlist-files-warning"_spr) || scene->getChildByTag(7302025))) return;
		Utils::showMDPopup("Playlist Files", "## ***<c-FF0000>MLR PLAYLIST FILES ARE __FOR PERSONAL USE ONLY__. MLR PLAYLIST FILES SHOULD __NOT__ BE SHARED BETWEEN DEVICES OR USERS.</c>***\n\n\n\n<cy>Problems created by, or as a result of, ignoring this basic advice are your sole responsibility.</c>", 7302025, "playlist-files");
	});
	listenForSettingChanges<bool>("playlistMode", [](const bool constantShuffleMode) {
		SongManager& songManager = SongManager::get();
		songManager.setConstantShuffleMode();
		if (songManager.isOriginalMenuLoop()) return;
		FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		Utils::queueUpdateFrontfacingLabelsInSCMAndSLL();
		if (constantShuffleMode) {
			GameManager::sharedState()->playMenuMusic();
			return ConstantShuffleModeWarning::create(songManager.getGeodify())->show();
		}
		GameManager::sharedState()->playMenuMusic();
	});
	listenForSettingChanges<std::filesystem::path>("specificSongOverride", [](const std::filesystem::path& specificSongOverride) {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
		const std::string& overrideString = Utils::toNormalizedString(specificSongOverride);
		songManager.setOverride(overrideString);
		if (!Utils::isSupportedFile(overrideString) && overrideString.empty()) {
			songManager.clearSongs();
			geode::log::info("repopulating vector from removing override");
			Utils::refreshTheVector();
			if (Utils::isSupportedFile(Mod::get()->getSavedValue<std::string>("lastMenuLoop")) && Utils::getBool("saveSongOnGameClose") && !originalOverrideWasEmpty) {
				if (SongManager::get().getAdvancedLogs()) log::info("setting songManager's current song to saved song from settings change");
				songManager.setCurrentSongToSavedSong();
			} else Utils::setNewSong();
		}
		geode::Loader::get()->queueInMainThread([] { Utils::queueUpdateFrontfacingLabelsInSCMAndSLL(); });
		GameManager::sharedState()->playMenuMusic();
		Utils::newCardAndDisplayNameFromCurrentSong();
	});
	listenForSettingChanges<bool>("dangerousBlacklisting", [](const bool dangerousBlacklisting) {
		if (!dangerousBlacklisting) return;
		log::info("=============== WARNING: USER HAS ENABLED `dangerousBlacklisting` SETTING ===============");
		if (GameManager::get()->m_playerUserID.value() == 925143 || GameManager::get()->m_playerUserID.value() == 7247326) return log::info("never mind, it's just aktimoose the beta tester. don't show the alert");
		Utils::showMDPopup("Blacklisting", "<c-ff0000>This is an experimental setting. You agree to hold yourself responsible for any issues that happen when this setting is enabled.</c>", 7302025, "dangerous-blacklisting");
	});
	listenForSettingChanges<bool>("randomizeWhenExitingLevel", [](const bool randomizeWhenExitingLevel) {
		if (!Mod::get()->getSettingValue<bool>("restoreWhenExitingLevel")) return;
		const std::string& mdPopupBody = randomizeWhenExitingLevel ? "# <c-ff0000>Menu loops will always be randomized when leaving a level.</c>\n<cy>*If you don't want this behavior, disable the \"Randomize Menu Loop on Level Exit\" setting.*</c>" : "# <cy>Menu loops will resume from where they stopped when leaving a level.</c>\n<cy>*If you don't want this behavior, enable the \"Randomize Menu Loop on Level Exit\" setting.*";
		Utils::showMDPopup("Randomize/Continuing Menu Loops", mdPopupBody, 12212025, "randomize-restore");
	});
	listenForSettingChanges<bool>("randomizeWhenExitingEditor", [](const bool randomizeWhenExitingEditor) {
		if (!Mod::get()->getSettingValue<bool>("restoreWhenExitingEditor")) return;
		const std::string& mdPopupBody = randomizeWhenExitingEditor ? "# <c-ff0000>Menu loops will always be randomized when leaving the level editor.</c>\n<cy>*If you don't want this behavior, disable the \"Randomize Menu Loop on Editor Exit\" setting.*</c>" : "# <cy>Menu loops will resume from where they stopped when leaving the level editor.</c>\n<cy>*If you don't want this behavior, enable the \"Randomize Menu Loop on Editor Exit\" setting.*";
		Utils::showMDPopup("Randomize/Continuing Menu Loops", mdPopupBody, 12212025, "randomize-restore");
	});
	listenForSettingChanges<bool>("restoreWhenExitingLevel", [](const bool restoreWhenExitingLevel) {
		if (!Mod::get()->getSettingValue<bool>("randomizeWhenExitingLevel")) return;
		const std::string& mdPopupBody = restoreWhenExitingLevel ? "# <c-ff0000>Menu loops will still be randomized when leaving a level.</c>\n<cy>*If you don't want this behavior, disable the \"Randomize Menu Loop on Level Exit\" setting.*</c>" : "# <cy>Menu loops will resume from where they stopped when leaving a level.</c>\n<cy>*If you don't want this behavior, disable the \"Continue Menu Loop on Level Exit\" setting.*";
		Utils::showMDPopup("Randomize/Continuing Menu Loops", mdPopupBody, 12212025, "randomize-restore");
	});
	listenForSettingChanges<bool>("restoreWhenExitingEditor", [](const bool restoreWhenExitingEditor) {
		if (!Mod::get()->getSettingValue<bool>("randomizeWhenExitingEditor")) return;
		const std::string& mdPopupBody = restoreWhenExitingEditor ? "# <c-ff0000>Menu loops will still be randomized when leaving the level editor.</c>\n<cy>*If you don't want this behavior, disable the \"Randomize Menu Loop on Editor Exit\" setting.*</c>" : "# <cy>Menu loops will resume from where they stopped when leaving the level editor.</c>\n<cy>*If you don't want this behavior, disable the \"Continue Menu Loop on Editor Exit\" setting.*";
		Utils::showMDPopup("Randomize/Continuing Menu Loops", mdPopupBody, 12212025, "randomize-restore");
	});
	listenForSettingChanges<bool>("showSearchBar", [](const bool showSearchBar) {
		if (SongManager::get().getUndefined0Alk1m123TouchPrio() || !showSearchBar) return;
		Utils::showMDPopup("Search Bar/Filtering Options", USE_BETTER_TOUCH_PRIO_DAMMIT, 12312025, "search-bar-song-sorting");
	});
	listenForSettingChanges<double>("compactModeScaleFactor", [](const double) {
		if (SongManager::get().getUndefined0Alk1m123TouchPrio()) return;
		Utils::showMDPopup("Compact Mode Scale Factor", USE_BETTER_TOUCH_PRIO_DAMMIT, 12312025, "compact-mode-scale-factor");
	});
	listenForSettingChanges<bool>("showSortSongOptions", [](const bool showSortSongOptions) {
		if (SongManager::get().getUndefined0Alk1m123TouchPrio() || !showSortSongOptions) return;
		Utils::showMDPopup("Search Bar/Filtering Options", USE_BETTER_TOUCH_PRIO_DAMMIT, 12312025, "search-bar-song-sorting");
	});
	listenForSettingChanges<bool>("showSortSongLength", [](const bool showSortSongLength) {
		if (SongManager::get().getUndefined0Alk1m123TouchPrio() || !showSortSongLength) return;
		Utils::showMDPopup("Search Bar/Filtering Options", USE_BETTER_TOUCH_PRIO_DAMMIT, 12312025, "search-bar-song-sorting");
	});
	#ifndef GEODE_IS_IOS
	listenForSettingChanges<bool>("useMiniaudioForLength", [](const bool useMiniaudioForLength) {
		if (SongManager::get().getUndefined0Alk1m123TouchPrio()) {
			if (useMiniaudioForLength) Utils::showMDPopup("+ Miniaudio Song Lengths", "You are now using Miniaudio to calculate song lengths. <c-ffff00>***__Changes only apply the next time Menu Loop Randomizer reloads its songs.__***</c>\n\nIf you prefer using FMOD for somewhat more precise long lengths at the cost of wait time, disable this option.", 20260105, "miniaudio-song-lengths-using-miniaudio");
			else Utils::showMDPopup("+ Miniaudio Song Lengths", "You are now using FMOD to calculate song lengths. <c-ffff00>***__Changes only apply the next time Menu Loop Randomizer reloads its songs.__***</c>\n\nIf you prefer using Miniaudio for somewhat less precise long lengths, enable this option.\n<c-aaaaaa>*(Note that FMOD may still be used if Miniaudio fails to find a length.)*</c>", 20260105, "miniaudio-song-lengths-using-fmod");
		}
		else Utils::showMDPopup("+ Miniaudio Song Lengths", USE_BETTER_TOUCH_PRIO_DAMMIT, 20260105, "miniaudio-song-lengths-btp-missing");
	});
	#endif
	listenForSettingChanges<bool>("showExtraInfoLabel", [](const bool showExtraInfoLabel) {
		if (SongManager::get().getUndefined0Alk1m123TouchPrio() || !showExtraInfoLabel) return;
		Utils::showMDPopup("Song List Extra Info Label", USE_BETTER_TOUCH_PRIO_DAMMIT, 20260105, "search-bar-song-sorting");
	});
	listenForSettingChanges<bool>("showPlaybackProgressAndControls", [](const bool showPlaybackProgressAndControls) {
		SongManager::get().setShowPlaybackProgressAndControls(showPlaybackProgressAndControls);
		if (SongManager::get().getUndefined0Alk1m123TouchPrio() || !showPlaybackProgressAndControls) return;
		Utils::showMDPopup("Control Panel Playback Progress Controls", USE_BETTER_TOUCH_PRIO_DAMMIT, 20260105, "show-playback-progress-controls");
	});
	listenForSettingChanges<bool>("showPlaybackProgressControlsSongList", [](const bool showPlaybackProgressControlsSongList) {
		SongManager::get().setShowPlaybackControlsSongList(showPlaybackProgressControlsSongList);
		if (SongManager::get().getUndefined0Alk1m123TouchPrio() || !showPlaybackProgressControlsSongList) return;
		Utils::showMDPopup("Song List Playback Progress Controls", USE_BETTER_TOUCH_PRIO_DAMMIT, 20260105, "show-playback-progress-controls");
	});
	listenForSettingChanges<int64_t>("incrementDecrementByMilliseconds", [](const int64_t incrementDecrementByMilliseconds) {
		SongManager::get().setIncrementDecrementByMilliseconds(incrementDecrementByMilliseconds);
		if (SongManager::get().getUndefined0Alk1m123TouchPrio()) return;
		Utils::showMDPopup("Control Panel Playback Progress Controls", USE_BETTER_TOUCH_PRIO_DAMMIT, 20260105, "playback-progress-incrdecr-amt");
	});
	listenForSettingChanges<bool>("songIndicatorsInControlPanel", [](const bool) {
		Utils::queueUpdateFrontfacingLabelsInSCMAndSLL();
	});
	#ifdef GEODE_DESKTOP
	listenForSettingChanges<bool>("youtubeAndVLCKeyboardShortcutsSongList", [](const bool youtubeAndVLCKeyboardShortcutsSongList) {
		SongManager::get().setYoutubeAndVLCKeyboardShortcutsSongList(youtubeAndVLCKeyboardShortcutsSongList);
	});
	listenForSettingChanges<bool>("youtubeAndVLCKeyboardShortcutsControlPanel", [](const bool youtubeAndVLCKeyboardShortcutsControlPanel) {
		SongManager::get().setYoutubeAndVLCKeyboardShortcutsControlPanel(youtubeAndVLCKeyboardShortcutsControlPanel);
	});
	#endif
	listenForSettingChanges<bool>("advancedLogs", [](const bool newAdvancedLogs) {
		SongManager::get().setAdvancedLogs(newAdvancedLogs);
	});
}