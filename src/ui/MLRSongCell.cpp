#include "MLRSongCell.hpp"
#include "../SongManager.hpp"
#include "../Utils.hpp"

MLRSongCell* MLRSongCell::create(const SongData& SongData, const bool isEven) {
	auto* ret = new MLRSongCell();

	if (ret->init(SongData, isEven)) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

MLRSongCell* MLRSongCell::createEmpty(const bool isEven) {
	auto* ret = new MLRSongCell();

	if (ret->initEmpty(isEven)) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

bool MLRSongCell::initEmpty(const bool isEven) {
	SongData dummySongData = SongData {"", "", "", "", SongType::Regular, false, true};
	return MLRSongCell::init(dummySongData, isEven);
}

bool MLRSongCell::init(const SongData& songData, const bool isEven) {
	if (!CCLayerColor::initWithColor(isEven ? cocos2d::ccColor4B{161, 88, 44, 255} : cocos2d::ccColor4B{194, 114, 62, 255})) return false;
	m_songData = songData;

	this->setContentSize({356.f, 36.f});

	if (songData.isEmpty) {
		this->setOpacity(0);
		this->setID("this-cell-intentionally-blank"_spr);
		this->setUserObject("this-cell-intentionally-blank"_spr, cocos2d::CCBool::create(true));
		this->setTag(11152025);
		return true;
	}

	const std::string& desiredFileName = geode::utils::string::replace(songData.fileName, songData.fileExtension, "");
	cocos2d::CCLabelBMFont* songNameLabel = cocos2d::CCLabelBMFont::create(songData.displayName.c_str(), "bigFont.fnt");
	songNameLabel->setAnchorPoint({.0f, .5f});
	songNameLabel->setPosition({15, getContentHeight() / 2.f + 1.f});

	if (songData.type == SongType::Favorited) songNameLabel->setFntFile("goldFont.fnt");
	else if (songData.type == SongType::Blacklisted) songNameLabel->setColor({128, 128, 128});

	MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
	const int songID = geode::utils::numFromString<int>(desiredFileName).unwrapOr(-1);
	if (songID > 0 && !songData.isFromConfigOrAltDir) {
		if (mdm->getSongInfoObject(songID)) {
			if (Utils::toProblematicString(songData.actualFilePath) != Utils::toProblematicString(static_cast<std::string>(mdm->pathForSong(songID)))) {
				songNameLabel->setSkewX(10.f);
			}
		}
	}
	songNameLabel->limitLabelWidth(356.f * .8f, .75f, .001f);
	this->setUserObject("song-name"_spr, cocos2d::CCString::create(songData.displayName));

	CCLayerColor* divider = CCLayerColor::create({0, 0, 0, 127});
	divider->setContentSize({356.f, 0.5f});
	divider->setAnchorPoint({0.f, 0.f});

	cocos2d::CCMenu* menu = cocos2d::CCMenu::create();
	menu->setContentSize({30.f, 30.f});
	menu->setAnchorPoint({1.f, .5f});
	menu->setPosition({340.f, this->getContentHeight() / 2.f});
	menu->ignoreAnchorPointForPosition(false);
	menu->setLayout(
		geode::RowLayout::create()
			->setGap(10.f)
			->setAutoScale(true)
			->setAxisReverse(true)
			->setDefaultScaleLimits(.0001f, .75f)
	);

	CCMenuItemSpriteExtra* playButton = CCMenuItemSpriteExtra::create(
		cocos2d::CCSprite::createWithSpriteFrameName("GJ_playMusicBtn_001.png"),
		this, menu_selector(MLRSongCell::onPlaySong)
	);
	playButton->setID("song-cell-play-button"_spr);
	menu->addChild(playButton);
	menu->updateLayout();

	m_menu = menu;
	m_songNameLabel = songNameLabel;
	m_divider = divider;
	m_playButton = playButton;

	this->addChild(songNameLabel);
	this->addChild(divider);
	this->addChild(menu);

	menu->setID("song-cell-menu"_spr);
	songNameLabel->setID("song-cell-label"_spr);
	divider->setID(fmt::format("song-cell-divider-{}"_spr, isEven));
	this->setID(fmt::format("song-cell-{}"_spr, isEven));

	this->scheduleUpdate();

	return true;
}

void MLRSongCell::onPlaySong(CCObject*) {
	SongManager& songManager = SongManager::get();
	const std::string& currentSong = songManager.getCurrentSong();
	if (this->m_songData.actualFilePath == currentSong) return;
	FMODAudioEngine* fmod = FMODAudioEngine::get();
	songManager.setPreviousSong(currentSong);
	songManager.setCurrentSong(m_songData.actualFilePath);
	fmod->m_backgroundMusicChannel->stop();
	GameManager::sharedState()->playMenuMusic();
	Utils::newCardAndDisplayNameFromCurrentSong();
}

void MLRSongCell::update(float delta) {
	const bool isCurrentSong = this->m_songData.actualFilePath == SongManager::get().getCurrentSong();

	if (isCurrentSong) {
		if (this->m_songData.type == SongType::Favorited) this->m_songNameLabel->setColor({0, 255, 255});
		else this->m_songNameLabel->setColor({0, 255, 0});
	}
	else this->m_songNameLabel->setColor({255, 255, 255});

	this->m_menu->setVisible(!isCurrentSong);
	this->m_playButton->setEnabled(!isCurrentSong);
}