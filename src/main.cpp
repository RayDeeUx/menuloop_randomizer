#include "SongManager.hpp"
#include "Utils.hpp"
#include "Settings.hpp"
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

SongManager &songManager = SongManager::get();
std::filesystem::path configDir = Mod::get()->getConfigDir();

$on_mod(Loaded) {
	Mod::get()->addCustomSetting<MySettingValue>("configdir", "none");

	auto blacklistTxt = configDir / R"(blacklist.txt)";
	if (!std::filesystem::exists(blacklistTxt)) {
		std::string content = R"(# Welcome to the Menu Loop Randomizer song blacklist!
# Each line that doesn't start with a "#" will be treated as a blacklisted song file.
# All lines that start with a "#" are ignored. This means you can un-blacklist a song by adding "#" next to it.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# --RayDeeUx)";
		auto result = utils::file::writeString(blacklistTxt, content);
		if (result.isErr()) {
			log::error("Error writing to blacklist.txt");
		}
	}

	Utils::populateVector(Utils::getBool("useCustomSongs"));

	std::string lastMenuLoop = Mod::get()->getSavedValue<std::string>("lastMenuLoop");
	bool saveSongOnGameClose = Utils::getBool("saveSongOnGameClose");
	bool loopExists = std::filesystem::exists(lastMenuLoop);
	log::info("\n=== 'REMEMBER LAST MENU LOOP' DEBUG INFO ===\nlast menu loop: {}\n'saveSongOnGameClose' setting: {}\nloopExists: {}", lastMenuLoop, saveSongOnGameClose, loopExists);
	if (!lastMenuLoop.empty() && Utils::isSupportedExtension(lastMenuLoop) && loopExists && saveSongOnGameClose) {
		log::info("setting songManager's current song to saved song from on_mod(Loaded)");
		songManager.setCurrentSongToSavedSong();
	} else {
		log::info("randomizing songManager's current song through on_mod(Loaded)");
		songManager.pickRandomSong();
	}

	if (!std::filesystem::exists(configDir / R"(store_your_disabled_menuloops_here)")) {
		std::filesystem::create_directory(configDir / R"(store_your_disabled_menuloops_here)");
	}
}

$execute {
	listenForSettingChanges<bool>("useCustomSongs", [](bool value) {
		// make sure m_songs is empty, we don't want to make a mess here --elnexreal
		songManager.clearSongs();

		/*
			for every custom song file, push its path to m_songs
			if they're ng songs also push the path bc we're going to use getPathForSong
			--elnexreal
		*/
		Utils::populateVector(value);

		// change the song when you click apply, stoi will not like custom names. --elnexreal

		Utils::setNewSong();
	});
	listenForSettingChanges<bool>("playlistMode", [](bool isPlaylistMode) {
		if (SongManager::get().isOriginalMenuLoop()) return;
		FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
		if (GameManager::sharedState()->getGameVariable("0122")) return;
		if (isPlaylistMode) {
			return FMODAudioEngine::get()->playMusic(SongManager::get().getCurrentSong(), true, 1.0f, 1);
		}
		GameManager::sharedState()->playMenuMusic();
	});
}