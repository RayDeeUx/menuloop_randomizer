#pragma once

class Utils {
  public:
	static int randomIndex(int);
	static bool isSupportedFile(const std::string_view);
	static bool goodExtension(std::string_view path);
	static bool getBool(const std::string& setting);
	static std::string getString(const std::string& setting);
	static void removeCard();
	static cocos2d::CCNode* findCard();
	static cocos2d::CCNode* findCardRemotely();
	static void setNewSong();
	static void playlistModeNewSong(const bool fromGJBGL = false);
	static void newNotification(const std::string& notifString, const bool checkSetting = false);
	static std::string composedNotifString(std::string notifString, const std::string& middle, const std::string& suffix);
	static void newCardAndDisplayNameFromCurrentSong();
	static std::string getFormattedNGMLSongName(SongInfoObject* songInfo);
	static void playlistModePLAndEPL();
	static void copyCurrentSongName();
	static void populateVector(const bool customSongs, const std::filesystem::path& path = "", std::vector<std::string> textFileBlacklist = {}, std::vector<std::string> textFileFavorites = {});
	static void refreshTheVector();
	static void resetSongManagerRefreshVectorSetNewSongBecause(const std::string_view reasonUsuallySettingName);
	static SongInfoObject* getSongInfoObject();
	static std::string getSongName();
	static std::string getSongArtist();
	static int getSongID();
	static std::string currentCustomSong();
	static std::string toNormalizedString(const std::filesystem::path& path);
	static std::string toProblematicString(const std::string& path);
	static void fadeOutCardRemotely(cocos2d::CCNode* card = Utils::findCardRemotely());
	static void removeCardRemotely(cocos2d::CCNode* card = Utils::findCardRemotely());
	static void queueUpdateSCMLabel();
	static void addButton(const std::string& name, const cocos2d::SEL_MenuHandler function, cocos2d::CCMenu* menu, cocos2d::CCNode* target);
	static bool notFavoritesNorBlacklist(std::filesystem::path fileForWriting);
	static std::string getPlatform();
	static std::string generatePlatformWarning();
	static void writeToFile(const std::string& toWriteToFile, std::filesystem::path fileForWriting);
};