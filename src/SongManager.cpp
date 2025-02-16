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
	if (!getSpecificSongOverride().empty()) m_currentSong = getSpecificSongOverride();
	else if (!m_songs.empty()) {
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
	if (!getSpecificSongOverride().empty()) return getSpecificSongOverride();
	return m_currentSong;
}

void SongManager::setCurrentSong(const std::string& song) {
	if (!getSpecificSongOverride().empty()) m_currentSong = getSpecificSongOverride();
	else m_currentSong = song;
}

void SongManager::setCurrentSongToSavedSong() {
	if (m_isMenuLoop || !getSpecificSongOverride().empty()) return;
	m_currentSong = geode::Mod::get()->getSavedValue<std::string>("lastMenuLoop");
}

bool SongManager::isOriginalMenuLoop() const {
	return m_isMenuLoop;
}

void SongManager::setPlaylistMode() {
	m_playlistMode = Utils::getBool("playlistMode");
}

void SongManager::update(float dt) const {
	// split for readability
	if (GameManager::sharedState()->getGameVariable("0122") || !m_playlistMode) return;
	if (GJBaseGameLayer::get() || m_isMenuLoop || m_songs.size() < 2) return;
	auto fmod = FMODAudioEngine::get();
	if (!fmod) return;
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
	if (!getSpecificSongOverride().empty()) return;
	m_heldSong = value;
}

std::string SongManager::getHeldSong() {
	return m_heldSong;
}

void SongManager::addToBlacklist(const std::string& song) {
	if (!getSpecificSongOverride().empty()) return;
	m_blacklist.push_back(song);
}

void SongManager::addToBlacklist() {
	if (!getSpecificSongOverride().empty()) return;
	m_blacklist.push_back(m_currentSong);
}

std::vector<std::string> SongManager::getBlacklist() {
	return m_blacklist;
}

void SongManager::setOverride(const std::string_view path) {
	if (!Utils::goodExtension(path) && !path.empty()) return geode::log::info("invalid file offered for override song: {}", path);
	if (Utils::isSupportedFile(path)) {
		m_overrideSong = path;
		m_isOverride = true;
		return geode::log::info("set override to true and override path to: {}", path);
	}
	m_overrideSong = "";
	m_isOverride = false;
	geode::log::info("set override to false and override path to blank");
}

std::string SongManager::getSpecificSongOverride() {
	geode::log::info("fetching override song and seeing if it's valid");
	if (!Utils::isSupportedFile(m_overrideSong)) return "";
	return m_overrideSong;
}

bool SongManager::isOverride() const {
	return m_isOverride;
}

void SongManager::setCurrentSongToOverride() {
	geode::log::info("setting current song to override song if it exists");
	const std::string& override = getSpecificSongOverride();
	if (override.empty() || !Utils::isSupportedFile(override)) return;
	m_currentSong = override;
}