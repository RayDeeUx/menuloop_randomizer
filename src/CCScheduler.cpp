#include "Utils.hpp"
#include "SongManager.hpp"
#include <Geode/modify/CCScheduler.hpp>

using namespace geode::prelude;

class $modify(MenuLoopCCSchedulerHook, CCScheduler) {
	void update(float dt) {
		CCScheduler::update(dt);
		if (!Utils::getBool("playlistMode")) { return; }
		auto menuLayer = GameManager::get()->m_menuLayer;
		if (!menuLayer) { return; }
		/*
		if GameManager::get()->m_menuLayer is not included in the conditional statement
		then music will be played at full blast before being set to user's preferred volume.
		i don't think anyone wants that.
		-- raydeeux
		*/
		SongManager::get().update(dt);
	}
};