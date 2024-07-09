#include "SongManager.hpp"
#include "ui/PlayingCard.hpp"
#include <Geode/Geode.hpp>
#include <Geode/loader/SettingEvent.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

SongManager &songManager = SongManager::get();

struct GameManagerHook : Modify<GameManagerHook, GameManager> {
	gd::string getMenuMusicFile() {
		if (songManager.getCurrentSong().empty())
			return GameManager::getMenuMusicFile();

		return songManager.getCurrentSong();
	}
};

struct PauseLayerHook : Modify<PauseLayerHook, PauseLayer> {
	void onQuit(CCObject *sender) {
		if (Mod::get()->getSettingValue<bool>("randomizeWhenExitingLevel")) {
			songManager.pickRandomSong();
		}

		PauseLayer::onQuit(sender);
	}
};

struct EditorPauseLayerHook : Modify<EditorPauseLayerHook, EditorPauseLayer> {
	void onExitEditor(CCObject *sender) {
		if (Mod::get()->getSettingValue<bool>("randomizeWhenExitingEditor")) {
			songManager.pickRandomSong();
		}

		EditorPauseLayer::onExitEditor(sender);
	}
};

struct MenuLayerHook : Modify<MenuLayerHook, MenuLayer> {
	bool init() {
		if (!MenuLayer::init())
			return false;

		auto downloadManager = MusicDownloadManager::sharedState();

		// // create notif card stuff
		// auto screenSize = CCDirector::get()->getWinSize();
		// auto cardSettingValue = Mod::get()->getSettingValue<bool>("nowPlayingCard");
		// auto screenTime = Mod::get()->getSettingValue<double>("notificationTime");

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

		// add a shuffle button
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
			geode::log::debug("Adding song path: {}", file.path().string());
			songManager.addSong(file.path().string());
		}
	} else {
		auto downloadManager = MusicDownloadManager::sharedState();

		CCArrayExt<SongInfoObject *> songs = downloadManager->getDownloadedSongs();
		for (auto song : songs) {
			songManager.addSong(downloadManager->pathForSong(song->m_songID));
		}
	}

	songManager.pickRandomSong();
}

$execute {
	listenForSettingChanges<bool>("useCustomSongsPath", [](bool value) {
		songManager.clearSongs();

		if (value) {
			auto configPath = geode::Mod::get()->getConfigDir();

			for (auto file : std::filesystem::directory_iterator(configPath)) {
				if (file.path().string().ends_with(".mp3")) {
					log::debug("Adding custom song: {}", file.path().string());
					songManager.addSong(file.path().string());
				}
			}
		} else {
			auto downloadManager = MusicDownloadManager::sharedState();

			CCArrayExt<SongInfoObject *> songs = downloadManager->getDownloadedSongs();
			for (auto song : songs) {
				auto songPath = downloadManager->pathForSong(song->m_songID);

				if (songPath.ends_with("mp3")) {
					log::debug("Adding NG song: {}", downloadManager->pathForSong(song->m_songID));
					songManager.addSong(downloadManager->pathForSong(song->m_songID));
				}
			}
		}
	});
}
