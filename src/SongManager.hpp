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
	std::string getCurrentSong();
	void setCurrentSong(const std::string &);
	void setCurrentSongToSavedSong();
	void update(float);
	bool getCalledOnce() const;
	void setCalledOnce(const bool);
	bool getGeodify() const;
	void setGeodify(const bool);
	void setHeldSong(const std::string_view value);
	std::string getHeldSong();
	void addToBlacklist(const std::string&);
	void addToBlacklist();
	std::vector<std::string> getBlacklist();

  private:
	SongManager();
	SongManager(const SongManager &) = delete;
	std::vector<std::string> m_songs;
	std::string m_currentSong;
	std::string m_heldSong = "";
	bool m_isMenuLoop;
	bool m_calledOnce = false;
	bool m_geodify = false;
	std::vector<std::string> m_blacklist;
};
