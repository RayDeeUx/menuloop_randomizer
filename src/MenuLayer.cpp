#include "SongManager.hpp"
#include "Utils.hpp"
#include "ui/PlayingCard.hpp"
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLayerMLHook, MenuLayer) {
	struct Fields {
		SongManager &songManager = SongManager::get();
		std::filesystem::path blacklistFile = Mod::get()->getConfigDir() / R"(blacklist.txt)";
	};

	bool init() {
		if (!MenuLayer::init())
			return false;

		Utils::removeCard();

		if (Utils::getBool("enableNotification"))
			Utils::generateNotification();

		if (Utils::getBool("enableShuffleButton"))
			MenuLayerMLHook::addShuffleButton();

		if (Utils::getBool("enableNotification") && Utils::getBool("enableNewNotification"))
			MenuLayerMLHook::addRegenButton();

		if (Utils::getBool("enableCopySongID"))
			MenuLayerMLHook::addCopyButton();

		if (Utils::getBool("enableBlacklistButton"))
			MenuLayerMLHook::addBlacklistButton();

		return true;
	}

	void addShuffleButton() {
		auto menu = getChildByID("right-side-menu");

		auto btn = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::create(CCSprite::create("shuffle-btn-sprite.png"_spr)),
			this,
			menu_selector(MenuLayerMLHook::onShuffleBtn)
		);
		btn->setID("shuffle-button"_spr);

		menu->addChild(btn);
		menu->updateLayout();
	}

	void onShuffleBtn(CCObject*) {
		Utils::removeCard();

		Utils::setNewSong();

		if (Utils::getBool("enableNotification"))
			Utils::generateNotification();
	}

	void addRegenButton() {
		auto menu = getChildByID("right-side-menu");

		auto btn = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::create(CCSprite::create("regen-btn-sprite.png"_spr)),
			this,
			menu_selector(MenuLayerMLHook::onRegenButton)
		);
		btn->setID("regen-button"_spr);

		menu->addChild(btn);
		menu->updateLayout();
	}

	void onRegenButton(CCObject*) {
		Utils::removeCard();

		if (Utils::getBool("enableNotification"))
			Utils::generateNotification();
	}

	void addCopyButton() {
		auto menu = getChildByID("right-side-menu");

		auto btn = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::create(CCSprite::create("copy-btn-sprite.png"_spr)),
			this,
			menu_selector(MenuLayerMLHook::onCopyButton)
		);
		btn->setID("copy-button"_spr);

		menu->addChild(btn);
		menu->updateLayout();
	}

	void onCopyButton(CCObject*) {
		Utils::copyCurrentSongName();
		Notification::create("[MLR] Current song name copied!", NotificationIcon::None, Mod::get()->getSettingValue<double>("notificationTime"));
	}

	void addBlacklistButton() {
		auto menu = getChildByID("right-side-menu");

		auto btn = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::create(CCSprite::create("blacklist-btn-sprite.png"_spr)),
			this,
			menu_selector(MenuLayerMLHook::onBlacklistButton)
		);
		btn->setID("blacklist-button"_spr);

		menu->addChild(btn);
		menu->updateLayout();
	}

	void onBlacklistButton(CCObject*) {
		if (m_fields->songManager.isOriginalMenuLoop()) return;
		auto currentSong = m_fields->songManager.getCurrentSong();
		const bool useCustomSongs = Utils::getBool("useCustomSongs");
		log::info("blacklisting: {}", currentSong);
		std::string toWriteToFile = currentSong;
		std::string songName = Utils::getSongName();
		std::string songArtist = Utils::getSongArtist();
		int songID = Utils::getSongID();
		if (!useCustomSongs) toWriteToFile = toWriteToFile.append(fmt::format(" # Song: {} by {}", songName, songArtist));
		auto test = utils::file::readString(m_fields->blacklistFile);
		if (test.isErr()) return log::info("error reading blacklist file!");
		auto result = geode::utils::file::writeString(m_fields->blacklistFile, fmt::format("{}\n{}", test.unwrap(), toWriteToFile));
		if (result.isErr()) return log::info("error blacklisting song {}", currentSong);
		if (!useCustomSongs) Utils::makeNewCard(fmt::format("Blacklisted {} by {} ({})!", songName, songArtist, songID));
		else Utils::makeNewCard(fmt::format("Blacklisted {})!", currentSong));
		m_fields->songManager.clearSongs();
		Utils::populateVector(useCustomSongs);
		if (!Utils::getBool("playlistMode")) Utils::setNewSong();
		else Utils::playlistModeNewSong();
	}
};