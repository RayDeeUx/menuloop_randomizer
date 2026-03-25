#include "ConstantShuffleModeWarning.hpp"
#include "SongManager.hpp"
#include "Utils.hpp"
#include "Settings.hpp"
#include "ui/SongListLayer.hpp"
#include "ui/SongControlMenu.hpp"
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

SongManager& songManager = SongManager::get();
const std::filesystem::path& configDir = Mod::get()->getConfigDir();
bool originalOverrideWasEmpty = false;

/* CHANGE THIS LATER */ #define VOBECIDED_JOTA Loader::get()->getInstalledMod("jotabelike.foobarbaz")
#define VIBECODED_RADIO Loader::get()->getInstalledMod("joseii.ventilla")
#define BTP Loader::get()->getInstalledMod("alk.better-touch-prio")
#define USE_BETTER_TOUCH_PRIO_DAMMIT "# <c-ff0000>***__Please install \"Better Touch Prio\" to use this feature.__***</c>\n\n<c-ff0000>Your setting was saved, but nothing more will happen unless if you install the \"Better Touch Prio\" mod.</c>"

$on_mod(Loaded) {
	(void) Mod::get()->registerCustomSettingType("configdir", &MyButtonSettingV3::parse);
	songManager.setOsu(Mod::get()->getSettingValue<bool>("osuLazerBootleg"));
	songManager.setConstantShuffleMode();
	songManager.setLastMenuLoopPosition(0);
	songManager.setShouldRestoreMenuLoopPoint(true);
	songManager.setFinishedCalculatingSongLengths(false);
	songManager.setAdvancedLogs(Mod::get()->getSettingValue<bool>("advancedLogs"));
	songManager.setShowPlaybackProgressAndControls(Mod::get()->getSettingValue<bool>("showPlaybackProgressAndControls"));
	songManager.setShowPlaybackControlsSongList(Mod::get()->getSettingValue<bool>("showPlaybackProgressControlsSongList"));
	GEODE_MOBILE(songManager.setYoutubeAndVLCKeyboardShortcutsSongList(false);)
	GEODE_MOBILE(songManager.setYoutubeAndVLCKeyboardShortcutsControlPanel(false);)
	GEODE_DESKTOP(songManager.setYoutubeAndVLCKeyboardShortcutsSongList(Mod::get()->getSettingValue<bool>("youtubeAndVLCKeyboardShortcutsSongList"));)
	GEODE_DESKTOP(songManager.setYoutubeAndVLCKeyboardShortcutsControlPanel(Mod::get()->getSettingValue<bool>("youtubeAndVLCKeyboardShortcutsControlPanel"));)
	songManager.setIncrementDecrementByMilliseconds(Mod::get()->getSettingValue<int64_t>("incrementDecrementByMilliseconds"));
	songManager.setVibecodedVentilla(VIBECODED_RADIO && (VIBECODED_RADIO->isOrWillBeEnabled() || VIBECODED_RADIO->shouldLoad()));
	/* CHANGE THIS LATER */ // songManager.setVobecidedJota(VOBECIDED_JOTA && (VOBECIDED_JOTA->isOrWillBeEnabled() || VOBECIDED_JOTA->shouldLoad()));
	/* REMOVE THIS LATER */ songManager.setVobecidedJota(false);
	songManager.setUndefined0Alk1m123TouchPrio(BTP && (BTP->isOrWillBeEnabled() || BTP->shouldLoad()) && !BTP->hasUnresolvedDependencies() && !BTP->hasUnresolvedIncompatibilities());
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

	int i = 0;
	if (Mod::get()->getSavedValue<bool>("songListSortAlphabetically")) i++;
	if (Mod::get()->getSavedValue<bool>("songListSortDateAdded")) i++;
	if (Mod::get()->getSavedValue<bool>("songListSortSongLength")) i++;
	if (Mod::get()->getSavedValue<bool>("songListSortFileSize")) i++;
	if (Mod::get()->getSavedValue<bool>("songListSortFileExtn")) i++;
	if (i > 1) {
		// because i don't trust toddlers to not edit saved.json nilly willy
		Mod::get()->setSavedValue<bool>("songListSortAlphabetically", false);
		Mod::get()->setSavedValue<bool>("songListSortDateAdded", false);
		Mod::get()->setSavedValue<bool>("songListSortSongLength", false);
		Mod::get()->setSavedValue<bool>("songListSortFileSize", false);
		Mod::get()->setSavedValue<bool>("songListSortFileExtn", false);
	}

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
	listenForSettingChanges<bool>("osuLazerBootleg", [](const bool osuLazerBootlegNew) {
		if (osuLazerBootlegNew || !CCScene::get()) {
			songManager.setOsu(osuLazerBootlegNew);
			return;
		}
		SongControlMenu* scm = CCScene::get()->getChildByType<SongControlMenu>(0);
		if (!scm || !scm->m_osu) {
			songManager.setOsu(osuLazerBootlegNew);
			return;
		}
		if (MenuLayer::get() && !MenuLayer::get()->isVisible() && scm->m_osu) MenuLayer::get()->setVisible(true);
		scm->toggleOsu();
		songManager.setOsu(osuLazerBootlegNew);
	});
	listenForSettingChanges<bool>("advancedLogs", [](const bool newAdvancedLogs) {
		SongManager::get().setAdvancedLogs(newAdvancedLogs);
	});
}

