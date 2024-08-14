#include "SongManager.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(MenuLoopGMHook, GameManager) {
    struct Fields {
        SongManager &songManager = SongManager::get();
    };
    gd::string getMenuMusicFile() {
        return m_fields->songManager.getCurrentSong();
    }
};