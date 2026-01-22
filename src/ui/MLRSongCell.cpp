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
	songNameLabel->limitLabelWidth(356.f * (.8f / compactModeFactor), std::clamp<float>((.75f / compactModeFactor), .3, .75), .001f);
	this->setUserObject("song-name"_spr, cocos2d::CCString::create(songData.displayName));

	cocos2d::CCLabelBMFont* extraInfoLabl = nullptr;
	if (isCompact && Utils::getBool("showExtraInfoLabel")) {
		std::chrono::system_clock::time_point timepoint = std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(songData.songWriteTime.time_since_epoch()));
		auto datepoint = floor<std::chrono::days>(floor<std::chrono::seconds>(timepoint));
		const std::chrono::year_month_day yearMonthDate{std::chrono::sys_days{datepoint}};
		extraInfoLabl = cocos2d::CCLabelBMFont::create("PLACEHOLDER SO NOTHING CRASHES", "chatFont.fnt");
		if (SongManager::get().getUndefined0Alk1m123TouchPrio()) {
			extraInfoLabl->setString(fmt::format("{} | {:.2f} sec | {:.2f} MB | {} {:02}, {:04}", songData.fileExtension, songData.songLength / 1000.f, songData.songFileSize / 1000000.f, months[static_cast<unsigned>(yearMonthDate.month()) - 1], static_cast<unsigned>(yearMonthDate.day()), static_cast<int>(yearMonthDate.year())).c_str());
		} else {
			extraInfoLabl->setString(fmt::format("{} | {:.2f} MB | {} {:02}, {:04}", songData.fileExtension, songData.songFileSize / 1000000.f, months[static_cast<unsigned>(yearMonthDate.month()) - 1], static_cast<unsigned>(yearMonthDate.day()), static_cast<int>(yearMonthDate.year())).c_str());
		}
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
	if (songManager.getFinishedCalculatingSongLengths() && Utils::getBool("showPlaybackProgressControlsSongList") && songManager.getUndefined0Alk1m123TouchPrio()) {
		CCMenuItemSpriteExtra* skipBkwd = Utils::addButton("skip-bkwd", menu_selector(MLRSongCell::onSkipBkwdButton), menu, this, true);
		CCMenuItemSpriteExtra* skipFwrd = Utils::addButton("skip-fwrd", menu_selector(MLRSongCell::onSkipFwrdButton), menu, this, true);
		skipBkwd->setVisible(false);
		skipFwrd->setVisible(false);

		CCLayerColor* total = CCLayerColor::create({0, 0, 0, 127}, 356.f, 5.f / compactModeFactor);
		total->setAnchorPoint({0.f, 0.f});

		CCLayerColor* current = CCLayerColor::create({255, 255, 255, 255}, 356.f, 5.f / compactModeFactor);
		current->setAnchorPoint({0.f, 0.f});

		total->addChild(current);
		current->setContentWidth(0.f);
		total->setPosition({0.f, .5f});

		total->setID("total-progress"_spr);
		current->setID("current-progress"_spr);

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
	FMODAudioEngine* fmod = FMODAudioEngine::get();
	const std::string& currSong = songManager.getCurrentSong();
	if (fmod->getActiveMusic(0) == currSong && songManager.getSongToSongDataEntries().contains(currSong)) {
		const int fullLength = songManager.getSongToSongDataEntries().find(songManager.getCurrentSong())->second.songLength;
		const int lastPosition = songManager.getLastMenuLoopPosition();
		this->m_currentB->setContentWidth(((1.f * lastPosition) / (1.f * fullLength)) * this->m_totalBar->getContentWidth());
	}
}

void MLRSongCell::checkIfCurrentSong() const {
	if (this->m_songData.isEmpty || !this->m_songNameLabel || !this->m_songNameLabel->getParent() || !this->m_menu) return;
	const bool isCurrentSong = this->m_songData.actualFilePath == SongManager::get().getCurrentSong();

	auto children = m_menu->getChildrenExt();
	int numVisible = 0;
	for (CCNode* node : children) {
		if (!node || !node->isVisible()) continue;
		numVisible++;
	}

	if (isCurrentSong) {
		this->m_songNameLabel->getParent()->setTag(12192025);
		if (this->m_songData.type == SongType::Favorited) this->m_songNameLabel->setColor({0, 255, 255});
		else this->m_songNameLabel->setColor({0, 255, 0});
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
	for (CCNode* node : children) {
		if (!node || !node->isVisible()) continue;
		newVisible++;
	}

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
	MLRSongCell::checkIfCurrentSong();
}

void MLRSongCell::onSkipBkwdButton(CCObject*) {
	if (SongManager::get().getShowPlaybackControlsSongList()) SongControl::skipBackward();
}

void MLRSongCell::onSkipFwrdButton(CCObject*) {
	if (SongManager::get().getShowPlaybackControlsSongList()) SongControl::skipForward();
}