#include "PlayingCard.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <random>
#include <vector>

using namespace geode::prelude;

class Song {
  public:
	std::string songId;
	std::string songPath;
};

Song selectedSong;

$execute {
	std::vector<Song> songs;

	// get the path for the songs
	std::filesystem::path ngSongsPath = CCFileUtils::get()->getWritablePath().c_str();

	// add all the mp3 files to the vector
	for (auto &song : std::filesystem::directory_iterator(ngSongsPath)) {
		if (song.path().string().ends_with(".mp3")) {
			Song *currentSong = new Song();
			currentSong->songId = song.path().filename().string().substr(0, song.path().filename().string().length() - 4);
			currentSong->songPath = song.path().string();
			songs.push_back(*currentSong);
		}

		// select a random item from the vector and return the path
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(0, songs.size() - 1);
		int randomIndex = dist(gen);
		selectedSong = songs[randomIndex];
	}
}

struct GameManagerHook : Modify<GameManagerHook, GameManager> {
	gd::string getMenuMusicFile() {
		// return GameManager::getMenuMusicFile();

		return selectedSong.songPath;
	}
};

struct MenuLayerHook : Modify<MenuLayerHook, MenuLayer> {
	bool init() {
		if (!MenuLayer::init())
			return false;

		auto cardSettingValue = Mod::get()->getSettingValue<bool>("nowPlayingCard");

		if (cardSettingValue) {
			auto card = PlayingCard::create(selectedSong.songId);
			auto defaultPos = card->position;
			auto posx = defaultPos.x;
			auto posy = defaultPos.y;

			card->setPosition(defaultPos);
			this->addChild(card);

			auto sequence = CCSequence::create(
				CCEaseInOut::create(CCMoveTo::create(2.f, {posx - 120, posy}), 2.0f),
				CCDelayTime::create(0.5f),
				CCEaseInOut::create(CCMoveTo::create(2.f, {posx, posy}), 2.0f),
				nullptr
			);
			card->runAction(sequence);
		}

		// geode::Notification::create("Now playing: " + selectedSong.songId, geode::NotificationIcon::Loading, 1.f)->show();

		return true;
	}
};
