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
		if (colon && colon->getSettingValue<bool>("enable")) return;
		if (!songManager.getShouldRestoreMenuLoopPoint()) return;
		if (oldTrack == fmod->getActiveMusic(0) && !songManager.getPauseSongPositionTracking()) return; // NOTE: THIS LINE CAN CAUSE ISSUES WHEN MENU LOOP AND LEVEL'S MOST RECENTLY PLAYED SONG ARE THE SAME
		songManager.restoreLastMenuLoopPosition();
		songManager.setShouldRestoreMenuLoopPoint(false);
		songManager.setPauseSongPositionTracking(false);
	}
	void encodeDataTo(DS_Dictionary* p0) {
		SongManager& songManager = SongManager::get();
		std::string currentSong = songManager.getCurrentSong();
		log::info("Game close detected. Most recent songManager song: {}", currentSong);

		if (songManager.isOverride()) log::info("however this is an override, so do not save it");
		else {
			Mod::get()->setSavedValue<std::string>("lastMenuLoop", currentSong);
			Mod::get()->setSavedValue<std::filesystem::path>("lastMenuLoopPath", Utils::toProblematicString(currentSong));
		}

		GameManager::encodeDataTo(p0);
	}
};