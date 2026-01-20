#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/GJBaseGameLayer.hpp>

class $modify(MenuLoopGJBGLHook, GJBaseGameLayer) {
	struct Fields {
		~Fields() {
			SongManager::get().setPauseSongPositionTracking(false);
		}
	};
	bool init() {
		SongManager::get().setPauseSongPositionTracking(true);
		if (!GJBaseGameLayer::init()) {
			SongManager::get().setPauseSongPositionTracking(false);
			return false;
		}
		SongManager::get().setShouldRestoreMenuLoopPoint(false);
		if (!Utils::getBool("playlistMode")) return true;
		const auto fmod = FMODAudioEngine::get();
		if (!fmod) return true;
		fmod->m_backgroundMusicChannel->stop();
		return true;
	}
};