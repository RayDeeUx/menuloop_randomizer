#include "SongManager.hpp"
#include "Utils.hpp"
#include "ui/PlayingCard.hpp"
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/SongInfoObject.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <filesystem>

using namespace geode::prelude;

class $modify(MenuLoopMLHook, MenuLayer) {
	struct Fields {
		SongManager &songManager = SongManager::get();
	};

	bool init() {
		if (!MenuLayer::init())
			return false;

		Utils::removeCard();

		if (Utils::getBool("enableNotification"))
			MenuLoopMLHook::generateNotifcation();

		if (Utils::getBool("enableShuffleButton"))
			MenuLoopMLHook::addShuffleButton();

		return true;
	}

	void generateNotifcation() {
		auto songFileName = std::filesystem::path(m_fields->songManager.getCurrentSong()).filename();

		std::string notifString;
		auto prefix = Mod::get()->getSettingValue<std::string>("customPrefix");
		if (prefix != "[Empty]")
			notifString = fmt::format("{}: ", prefix);

		if (Utils::getBool("useCustomSongs")) {
			notifString = notifString.append(songFileName.string());
		} else {
			// in case that the current file selected is the original menuloop, don't gather any info
			if (m_fields->songManager.isOriginalMenuLoop()) {
				notifString = notifString.append("Original Menu Loop by RobTop");
			} else {
				log::info("attempting to play {}", songFileName.string());
				// if its not menuLoop.mp3, then get info
				size_t dotPos = songFileName.string().find_last_of('.');

				if (dotPos == std::string::npos) {
					log::error("{} was not a valid file name..? [NG/Music Library]", songFileName.string());
					notifString = notifString.append("Unknown");
					return;
				}
				std::string songFileNameAsAtring = songFileName.string().substr(0, dotPos);

				Result<int> songFileNameAsID = geode::utils::numFromString<int>(songFileNameAsAtring);

				if (songFileNameAsID.isErr()) {
					log::error("{} had an invalid Song ID! [NG/Music Library]", songFileNameAsAtring);
					notifString = notifString.append("Unknown (Invalid Song ID)");
					return;
				}

				auto songInfo = MusicDownloadManager::sharedState()->getSongInfoObject(songFileNameAsID.unwrap());

				// sometimes songInfo is nullptr, so improvise
				if (songInfo) {
					// default: "Song Name, Artist, Song ID"
					// fmt::format("{} by {} ({})", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songID);
					std::string resultString = "";
					auto formatSetting = Mod::get()->getSettingValue<std::string>("songFormatNGML");
					if (formatSetting == "Song Name, Artist, Song ID") {
						resultString = fmt::format("{} by {} ({})", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songID);
					} else if (formatSetting == "Song Name + Artist") {
						resultString = fmt::format("{} by {}", songInfo->m_songName, songInfo->m_artistName);
					} else if (formatSetting == "Song Name + Song ID") {
						resultString = fmt::format("{} ({})", songInfo->m_songName, songInfo->m_songID);
					} else {
						resultString = fmt::format("{}", songInfo->m_songName);
					}
					notifString = notifString.append(resultString);
				} else {
					notifString = notifString.append(songFileName.string());
				}
			}
		}

		// create notif card stuff
		auto card = PlayingCard::create(notifString);
		auto screenSize = CCDirector::get()->getWinSize();

		card->position.x = screenSize.width / 2.0f;
		card->position.y = screenSize.height;

		auto defaultPos = card->position;
		auto posx = defaultPos.x;
		auto posy = defaultPos.y;

		card->setPosition(defaultPos);
		card->setZOrder(200);
		card->setID("now-playing"_spr);
		this->addChild(card);

		auto sequence = CCSequence::create(
			CCEaseInOut::create(CCMoveTo::create(1.5f, {posx, posy - 24.0f}), 2.0f),
			CCDelayTime::create(Mod::get()->getSettingValue<double>("notificationTime")),
			CCEaseInOut::create(CCMoveTo::create(1.5f, {posx, posy}), 2.0f),
			nullptr
		);
		card->runAction(sequence);
	}

	void addShuffleButton() {
		auto menu = getChildByID("right-side-menu");

		auto btn = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::create(CCSprite::create("shuffle-btn-sprite.png"_spr)),
			this,
			menu_selector(MenuLoopMLHook::onShuffleBtn)
		);
		btn->setID("shuffle-button"_spr);

		menu->addChild(btn);
		menu->updateLayout();
	}

	void onShuffleBtn(CCObject *sender) {
		Utils::removeCard();

		Utils::setNewSong();

		if (Utils::getBool("enableNotification"))
			MenuLoopMLHook::generateNotifcation();
	}
};