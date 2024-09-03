#include "SongManager.hpp"
#include "Utils.hpp"
#include "ui/PlayingCard.hpp"
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLayerMLHook, MenuLayer) {
	struct Fields {
		SongManager &songManager = SongManager::get();
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

		if (Utils::getBool("playlistMode")) Utils::playlistModeNewSong(true);
		else Utils::setNewSong();

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
};
