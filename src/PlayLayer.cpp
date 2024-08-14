#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLoopPLHook, PlayLayer) {
    struct Fields {
        SongManager &songManager = SongManager::get();
    };
    void onQuit() {
        if (Utils::getBool("randomizeWhenExitingLevel"))
            m_fields->songManager.pickRandomSong();

        PlayLayer::onQuit();
    }
};