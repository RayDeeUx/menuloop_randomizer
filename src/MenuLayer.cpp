#include "ui/SongControlMenu.hpp"
#include "ui/SongListLayer.hpp"
#include "SongControl.hpp"
#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/MenuLayer.hpp>

#define REST_OF_THE_OWL static_cast<cocos2d::CCMenu*>(this->getChildByID("right-side-menu")), this

using namespace geode::prelude;

bool shownVibecodedWarning = false;

class $modify(MenuLayerMLHook, MenuLayer) {
	static cocos2d::CCScene* scene(bool p0) {
		CCScene* scene = MenuLayer::scene(p0);
		if (SongManager::get().getVibecodedVentilla() && !shownVibecodedWarning && scene->getChildByType<MenuLayer>(0)) {
			FLAlertLayer* alert = FLAlertLayer::create(scene->getChildByType<MenuLayer>(0), "Uh oh!", "<c_>Another mod overriding the menu loop is active!</c>\n<cy>The specific menu music mod you have active does not respect menu loops created from other mods, and overrides all mods that change the behavior of the in-game menu loop.</c> <co>Additionally, this specific menu music mod relies on an Internet connection, and does not work with Cloudflare WARP.</c> <cy>If you want to use Menu Loop Randomizer, please check your loaded mods.</c>\n\n<cg>You will only see this warning once.</c>", "I Understand", nullptr, 420.f);
			scene->addChild(alert);
			shownVibecodedWarning = true;
		}
		return scene;
	}
	bool init() {
		if (!MenuLayer::init()) return false;

		Utils::removeCard();

		SongManager& songManager = SongManager::get();
		const Loader* loader = Loader::get();

		if (loader->isModLoaded("omgrod.geodify")) songManager.setGeodify(loader->getLoadedMod("omgrod.geodify")->getSettingValue<bool>("menu-loop"));

		songManager.setSawbladeCustomSongsFolder(loader->isModLoaded("sawblade.custom_song_folder"));
		songManager.setUndefined0Alk1m123TouchPrio(loader->isModLoaded("alk.better-touch-prio"));
		if (loader->isModLoaded("colon.menu_loop_start_time")) songManager.setColonMenuLoopStartTime(loader->getLoadedMod("colon.menu_loop_start_time"));

		const std::string& buttonMode = Utils::getString("buttonMode");

		if (buttonMode == "Minimal") {
			Utils::addButton("controls", menu_selector(MenuLayerMLHook::onControlsButton), REST_OF_THE_OWL);
			return true;
		}

		const bool noClassic = buttonMode != "Classic";

		Utils::newCardAndDisplayNameFromCurrentSong();

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
		if (Utils::getBool("enableAddToPlaylistButton") && (!Utils::getBool("loadPlaylistFile") || SongManager::get().getPlaylistIsEmpty())) Utils::addButton("add", menu_selector(MenuLayerMLHook::onAddToPlylstBtn), REST_OF_THE_OWL);
		if (Utils::getBool("enableViewSongListButton")) Utils::addButton("playlist", menu_selector(MenuLayerMLHook::onSongListButton), REST_OF_THE_OWL);
		return true;
	}
	void onShuffleButton(CCObject*) { SongControl::shuffleSong(); }
	void onRegenButton(CCObject*) { SongControl::regenSong(); }
	void onCopyButton(CCObject*) { SongControl::copySong(); }
	void onBlacklistButton(CCObject*) { SongControl::blacklistSong(); }
	void onFavoriteButton(CCObject*) { SongControl::favoriteSong(); }
	void onHoldSongButton(CCObject*) { SongControl::holdSong(); }
	void onPreviousButton(CCObject*) { SongControl::previousSong(); }
	void onAddToPlylstBtn(CCObject*) { SongControl::addSongToPlaylist(SongManager::get().getCurrentSong()); }
	void onSongListButton(CCObject*) { SongListLayer::create("GJ_square05.png")->show(); }
	void onControlsButton(CCObject*) {
		if (Utils::getString("buttonMode") == "Classic") return;
		if (CCScene::get()->getChildByType<SongControlMenu>(0)) return;
		SongControlMenu::create("GJ_square02.png")->show();
	}
};