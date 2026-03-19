#pragma once

#include "SongManager.hpp"

#define CAN_USE_PLAYBACK_CONTROLS (songManager.getFinishedCalculatingSongLengths() && songManager.getUndefined0Alk1m123TouchPrio())
#define INCREMENT_DECREMENT_AMOUNT (songManager.getIncrementDecrementByMilliseconds() * GEODE_MOBILE(1) GEODE_DESKTOP((cocos2d::CCKeyboardDispatcher::get()->getShiftKeyPressed() ? (GEODE_MACOS(cocos2d::CCKeyboardDispatcher::get()->getControlKeyPressed()) GEODE_WINDOWS(cocos2d::CCKeyboardDispatcher::get()->getAltKeyPressed()) ? 3 : 2) : 1)))

namespace SongControl {
	void woahThereBuddy(const std::string& reason);
	void previousSong();
	void holdSong();
	void favoriteSong();
	void blacklistSong();
	void copySong();
	void regenSong();
	void shuffleSong();
	void addSongToPlaylist(const std::string& songPath = SongManager::get().getCurrentSong());
	void skipBackward();
	void skipForward();
	void setSongPercentage(const int percentage);
}