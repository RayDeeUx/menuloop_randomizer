#include "SongControl.hpp"
#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLayerMLHook, MenuLayer) {

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

		if (Utils::getBool("enableFavoriteButton"))
			MenuLayerMLHook::addFavoriteButton();

		if (Utils::getBool("enableHoldSongButton"))
			MenuLayerMLHook::addHoldSongButton();

		if (Utils::getBool("enablePreviousButton"))
			MenuLayerMLHook::addPreviousButton();

		return true;
	}

	void addShuffleButton() {
		Utils::addButton("shuffle", menu_selector(MenuLayerMLHook::onShuffleButton), static_cast<cocos2d::CCMenu*>(this->getChildByID("right-side-menu")), this);
	}

	void onShuffleButton(CCObject*) {
		SongControl::shuffleSong();
	}

	void addRegenButton() {
		Utils::addButton("regen", menu_selector(MenuLayerMLHook::onRegenButton), static_cast<cocos2d::CCMenu*>(this->getChildByID("right-side-menu")), this);
	}

	void onRegenButton(CCObject*) {
		SongControl::regenSong();
	}

	void addCopyButton() {
		Utils::addButton("copy", menu_selector(MenuLayerMLHook::onCopyButton), static_cast<cocos2d::CCMenu*>(this->getChildByID("right-side-menu")), this);
	}

	void onCopyButton(CCObject*) {
		SongControl::copySong();
	}

	void addBlacklistButton() {
		Utils::addButton("blacklist", menu_selector(MenuLayerMLHook::onBlacklistButton), static_cast<cocos2d::CCMenu*>(this->getChildByID("right-side-menu")), this);
	}

	void onBlacklistButton(CCObject*) {
		SongControl::blacklistSong();
	}

	void addFavoriteButton() {
		Utils::addButton("favorite", menu_selector(MenuLayerMLHook::onFavoriteButton), static_cast<cocos2d::CCMenu*>(this->getChildByID("right-side-menu")), this);
	}

	void onFavoriteButton(CCObject*) {
		SongControl::favoriteSong();
	}

	void addHoldSongButton() {
		Utils::addButton("hold", menu_selector(MenuLayerMLHook::onHoldSongButton), static_cast<cocos2d::CCMenu*>(this->getChildByID("right-side-menu")), this);
	}

	void onHoldSongButton(CCObject*) {
		SongControl::holdSong();
	}

	void addPreviousButton() {
		Utils::addButton("prev", menu_selector(MenuLayerMLHook::onHoldSongButton), static_cast<cocos2d::CCMenu*>(this->getChildByID("right-side-menu")), this);
	}

	void onPreviousButton(CCObject*) {
		SongControl::previousSong();
	}
};