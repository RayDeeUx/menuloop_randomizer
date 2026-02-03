#pragma once

#include "SongManager.hpp"

#define CAN_USE_PLAYBACK_CONTROLS (songManager.getFinishedCalculatingSongLengths() && songManager.getUndefined0Alk1m123TouchPrio())
#define INCREMENT_DECREMENT_AMOUNT (songManager.getIncrementDecrementByMilliseconds() * (cocos2d::CCKeyboardDispatcher::get()->getShiftKeyPressed() ? (GEODE_MACOS(cocos2d::CCKeyboardDispatcher::get()->getCommandKeyPressed()) GEODE_WINDOWS(cocos2d::CCKeyboardDispatcher::get()->getControlKeyPressed()) ? 3 : 2) : 1))

namespace SongControl {
	void woahThereBuddy(const std::string& reason);
	void previousSong(SongManager& songManager = SongManager::get());
	void holdSong(SongManager& songManager = SongManager::get());
	void favoriteSong(SongManager& songManager = SongManager::get());
	void blacklistSong(SongManager& songManager = SongManager::get());
	void copySong();
	void regenSong();
	void shuffleSong(const SongManager& songManager = SongManager::get());
	void addSongToPlaylist(const std::string& songPath = SongManager::get().getCurrentSong());
	void skipBackward();
	void skipForward();
	void setSongPercentage(const int percentage);
}