#pragma once

// shorthand because haha yes
#define VANILLA_GD_MENU_LOOP_DISABLED GameManager::sharedState()->getGameVariable("0122")
#define CONFIG_DIR geode::Mod::get()->getConfigDir()
#define BLACKLIST_FILE CONFIG_DIR / R"(blacklist.txt)"
#define FAVORITES_FILE CONFIG_DIR / R"(favorites.txt)"

class SongManager {
  public:
	static SongManager& get() {
		static SongManager instance;
		return instance;
	}

	std::vector<std::string> getSongs();
	void addSong(const std::string&);
	void removeSong(const std::string& path);
	int getSongsSize() const;
	void clearSongs();
	void pickRandomSong();
	bool isOriginalMenuLoop() const;
	void setConstantShuffleMode();
	bool getConstantShuffleMode() const;
	void setPlaylistFileName();
	std::string getPlaylistFileName();
	std::string getCurrentSong();
	void setCurrentSong(const std::string&);
	void setCurrentSongToSavedSong();
	bool getCalledOnce() const;
	void setCalledOnce(const bool);
	bool getGeodify() const;
	void setGeodify(const bool);
	void setLavaChicken(const bool);
	bool getLavaChicken() const;
	void setPlaylistIsEmpty(const bool);
	bool getPlaylistIsEmpty() const;
	void setHeldSong(const std::string_view value);
	void resetHeldSong();
	std::string getHeldSong();
	void addToBlacklist(const std::string&);
	void addToBlacklist();
	std::vector<std::string> getBlacklist();
	void addToFavorites(const std::string& song);
	void addToFavorites();
	std::vector<std::string> getFavorites();
	void setOverride(const std::string_view);
	std::string getOverrideSong();
	bool isOverride() const;
	void setCurrentSongToOverride();
	void setPreviousSong(const std::string_view value);
	void resetPreviousSong();
	std::string getPreviousSong();
	bool isPreviousSong() const;
	bool songSizeIsBad() const;
	void setCurrentSongDisplayName(const std::string& displayName);
	std::string getCurrentSongDisplayName();
	void incrementTowerRepeatCount();
	void resetTowerRepeatCount();
	int getTowerRepeatCount() const;
	void setSawbladeCustomSongsFolder(const bool value);
	bool getSawbladeCustomSongsFolder() const;
	void setAdvancedLogs(const bool value);
	bool getAdvancedLogs() const;

  private:
	SongManager();
	SongManager(const SongManager &) = delete;
	std::vector<std::string> m_songs;
	std::string m_currentSong = "";
	std::string m_heldSong = "";
	std::string m_previousSong = "";
	std::string m_overrideSong = "";
	std::string m_displayName = "";
	std::string m_playlistName = "";
	bool m_isMenuLoop;
	bool m_constantShuffleMode;
	bool m_calledOnce = false;
	bool m_geodify = false;
	bool m_isOverride = false;
	bool m_isLavaChicken = false;
	bool m_playlistIsEmpty = false;
	bool m_sawbladeCustomSongsFolder = false;
	bool m_advancedLogs = false;
	std::vector<std::string> m_blacklist;
	std::vector<std::string> m_favorites;
	int m_towerRepeatCount;
};