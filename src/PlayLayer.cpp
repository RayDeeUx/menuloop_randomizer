#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLoopPLHook, PlayLayer) {
	void onQuit() {
		if (Utils::getBool("randomizeWhenExitingLevel")) SongManager::get().pickRandomSong();
		else SongManager::get().setShouldRestoreMenuLoopPoint(!Utils::getBool("randomizeWhenExitingLevel") && Utils::getBool("restoreWhenExitingLevel"));

		PlayLayer::onQuit();
		Utils::removeCardRemotely(Utils::findCardRemotely());
	}
};