#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLoopPLHook, PlayLayer) {
	void onQuit() {
		SongManager::get().setComingFromGJBGL(true);
		if (Utils::getBool("randomizeWhenExitingLevel")) SongManager::get().pickRandomSong();
		else SongManager::get().setShouldRestoreMenuLoopPoint(!Utils::getBool("randomizeWhenExitingLevel") && Utils::getBool("restoreWhenExitingLevel"));

		PlayLayer::onQuit();
		Utils::removeCardRemotely(Utils::findCardRemotely());
	}
};