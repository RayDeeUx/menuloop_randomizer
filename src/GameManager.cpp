#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MenuLoopGMHook, GameManager) {
	gd::string getMenuMusicFile() {
		return SongManager::get().getCurrentSong();
	}
	void fadeInMenuMusic() {
		auto oldTrack = FMODAudioEngine::sharedEngine()->getActiveMusic(0);
		GameManager::fadeInMenuMusic();
		if (oldTrack != FMODAudioEngine::sharedEngine()->getActiveMusic(0)) {
			SongManager::get().restoreLastMenuLoopPosition();
		}
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