#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/GJBaseGameLayer.hpp>

class $modify(MenuLoopGJBGLHook, GJBaseGameLayer) {
	struct Fields {
		~Fields() {
			SongManager::get().setPauseSongPositionTracking(true);
		}
	};
	bool init() {
		SongManager& songManager = SongManager::get();
		songManager.setPauseSongPositionTracking(true);
		if (!GJBaseGameLayer::init()) {
			songManager.setPauseSongPositionTracking(false);
			return false;
		}
		songManager.setShouldRestoreMenuLoopPoint(false);
		if (!Utils::getBool("playlistMode")) return true;
		const auto fmod = FMODAudioEngine::get();
		if (!fmod) return true;
		fmod->m_backgroundMusicChannel->stop();
		return true;
	}
};