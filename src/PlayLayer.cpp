#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLoopPLHook, PlayLayer) {
	void onQuit() {
		SongManager& songManager = SongManager::get();
		songManager.setPauseSongPositionTracking(true);
		if (Utils::getBool("randomizeWhenExitingLevel")) {
			songManager.pickRandomSong();
			if (songManager.eclipseIntegrationSuccessful || songManager.qolModIntegrationSuccessful) Utils::queueUpdateFrontfacingLabelsInSCMAndSLL();
		} else {
			songManager.setShouldRestoreMenuLoopPoint(!Utils::getBool("randomizeWhenExitingLevel") && Utils::getBool("restoreWhenExitingLevel"));
		}

		PlayLayer::onQuit();
		Utils::removeCardRemotely(Utils::findCardRemotely());
	}
};