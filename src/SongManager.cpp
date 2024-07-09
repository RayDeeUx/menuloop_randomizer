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
		m_currentSong = m_songs[Utils::randomIndex(m_songs.size())];
	} else {
		m_currentSong = "";
	}
}

std::string SongManager::getCurrentSong() {
	return m_currentSong;
}