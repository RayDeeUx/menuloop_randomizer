#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/loader/Log.hpp>

SongManager::SongManager() {}

void SongManager::addSong(std::string path) {
	m_songs.push_back(path);
}

void SongManager::clearSongs() {
	if (!m_songs.empty()) {
		m_songs.clear();
	}
}

void SongManager::pickRandomSong() {
	if (!m_songs.empty()) {
		m_isMenuLoop = false;
		if (m_songs.size() != 1) {
			auto randomIndex = Utils::randomIndex(m_songs.size());
			while (m_songs[randomIndex] == m_currentSong) {
				geode::log::debug("avoiding shuffling into the same song at index {}", randomIndex);
				randomIndex = Utils::randomIndex(m_songs.size());
			}
			m_currentSong = m_songs[randomIndex];
		} else m_currentSong = m_songs[0];
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