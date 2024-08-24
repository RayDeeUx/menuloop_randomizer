#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/loader/SettingEvent.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#ifndef GEODE_IS_ANDROID
#include <Geode/utils/Cocos.hpp>
#endif

using namespace geode::prelude;

SongManager &songManager = SongManager::get();
std::filesystem::path configDir = Mod::get()->getConfigDir();

void populateVector(bool customSongs) {
	/*
		if custom songs are enabled search for files in the config dir
		if not, just use the newgrounds songs
		--elnex
	*/
	if (customSongs) {
		for (auto file : std::filesystem::directory_iterator(configDir)) {
			if (!std::filesystem::exists(file)) continue;

			auto filePath = file.path();
			auto filePathString = filePath.string();

			if (!Utils::isSupportedExtension(filePathString)) continue;

			log::debug("Adding custom song: {}", filePath.filename().string());
			songManager.addSong(filePathString);
		}
	} else {
		auto downloadManager = MusicDownloadManager::sharedState();

		// for every downloaded song push it to the m_songs vector --elnex
		/*
		getDownloadedSongs() function call binding for macOS found
		from ninXout (ARM) and hiimjustin000 (Intel + verification)

		remarks:
		- getDownloadedSongs() grabs both music library and newgrounds
		songs lol.
		- only reason it didn't work was because file support was limited to `.mp3`.
		--raydeeux
		*/
		CCArrayExt<SongInfoObject*> songs = downloadManager->getDownloadedSongs();
		for (auto song : songs) {
			if (!song) continue;
			
			std::string songPath = downloadManager->pathForSong(song->m_songID);

			if (!Utils::isSupportedExtension(songPath)) continue;

			log::debug("Adding Newgrounds/Music Library song: {}", songPath);
			songManager.addSong(songPath);
		}
		// same thing as NG but for music library as well --ninXout
		// SPOILER: IT DOESN'T WORK CROSSPLATFORM (android specifically)! --raydeeux
		/*
		std::filesystem::path musicLibrarySongs = dirs::getGeodeDir().parent_path() / "Resources" / "songs";
		if (!std::filesystem::exists(musicLibrarySongs)) return;
		for (const std::filesystem::path& dirEntry : std::filesystem::recursive_directory_iterator(musicLibrarySongs)) {
			if (!std::filesystem::exists(dirEntry)) continue;

			std::string songPath = dirEntry.string();

			if (!Utils::isSupportedExtension(songPath)) continue;

			log::debug("Adding Music Library song: {}", songPath);
			songManager.addSong(songPath);
		}
		*/
	}
}

$on_mod(Loaded) {
	populateVector(Utils::getBool("useCustomSongs"));

	songManager.pickRandomSong();

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
		populateVector(value);

		// change the song when you click apply, stoi will not like custom names. --elnexreal

		Utils::setNewSong();
	});
}