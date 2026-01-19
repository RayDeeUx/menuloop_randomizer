#pragma once

enum class SongType {
	Regular,
	Favorited,
	Blacklisted
};

struct SongData {
	std::string actualFilePath = "";
	std::string fileExtension = "";
	std::string fileName = "";
	std::string displayName = "";
	SongType type = SongType::Regular;
	unsigned int songLength = std::numeric_limits<unsigned int>::max();
	std::uintmax_t songFileSize = std::numeric_limits<std::uintmax_t>::max();
	std::filesystem::file_time_type songWriteTime {};
	bool isFromConfigOrAltDir = false;
	bool isEmpty = false;
};

typedef std::unordered_map<std::filesystem::path, SongData> SongToSongData;

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

	std::vector<std::string>& getSongs();
	void addSong(const std::string&);
	void removeSong(const std::string_view);
	int getSongsSize() const;
	void clearSongs();
	SongToSongData& getSongToSongDataEntries();
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
	void setPlaylistIsEmpty(const bool);
	bool getPlaylistIsEmpty() const;
	void setHeldSong(const std::string&);
	void resetHeldSong();
	std::string getHeldSong();
	void addToBlacklist(const std::string&);
	void addToBlacklist();
	std::vector<std::string>& getBlacklist();
	void addToFavorites(const std::string&);
	void addToFavorites();
	std::vector<std::string>& getFavorites();
	void setOverride(const std::string&);
	std::string getOverrideSong();
	bool isOverride() const;
	void setCurrentSongToOverride();
	void setPreviousSong(const std::string&);
	void resetPreviousSong();
	std::string getPreviousSong();
	bool isPreviousSong() const;
	bool songSizeIsBad() const;
	void setCurrentSongDisplayName(const std::string&);
	std::string getCurrentSongDisplayName();
	void incrementTowerRepeatCount();
	void resetTowerRepeatCount();
	int getTowerRepeatCount() const;
	void setSawbladeCustomSongsFolder(const bool);
	bool getSawbladeCustomSongsFolder() const;
	void setAdvancedLogs(const bool);
	bool getAdvancedLogs() const;
	void setLastMenuLoopPosition(const int);
	int getLastMenuLoopPosition() const;
	static void restoreLastMenuLoopPosition();
	void setShouldRestoreMenuLoopPoint(const bool);
	bool getShouldRestoreMenuLoopPoint() const;
	void setUndefined0Alk1m123TouchPrio(const bool);
	bool getUndefined0Alk1m123TouchPrio() const;
	void setColonMenuLoopStartTime(geode::Mod*);
	geode::Mod* getColonMenuLoopStartTime() const;
	void setVibecodedVentilla(const bool);
	bool getVibecodedVentilla() const;
	void setFinishedCalculatingSongLengths(const bool);
	bool getFinishedCalculatingSongLengths() const;

  private:
	SongManager();
	SongManager(const SongManager&) = delete;
	std::vector<std::string> m_songs {};
	std::string m_currentSong = "";
	std::string m_heldSong = "";
	std::string m_previousSong = "";
	std::string m_overrideSong = "";
	std::string m_displayName = "";
	std::string m_playlistName = "";
	bool m_isMenuLoop = false;
	bool m_constantShuffleMode = false;
	bool m_calledOnce = false;
	bool m_isUndefined0Alk1m123TouchPrio = false;
	bool m_geodify = false;
	bool m_isOverride = false;
	bool m_isLavaChicken = false;
	bool m_playlistIsEmpty = false;
	bool m_sawbladeCustomSongsFolder = false;
	bool m_advancedLogs = false;
	bool m_shouldRestoreMenuLoopPoint = false;
	bool m_vibecodedVentilla = false;
	int m_lastPosition = 0;
	std::vector<std::string> m_blacklist {};
	std::vector<std::string> m_favorites {};
	int m_towerRepeatCount = 0;
	geode::Mod* m_colonMenuLoopStartTime {};
	SongToSongData m_songToSongDataMap {};
	bool m_finishedCalculatingSongLengths = false;
};