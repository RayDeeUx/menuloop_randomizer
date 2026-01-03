#pragma once

class PlayingCard : public cocos2d::CCNode {
protected:
	bool init(const std::string&);

public:
	static PlayingCard* create(const std::string&);
	cocos2d::CCPoint position;
	const cocos2d::CCPoint cardSize = {380.0f, 55.0f};
};
