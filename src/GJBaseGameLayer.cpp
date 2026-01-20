#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/GJBaseGameLayer.hpp>

class $modify(MenuLoopGJBGLHook, GJBaseGameLayer) {
	struct Fields {
		~Fields() {
			SongManager::get().setComingFromGJBGL(false);
		}
	};
	bool init() {
		SongManager::get().setComingFromGJBGL(false);
		if (!GJBaseGameLayer::init()) return false;
		SongManager::get().setShouldRestoreMenuLoopPoint(false);
		SongManager::get().setComingFromGJBGL(true);
		if (!Utils::getBool("playlistMode")) return true;
		const auto fmod = FMODAudioEngine::get();
		if (!fmod) return true;
		fmod->m_backgroundMusicChannel->stop();
		return true;
	}
};