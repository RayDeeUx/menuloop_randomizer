#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MenuLoopGMHook, GameManager) {
	struct Fields {
		SongManager &songManager = SongManager::get();
	};
	gd::string getMenuMusicFile() {
		return m_fields->songManager.getCurrentSong();
	}
	void encodeDataTo(DS_Dictionary* p0) {
		SongManager& songManager = m_fields->songManager;
		std::string currentSong = songManager.getCurrentSong();
		log::info("Game close detected. Most recent songManager song: {}", currentSong);

		if (songManager.isOverride()) log::info("however this is an override, so do not save it");
		else Mod::get()->setSavedValue<std::string>("lastMenuLoop", currentSong);

		GameManager::encodeDataTo(p0);
	}
};