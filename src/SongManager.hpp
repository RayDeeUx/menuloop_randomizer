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

  private:
	SongManager();
	SongManager(const SongManager &) = delete;
	std::vector<std::string> m_songs;
	std::string m_currentSong;
	bool m_isMenuLoop;
};