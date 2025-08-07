#include "SongManager.hpp"
#include "Utils.hpp"

SongManager::SongManager() {}

std::vector<std::string> SongManager::getSongs() {
	return m_songs;
}

void SongManager::addSong(const std::string& path) {
	m_songs.push_back(path);
}

void SongManager::removeSong(const std::string& path) {
	m_songs.erase(std::remove(m_songs.begin(), m_songs.end(), path));
}

int SongManager::getSongsSize() const {
	return m_songs.size();
}

void SongManager::clearSongs() {
	if (!m_songs.empty()) {
		m_songs.clear();
	}
}

void SongManager::pickRandomSong() {
	if (m_isOverride) {
		m_isMenuLoop = false;
		m_currentSong = m_overrideSong;
	} else if (!m_songs.empty()) {
		m_isMenuLoop = false;
		if (m_songs.size() != 1) {
			auto randomIndex = Utils::randomIndex(m_songs.size());
			geode::log::info("entering a while loop maybe");
			if (!std::filesystem::exists(Utils::toProblematicString(m_songs[randomIndex]))) {
				m_isMenuLoop = true;
				m_currentSong = "menuLoop.mp3";
				geode::Notification::create(
					fmt::format("Unable to find song at index {}! Check logs.", randomIndex),
					geode::NotificationIcon::Error, 10.f
				)->show();
				return geode::log::info("unable to find song {}!", m_songs[randomIndex]);
			}
			while (m_songs[randomIndex] == m_currentSong && std::ranges::find(m_favorites, m_songs[randomIndex]) == m_favorites.end()) {
				geode::log::info("avoiding shuffling into the same song at index {} because it is NOT a favorite song", randomIndex);
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
	if (!getOverrideSong().empty()) return getOverrideSong();
	return m_currentSong;
}

void SongManager::setCurrentSong(const std::string& song) {
	if (!getOverrideSong().empty()) m_currentSong = getOverrideSong();
	else m_currentSong = song;
}

void SongManager::setCurrentSongToSavedSong() {
	if (m_isMenuLoop || !getOverrideSong().empty()) return;
	const auto lastMenuLoop = geode::Mod::get()->getSavedValue<std::string>("lastMenuLoop");
	const auto lastMenuLoopPath = geode::Mod::get()->getSavedValue<std::filesystem::path>("lastMenuLoopPath");
	if (std::filesystem::exists(Utils::toProblematicString(lastMenuLoop))) m_currentSong = lastMenuLoop;
	else if (std::filesystem::exists(lastMenuLoopPath)) m_currentSong = lastMenuLoopPath;
}

bool SongManager::isOriginalMenuLoop() const {
	return m_isMenuLoop;
}

void SongManager::setConstantShuffleMode() {
	m_constantShuffleMode = Utils::getBool("playlistMode");
}

bool SongManager::getConstantShuffleMode() const {
	return m_constantShuffleMode;
}

void SongManager::setPlaylistFileName() {
	m_playlistName = geode::utils::string::replace(Utils::toNormalizedString(geode::Mod::get()->getSettingValue<std::filesystem::path>("playlistFile").filename()), ".txt", "");
}

std::string SongManager::getPlaylistFileName() {
	return m_playlistName;
}

void SongManager::update(float dt) const {
	// split for readability
	if (VANILLA_GD_MENU_LOOP_DISABLED || !m_constantShuffleMode) return;
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
	Utils::constantShuffleModeNewSong();
	Utils::newCardAndDisplayNameFromCurrentSong();
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

void SongManager::setLavaChicken(const bool value) {
	m_isLavaChicken = value;
}

bool SongManager::getLavaChicken() const {
	return m_isLavaChicken;
}

void SongManager::setPlaylistIsEmpty(const bool value) {
	m_playlistIsEmpty = value;
}

bool SongManager::getPlaylistIsEmpty() const {
	return m_playlistIsEmpty;
}

void SongManager::setHeldSong(const std::string_view value) {
	if (!getOverrideSong().empty()) return;
	m_heldSong = value;
}

void SongManager::resetHeldSong() {
	m_heldSong = "";
}

std::string SongManager::getHeldSong() {
	return m_heldSong;
}

void SongManager::addToBlacklist(const std::string& song) {
	if (!getOverrideSong().empty()) return;
	if (std::ranges::find(m_favorites, song) != m_favorites.end()) return geode::log::info("tried to blacklist a favorited song: {}", song);
	m_blacklist.push_back(song);
}

void SongManager::addToBlacklist() {
	if (!getOverrideSong().empty()) return;
	if (std::ranges::find(m_favorites, m_currentSong) != m_favorites.end()) return geode::log::info("tried to blacklist a favorited song: {}", m_currentSong);
	m_blacklist.push_back(m_currentSong);
}

std::vector<std::string> SongManager::getBlacklist() {
	return m_blacklist;
}

void SongManager::addToFavorites(const std::string& song) {
	if (!getOverrideSong().empty()) return;
	if (std::ranges::find(m_blacklist, song) != m_blacklist.end()) return geode::log::info("tried to favorite a blacklisted song: {}", song);
	m_favorites.push_back(song);
}

void SongManager::addToFavorites() {
	if (!getOverrideSong().empty()) return;
	if (std::ranges::find(m_blacklist, m_currentSong) != m_blacklist.end()) return geode::log::info("tried to favorite a blacklisted song: {}", m_currentSong);
	m_favorites.push_back(m_currentSong);
}

std::vector<std::string> SongManager::getFavorites() {
	return m_favorites;
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

std::string SongManager::getOverrideSong() {
	if (!Utils::isSupportedFile(m_overrideSong)) return "";
	return m_overrideSong;
}

bool SongManager::isOverride() const {
	return m_isOverride;
}

void SongManager::setCurrentSongToOverride() {
	geode::log::info("setting current song to override song if it exists");
	const std::string& override = getOverrideSong();
	if (override.empty() || !Utils::isSupportedFile(override)) return geode::log::info("override is not valid");
	m_currentSong = override;
}

void SongManager::setPreviousSong(const std::string_view value) {
	if (!Utils::isSupportedFile(value)) return geode::log::info("previous song is not valid");
	m_previousSong = value;
}

void SongManager::resetPreviousSong() {
	m_previousSong = "";
}

std::string SongManager::getPreviousSong() {
	return m_previousSong;
}

bool SongManager::isPreviousSong() const {
	return m_currentSong == m_previousSong;
}

bool SongManager::songSizeIsBad() const {
	return m_songs.empty() || m_songs.size() < 2;
}

void SongManager::setCurrentSongDisplayName(const std::string& displayName) {
	// set from Utils::newCardAndDisplayNameFromCurrentSong
	m_displayName = displayName;
}

std::string SongManager::getCurrentSongDisplayName() {
	return m_displayName;
}

void SongManager::incrementTowerRepeatCount() {
	m_towerRepeatCount = m_towerRepeatCount + 1;
}

void SongManager::resetTowerRepeatCount() {
	m_towerRepeatCount = 0;
}

int SongManager::getTowerRepeatCount() const {
	return m_towerRepeatCount;
}

void SongManager::setSawbladeCustomSongsFolder(const bool value) {
	m_sawbladeCustomSongsFolder = value;
}

bool SongManager::getSawbladeCustomSongsFolder() const {
	return m_sawbladeCustomSongsFolder;
}