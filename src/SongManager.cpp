#include "SongManager.hpp"
#include "Utils.hpp"

SongManager::SongManager() {}

void SongManager::addSong(const std::string& path) {
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
				geode::log::info("avoiding shuffling into the same song at index {}", randomIndex);
				randomIndex = Utils::randomIndex(m_songs.size());
			}
			m_currentSong = m_songs[randomIndex];
			geode::log::info("new song: {}", m_currentSong);
		} else m_currentSong = m_songs[0];
	} else {
		m_isMenuLoop = true;
		m_currentSong = "menuLoop.mp3";
	}
}

std::string SongManager::getCurrentSong() {
	return m_currentSong;
}

void SongManager::setCurrentSong(const std::string& song) {
	m_currentSong = song;
}

void SongManager::setCurrentSongToSavedSong() {
	if (m_isMenuLoop) return;
	m_currentSong = geode::Mod::get()->getSavedValue<std::string>("lastMenuLoop");
}

bool SongManager::isOriginalMenuLoop() const {
	return m_isMenuLoop;
}

void SongManager::update(float dt) {
	if (GameManager::sharedState()->getGameVariable("0122")) return;
	auto fmod = FMODAudioEngine::get();
	if (!Utils::getBool("playlistMode") || GJBaseGameLayer::get() || m_isMenuLoop || m_songs.size() < 2 || !fmod) return;
	// geode::log::info("channelIsPlaying: {}", fmod->isMusicPlaying(0));
	#ifdef GEODE_IS_WINDOWS
	if (fmod->isMusicPlaying(0)) return;
	#else
	if (fmod->isMusicPlaying(1)) return;
	#endif
	geode::log::info("song is probably finished. Switching songs.");
	Utils::removeCard();
	Utils::playlistModeNewSong();
	if (Utils::getBool("enableNotification"))
		Utils::generateNotification();
}

void SongManager::setCalledOnce(const bool value) {
	m_calledOnce = value;
}

bool SongManager::getCalledOnce() const {
	return m_calledOnce;
}

void SongManager::setGeodify(const bool value) {
	m_geodify = value;
}

bool SongManager::getGeodify() const {
	return m_geodify;
}

void SongManager::setHeldSong(const std::string_view value) {
	m_heldSong = value;
}

std::string SongManager::getHeldSong() {
	return m_heldSong;
}

void SongManager::addToBlacklist(const std::string& song) {
	m_blacklist.push_back(song);
}

void SongManager::addToBlacklist() {
	m_blacklist.push_back(m_currentSong);
}

std::vector<std::string> SongManager::getBlacklist() {
	return m_blacklist;
}