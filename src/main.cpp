#include "PlayingCard.hpp"
#include "Song.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <random>
#include <vector>

using namespace geode::prelude;

// global variables
std::vector<Song> songs;
Song *selectedSong;

$on_mod(Loaded) {
	// get the path for the songs
	std::filesystem::path ngSongsPath = CCFileUtils::get()->getWritablePath().c_str();

	// add all the mp3 files to the vector
	for (auto &song : std::filesystem::directory_iterator(ngSongsPath)) {
		if (song.path().string().ends_with(".mp3")) {
			auto id = song.path().filename().string().substr(0, song.path().filename().string().length() - 4);
			auto path = song.path().string();

			Song *currentSong = new Song();
			currentSong->id = id;
			currentSong->path = path;
			songs.push_back(*currentSong);
		}
	}

	// select a random item from the vector and return the path
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, songs.size() - 1);
	int randomIndex = dist(gen);
	selectedSong = &songs[randomIndex];
}

struct GameManagerHook : Modify<GameManagerHook, GameManager> {
	gd::string getMenuMusicFile() {
		return selectedSong->path;
	}
};

struct MenuLayerHook : Modify<MenuLayerHook, MenuLayer> {
	bool init() {
		if (!MenuLayer::init())
			return false;

		auto downloadManager = MusicDownloadManager::sharedState();

		auto screenSize = CCDirector::get()->getWinSize();
		auto cardSettingValue = Mod::get()->getSettingValue<bool>("nowPlayingCard");

		if (cardSettingValue) {
			if (auto songObject = downloadManager->getSongInfoObject(stoi(selectedSong->id))) {
				selectedSong->name = songObject->m_songName;
			} else {
				selectedSong->name = "Unknown";
			}

			auto card = PlayingCard::create(selectedSong->name, selectedSong->id);
			card->position.x = screenSize.width / 2.0f;
			card->position.y = screenSize.height;

			auto defaultPos = card->position;
			auto posx = defaultPos.x;
			auto posy = defaultPos.y;

			card->setPosition(defaultPos);
			this->addChild(card);

			auto sequence = CCSequence::create(
				CCEaseInOut::create(CCMoveTo::create(1.5f, {posx, posy - 25.0f}), 2.0f),
				CCDelayTime::create(0.5f),
				CCEaseInOut::create(CCMoveTo::create(1.5f, {posx, posy}), 2.0f),
				nullptr
			);
			card->runAction(sequence);
		}

		return true;
	}
};
