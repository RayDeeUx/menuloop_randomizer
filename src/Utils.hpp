#pragma once

class Utils {
  public:
	static int randomIndex(int size);
	static int stoi(std::string);
	static bool isSupportedExtension(std::string);
	static bool getBool(std::string);
	static void keepCardAcrossScenes();
	static void removeCard();
	static void setNewSong();
};