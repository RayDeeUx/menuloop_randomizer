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
		std::string currentSong = m_fields->songManager.getCurrentSong();
		log::debug("Game close detected. Most recent songManager song: {}", currentSong);
		Mod::get()->setSavedValue<std::string>("lastMenuLoop", currentSong);

		GameManager::encodeDataTo(p0);
	}
};