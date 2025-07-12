#include "MLRSongCell.hpp"

MLRSongCell* MLRSongCell::create(const SongData& SongData, const bool isEven) {
	auto* ret = new MLRSongCell();

	if (ret->init(SongData, isEven)) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

bool MLRSongCell::init(const SongData& SongData, const bool isEven) {
	if (!CCLayerColor::initWithColor(isEven ? cocos2d::ccColor4B{161, 88, 44, 255} : cocos2d::ccColor4B{194, 114, 62, 255})) return false;
	m_SongData = SongData;

	this->setContentSize({356.f, 36.f});

	cocos2d::CCLabelBMFont* songNameLabel = cocos2d::CCLabelBMFont::create(SongData.fileName.c_str(), "bigFont.fnt");
	songNameLabel->setAnchorPoint({.0f, .5f});
	songNameLabel->setPosition({15, getContentHeight() / 2.f + 1.f});
	songNameLabel->limitLabelWidth(356.f * .65f, .75f, .001f);

	CCLayerColor* divider = CCLayerColor::create({0, 0, 0, 127});
	divider->setContentSize({356.f, 0.5f});
	divider->setAnchorPoint({0.f, 0.f});

	this->addChild(songNameLabel);
	this->addChild(divider);

	songNameLabel->setID("song-label"_spr);
	divider->setID(fmt::format("song-cell-divider-{}"_spr, isEven));
	this->setID(fmt::format("song-cell-{}"_spr, isEven));

	return true;
}