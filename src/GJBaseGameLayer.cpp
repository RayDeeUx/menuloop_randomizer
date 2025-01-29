#include "Utils.hpp"
#include <Geode/modify/GJBaseGameLayer.hpp>

class $modify(MenuLoopGJBGLHook, GJBaseGameLayer) {
	bool init() {
		if (!GJBaseGameLayer::init()) return false;
		if (!Utils::getBool("playlistMode")) return true;
		const auto fmod = FMODAudioEngine::get();
		if (!fmod) return true;
		fmod->m_backgroundMusicChannel->stop();
		return true;
	}
};