#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MenuLoopGMHook, GameManager) {
	gd::string getMenuMusicFile() {
		return SongManager::get().getCurrentSong();
	}
	void fadeInMenuMusic() {
		// code taken verbatim from colon. i've had two too many toddlers nagging me to add this feature to the mod now so here we are
		FMODAudioEngine* fmod = FMODAudioEngine::get();
		SongManager& songManager = SongManager::get();
		const auto oldTrack = fmod->getActiveMusic(0);
		GameManager::fadeInMenuMusic();
		// ensure compat with colon
		const geode::Mod* colon = songManager.getColonMenuLoopStartTime();
		if ((colon && colon->getSettingValue<bool>("enable")) || !songManager.getShouldRestoreMenuLoopPoint()) {
			songManager.setPauseSongPositionTracking(false);
			return;
		}
		// NOTE: THIS LINE CAN CAUSE ISSUES WHEN MENU LOOP AND LEVEL'S MOST RECENTLY PLAYED SONG ARE THE SAME
		if (oldTrack == fmod->getActiveMusic(0) && !songManager.getPauseSongPositionTracking()) {
			songManager.setPauseSongPositionTracking(false);
			return;
		}
		songManager.restoreLastMenuLoopPosition();
		songManager.setShouldRestoreMenuLoopPoint(false);
		songManager.setPauseSongPositionTracking(false);
	}
	void encodeDataTo(DS_Dictionary* p0) {
		Utils::rememberLastMenuLoop();
		GameManager::encodeDataTo(p0);
	}
};