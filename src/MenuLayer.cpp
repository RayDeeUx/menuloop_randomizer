#include "ui/SongControlMenu.hpp"
#include "SongControl.hpp"
#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/MenuLayer.hpp>

#define REST_OF_THE_OWL static_cast<cocos2d::CCMenu*>(this->getChildByID("right-side-menu")), this

using namespace geode::prelude;

class $modify(MenuLayerMLHook, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		Utils::removeCard();

		if (Loader::get()->isModLoaded("omgrod.geodify"))
			SongManager::get().setGeodify(Loader::get()->getLoadedMod("omgrod.geodify")->getSettingValue<bool>("menu-loop"));

		const bool noClassic = Utils::getString("buttonMode") != "Classic";

		if (Utils::getBool("enableNotification")) Utils::newCardFromCurrentSong();

		// move to front of the menu (for OCM) first
		if (noClassic)
			Utils::addButton("controls", menu_selector(MenuLayerMLHook::onControlsButton), REST_OF_THE_OWL);
		if (Utils::getBool("enableShuffleButton")) Utils::addButton("shuffle", menu_selector(MenuLayerMLHook::onShuffleButton), REST_OF_THE_OWL);
		if (Utils::getBool("enableNotification") && Utils::getBool("enableNewNotification"))
			Utils::addButton("regen", menu_selector(MenuLayerMLHook::onRegenButton), REST_OF_THE_OWL);
		if (Utils::getBool("enableCopySongID")) Utils::addButton("copy", menu_selector(MenuLayerMLHook::onCopyButton), REST_OF_THE_OWL);

		if (noClassic) return true;

		if (Utils::getBool("enableBlacklistButton")) Utils::addButton("blacklist", menu_selector(MenuLayerMLHook::onBlacklistButton), REST_OF_THE_OWL);
		if (Utils::getBool("enableFavoriteButton")) Utils::addButton("favorite", menu_selector(MenuLayerMLHook::onFavoriteButton), REST_OF_THE_OWL);
		if (Utils::getBool("enableHoldSongButton")) Utils::addButton("hold", menu_selector(MenuLayerMLHook::onHoldSongButton), REST_OF_THE_OWL);
		if (Utils::getBool("enablePreviousButton")) Utils::addButton("prev", menu_selector(MenuLayerMLHook::onPreviousButton), REST_OF_THE_OWL);
		return true;
	}
	void onShuffleButton(CCObject*) { SongControl::shuffleSong(); }
	void onRegenButton(CCObject*) { SongControl::regenSong(); }
	void onCopyButton(CCObject*) { SongControl::copySong(); }
	void onBlacklistButton(CCObject*) { SongControl::blacklistSong(); }
	void onFavoriteButton(CCObject*) { SongControl::favoriteSong(); }
	void onHoldSongButton(CCObject*) { SongControl::holdSong(); }
	void onPreviousButton(CCObject*) { SongControl::previousSong(); }
	void onControlsButton(CCObject*) {
		if (Utils::getString("buttonMode") == "Classic") return;
		SongControlMenu::create(SongManager::get().getCurrentSong())->show();
	}
};