#define ECLIPSE_MODULES_HPP
#include <eclipse.eclipse-menu/include/eclipse.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include "SongControl.hpp"

using namespace eclipse;
using namespace geode::prelude;

class EclipsePlaybackProgressDummyNode final : public CCNode {
public:
	void playbackProgressScheduler(float) { playbackProgress(); }
	static void playbackProgress() {
		SongManager& sm = SongManager::get();
		if (!sm.eclipseSongDurationLabel.has_value() || !sm.eclipseSongProgressBarLabel.has_value()) return;

		if (sm.eclipseIsCocosStyle) {
			if (sm.eclipseSongNameLabel.has_value()) sm.eclipseSongNameLabel.value().setText("Song progress is unavailable in this menu style!");
			if (sm.isQOLMod && sm.qolModIntegrationSuccessful) {
				sm.eclipseSongDurationLabel.value().setText("Please view/control song progress from QOLMod instead.");
				GEODE_DESKTOP(sm.eclipseSongProgressBarLabel.value().setText("(Or switch to the ImGui menu style.)");)
				GEODE_MOBILE(sm.eclipseSongProgressBarLabel.value().setText("(This is a non-negotiable solution.)");)
			} else {
				GEODE_DESKTOP(
					sm.eclipseSongDurationLabel.value().setText("Switch to the ImGui menu style to view this progress.");
					sm.eclipseSongProgressBarLabel.value().setText("(This is a non-negotiable solution.)");
				)
				GEODE_MOBILE(
					sm.eclipseSongDurationLabel.value().setText("Feel free to scroll towards the bottom");
					sm.eclipseSongProgressBarLabel.value().setText("and use the shortcuts from there instead.");
				)
			}
			return;
		}

		constexpr int progressBarWidth = 20;
		FMODAudioEngine* fmod = FMODAudioEngine::get();
		if (std::hash<std::string>{}(fmod->getActiveMusic(0)) != std::hash<std::string>{}(sm.getCurrentSong())) {
			sm.eclipseSongDurationLabel.value().setText("Progress: [CURRENTLY PAUSED]");
			sm.eclipseSongProgressBarLabel.value().setText(fmt::format("[{:–<{}}]", "", progressBarWidth));
			return;
		}

		const int fullLength = fmod->getMusicLengthMS(0);
		const int lastPosition = sm.getLastMenuLoopPosition();

		const int percent = std::clamp<int>((lastPosition * 100.f / fullLength * 1.f), 0, 100);
		int completed = (percent * progressBarWidth * 1.f) / 100.f;
		int remaining = progressBarWidth - completed;

		sm.eclipseSongDurationLabel.value().setText(fmt::format("Progress: {}:{:02} / {}:{:02}", ((lastPosition / 1000) / 60), ((lastPosition / 1000) % 60), ((fullLength / 1000) / 60), ((fullLength / 1000) % 60)).c_str());
		sm.eclipseSongProgressBarLabel.value().setText(fmt::format("[{:+<{}}{:-<{}}]", "", completed, "", remaining));
	}
};

