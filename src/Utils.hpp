#pragma once
#include <string>
#include <Geode/cocos/base_nodes/CCNode.h>

class Utils {
  public:
	static int randomIndex(int size);
	static uint64_t stoi(std::string);
	static bool isSupportedExtension(std::string);
	static bool getBool(std::string);
	static void removeCard();
	static cocos2d::CCNode* findCard();
	static cocos2d::CCNode* findCardRemotely();
	static void setNewSong();
};
