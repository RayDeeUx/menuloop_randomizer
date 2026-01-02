#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MenuLoopGMHook, GameManager) {
	static void onModify(auto& self) {
		(void) self.setHookPriority("GameManger::getMenuMusicFile", -3999);
		(void) self.setHookPriority("GameManger::playMenuMusic", -3999);
		(void) self.setHookPriority("GameManger::fadeInMenuMusic", -3999);
	}
	gd::string getMenuMusicFile() {
		return SongManager::get().getCurrentSong();
	}
	void playMenuMusic() {
		// manual reconstruction
		FMODAudioEngine* fmod = FMODAudioEngine::get();
		if (VANILLA_GD_MENU_LOOP_DISABLED) return fmod->stopAllMusic(true);
		if (!fmod->isMusicPlaying(GameManager::get()->getMenuMusicFile(), 0)) fmod->playMusic(GameManager::get()->getMenuMusicFile(), true, 0, 0);
	}
	void fadeInMenuMusic() {
		// code taken verbatim from colon. i've had two too many toddlers nagging me to add this feature to the mod now so here we are
		FMODAudioEngine* fmod = FMODAudioEngine::get();
		if (VANILLA_GD_MENU_LOOP_DISABLED) return fmod->stopAllMusic(true);

		SongManager& songManager = SongManager::get();

		// ensure compat with colon
		const geode::Mod* colon = songManager.getColonMenuLoopStartTime();
		if (colon && colon->getSettingValue<bool>("enable")) return GameManager::fadeInMenuMusic();

		// manual function reconstruction
		const auto oldTrack = fmod->getActiveMusic(0);
		if (!fmod->isMusicPlaying(GameManager::get()->getMenuMusicFile(), 0)) fmod->playMusic(GameManager::get()->getMenuMusicFile(), true, 1, 0);

		if (!songManager.getShouldRestoreMenuLoopPoint() || oldTrack == fmod->getActiveMusic(0)) return;
		songManager.restoreLastMenuLoopPosition();
		songManager.setShouldRestoreMenuLoopPoint(false);
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