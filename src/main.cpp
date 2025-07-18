#include "ConstantShuffleModeWarning.hpp"
#include "SongManager.hpp"
#include "Utils.hpp"
#include "Settings.hpp"
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

SongManager &songManager = SongManager::get();
std::filesystem::path configDir = Mod::get()->getConfigDir();
bool originalOverrideWasEmpty = false;

$on_mod(Loaded) {
	(void) Mod::get()->registerCustomSettingType("configdir", &MyButtonSettingV3::parse);
	songManager.setConstantShuffleMode();

	auto blacklistTxt = configDir / R"(blacklist.txt)";
	if (!std::filesystem::exists(blacklistTxt)) {
		std::string blacklistFileIntro = R"(# Welcome to the Menu Loop Randomizer song blacklist!
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
		std::string favoritesFileIntro = R"(# Welcome to the Menu Loop Randomizer favorites list!
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

	geode::log::info("repopulating vector from on_mod(Loaded)");
	Utils::refreshTheVector();

	std::string override = Mod::get()->getSettingValue<std::filesystem::path>("specificSongOverride").string();
	originalOverrideWasEmpty = override.empty();
	songManager.setOverride(override);

	const std::string& lastMenuLoop = Mod::get()->getSavedValue<std::string>("lastMenuLoop");
	const bool saveSongOnGameClose = Utils::getBool("saveSongOnGameClose");
	const bool loopExists = std::filesystem::exists(lastMenuLoop);
	log::info("\n=== 'REMEMBER LAST MENU LOOP' DEBUG INFO ===\nlast menu loop: {}\n'saveSongOnGameClose' setting: {}\nloopExists: {}\noverride: {}", lastMenuLoop, saveSongOnGameClose, loopExists, override);
	if (!override.empty() && Utils::isSupportedFile(override)) {
		log::info("setting songManager's current song to overridden song from settings");
		songManager.setCurrentSongToOverride();
	} else if (!lastMenuLoop.empty() && Utils::isSupportedFile(lastMenuLoop) && loopExists && saveSongOnGameClose) {
		log::info("setting songManager's current song to saved song from on_mod(Loaded)");
		songManager.setCurrentSongToSavedSong();
	} else {
		log::info("randomizing songManager's current song through on_mod(Loaded)");
		songManager.pickRandomSong();
	}

	if (!std::filesystem::exists(configDir / R"(store_your_disabled_menuloops_here)")) {
		std::filesystem::create_directory(configDir / R"(store_your_disabled_menuloops_here)");
	}
	GameManager::get()->schedule(reinterpret_cast<SEL_SCHEDULE>(&SongManager::updateWrapper));
	listenForSettingChanges<bool>("useCustomSongs", [](bool useCustomSongs) {
		Utils::resetSongManagerRefreshVectorSetNewSongBecause("useCustomSongs");
	});
	listenForSettingChanges<bool>("searchDeeper", [](bool searchDeeper) {
		Utils::resetSongManagerRefreshVectorSetNewSongBecause("searchDeeper");
	});
	listenForSettingChanges<std::filesystem::path>("additionalFolder", [](std::filesystem::path searchDeeper) {
		Utils::resetSongManagerRefreshVectorSetNewSongBecause("additionalFolder");
	});
	listenForSettingChanges<bool>("loadPlaylistFile", [](bool loadPlaylistFile) {
		Utils::resetSongManagerRefreshVectorSetNewSongBecause("loadPlaylistFile");
	});
	listenForSettingChanges<std::filesystem::path>("playlistFile", [](std::filesystem::path playlistFile) {
		Utils::resetSongManagerRefreshVectorSetNewSongBecause("playlistFile");
	});
	listenForSettingChanges<bool>("playlistMode", [](bool constantShuffleMode) {
		SongManager& songManager = SongManager::get();
		songManager.setConstantShuffleMode();
		if (songManager.isOriginalMenuLoop()) return;
		FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		Utils::queueUpdateSCMLabel();
		if (constantShuffleMode) {
			FMODAudioEngine::get()->playMusic(songManager.getCurrentSong(), true, 1.0f, 1);
			return ConstantShuffleModeWarning::create(songManager.getGeodify())->show();
		}
		GameManager::sharedState()->playMenuMusic();
	});
	listenForSettingChanges<std::filesystem::path>("specificSongOverride", [](std::filesystem::path specificSongOverride) {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
		const std::string& overrideString = specificSongOverride.string();
		songManager.setOverride(overrideString);
		if (!Utils::isSupportedFile(overrideString)) {
			songManager.clearSongs();
			geode::log::info("repopulating vector from removing override");
			Utils::refreshTheVector();
			if (Utils::isSupportedFile(Mod::get()->getSavedValue<std::string>("lastMenuLoop")) && Utils::getBool("saveSongOnGameClose") && !originalOverrideWasEmpty) {
				log::info("setting songManager's current song to saved song from settings change");
				songManager.setCurrentSongToSavedSong();
			} else Utils::setNewSong();
		}
		geode::Loader::get()->queueInMainThread([] { Utils::queueUpdateSCMLabel(); });
		if (Utils::getBool("playlistMode")) return FMODAudioEngine::get()->playMusic(SongManager::get().getCurrentSong(), true, 1.0f, 1);
		GameManager::sharedState()->playMenuMusic();
	});
	listenForSettingChanges<bool>("dangerousBlacklisting", [](bool dangerousBlacklisting) {
		if (!dangerousBlacklisting) return;
		log::info("=============== WARNING: USER HAS ENABLED `dangerousBlacklisting` SETTING ===============");
		if (GameManager::get()->m_playerUserID.value() == 925143 || GameManager::get()->m_playerUserID.value() == 7247326) return log::info("never mind, it's just aktimoose the beta tester. don't show the alert");
		FLAlertLayer::create("Menu Loop Randomizer", "<c_>This is an experimental setting. You agree to hold yourself responsible for any issues that happen when this setting is enabled.</c>", "I understand")->show();
	});
}