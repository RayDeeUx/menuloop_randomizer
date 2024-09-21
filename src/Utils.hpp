#pragma once
#include <regex>

class Utils {
  public:
	static int randomIndex(int);
	static bool isSupportedExtension(std::string);
	static bool getBool(std::string);
	static void removeCard();
	static cocos2d::CCNode* findCard();
	static cocos2d::CCNode* findCardRemotely();
	static void setNewSong();
	static void playlistModeNewSong();
	static void makeNewCard(const std::string &);
	static void generateNotification();
	static void playlistModePLAndEPL();
	static void copyCurrentSongName();
};