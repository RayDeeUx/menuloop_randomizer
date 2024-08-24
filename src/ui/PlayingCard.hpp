#pragma once
#include <Geode/cocos/base_nodes/CCNode.h>

class PlayingCard : public cocos2d::CCNode {
  protected:
	bool init(std::string);

  public:
	static PlayingCard *create(std::string);
	cocos2d::CCPoint position;
	const cocos2d::CCPoint cardSize = {380.0f, 55.0f};
};
