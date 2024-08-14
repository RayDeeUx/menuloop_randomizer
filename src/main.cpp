#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/loader/SettingEvent.hpp>
#include <Geode/loader/Dirs.hpp>

using namespace geode::prelude;

SongManager &songManager = SongManager::get();
std::filesystem::path configDir = Mod::get()->getConfigDir();

void populateVector(bool customSongs) {
	/*
		if custom songs are enabled search for files in the config dir
		if not, just use the newgrounds songs
	*/
	if (customSongs) {
		for (auto file : std::filesystem::directory_iterator(configDir)) {
			auto filePathString = file.path().string();

			if (!Utils::isSupportedExtension(filePathString))
				continue;

			log::debug("Adding custom song: {}", file.path().filename().string());
			songManager.addSong(filePathString);
		}
	} else {
		auto downloadManager = MusicDownloadManager::sharedState();

		// for every downloaded song push it to the m_songs vector
		/*
		getDownloadedSongs() function call binding for macOS found
		from ninXout (ARM) and hiimjustin000 (Intel + verification)
		*/
		CCArrayExt<SongInfoObject *> songs = downloadManager->getDownloadedSongs();
		for (auto song : songs) {
			std::string songPath = downloadManager->pathForSong(song->m_songID);

			if (!Utils::isSupportedExtension(songPath))
				continue;

			log::debug("Adding Newgrounds song: {}", songPath);
			songManager.addSong(songPath);
		}
		// same thing as NG but for music library as well --ninXout
		std::filesystem::path mlsongs = dirs::getGeodeDir().parent_path() / "Resources" / "songs";
		for (const std::filesystem::path& dirEntry : std::filesystem::recursive_directory_iterator(mlsongs)) {
			std::string songPath = dirEntry.string();

			if (!Utils::isSupportedExtension(songPath))
				continue;

			log::debug("Adding Music Library song: {}", songPath);
			songManager.addSong(songPath);
		}
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
		// make sure m_songs is empty, we don't want to make a mess here
		songManager.clearSongs();

		/*
			for every custom song file, push its path to m_songs
			if they're ng songs also push the path bc we're going to use getPathForSong
		*/
		populateVector(value);

		// change the song when you click apply, stoi will not like custom names.

		Utils::setNewSong();
	});
}