$on_mod(Loaded) {
	Loader::get()->queueInMainThread([]() {
		if (!Mod::get()->getSettingValue<bool>("eclipseIntegration")) return;
		if (geode::Loader::get()->getInstalledMod("eclipse.eclipse-menu")) SongManager::get().eclipseIsCocosStyle = geode::utils::string::contains(geode::utils::string::toLower(geode::Loader::get()->getInstalledMod("eclipse.eclipse-menu")->getSettingValue<std::string>("menu-style")), "cocos");

		listenForSettingChanges<std::string>("menu-style", [](const std::string& newMenuStyle) {
			SongManager& sm = SongManager::get();
			if (!sm.isEclipse || !sm.eclipseIntegrationSuccessful) return;
			sm.eclipseIsCocosStyle = geode::utils::string::contains(geode::utils::string::toLower(newMenuStyle), "cocos");
			// bit of a hack ngl but FUCK IT WE BALL
			if (sm.eclipseSongNameLabel.has_value()) {
				if (!sm.eclipseIsCocosStyle) sm.eclipseSongNameLabel.value().setText(fmt::format("Current song: {}", Utils::getFullNameOfCurrentSongForIntegrationsAndControlPanel()));
				else sm.eclipseSongNameLabel.value().setText("Song progress is unavailable in this menu style!");
			}
			for (std::pair<std::optional<eclipse::components::Label>, std::string>& labelStringPair : sm.dynamicEclipseIntegrationHeaders) {
				if (!labelStringPair.first.has_value()) continue;
				labelStringPair.first.value().setText(sm.eclipseIsCocosStyle ? fmt::format("{}\n", labelStringPair.second) : labelStringPair.second);
			}
		}, geode::Loader::get()->getInstalledMod("eclipse.eclipse-menu"));

		auto tab = MenuTab::find("Menu Loop Randomizer");

		SongManager::get().eclipseSongNameLabel = std::make_optional<eclipse::components::Label>(tab.addLabel("Current song: [[Hold on, MLR is still loading things!]]"));
		SongManager::get().eclipseSongDurationLabel = std::make_optional<eclipse::components::Label>(tab.addLabel("Progress: [[Hold on, MLR is still loading things!]]"));
		SongManager::get().eclipseSongProgressBarLabel = std::make_optional<eclipse::components::Label>(tab.addLabel("[[Hold on, MLR is still loading things!]]"));

		EclipsePlaybackProgressDummyNode::playbackProgress();
		GameManager::get()->schedule(schedule_selector(EclipsePlaybackProgressDummyNode::playbackProgressScheduler), .125f);

		(void) tab.addLabel("\n");

		SongManager::get().dynamicEclipseIntegrationHeaders.push_back(std::pair<std::optional<eclipse::components::Label>, std::string>{std::make_optional<eclipse::components::Label>(tab.addLabel("Song Selection")), "Song Selection"});
		tab.addButton("Shuffle Song", []() {
			SongControl::shuffleSong();
		});
		tab.addButton("Previous Song (Go back once)", []() {
			SongControl::previousSong();
		});
		tab.addButton("Hold Song (like Tetris)", []() {
			SongControl::holdSong();
		});

		SongManager::get().dynamicEclipseIntegrationHeaders.push_back(std::pair<std::optional<eclipse::components::Label>, std::string>{std::make_optional<eclipse::components::Label>(tab.addLabel("Playback/Seeking")), "Playback/Seeking"});
		tab.addButton("Seek Backward", []() {
			SongControl::skipBackward();
		});
		tab.addButton("Seek Forward", []() {
			SongControl::skipForward();
		});
		tab.addButton("Seek to 0%", []() {
			SongControl::setSongPercentage(0);
		});
		tab.addButton("Seek to 25%", []() {
			SongControl::setSongPercentage(25);
		});
		tab.addButton("Seek to 50%", []() {
			SongControl::setSongPercentage(50);
		});
		tab.addButton("Seek to 75%", []() {
			SongControl::setSongPercentage(75);
		});

		SongManager::get().dynamicEclipseIntegrationHeaders.push_back(std::pair<std::optional<eclipse::components::Label>, std::string>{std::make_optional<eclipse::components::Label>(tab.addLabel("Other Controls")), "Other Controls"});
		tab.addButton("Favorite Song", []() {
			SongControl::favoriteSong();
		});
		tab.addButton("Blacklist Song", []() {
			SongControl::blacklistSong();
		});
		tab.addButton("Copy Song Name/ID", []() {
			SongControl::copySong();
		});
		tab.addButton("New Notification", []() {
			SongControl::regenSong();
		});

		SongManager::get().dynamicEclipseIntegrationHeaders.push_back(std::pair<std::optional<eclipse::components::Label>, std::string>{std::make_optional<eclipse::components::Label>(tab.addLabel("Shortcuts (Use them wisely!)")), "Shortcuts (Use them wisely!)"});
		tab.addButton("Open Control Panel", []() {
			if (VANILLA_GD_MENU_LOOP_DISABLED || GJBaseGameLayer::get() || (CCScene::get() && CCScene::get()->getChildByType<SongControlMenu>(0)) || FMODAudioEngine::get()->getActiveMusic(0) != SongManager::get().getCurrentSong()) return;
			if (CCScene::get() && CCScene::get()->getChildByType<AndroidUI>(0) && SongManager::get().qolModIntegrationSuccessful) return;
			if (SongListLayer* foo = CCScene::get()->getChildByType<SongListLayer>(0); foo) {
				#if GEODE_COMP_GD_VERSION == 22081
				geode::Popup::CloseEvent(foo).send();
				foo->setKeypadEnabled(false);
				foo->setTouchEnabled(false);
				foo->removeFromParent();
				#endif

				#if GEODE_COMP_GD_VERSION == 22074
				foo->keyBackClicked();
				#endif
			}
			SongControlMenu::create()->show();
		});
		tab.addButton("Open Songs List", []() {
			if (VANILLA_GD_MENU_LOOP_DISABLED || GJBaseGameLayer::get() || !CCScene::get() || CCScene::get()->getChildByType<SongListLayer>(0) || FMODAudioEngine::get()->getActiveMusic(0) != SongManager::get().getCurrentSong()) return;
			if (CCScene::get() && CCScene::get()->getChildByType<AndroidUI>(0) && SongManager::get().qolModIntegrationSuccessful) return;
			if (SongControlMenu* foo = CCScene::get()->getChildByType<SongControlMenu>(0); foo) {
				if (foo->m_osu && MenuLayer::get() && !MenuLayer::get()->isVisible()) MenuLayer::get()->setVisible(true);
				#if GEODE_COMP_GD_VERSION == 22081
				geode::Popup::CloseEvent(foo).send();
				foo->setKeypadEnabled(false);
				foo->setTouchEnabled(false);
				foo->removeFromParent();
				#endif

				#if GEODE_COMP_GD_VERSION == 22074
				foo->keyBackClicked();
				#endif
			}
			SongListLayer::create()->show();
		});
		tab.addButton("Open Mod Settings (from main menu only!)", []() {
			if (VANILLA_GD_MENU_LOOP_DISABLED || GJBaseGameLayer::get() || !CCScene::get() || !MenuLayer::get() || CCScene::get()->getChildByType<MenuLayer>(0) != MenuLayer::get()) return;
			if (CCScene::get() && CCScene::get()->getChildByType<AndroidUI>(0) && SongManager::get().qolModIntegrationSuccessful) return;
			if (SongControlMenu* foo = CCScene::get()->getChildByType<SongControlMenu>(0); foo) {
				if (foo->m_osu && MenuLayer::get() && !MenuLayer::get()->isVisible()) MenuLayer::get()->setVisible(true);
				#if GEODE_COMP_GD_VERSION == 22081
				geode::Popup::CloseEvent(foo).send();
				foo->setKeypadEnabled(false);
				foo->setTouchEnabled(false);
				foo->removeFromParent();
				#endif

				#if GEODE_COMP_GD_VERSION == 22074
				foo->keyBackClicked();
				#endif
			}
			if (SongListLayer* bar = CCScene::get()->getChildByType<SongListLayer>(0); bar) {
				#if GEODE_COMP_GD_VERSION == 22081
				geode::Popup::CloseEvent(bar).send();
				bar->setKeypadEnabled(false);
				bar->setTouchEnabled(false);
				bar->removeFromParent();
				#endif

				#if GEODE_COMP_GD_VERSION == 22074
				bar->keyBackClicked();
				#endif
			}

			for (CCNode* node : CCArrayExt<CCNode*>(CCScene::get()->getChildren())) {
				if (geode::cocos::getObjectName(node) != "ModSettingsPopup") continue;
				if (!node->getChildByType<CCLayer>(0)) continue;

				CCLabelBMFont* title = node->getChildByType<CCLayer>(0)->getChildByType<CCLabelBMFont>(0);
				if (!title || static_cast<std::string>(title->getFntFile()) != "goldFont.fnt" || !geode::utils::string::endsWith(title->getString(), Mod::get()->getName())) continue;
				return;
			}
			geode::openSettingsPopup(Mod::get());
		});

		SongManager::get().eclipseIntegrationSuccessful = true;

		// surely. surely this will work.
		Loader::get()->queueInMainThread([]() {
			SongManager& sm = SongManager::get();
			if (!sm.eclipseIsCocosStyle) sm.eclipseSongNameLabel.value().setText(fmt::format("Current song: {}", Utils::getFullNameOfCurrentSongForIntegrationsAndControlPanel()));
			else sm.eclipseSongNameLabel.value().setText("Song progress is unavailable in this menu style!");
			for (std::pair<std::optional<eclipse::components::Label>, std::string>& labelStringPair : sm.dynamicEclipseIntegrationHeaders) {
				if (!labelStringPair.first.has_value()) continue;
				labelStringPair.first.value().setText(sm.eclipseIsCocosStyle ? fmt::format("{}\n", labelStringPair.second) : labelStringPair.second);
			}
		});
	});
}

