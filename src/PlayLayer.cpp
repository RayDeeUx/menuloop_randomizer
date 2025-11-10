#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLoopPLHook, PlayLayer) {
	void onQuit() {
		if (Utils::getBool("randomizeWhenExitingLevel")) SongManager::get().pickRandomSong();

		PlayLayer::onQuit();
		Utils::removeCardRemotely();

		// if (Utils::getBool("playlistMode")) {
		// 	Utils::constantShuffleModePLAndEPL();
		// 	Utils::constantShuffleModeNewSong();
		// }
	}
};