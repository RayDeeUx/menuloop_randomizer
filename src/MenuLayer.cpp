#include "SongManager.hpp"
#include "Utils.hpp"
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
		std::string customSong = Utils::currentCustomSong();
		if (!std::filesystem::exists(m_fields->blacklistFile)) return log::info("error finding blacklist file!");
		if (!useCustomSongs) toWriteToFile = toWriteToFile.append(fmt::format(" # [MLR] Song: {} by {} [MLR] #", songName, songArtist));
		std::ofstream blacklistFileOutput;
		blacklistFileOutput.open(m_fields->blacklistFile, std::ios_base::app);
		blacklistFileOutput << std::endl << toWriteToFile;
		blacklistFileOutput.close();
		m_fields->songManager.addToBlacklist();
		m_fields->songManager.clearSongs();
		Utils::populateVector(useCustomSongs);
		if (!Utils::getBool("playlistMode")) Utils::setNewSong();
		else Utils::playlistModeNewSong();
		if (!Utils::getBool("enableNotification")) return;
		if (m_fields->songManager.isOriginalMenuLoop()) {
			if (useCustomSongs && !customSong.empty()) return Utils::makeNewCard(fmt::format("Blacklisted {}. Have fun with the original menu loop. :)", customSong));
			if (!useCustomSongs && !songName.empty()) return Utils::makeNewCard(fmt::format("Blacklisted {}. Have fun with the original menu loop. :)", songName));
		}
		if (!useCustomSongs) return Utils::makeNewCard(fmt::format("Blacklisted {} by {} ({}), now playing {}.", songName, songArtist, songID, Utils::getSongName()));
		if (!customSong.empty()) return Utils::makeNewCard(fmt::format("Blacklisted {}, now playing {}.", customSong, Utils::currentCustomSong()));
	}
};