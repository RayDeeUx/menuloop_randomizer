#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLayerMLHook, MenuLayer) {
	struct Fields {
		SongManager &songManager = SongManager::get();
		std::filesystem::path configDir = Mod::get()->getConfigDir();
		std::filesystem::path blacklistFile = configDir / R"(blacklist.txt)";
	};

	void woahThereBuddy(const std::string& reason) const {
		geode::createQuickPopup(
			"Menu Loop Randomizer", reason,
			"Never Mind", "Open Mod Settings",
			[this](FLAlertLayer*, bool openConfig) {
				if (!openConfig) return;
				openSettingsPopup(Mod::get());
			}
		);
	}

	bool init() {
		if (!MenuLayer::init())
			return false;

		Utils::removeCard();

		if (Loader::get()->isModLoaded("omgrod.geodify"))
			SongManager::get().setGeodify(Loader::get()->getLoadedMod("omgrod.geodify")->getSettingValue<bool>("menu-loop"));

		if (Utils::getBool("enableNotification"))
			Utils::newCardFromCurrentSong();

		if (Utils::getBool("enableShuffleButton"))
			MenuLayerMLHook::addShuffleButton();

		if (Utils::getBool("enableNotification") && Utils::getBool("enableNewNotification"))
			MenuLayerMLHook::addRegenButton();

		if (Utils::getBool("enableCopySongID"))
			MenuLayerMLHook::addCopyButton();

		if (Utils::getBool("enableBlacklistButton"))
			MenuLayerMLHook::addBlacklistButton();

		if (Utils::getBool("enableHoldSongButton"))
			MenuLayerMLHook::addHoldSongButton();

		if (Utils::getBool("enablePreviousButton"))
			MenuLayerMLHook::addPreviousButton();

		return true;
	}

	void addShuffleButton() {
		CCNode* menu = getChildByID("right-side-menu");

		CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(
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
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;

		if (m_fields->songManager.isOriginalMenuLoop()) Utils::populateVector(Utils::getBool("useCustomSongs"));

		Utils::setNewSong();

		if (Utils::getBool("enableNotification"))
			Utils::newCardFromCurrentSong();
	}

	void addRegenButton() {
		CCNode* menu = getChildByID("right-side-menu");

		CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(
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
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;

		if (Utils::getBool("enableNotification"))
			Utils::newCardFromCurrentSong();
	}

	void addCopyButton() {
		CCNode* menu = getChildByID("right-side-menu");

		CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::create(CCSprite::create("copy-btn-sprite.png"_spr)),
			this,
			menu_selector(MenuLayerMLHook::onCopyButton)
		);
		btn->setID("copy-button"_spr);

		menu->addChild(btn);
		menu->updateLayout();
	}

	void onCopyButton(CCObject*) {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		Utils::copyCurrentSongName();
		Notification::create("[MLR] Current song name copied!", NotificationIcon::None, Mod::get()->getSettingValue<double>("notificationTime"));
	}

	void addBlacklistButton() {
		CCNode* menu = getChildByID("right-side-menu");

		CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::create(CCSprite::create("blacklist-btn-sprite.png"_spr)),
			this,
			menu_selector(MenuLayerMLHook::onBlacklistButton)
		);
		btn->setID("blacklist-button"_spr);

		menu->addChild(btn);
		menu->updateLayout();
	}

	void onBlacklistButton(CCObject*) {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		SongManager& songManager = m_fields->songManager;
		if (songManager.isOriginalMenuLoop()) return MenuLayerMLHook::woahThereBuddy("There's nothing to blacklist! Open Menu Loop Randomizer's config directory and edit its <cj>blacklist.txt</c> file to bring back some songs.");
		if (songManager.isOverride()) return MenuLayerMLHook::woahThereBuddy("You're trying to blacklist your own <cy>override</c>. Double-check your settings again.");
		auto currentSong = songManager.getCurrentSong();
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
		songManager.addToBlacklist();
		songManager.clearSongs();
		Utils::populateVector(useCustomSongs);
		if (!Utils::getBool("playlistMode")) Utils::setNewSong();
		else Utils::playlistModeNewSong();
		if (!Utils::getBool("enableNotification")) return;
		if (songManager.isOriginalMenuLoop()) {
			if (useCustomSongs && !customSong.empty()) return Utils::newNotification(fmt::format("Blacklisted {}. Have fun with the original menu loop. :)", customSong));
			if (!useCustomSongs && !songName.empty()) return Utils::newNotification(fmt::format("Blacklisted {}. Have fun with the original menu loop. :)", songName));
		}
		if (!useCustomSongs) return Utils::newNotification(fmt::format("Blacklisted {} by {} ({}), now playing {}.", songName, songArtist, songID, Utils::getSongName()));
		if (!customSong.empty()) return Utils::newNotification(fmt::format("Blacklisted {}, now playing {}.", customSong, Utils::currentCustomSong()));
	}

	void addHoldSongButton() {
		CCNode* menu = getChildByID("right-side-menu");

		CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::create(CCSprite::create("hold-btn-sprite.png"_spr)),
			this,
			menu_selector(MenuLayerMLHook::onHoldSongButton)
		);
		btn->setID("hold-song-button"_spr);

		menu->addChild(btn);
		menu->updateLayout();
	}

	void onHoldSongButton(CCObject*) {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		SongManager& songManager = m_fields->songManager;
		if (songManager.isOverride()) return MenuLayerMLHook::woahThereBuddy("You're currently playing a menu loop <cy>override</c>. Double-check your settings again.");
		if (songManager.songSizeIsBad()) return MenuLayerMLHook::woahThereBuddy("You don't have enough songs available to do this. Visit the config directory through the mod settings and try again.");
		const std::string& formerHeldSong = songManager.getHeldSong();
		if (songManager.getCurrentSong() == formerHeldSong) {
			if (Utils::getBool("enableNotification")) return Utils::newNotification("You're already holding that song! :D");
			return FLAlertLayer::create("Menu Loop Randomizer", "You're already holding that song! <cl>:D</c>", "Close")->show();
		}
		songManager.setHeldSong(songManager.getCurrentSong());
		if (!formerHeldSong.empty()) {
			FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
			songManager.setCurrentSong(formerHeldSong);
			if (Utils::getBool("playlistMode")) FMODAudioEngine::get()->playMusic(songManager.getCurrentSong(), true, 1.0f, 1);
			else GameManager::sharedState()->playMenuMusic();
			if (!Utils::getBool("enableNotification")) return;
			return Utils::newCardFromCurrentSong();
		}
		if (!Utils::getBool("playlistMode")) Utils::setNewSong();
		else Utils::playlistModeNewSong();
		if (Utils::getBool("enableNotification")) Utils::newCardFromCurrentSong();
	}

	void addPreviousButton() {
		CCNode* menu = getChildByID("right-side-menu");

		CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(
			CircleButtonSprite::create(CCSprite::create("prev-btn-sprite.png"_spr)),
			this,
			menu_selector(MenuLayerMLHook::onPreviousButton)
		);
		btn->setID("hold-song-button"_spr);

		menu->addChild(btn);
		menu->updateLayout();
	}

	void onPreviousButton(CCObject*) {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		SongManager& songManager = m_fields->songManager;
		if (songManager.isOverride()) return MenuLayerMLHook::woahThereBuddy("You're currently playing a menu loop <cy>override</c>. Double-check your settings again.");
		if (songManager.songSizeIsBad()) return MenuLayerMLHook::woahThereBuddy("You don't have enough songs available to do this. Visit the config directory through the mod settings and try again.");
		if (songManager.isPreviousSong()) {
			if (Utils::getBool("enableNotification")) return Utils::newNotification("You're already playing the previous song! :)");
			return FLAlertLayer::create("Menu Loop Randomizer", "You're already playing the previous song! <cl>:)</c>", "Close")->show();
		}
		const std::string& previousSong = songManager.getPreviousSong();
		if (previousSong.empty()) {
			if (Utils::getBool("enableNotification")) return Utils::newNotification("There's no previous song to go back to! :(");
			return FLAlertLayer::create("Menu Loop Randomizer", "There's no previous song to go back to! <cl>:(</c>", "Close")->show();
		}
		FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
		songManager.setCurrentSong(previousSong);
		if (Utils::getBool("playlistMode")) FMODAudioEngine::get()->playMusic(songManager.getCurrentSong(), true, 1.0f, 1);
		else GameManager::sharedState()->playMenuMusic();
		if (Utils::getBool("enableNotification")) return Utils::newCardFromCurrentSong();
	}
};