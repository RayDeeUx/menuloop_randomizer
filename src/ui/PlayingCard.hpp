#pragma once

class PlayingCard : public cocos2d::CCNode {
  protected:
	bool init(gd::string, gd::string);

  public:
	static PlayingCard *create(gd::string, gd::string);
	cocos2d::CCPoint position;
	const cocos2d::CCPoint cardSize = {380.0f, 55.0f};
};