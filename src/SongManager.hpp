#pragma once

class SongManager {
  public:
	static SongManager &get() {
		static SongManager instance;

		return instance;
	}

	void addSong(const std::string&);
	void clearSongs();
	void pickRandomSong();
	bool isOriginalMenuLoop() const;

	void setPlaylistMode();

	std::string getCurrentSong();
	void setCurrentSong(const std::string &);
	void setCurrentSongToSavedSong();
	void update(float) const;
	bool getCalledOnce() const;
	void setCalledOnce(const bool);
	bool getGeodify() const;
	void setGeodify(const bool);
	void setHeldSong(const std::string_view value);
	std::string getHeldSong();
	void addToBlacklist(const std::string&);
	void addToBlacklist();
	std::vector<std::string> getBlacklist();
	void setOverride(const std::string_view);
	std::string getOverrideSong();
	bool isOverride() const;
	void setCurrentSongToOverride();

  private:
	SongManager();
	SongManager(const SongManager &) = delete;
	std::vector<std::string> m_songs;
	std::string m_currentSong;
	std::string m_heldSong = "";
	std::string m_overrideSong = "";
	bool m_isMenuLoop;
	bool m_playlistMode;
	bool m_calledOnce = false;
	bool m_geodify = false;
	bool m_isOverride = false;
	std::vector<std::string> m_blacklist;
};