#include "CategoryExt.hpp"

$on_mod(Loaded) {
	Loader::get()->queueInMainThread([]() {
		if (!Mod::get()->getSettingValue<bool>("qolModIntegration")) return;
		if (!SongManager::get().qolModIntegrationSuccessful) SongManager::get().qolModIntegrationSuccessful = true;
		qolmod::ext::addCustomCategory({1, Mod::get()->getName(), "qolmod.png"_spr, Mod::get()->getID(), [](cocos2d::CCMenu* layer) {
			if (!layer) return;
			if (SongControlMenu* foo = CCScene::get()->getChildByType<SongControlMenu>(0); foo) {
				if (foo->m_osu && MenuLayer::get() && !MenuLayer::get()->isVisible()) MenuLayer::get()->setVisible(true);
				#if GEODE_COMP_GD_VERSION == 22081
				geode::Popup::CloseEvent(foo).send();
				foo->setKeypadEnabled(false);
				foo->setTouchEnabled(false);
				foo->removeFromParent();
				#endif

				#if GEODE_COMP_GD_VERSION == 22074
				foo->keyBackClicked();
				#endif
			}
			if (SongListLayer* bar = CCScene::get()->getChildByType<SongListLayer>(0); bar) {
				#if GEODE_COMP_GD_VERSION == 22081
				geode::Popup::CloseEvent(bar).send();
				bar->setKeypadEnabled(false);
				bar->setTouchEnabled(false);
				bar->removeFromParent();
				#endif

				#if GEODE_COMP_GD_VERSION == 22074
				bar->keyBackClicked();
				#endif
			}

			// if you're going to steal my source code then at least watch Heated Rivalry S1E6 to understand these variable name references, you cheap clankers and/or fuckwads --raydeeux
			CCLabelBMFont* iHaveThisProblem = CCLabelBMFont::create("[If you're viewing this inside the QOLMod Discord server, whoever sent this has never learned how to read.]", "chatFont.fnt");
			iHaveThisProblem->limitLabelWidth(layer->getContentWidth() * .65f, 1.f, .0001f);
			iHaveThisProblem->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
			iHaveThisProblem->setScale(.4f);

			CCLabelBMFont* iLikeWomenYesAndEverywhereIGoImSurroundedByBeautifulWomenAndTheyLoveMeAndTheseWomenTheyreSoSexyAndFun = CCLabelBMFont::create("Heads up! This menu is NOT native to QOLMod!!", "goldFont.fnt");
			iLikeWomenYesAndEverywhereIGoImSurroundedByBeautifulWomenAndTheyLoveMeAndTheseWomenTheyreSoSexyAndFun->limitLabelWidth(layer->getContentWidth() * .85f, 1.f, .0001f);
			iLikeWomenYesAndEverywhereIGoImSurroundedByBeautifulWomenAndTheyLoveMeAndTheseWomenTheyreSoSexyAndFun->setScale(.5f);

			CCLabelBMFont* butImAlwaysThinkgAboutThisSlowFuckingHockeyPlayerWithBeautifulFrecklesAndAWeakBackhandAndHesSoBoringAndHeDrivesThisTerribleCar = CCLabelBMFont::create("This menu comes from Menu Loop Randomizer!", "bigFont.fnt");
			butImAlwaysThinkgAboutThisSlowFuckingHockeyPlayerWithBeautifulFrecklesAndAWeakBackhandAndHesSoBoringAndHeDrivesThisTerribleCar->limitLabelWidth(layer->getContentWidth() * .75f, 1.f, .0001f);
			butImAlwaysThinkgAboutThisSlowFuckingHockeyPlayerWithBeautifulFrecklesAndAWeakBackhandAndHesSoBoringAndHeDrivesThisTerribleCar->setScale(.4f);

			CCLabelBMFont* iAmAlwaysWishingThatTheseWomenWereHimItsATerribleProblemIDontEverWantThatProblemToEverGoAway = CCLabelBMFont::create("Any bugs from using this menu ***__MUST__*** be reported to RayDeeUx and ***__ONLY__*** RayDeeUx!!", "chatFont.fnt");
			iAmAlwaysWishingThatTheseWomenWereHimItsATerribleProblemIDontEverWantThatProblemToEverGoAway->limitLabelWidth(layer->getContentWidth() * .65f, 1.f, .0001f);
			iAmAlwaysWishingThatTheseWomenWereHimItsATerribleProblemIDontEverWantThatProblemToEverGoAway->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
			iAmAlwaysWishingThatTheseWomenWereHimItsATerribleProblemIDontEverWantThatProblemToEverGoAway->setScale(.45f);

			layer->addChildAtPosition(iLikeWomenYesAndEverywhereIGoImSurroundedByBeautifulWomenAndTheyLoveMeAndTheseWomenTheyreSoSexyAndFun, geode::Anchor::Top, {0.f, -10.f});
			layer->addChildAtPosition(butImAlwaysThinkgAboutThisSlowFuckingHockeyPlayerWithBeautifulFrecklesAndAWeakBackhandAndHesSoBoringAndHeDrivesThisTerribleCar, geode::Anchor::Top, {0.f, -25.f});
			layer->addChildAtPosition(iAmAlwaysWishingThatTheseWomenWereHimItsATerribleProblemIDontEverWantThatProblemToEverGoAway, geode::Anchor::Top, {0.f, -37.5f});
			layer->addChildAtPosition(iHaveThisProblem, geode::Anchor::Top, {0.f, -45.f});

			if (VANILLA_GD_MENU_LOOP_DISABLED || GJBaseGameLayer::get() || FMODAudioEngine::get()->getActiveMusic(0) != SongManager::get().getCurrentSong()) {
				CCLabelBMFont* dontMarrySvetlana = CCLabelBMFont::create("Menu Loop Randomizer is currently not active!", "bigFont.fnt");
				dontMarrySvetlana->limitLabelWidth(layer->getContentWidth() * .75f, 1.f, .0001f);

				CCLabelBMFont* justDont = CCLabelBMFont::create("(Might wanna get that checked out.)", "chatFont.fnt");
				if (GJBaseGameLayer::get()) justDont->setString("(You're in a level right now. Try again later.)");
				else if (!VANILLA_GD_MENU_LOOP_DISABLED && FMODAudioEngine::get()->getActiveMusic(0) != SongManager::get().getCurrentSong()) justDont->setString("(This menu loop isn't from MLR. Go figure out why ON YOUR OWN.)");
				justDont->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
				justDont->limitLabelWidth(layer->getContentWidth() * .5f, 1.f, .0001f);
				justDont->setScale(.75f);

				layer->addChildAtPosition(dontMarrySvetlana, geode::Anchor::Center, {0.f, 10.f});
				layer->addChildAtPosition(justDont, geode::Anchor::Center, {0.f, -5.f});

				return;
			}

			if (!SongManager::get().getFinishedCalculatingSongLengths()) {
				CCLabelBMFont* dontMarrySvetlana = CCLabelBMFont::create("Menu Loop Randomizer is currently busy!", "bigFont.fnt");
				dontMarrySvetlana->limitLabelWidth(layer->getContentWidth() * .75f, 1.f, .0001f);

				CCLabelBMFont* justDont = CCLabelBMFont::create("(Check back again in a bit.)", "chatFont.fnt");
				justDont->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
				justDont->limitLabelWidth(layer->getContentWidth() * .5f, 1.f, .0001f);
				justDont->setScale(.75f);

				layer->addChildAtPosition(dontMarrySvetlana, geode::Anchor::Center, {0.f, 10.f});
				layer->addChildAtPosition(justDont, geode::Anchor::Center, {0.f, -5.f});

				return;
			}

			SongManager::get().songControlMenuForQOLMod = nullptr;
			SongManager::get().addingToQOLModRightNow = true;
			SongControlMenu* iKnowItWouldntBeForLoveOrWhatever = SongControlMenu::create();
			iKnowItWouldntBeForLoveOrWhatever->show();
			SongManager::get().addingToQOLModRightNow = false;
			SongManager::get().songControlMenuForQOLMod = geode::Ref(iKnowItWouldntBeForLoveOrWhatever);

			#if GEODE_COMP_GD_VERSION > 22074
			iKnowItWouldntBeForLoveOrWhatever->setKeypadEnabled(false);
			iKnowItWouldntBeForLoveOrWhatever->setTouchEnabled(false);
			iKnowItWouldntBeForLoveOrWhatever->removeFromParent();
			#endif

			#if GEODE_COMP_GD_VERSION < 22081
			iKnowItWouldntBeForLoveOrWhatever->setKeypadEnabled(false);
			iKnowItWouldntBeForLoveOrWhatever->setTouchEnabled(false);
			iKnowItWouldntBeForLoveOrWhatever->removeFromParentAndCleanup(true);
			#endif

			layer->addChildAtPosition(SongManager::get().songControlMenuForQOLMod, geode::Anchor::Center);
			SongManager::get().songControlMenuForQOLMod->ignoreAnchorPointForPosition(false);
			SongManager::get().songControlMenuForQOLMod->schedule(schedule_selector(SongControlMenu::checkDaSongPositions), 2.f / 60.f);
			SongManager::get().songControlMenuForQOLMod->schedule(schedule_selector(SongControlMenu::forceSharpCornerIllusionScheduler));
			SongManager::get().songControlMenuForQOLMod->schedule(schedule_selector(SongControlMenu::pressAndHoldScheduler), .125f);
		}});
	});
}