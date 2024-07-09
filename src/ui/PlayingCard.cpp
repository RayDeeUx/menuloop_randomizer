#include "PlayingCard.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

PlayingCard *PlayingCard::create(std::string output) {
	PlayingCard *ret = new PlayingCard();
	if (ret && ret->init(output)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool PlayingCard::init(std::string output) {
	if (!CCNode::init())
		return false;

	if (output.empty())
		output = "Unknown";

	auto mainNode = CCNode::create();

	this->setID("now-playing-card"_spr);

	auto bg = CCScale9Sprite::create("black-square.png"_spr);
	bg->setContentSize(cardSize);
	bg->setAnchorPoint({0.5f, 0.0f});

	auto nowPlayingLabel = CCLabelBMFont::create("", "gjFont17.fnt");
	nowPlayingLabel->setString(output.c_str());
	nowPlayingLabel->limitLabelWidth(cardSize.x - 4.0f, 0.5f, 0.1f);
	nowPlayingLabel->setPositionY(12.0f);

	mainNode->addChild(bg);
	mainNode->addChild(nowPlayingLabel);

	this->addChild(mainNode);

	return true;
}