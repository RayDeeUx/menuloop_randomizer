#include "SongManager.hpp"
#include "ui/PlayingCard.hpp"
#include <Geode/Geode.hpp>
#include <Geode/loader/SettingEvent.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/MusicDownloadManager.hpp>
#include <Geode/modify/OptionsLayer.hpp>
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
		auto notificationEnabled = Mod::get()->getSettingValue<bool>("enableNotification");

		if (notificationEnabled) {
			// create notif card stuff
			auto screenSize = CCDirector::get()->getWinSize();
			auto notificationTime = Mod::get()->getSettingValue<double>("notificationTime");

			auto songFileName = std::filesystem::path(songManager.getCurrentSong()).filename();

			PlayingCard *card;

			if (Mod::get()->getSettingValue<bool>("useCustomSongs")) {
				card = PlayingCard::create(fmt::format("Now playing: {}", songFileName));
			} else {
				size_t dotPos = songFileName.string().find_last_of(".");

				if (dotPos != std::string::npos)
					songFileName = songFileName.string().substr(0, dotPos);

				auto songInfo = downloadManager->getSongInfoObject(std::stoi(songFileName));

				card = PlayingCard::create(fmt::format("Now playing: {} ({})", songInfo->m_songName, songInfo->m_songID));
			}

			card->position.x = screenSize.width / 2.0f;
			card->position.y = screenSize.height;

			auto defaultPos = card->position;
			auto posx = defaultPos.x;
			auto posy = defaultPos.y;

			card->setPosition(defaultPos);
			this->addChild(card);

			auto sequence = CCSequence::create(
				CCEaseInOut::create(CCMoveTo::create(1.5f, {posx, posy - 24.0f}), 2.0f),
				CCDelayTime::create(notificationTime),
				CCEaseInOut::create(CCMoveTo::create(1.5f, {posx, posy}), 2.0f),
				nullptr
			);
			card->runAction(sequence);
		}

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
		auto gameManager = GameManager::sharedState();
		auto audioEngine = FMODAudioEngine::sharedEngine();

		audioEngine->m_backgroundMusicChannel->stop();
		songManager.pickRandomSong();
		gameManager->playMenuMusic();
	}
};

struct OptionsLayerHook : Modify<OptionsLayerHook, OptionsLayer> {
	void customSetup() {
		OptionsLayer::customSetup();

		// add the folder btn to the settings layer
		auto menu = CCMenu::create();

		auto btn = CCMenuItemSpriteExtra::create(
			geode::CircleButtonSprite::create(
				CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png"),
				geode::CircleBaseColor::Pink
			),
			this,
			menu_selector(OptionsLayerHook::onSongsBtn)
		);

		btn->setPosition({-144.0f, -60.0f});
		btn->m_scaleMultiplier = 1.1f;
		menu->addChild(btn);
		m_mainLayer->addChild(menu);
	}

	void onSongsBtn(CCObject *sender) {
		utils::file::openFolder(Mod::get()->getConfigDir());
	}
};

$on_mod(Loaded) {
	/*
		if custom songs are enabled search for files in the config dir
		if not, just use the newgrounds songs
	*/
	if (Mod::get()->getSettingValue<bool>("useCustomSongs")) {
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
	listenForSettingChanges<bool>("useCustomSongs", [](bool value) {
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
				std::string songPath = downloadManager->pathForSong(song->m_songID);

				if (songPath.ends_with(".mp3")) {
					log::debug("Adding NG song: {}", downloadManager->pathForSong(song->m_songID));
					songManager.addSong(downloadManager->pathForSong(song->m_songID));
				}
			}
		}

		// change the song when you click apply.
		auto gameManager = GameManager::sharedState();
		auto audioEngine = FMODAudioEngine::sharedEngine();

		audioEngine->m_backgroundMusicChannel->stop();
		songManager.pickRandomSong();
		gameManager->playMenuMusic();
	});
}
