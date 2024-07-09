#include "SongManager.hpp"
#include "ui/PlayingCard.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

SongManager *songManager = SongManager::get();

struct GameManagerHook : Modify<GameManagerHook, GameManager> {
	gd::string getMenuMusicFile() {
		if (!songManager->getCurrentSong().empty())
			return songManager->getCurrentSong();

		return GameManager::getMenuMusicFile();
	}
};

struct PauseLayerHook : Modify<PauseLayerHook, PauseLayer> {
	void onQuit(CCObject *sender) {
		if (Mod::get()->getSettingValue<bool>("randomizeWhenExitingLevel")) {
			songManager->pickRandomSong();
		}

		PauseLayer::onQuit(sender);
	}
};

struct EditorPauseLayerHook : Modify<EditorPauseLayerHook, EditorPauseLayer> {
	void onExitEditor(CCObject *sender) {
		if (Mod::get()->getSettingValue<bool>("randomizeWhenExitingEditor")) {
			songManager->pickRandomSong();
		}

		EditorPauseLayer::onExitEditor(sender);
	}
};

struct MenuLayerHook : Modify<MenuLayerHook, MenuLayer> {
	bool init() {
		if (!MenuLayer::init())
			return false;

		auto downloadManager = MusicDownloadManager::sharedState();

		// create notif card stuff
		auto screenSize = CCDirector::get()->getWinSize();
		auto cardSettingValue = Mod::get()->getSettingValue<bool>("nowPlayingCard");
		auto screenTime = Mod::get()->getSettingValue<double>("notificationTime");

		// if (cardSettingValue) {
		// 	if (auto songObject = downloadManager->getSongInfoObject(stoi(selectedSong.id))) {
		// 		selectedSong.name = songObject->m_songName;
		// 	} else {
		// 		selectedSong.name = "Unknown";
		// 	}

		// 	auto card = PlayingCard::create(selectedSong.name, selectedSong.id);
		// 	card->position.x = screenSize.width / 2.0f;
		// 	card->position.y = screenSize.height;

		// 	auto defaultPos = card->position;
		// 	auto posx = defaultPos.x;
		// 	auto posy = defaultPos.y;

		// 	card->setPosition(defaultPos);
		// 	this->addChild(card);

		// 	auto sequence = CCSequence::create(
		// 		CCEaseInOut::create(CCMoveTo::create(1.5f, {posx, posy - 24.0f}), 2.0f),
		// 		CCDelayTime::create(screenTime),
		// 		CCEaseInOut::create(CCMoveTo::create(1.5f, {posx, posy}), 2.0f),
		// 		nullptr
		// 	);
		// 	card->runAction(sequence);
		// }

		// // add a shuffle button
		auto menu = getChildByID("right-side-menu");

		auto btn = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::create(
				CCSprite::create("shuffle-btn-sprite.png"_spr)
			),
			this,
			menu_selector(MenuLayerHook::shuffleBtn)
		);

		menu->addChild(btn);

		menu->updateLayout();

		return true;
	}

	void shuffleBtn(CCObject *sender) {
		// todo
	}
};

$on_mod(Loaded) {
	if (Mod::get()->getSettingValue<bool>("useCustomSongsPath")) {
		auto configPath = geode::Mod::get()->getConfigDir();

		for (auto file : std::filesystem::directory_iterator(configPath)) {
			songManager->addSong(file.path().string());
		}
	} else {
		auto downloadManager = MusicDownloadManager::sharedState();

		CCArrayExt<SongInfoObject *> songs = downloadManager->getDownloadedSongs();
		for (auto song : songs) {
			songManager->addSong(downloadManager->pathForSong(song->m_songID));
		}
	}

	songManager->pickRandomSong();
}