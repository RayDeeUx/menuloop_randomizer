#pragma once
#include <regex>

class Utils {
  public:
	static int randomIndex(int);
	static bool isSupportedFile(const std::string_view);
	static bool goodExtension(std::string_view path);
	static bool getBool(std::string);
	static void removeCard();
	static cocos2d::CCNode* findCard();
	static cocos2d::CCNode* findCardRemotely();
	static void setNewSong();
	static void playlistModeNewSong(const bool fromGJBGL = false);
	static void makeNewCard(const std::string &);
	static void generateNotification();
	static void playlistModePLAndEPL();
	static void copyCurrentSongName();
	static void populateVector(bool);
	static SongInfoObject* getSongInfoObject();
	static std::string getSongName();
	static std::string getSongArtist();
	static int getSongID();
	static std::string currentCustomSong();
	static std::string toNormalizedString(const std::filesystem::path& path);
	static void fadeOutCardRemotely(cocos2d::CCNode* card = Utils::findCardRemotely());
	static void removeCardRemotely(cocos2d::CCNode* card = Utils::findCardRemotely());
};