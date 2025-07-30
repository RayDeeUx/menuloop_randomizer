#pragma once

#include "SongManager.hpp"

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
}