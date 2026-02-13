#include "MLRSongCell.hpp"
#include "../SongControl.hpp"
#include "../Utils.hpp"

MLRSongCell* MLRSongCell::create(const SongData& songData, const bool isEven, const bool isCompact) {
	auto* ret = new MLRSongCell();

	if (ret->init(songData, isEven, isCompact)) {
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
	return MLRSongCell::init(SongData{.isEmpty = true}, isEven, false);
}

bool MLRSongCell::init(const SongData& songData, const bool isEven, const bool isCompact) {
	if (!CCLayerColor::initWithColor(isEven ? cocos2d::ccColor4B{160, 90, 45, 254} : cocos2d::ccColor4B{195, 115, 60, 254})) return false;
	this->m_songData = songData;

	this->setContentWidth(356.f);
	this->setColor({0, 0, 0});

	if (songData.isEmpty) {
		this->setOpacity(0);
		this->setTag(11152025);
		this->setContentHeight(isEven ? 3.5f : 36.f); // for empty cells isEven == isEndOfList, !isEven == isForSearchBarPadding
		this->setID("this-cell-intentionally-blank"_spr);
		this->setUserObject("this-cell-intentionally-blank"_spr, cocos2d::CCBool::create(true));
		return true;
	}

	const float compactModeFactor = isCompact ? static_cast<float>(std::clamp<double>(geode::Mod::get()->getSettingValue<double>("compactModeScaleFactor"), 1.5, 2.0)) : 1.f;
	this->setContentHeight(36.f / compactModeFactor);

	cocos2d::CCLabelBMFont* songNameLabel = cocos2d::CCLabelBMFont::create(songData.fullDisplayNameForControlPanelAndSongList.c_str(), "bigFont.fnt");
	songNameLabel->setAnchorPoint({.0f, .5f});
	songNameLabel->setPosition({15, (this->getContentHeight() / 2.f) + 1.f});

	if (songData.type == SongType::Favorited) songNameLabel->setFntFile("goldFont.fnt");
	else if (songData.type == SongType::Blacklisted) songNameLabel->setOpacity(128); // opacity is more apparopriate
	// else if (songData.type == SongType::Regular) songNameLabel->setColor({255, 255, 255}); // filler code lol!

	if (songData.isFromJukeboxDirectory) {
		// dont modify songnamelabel, add gradient
		cocos2d::CCLayerGradient* jukeboxGradient = cocos2d::CCLayerGradient::create({96, 96, 96, 128}, {0, 0, 0, 0});
		jukeboxGradient->setContentSize({356.f, this->getContentHeight() / 4.f});
		jukeboxGradient->ignoreAnchorPointForPosition(false);
		jukeboxGradient->setAnchorPoint({.5f, 1.f});
		jukeboxGradient->setID("jukebox-gradient"_spr);
		this->addChildAtPosition(jukeboxGradient, geode::Anchor::Top);
	}

	if (songData.isInNonVanillaNGMLSongLocation && !songData.isFromConfigOrAltDir && songData.couldPossiblyExistInMusicDownloadManager && !songData.isFromJukeboxDirectory) songNameLabel->setSkewX(10.f);
	songNameLabel->limitLabelWidth(356.f * (.8f / compactModeFactor), std::clamp<float>((.75f / compactModeFactor), .3, .75), .001f);
	this->setUserObject("song-name"_spr, cocos2d::CCString::create(songData.fullDisplayNameForControlPanelAndSongList));

	cocos2d::CCLabelBMFont* extraInfoLabl = nullptr;
	if (isCompact && Utils::getBool("showExtraInfoLabel")) {
		extraInfoLabl = cocos2d::CCLabelBMFont::create(songData.extraInfoText.c_str(), "chatFont.fnt");
		extraInfoLabl->setAnchorPoint({.0f, .5f});
		extraInfoLabl->setPosition({songNameLabel->getPositionX() + songNameLabel->getScaledContentWidth() + 5.f, this->getContentHeight() / 2.f});
		extraInfoLabl->limitLabelWidth(356.f * (.7f / compactModeFactor), std::clamp<float>((.5f / compactModeFactor), .3, .75), .001f);
		extraInfoLabl->setColor({0, 0, 0});
		extraInfoLabl->setOpacity(175);
	}

	CCLayerColor* divider = CCLayerColor::create({0, 0, 0, 127}, 356.f, .5f);
	divider->setAnchorPoint({0.f, 0.f});

	cocos2d::CCMenu* menu = cocos2d::CCMenu::create();
	menu->setAnchorPoint({1.f, .5f});
	menu->setContentSize({30.f / compactModeFactor, 30.f});
	menu->setPosition({340.f, this->getContentHeight() / 2.f});
	menu->ignoreAnchorPointForPosition(false);

	CCMenuItemSpriteExtra* playButton = CCMenuItemSpriteExtra::create(cocos2d::CCSprite::createWithSpriteFrameName("GJ_playMusicBtn_001.png"), this, menu_selector(MLRSongCell::onPlaySong));
	playButton->setID("song-cell-play-button"_spr);

	SongManager& songManager = SongManager::get();
	const bool canPlaybackProgress = songManager.getFinishedCalculatingSongLengths() && Utils::getBool("showPlaybackProgressControlsSongList") && songManager.getUndefined0Alk1m123TouchPrio();
	if (canPlaybackProgress) {
		CCMenuItemSpriteExtra* skipBkwd = Utils::addButton("skip-bkwd", menu_selector(MLRSongCell::onSkipBkwdButton), menu, this, true);
		CCMenuItemSpriteExtra* skipFwrd = Utils::addButton("skip-fwrd", menu_selector(MLRSongCell::onSkipFwrdButton), menu, this, true);
		skipBkwd->setVisible(false);
		skipFwrd->setVisible(false);

		CCLayerColor* total = CCLayerColor::create({0, 0, 0, 127}, 356.f, 2.5f / compactModeFactor);
		total->setAnchorPoint({0.f, 0.f});

		CCLayerColor* current = CCLayerColor::create({255, 255, 255, 255}, 349.f, 2.5f / compactModeFactor);
		current->setAnchorPoint({0.f, 0.f});
		current->setPositionX(current->getPositionX() + 3.5f);

		total->addChild(current);
		total->setPosition({0.f, .5f});

		total->setID("total-progress"_spr);
		current->setID("current-progress"_spr);

		this->addChild(total);

		this->m_totalBar = total;
		this->m_currentB = current;
		this->m_bkwdButton = skipBkwd;
		this->m_ffwdButton = skipFwrd;
	}

	geode::Layout* layout = geode::RowLayout::create()->setGap(0.f)->setAutoScale(true)->setAxisReverse(false)->setDefaultScaleLimits(.0001f, .75f);
	layout->ignoreInvisibleChildren(true);

	menu->addChild(playButton);
	menu->setLayout(layout);

	this->m_menu = menu;
	this->m_songNameLabel = songNameLabel;
	if (extraInfoLabl) this->m_extraInfoLabl = extraInfoLabl;
	this->m_divider = divider;
	this->m_playButton = playButton;

	this->addChild(songNameLabel);
	if (extraInfoLabl) this->addChild(extraInfoLabl);
	this->addChild(divider);
	this->addChild(menu);

	menu->setID("song-cell-menu"_spr);
	songNameLabel->setID("song-cell-label"_spr);
	if (extraInfoLabl) extraInfoLabl->setID("extra-info-label"_spr);
	divider->setID(fmt::format("song-cell-divider"_spr));

	this->setOpacity(255);

	MLRSongCell::checkIfCurrentSong(); // call immediately
	this->schedule(schedule_selector(MLRSongCell::checkIfCurrentSongScheduler), .125f); // schedule this function less often
	if (canPlaybackProgress) this->schedule(schedule_selector(MLRSongCell::pressAndHoldScheduler), .125f); // at this point it's already clear it's the current song

	return true;
}

void MLRSongCell::onPlaySong(CCObject*) {
	if (this->m_songData.type == SongType::Blacklisted || this->getTag() == 12192025) return;
	SongManager& songManager = SongManager::get();
	if (songManager.isOverride()) return;
	const std::string& currentSong = songManager.getCurrentSong();
	const std::string& futureSong = this->m_songData.actualFilePath;
	if (futureSong == currentSong) return;
	songManager.setPreviousSong(currentSong);
	songManager.setCurrentSong(futureSong);
	FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
	GameManager::sharedState()->playMenuMusic();
	Utils::newCardAndDisplayNameFromCurrentSong();
}

void MLRSongCell::updateProgressBar() const {
	SongManager& songManager = SongManager::get();
	if (!songManager.getFinishedCalculatingSongLengths() || !this->m_currentB || !this->m_totalBar) return;
	FMODAudioEngine* fmod = FMODAudioEngine::get();
	const std::string& currSong = songManager.getCurrentSong();
	if (fmod->getActiveMusic(0) != currSong || !songManager.getSongToSongDataEntries().contains(currSong)) return;
	this->m_currentB->setContentWidth(std::clamp<float>(((1.f * songManager.getLastMenuLoopPosition()) / (1.f * std::max<int>(Utils::getSongDataOfCurrentSong().songLength, fmod->getMusicLengthMS(0)))), 0.f, 1.f) * 349.f); // some songs hae lengths calculated by miniaudio, so clamp it to 1.f
}

void MLRSongCell::checkIfCurrentSong() const {
	if (this->m_songData.isEmpty || !this->m_songNameLabel || !this->m_songNameLabel->getParent() || !this->m_menu) return;
	const bool isCurrentSong = this->m_songData.hashedPath == SongManager::get().getHashedCurrentSong();

	geode::cocos::CCArrayExt<CCNode> childNodes = m_menu->getChildrenExt();
	int numVisible = 0;
	for (CCNode* node : childNodes) if (node && node->isVisible()) numVisible++;

	if (isCurrentSong) {
		this->m_songNameLabel->getParent()->setTag(12192025);
		this->m_songNameLabel->setColor(this->m_songData.type == SongType::Favorited ? cocos2d::ccColor3B{0, 255, 255} : cocos2d::ccColor3B{0, 255, 0});
		if (this->m_currentB && this->m_totalBar) MLRSongCell::updateProgressBar();
	} else {
		this->m_songNameLabel->getParent()->setTag(-1);
		this->m_songNameLabel->setColor({255, 255, 255});
	}

	if (this->m_totalBar) this->m_totalBar->setVisible(isCurrentSong);
	if (this->m_currentB) this->m_currentB->setVisible(isCurrentSong);

	if (this->m_playButton) {
		this->m_playButton->setVisible(!isCurrentSong);
		this->m_playButton->setEnabled(!isCurrentSong);
	}
	if (this->m_bkwdButton) {
		this->m_bkwdButton->setVisible(isCurrentSong);
		this->m_bkwdButton->setEnabled(isCurrentSong);
	}
	if (this->m_ffwdButton) {
		this->m_ffwdButton->setVisible(isCurrentSong);
		this->m_ffwdButton->setEnabled(isCurrentSong);
	}

	int newVisible = 0;
	for (CCNode* node : childNodes) if (node && node->isVisible()) newVisible++;

	if (numVisible != newVisible && this->m_menu->getLayout()) {
		this->m_menu->setContentWidth(std::clamp<float>(30.f / (36.f / this->getContentHeight()) * newVisible, 15.f, 30.f));
		this->m_menu->updateLayout();
	}
}

void MLRSongCell::toggleEven(const bool isEven) {
	if (this->m_songData.isEmpty) return;
	this->setColor(isEven ? cocos2d::ccColor3B{161, 88, 44} : cocos2d::ccColor3B{194, 114, 62});
	this->setID(fmt::format("song-cell-{}"_spr, isEven));
}

void MLRSongCell::checkIfCurrentSongScheduler(float) {
	const bool wasRegular = this->m_songData.type == SongType::Regular;
	MLRSongCell::checkIfCurrentSong();
	if (!this->m_songNameLabel || this->getTag() != 12192025 || this->m_songData.hashedPath != SongManager::get().getHashedCurrentSong() || wasRegular && Utils::getSongDataOfSongPath(this->m_songData.actualFilePath).type == SongType::Regular) return;
	const float compactModeFactor = 36.f / this->getContentHeight();
	this->m_songNameLabel->setFntFile("goldFont.fnt");
	this->m_songNameLabel->setColor(cocos2d::ccColor3B{0, 255, 255});
	this->m_songNameLabel->limitLabelWidth(356.f * (.8f / compactModeFactor), std::clamp<float>((.75f / compactModeFactor), .3, .75), .001f);
	if (compactModeFactor != 1.0 && this->m_extraInfoLabl) {
		this->m_extraInfoLabl->limitLabelWidth(356.f * (.7f / compactModeFactor), std::clamp<float>((.5f / compactModeFactor), .3, .75), .001f);
	}
}

void MLRSongCell::pressAndHoldScheduler(float dt) {
	m_time += dt;

	CCMenuItemSpriteExtra* ffwd = this->m_ffwdButton;
	CCMenuItemSpriteExtra* bkwd = this->m_bkwdButton;
	if (!ffwd || !bkwd) {
		m_time = 0.f;
		return;
	}

	const bool isCurrentSong = this->getTag() == 12192025 && ffwd->isEnabled() && ffwd->isVisible() && bkwd->isEnabled() && bkwd->isVisible();
	if (!isCurrentSong) {
		m_time = 0.f;
		return;
	}

	if (m_time < .5f) return;

	if (ffwd->isSelected() && ffwd->m_pListener && ffwd->m_pfnSelector) (ffwd->m_pListener->*ffwd->m_pfnSelector)(ffwd);
	else if (bkwd->isSelected()&& bkwd->m_pListener && bkwd->m_pfnSelector) (bkwd->m_pListener->*bkwd->m_pfnSelector)(bkwd);
	else m_time = 0;
}


void MLRSongCell::onSkipBkwdButton(CCObject*) {
	if (SongManager::get().getShowPlaybackControlsSongList()) SongControl::skipBackward();
}

void MLRSongCell::onSkipFwrdButton(CCObject*) {
	if (SongManager::get().getShowPlaybackControlsSongList()) SongControl::skipForward();
}