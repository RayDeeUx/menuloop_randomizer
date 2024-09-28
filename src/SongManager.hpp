#pragma once

class SongManager {
  public:
	static SongManager &get() {
		static SongManager instance;

		return instance;
	}

	void addSong(std::string);
	void clearSongs();
	void pickRandomSong();
	bool isOriginalMenuLoop();
	std::string getCurrentSong();
	void setCurrentSong(std::string);
	void setCurrentSongToSavedSong();
	void update(float);
	bool getCalledOnce();
	void setCalledOnce(bool);
	void addToBlacklist(std::string);
	std::vector<std::string> getBlacklist();

  private:
	SongManager();
	SongManager(const SongManager &) = delete;
	std::vector<std::string> m_songs;
	std::string m_currentSong;
	bool m_isMenuLoop;
	bool m_calledOnce = false;
	std::vector<std::string> m_blacklist;
};
