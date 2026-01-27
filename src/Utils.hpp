#pragma once
#include "SongManager.hpp"

class Utils {
  public:
	static int randomIndex(int);
	static bool isSupportedFile(const std::string_view);
	static bool goodExtension(const std::string_view);
	static bool getBool(const std::string_view);
	static std::string getString(const std::string_view);
	static void removeCard();
	static cocos2d::CCNode* findCard();
	static cocos2d::CCNode* findCardRemotely();
	static void setNewSong();
	static void constantShuffleModeNewSong();
	static void newNotification(const std::string& notifString, const bool checkSetting = false);
	static void composeAndSetCurrentSongDisplayNameOnlyOnLoadOrWhenBlacklistingSongs();
	static std::string composedNotifString(std::string notifString, const std::string& middle, const std::string& suffix);
	static void newCardAndDisplayNameFromCurrentSong();
	static bool adjustSongInfoIfJukeboxReplacedIt(SongInfoObject* songInfo) ;
	static std::string getFormattedNGMLSongName(SongInfoObject*);
	static void copyCurrentSongName();
	static void loadFromPlaylistFile(const std::filesystem::path&);
	static void populateVector(const bool customSongs, const std::filesystem::path& path = "", std::vector<std::string> textFileBlacklist = {}, std::vector<std::string> textFileFavorites = {});
	static void popualteSongToSongDataMap();
	static std::vector<std::string> parseBlacklistFile(const std::filesystem::path&);
	static std::vector<std::string> parseFavoritesFile(const std::filesystem::path&);
	static void refreshTheVector();
	static void showFreezeWarning();
	static void resetSongManagerRefreshVectorSetNewSongBecause(const std::string_view reasonUsuallySettingName);
	static bool isFromConfigOrAlternateDir(const std::filesystem::path&);
	static SongInfoObject* getSongInfoObject();
	static std::string getSongName();
	static std::string getSongArtist();
	static int getSongID();
	static std::string currentCustomSong();
	static std::string toNormalizedString(const std::filesystem::path&);
	static std::filesystem::path toProblematicString(const std::string_view);
	static void fadeOutCardRemotely(cocos2d::CCNode*);
	static void removeCardRemotely(cocos2d::CCNode*);
	static void queueUpdateFrontfacingLabelsInSCMAndSLL();
	static CCMenuItemSpriteExtra* addButton(const std::string& name, const cocos2d::SEL_MenuHandler function, cocos2d::CCMenu* menu, cocos2d::CCNode* target, const bool dontAddBG = false);
	static CCMenuItemToggler* addViewModeToggle(const bool relevantBoolean, const std::string& toggleIcon, const std::string_view nodeID, cocos2d::SEL_MenuHandler function, cocos2d::CCMenu* menu, cocos2d::CCNode* target, const bool dontAddBG = false);
	static void showMDPopup(const std::string_view title, const std::string& bodyText, const int tag, const std::string_view nodeID);
	static bool notFavoritesNorBlacklist(const std::filesystem::path&);
	static std::string getPlatform();
	static std::string generatePlatformWarning();
	static void writeToFile(const std::string& toWriteToFile, const std::filesystem::path& fileForWriting);
};
