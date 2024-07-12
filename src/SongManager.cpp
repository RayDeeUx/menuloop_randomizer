#include "SongManager.hpp"
#include "Utils.hpp"

SongManager::SongManager() {}

void SongManager::addSong(std::string path) {
	m_songs.push_back(path);
}

void SongManager::clearSongs() {
	if (m_songs.size() >= 1) {
		m_songs.clear();
	}
}

void SongManager::pickRandomSong() {
	if (m_songs.size() >= 1) {
		m_isMenuLoop = false;
		m_currentSong = m_songs[Utils::randomIndex(m_songs.size())];
	} else {
		m_isMenuLoop = true;
		m_currentSong = "menuLoop.mp3";
	}
}

std::string SongManager::getCurrentSong() {
	return m_currentSong;
}

bool SongManager::isOriginalMenuLoop() {
	return m_isMenuLoop;
}