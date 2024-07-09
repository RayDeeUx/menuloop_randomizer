#pragma once

class SongManager {
  public:
	static SongManager *get() {
		static SongManager *instance;

		return instance;
	}

	void addSong(std::string);
	void clearSongs();
	void pickRandomSong();
	std::string getCurrentSong();

  private:
	SongManager();
	std::vector<std::string> m_songs;
	std::string m_currentSong;
};