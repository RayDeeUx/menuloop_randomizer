#include "PlayingCard.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

PlayingCard *PlayingCard::create(gd::string songId) {
	PlayingCard *ret = new PlayingCard();
	if (ret && ret->init(songId)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool PlayingCard::init(gd::string songId) {
	if (!CCNode::init())
		return false;

	this->setID("now-playing-card"_spr);

	auto bg = CCScale9Sprite::create("GE_square03.png");
	bg->setContentSize({90, 55});

	auto nowPlayingLabel = CCLabelBMFont::create("Now Playing", "bigFont.fnt");
	nowPlayingLabel->setScale(0.3f);
	nowPlayingLabel->setPositionY(12.0f);

	auto idLabel = CCLabelBMFont::create(songId.c_str(), "bigFont.fnt");
	idLabel->setScale(0.3f);
	idLabel->limitLabelWidth(85.0f, 0.45f, 0.05f);
	idLabel->setPositionY(-10.0f);

	this->addChild(bg);
	this->addChild(nowPlayingLabel);
	this->addChild(idLabel);

	return true;
}