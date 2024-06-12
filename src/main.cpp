#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <random>
#include <vector>

using namespace geode::prelude;

std::string selectedSong;

$execute {
	std::vector<std::string> songs;

	// get the path for the songs
	std::filesystem::path ngSongsPath = CCFileUtils::get()->getWritablePath().c_str();

	// add all the mp3 files to the vector
	for (auto &song : std::filesystem::directory_iterator(ngSongsPath)) {
		if (song.path().string().ends_with(".mp3"))
			songs.push_back(song.path().string());
	}

	// select a random item from the vector and return the path
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, songs.size() - 1);
	int randomIndex = dist(gen);

	selectedSong = songs[randomIndex];
}

struct GameManagerHook : Modify<GameManagerHook, GameManager> {
	gd::string getMenuMusicFile() {
		// return GameManager::getMenuMusicFile();

		return selectedSong;
	}
};