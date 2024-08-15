#pragma once

class Utils {
  public:
	static int randomIndex(int size);
	static int stoi(std::string);
	static bool isSupportedExtension(std::string);
	static bool getBool(std::string);
	static void removeCard();
	static cocos2d::CCNode* findCard();
	static cocos2d::CCNode* findCardRemotely();
	static void setNewSong();
};