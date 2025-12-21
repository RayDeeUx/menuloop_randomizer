#include "MLRSongCell.hpp"
#include "../SongManager.hpp"
#include "../Utils.hpp"

MLRSongCell* MLRSongCell::create(const SongData& SongData, const bool isEven, const bool isCompact) {
	auto* ret = new MLRSongCell();

	if (ret->init(SongData, isEven, isCompact)) {
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
	return MLRSongCell::init({"", "", "", "", SongType::Regular, false, true}, isEven, false);
}

bool MLRSongCell::init(const SongData& songData, const bool isEven, const bool isCompact) {
	if (!CCLayerColor::initWithColor(isEven ? cocos2d::ccColor4B{160, 90, 45, 254} : cocos2d::ccColor4B{195, 115, 60, 254})) return false;
	this->m_songData = songData;

	this->setContentWidth(356.f);

	if (songData.isEmpty) {
		this->setOpacity(0);
		this->setTag(11152025);
		this->setColor({0, 0, 0});
		this->setContentHeight(36.f);
		this->setID("this-cell-intentionally-blank"_spr);
		this->setUserObject("this-cell-intentionally-blank"_spr, cocos2d::CCBool::create(true));
		return true;
	}

	const float compactModeFactor = isCompact ? static_cast<float>(std::clamp<double>(geode::Mod::get()->getSettingValue<double>("compactModeScaleFactor"), 1.5, 2.0)) : 1.f;
	this->setContentHeight(36.f / compactModeFactor);

	const std::string& desiredFileName = geode::utils::string::replace(songData.fileName, songData.fileExtension, "");
	cocos2d::CCLabelBMFont* songNameLabel = cocos2d::CCLabelBMFont::create(songData.displayName.c_str(), "bigFont.fnt");
	songNameLabel->setAnchorPoint({.0f, .5f});
	songNameLabel->setPosition({15, (this->getContentHeight() / 2.f) + 1.f});

	if (songData.type == SongType::Favorited) songNameLabel->setFntFile("goldFont.fnt");
	else if (songData.type == SongType::Blacklisted) songNameLabel->setOpacity(128); // opacity is more apparopriate
	// else if (songData.type == SongType::Regular) songNameLabel->setColor({255, 255, 255}); // filler code lol!
	else if (geode::utils::string::contains(songData.actualFilePath, "fleym.nongd")) {
		// dont modify songnamelabel, add gradient
		cocos2d::CCLayerGradient* jukeboxGradient = cocos2d::CCLayerGradient::create({96, 96, 96, 128}, {0, 0, 0, 0});
		jukeboxGradient->setContentSize({356.f, this->getContentHeight() / 4.f});
		jukeboxGradient->ignoreAnchorPointForPosition(false);
		jukeboxGradient->setAnchorPoint({.5f, 1.f});
		jukeboxGradient->setID("jukebox-gradient"_spr);
		this->addChildAtPosition(jukeboxGradient, geode::Anchor::Top);
	}

	MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
	const int songID = geode::utils::numFromString<int>(desiredFileName).unwrapOr(-1);
	if (songID > 0 && !songData.isFromConfigOrAltDir && mdm->getSongInfoObject(songID)) {
		if (Utils::toProblematicString(songData.actualFilePath) != Utils::toProblematicString(static_cast<std::string>(mdm->pathForSong(songID)))) {
			songNameLabel->setSkewX(10.f);
		}
	}
	songNameLabel->limitLabelWidth(356.f * (.8 / compactModeFactor), .75f, .001f);
	this->setUserObject("song-name"_spr, cocos2d::CCString::create(songData.displayName));

	CCLayerColor* divider = CCLayerColor::create({0, 0, 0, 127}, 356.f, .5f);
	divider->setAnchorPoint({0.f, 0.f});

	cocos2d::CCMenu* menu = cocos2d::CCMenu::create();
	menu->setAnchorPoint({1.f, .5f});
	menu->setContentSize({30.f / compactModeFactor, 30.f});
	menu->setPosition({340.f, this->getContentHeight() / 2.f});
	menu->ignoreAnchorPointForPosition(false);

	CCMenuItemSpriteExtra* playButton = CCMenuItemSpriteExtra::create(cocos2d::CCSprite::createWithSpriteFrameName("GJ_playMusicBtn_001.png"), this, menu_selector(MLRSongCell::onPlaySong));
	playButton->setID("song-cell-play-button"_spr);
	menu->addChild(playButton);

	menu->setLayout(geode::RowLayout::create()->setGap(0.f)->setAutoScale(true)->setAxisReverse(true)->setDefaultScaleLimits(.0001f, .75f));

	this->m_menu = menu;
	this->m_songNameLabel = songNameLabel;
	this->m_divider = divider;
	this->m_playButton = playButton;

	this->addChild(songNameLabel);
	this->addChild(divider);
	this->addChild(menu);

	menu->setID("song-cell-menu"_spr);
	songNameLabel->setID("song-cell-label"_spr);
	divider->setID(fmt::format("song-cell-divider-{}"_spr, isEven));
	this->setID(fmt::format("song-cell-{}"_spr, isEven));

	this->setOpacity(255);
	this->setColor(isEven ? cocos2d::ccColor3B{161, 88, 44} : cocos2d::ccColor3B{194, 114, 62});

	MLRSongCell::checkIfCurrentSong(); // call immediately
	this->schedule(schedule_selector(MLRSongCell::update), .125f); // schedule this function less often

	return true;
}

void MLRSongCell::onPlaySong(CCObject* sender) {
	if (this->m_songData.type == SongType::Blacklisted) return;
	SongManager& songManager = SongManager::get();
	const std::string& currentSong = songManager.getCurrentSong();
	const std::string& futureSong = this->m_songData.actualFilePath;
	if (futureSong == currentSong) return;
	songManager.setPreviousSong(currentSong);
	songManager.setCurrentSong(futureSong);
	FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
	GameManager::sharedState()->playMenuMusic();
	Utils::newCardAndDisplayNameFromCurrentSong();
}

void MLRSongCell::checkIfCurrentSong() const {
	const bool isCurrentSong = this->m_songData.actualFilePath == SongManager::get().getCurrentSong();

	if (isCurrentSong) {
		this->m_songNameLabel->getParent()->setTag(12192025);
		if (this->m_songData.type == SongType::Favorited) this->m_songNameLabel->setColor({0, 255, 255});
		else this->m_songNameLabel->setColor({0, 255, 0});
	} else {
		this->m_songNameLabel->getParent()->setTag(-1);
		this->m_songNameLabel->setColor({255, 255, 255});
	}

	this->m_menu->setVisible(!isCurrentSong);
	this->m_playButton->setEnabled(!isCurrentSong);
}

void MLRSongCell::update(float delta) {
	MLRSongCell::checkIfCurrentSong();
}