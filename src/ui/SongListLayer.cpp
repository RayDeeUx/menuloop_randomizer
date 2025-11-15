#include <Geode/ui/GeodeUI.hpp>
#include "SongListLayer.hpp"
#include "MLRSongCell.hpp"
#include "SongControlMenu.hpp"
#include "../Utils.hpp"
#include "../SongControl.hpp"
#include "../SongManager.hpp"

SongListLayer* SongListLayer::create(const std::string& id) {
	auto* ret = new SongListLayer();
	if (ret->initAnchored(420.f, 290.f, id)) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

void SongListLayer::addSongsToScrollLayer(geode::ScrollLayer* scrollLayer, SongManager& songManager, const std::string& queryString) {
	const std::vector<std::string> blacklist = songManager.getBlacklist();
	const std::vector<std::string> favorites = songManager.getFavorites();
	const std::vector<std::string> songs = songManager.getSongs();
	std::vector<std::string> alreadyAdded {};

	bool isEven = false;
	scrollLayer->m_contentLayer->addChild(MLRSongCell::createEmpty(false));

	for (const std::string& song : songs) {
		if (std::ranges::find(alreadyAdded.begin(), alreadyAdded.end(), song) != alreadyAdded.end()) continue;

		std::filesystem::path songFilePath = Utils::toProblematicString(song);
		SongData songData = {
			Utils::toNormalizedString(songFilePath),
			Utils::toNormalizedString(songFilePath.extension()),
			Utils::toNormalizedString(songFilePath.filename()), "", SongType::Regular,
			Utils::isFromConfigOrAlternateDir(songFilePath.parent_path()), false
		};
		if (std::ranges::find(blacklist.begin(), blacklist.end(), song) != blacklist.end()) songData.type = SongType::Blacklisted;
		else if (std::ranges::find(favorites.begin(), favorites.end(), song) != favorites.end()) songData.type = SongType::Favorited;

		songData.displayName = SongListLayer::generateDisplayName(songData);

		if (!queryString.empty()) {
			const bool contains = geode::utils::string::contains(geode::utils::string::toLower(songData.displayName), geode::utils::string::toLower(queryString));
			geode::log::info("songData.displayName: {}", songData.displayName);
			geode::log::info("queryString: {}", queryString);
			geode::log::info("contains: {}", contains);
			geode::log::info("==============================================");
			if (!contains) continue;
		}

		if (MLRSongCell* songCell = MLRSongCell::create(songData, isEven)) {
			scrollLayer->m_contentLayer->addChild(songCell);
			alreadyAdded.push_back(song);
		}
		isEven = !isEven;
	}

	scrollLayer->m_contentLayer->updateLayout();
	scrollLayer->m_contentLayer->setContentHeight((alreadyAdded.size() * 36.f) + 36.f);
	scrollLayer->m_disableMovement = alreadyAdded.size() < 6;
	if (alreadyAdded.size() > 5) scrollLayer->scrollToTop();
	else scrollLayer->m_contentLayer->setPositionY(0.f);

	if (CCNode* scrollBar = this->m_mainLayer->getChildByID("song-list-scrollbar"_spr)) {
		scrollBar->setVisible(alreadyAdded.size() > 5);
	}
}

bool SongListLayer::setup(const std::string&) {
	this->setUserObject("user95401.scrollbar_everywhere/scrollbar", cocos2d::CCBool::create(true)); // fuck off, user95401.
	this->m_noElasticity = true;
	this->setTitle("Menu Loop Randomizer - Your Songs");
	this->m_title->setID("song-list-title"_spr);

	const cocos2d::CCSize layerSize = this->m_mainLayer->getContentSize();
	cocos2d::extension::CCScale9Sprite* background = this->m_bgSprite;
	CCLayer* mainLayer = this->m_mainLayer;
	mainLayer->setID("main-layer"_spr);

	background->initWithFile("GJ_square02.png");
	background->setContentSize(layerSize);

	geode::ScrollLayer* scrollLayer = geode::ScrollLayer::create({356, 220});
	geode::AxisLayout* WHOTHEFUCKMADEIGNOREINVISIBLECHILDRENAVOIDRETURNTYPE = geode::ColumnLayout::create()
		->setAxisReverse(true)
		->setAxisAlignment(geode::AxisAlignment::End)
		->setAutoGrowAxis(220.f)
		->setGap(.0f);
	WHOTHEFUCKMADEIGNOREINVISIBLECHILDRENAVOIDRETURNTYPE->ignoreInvisibleChildren(true);
	scrollLayer->m_contentLayer->setLayout(WHOTHEFUCKMADEIGNOREINVISIBLECHILDRENAVOIDRETURNTYPE);

	SongManager& songManager = SongManager::get();
	SongListLayer::addSongsToScrollLayer(scrollLayer, songManager);

	scrollLayer->setID("list-of-songs"_spr);
	scrollLayer->ignoreAnchorPointForPosition(false);
	this->m_mainLayer->addChildAtPosition(scrollLayer, geode::Anchor::Center);

	// search code UI graciously provided by hiimjasmine00
	cocos2d::CCMenu* searchBarMenu = cocos2d::CCMenu::create();
	searchBarMenu->setContentSize({ 350.f, 35.f });
	searchBarMenu->setPosition({ 35.f, 218.f });
	searchBarMenu->setID("song-list-search-menu"_spr);
	this->m_mainLayer->addChild(searchBarMenu);

	auto searchBackground = CCLayerColor::create({ 194, 114, 62, 255 }, 350.f, 35.f);
	searchBackground->setID("song-list-search-background"_spr);
	searchBarMenu->addChild(searchBackground);

	CCMenuItemSpriteExtra* searchButton = geode::cocos::CCMenuItemExt::createSpriteExtraWithFrameName("gj_findBtn_001.png", 0.7f, [this](auto) {
		CCNode* searchBar = this->m_mainLayer->getChildByIDRecursive("song-list-search-bar"_spr);
		if (!searchBar) return;
		SongListLayer::searchSongs(static_cast<geode::TextInput*>(searchBar)->getString());
	});
	searchButton->setPosition({ 302.f, 17.f });
	searchButton->setID("song-list-search-button"_spr);
	searchBarMenu->addChild(searchButton);

	CCMenuItemSpriteExtra* clearButton = geode::cocos::CCMenuItemExt::createSpriteExtraWithFrameName("GJ_editHSVBtn2_001.png", 0.7f, [this](auto) {
		CCNode* searchBar = this->m_mainLayer->getChildByIDRecursive("song-list-search-bar"_spr);
		if (!searchBar) return;
		static_cast<geode::TextInput*>(searchBar)->setString("", false);
		SongListLayer::searchSongs(static_cast<geode::TextInput*>(searchBar)->getString());
	});
	clearButton->setPosition({ 330.f, 17.f });
	clearButton->setID("song-list-clear-button"_spr);
	searchBarMenu->addChild(clearButton);

	cocos2d::CCSprite* coverItUpSquare = cocos2d::CCSprite::createWithSpriteFrameName("pixelart_base_001.png");
	coverItUpSquare->setColor({155, 154, 155});
	coverItUpSquare->setScale(16.f);
	coverItUpSquare->setID("dont-tell-people-that-my-parent-is-an-edit-hsv-button-lmao"_spr);
	clearButton->addChildAtPosition(coverItUpSquare, geode::Anchor::Center);

	cocos2d::CCSprite* coverItUpX = cocos2d::CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
	coverItUpX->setScale(.6f);
	coverItUpX->setID("dont-tell-people-that-im-not-actually-a-delete-button-lmao"_spr);
	clearButton->addChildAtPosition(coverItUpX, geode::Anchor::Center);

	geode::TextInput* searchBar = geode::TextInput::create(370.f, "Search Songs...");
	searchBar->setTextAlign(geode::TextInputAlign::Left);
	searchBar->setPosition({ 145.f, 17.f });
	searchBar->setScale(.75f);
	searchBar->setID("song-list-search-bar"_spr);
	searchBarMenu->addChild(searchBar);

	CCTextInputNode* inputNode = searchBar->getInputNode();
	inputNode->setLabelPlaceholderScale(.5f);
	inputNode->setMaxLabelScale(.5f);

	CCLayerColor* searchBarDivider = CCLayerColor::create({0, 0, 0, 127});
	searchBarDivider->setContentSize({350.f, .5f});
	searchBarDivider->setAnchorPoint({0.f, 0.f});
	searchBarDivider->setID("song-list-search-divider"_spr);
	searchBarMenu->addChild(searchBarDivider);

	geode::ListBorders* listBorder = geode::ListBorders::create();
	listBorder->setSpriteFrames("GJ_commentTop2_001.png", "GJ_commentSide2_001.png");
	listBorder->setContentSize(scrollLayer->getContentSize());
	listBorder->ignoreAnchorPointForPosition(false);
	listBorder->setID("songs-list-border"_spr);
	this->m_mainLayer->addChildAtPosition(listBorder, geode::Anchor::Center);

	geode::Scrollbar* scrollBar = geode::Scrollbar::create(scrollLayer);
	this->m_mainLayer->addChild(scrollBar);
	scrollBar->setID("song-list-scrollbar"_spr);
	scrollBar->setPositionY(scrollLayer->getPositionY());
	scrollBar->setPositionX(scrollLayer->getPositionX() + (scrollLayer->getContentWidth() / 2.f) + 5.f);
	scrollBar->setVisible(scrollLayer->m_contentLayer->getChildrenCount() > 6);

	cocos2d::CCMenu* infoMenu = cocos2d::CCMenu::create();
	infoMenu->setLayout(
		geode::RowLayout::create()
			->setAutoScale(false)
			->setAxis(geode::Axis::Row)
			->setGap(.0f)
	);
	infoMenu->setContentSize({24.f * .75f, 23.f * .75f});
	InfoAlertButton* infoBtn = InfoAlertButton::create(
		"Menu Loop Randomizer - Help/FAQ",
		"Yellow = <cy>favorited</c> song. Green = <cg>current</c> song.\n"
		"Italicized = NG/ML song from a different path than expected.\n"
		"You can <cl>shuffle songs</c>, <cl>copy the current song's name</c>,\n"
		"<cl>go back one song</c>, and <cl>reopen the control panel</c>.\n\n"
		"<cy>Q: I'm seeing numbers for most of my songs?!</c>\n"
		"A: Someone probably goofed up, which is out of my reach.\n\n"
		"<cy>Q: Add a search bar!</c>\n"
		"A: <c_>No. Never. Learn how to scroll through a list.</c>",
		.75f
	);
	infoMenu->addChildAtPosition(infoBtn, geode::Anchor::Center);
	infoMenu->setPosition({layerSize - 3.f});
	infoMenu->setID("songlayerlist-info-menu"_spr);
	infoBtn->setID("songlayerlist-info-button"_spr);
	this->m_mainLayer->addChild(infoMenu);

	cocos2d::CCMenu* settingsMenu = cocos2d::CCMenu::create();
	settingsMenu->setLayout(
		geode::RowLayout::create()
			->setAutoScale(true)
			->setAxis(geode::Axis::Row)
			->setGap(.0f)
	);
	settingsMenu->setContentSize({24.f, 23.f});
	settingsMenu->setID("songlayerlist-settings-menu"_spr);
	Utils::addButton("settings", menu_selector(SongListLayer::onSettingsButton), settingsMenu, this);
	if (CCMenuItemSpriteExtra* button = settingsMenu->getChildByType<CCMenuItemSpriteExtra>(0)) button->setPosition(settingsMenu->getContentSize() / 2.f);
	settingsMenu->setPosition({395.f, this->m_title->getPositionY()});
	settingsMenu->setScale(.825f);
	this->m_mainLayer->addChild(settingsMenu);

	constexpr float labelWidth = 280.f * .45f;

	const std::string& playlistFileName = songManager.getPlaylistIsEmpty() ? "None" : songManager.getPlaylistFileName();
	cocos2d::CCLabelBMFont* currentPlaylistLabel = cocos2d::CCLabelBMFont::create(fmt::format("Playlist: {}", playlistFileName).c_str(), "bigFont.fnt");
	currentPlaylistLabel->limitLabelWidth(labelWidth, 1.f, .0001f);

	const std::string& loadPlaylistFileInstead = Utils::getBool("loadPlaylistFile") ? "ON" : "OFF";
	cocos2d::CCLabelBMFont* loadPlaylistFileLabel = cocos2d::CCLabelBMFont::create(fmt::format("Load Playlist File: {}", loadPlaylistFileInstead).c_str(), "bigFont.fnt");
	loadPlaylistFileLabel->limitLabelWidth(labelWidth, 1.f, .0001f);

	const std::string& isConstantShuffleMode = songManager.getConstantShuffleMode() ? "ON" : "OFF";
	cocos2d::CCLabelBMFont* constantShuffleModeLabel = cocos2d::CCLabelBMFont::create(fmt::format("Constant Shuffle Mode: {}", isConstantShuffleMode).c_str(), "bigFont.fnt");
	constantShuffleModeLabel->limitLabelWidth(labelWidth, 1.f, .0001f);

	cocos2d::CCLabelBMFont* platformLabel = cocos2d::CCLabelBMFont::create(fmt::format("Platform: {}", Utils::getPlatform()).c_str(), "bigFont.fnt");
	platformLabel->limitLabelWidth(labelWidth, 1.f, .0001f);

	currentPlaylistLabel->setID("current-playlist-label"_spr);
	loadPlaylistFileLabel->setID("load-playlist-file-label"_spr);
	constantShuffleModeLabel->setID("constant-shuffle-mode-label"_spr);
	platformLabel->setID("platform-label"_spr);

	currentPlaylistLabel->ignoreAnchorPointForPosition(false);
	loadPlaylistFileLabel->ignoreAnchorPointForPosition(false);
	constantShuffleModeLabel->ignoreAnchorPointForPosition(false);
	platformLabel->ignoreAnchorPointForPosition(false);

	const float sllWidth = scrollLayer->getContentWidth();
	constexpr float offset = 25.f;
	constexpr float topRowYPos = 27.5f;
	constexpr float bottomRowYPos = 16.5f;
	constexpr float arbitraryPaddingValue = 10.f;
	constexpr float abridgedControlsMenuYPos = (bottomRowYPos + topRowYPos) / 2.f;

	this->m_mainLayer->addChildAtPosition(currentPlaylistLabel, geode::Anchor::BottomLeft, {(layerSize.width - sllWidth) + arbitraryPaddingValue + offset, bottomRowYPos});
	this->m_mainLayer->addChildAtPosition(loadPlaylistFileLabel, geode::Anchor::BottomLeft, {(layerSize.width - sllWidth) + arbitraryPaddingValue + offset, topRowYPos});
	this->m_mainLayer->addChildAtPosition(constantShuffleModeLabel, geode::Anchor::BottomLeft, {sllWidth - offset - arbitraryPaddingValue, bottomRowYPos});
	this->m_mainLayer->addChildAtPosition(platformLabel, geode::Anchor::BottomLeft, {sllWidth - offset - arbitraryPaddingValue, topRowYPos});

	cocos2d::CCMenu* abridgedControlsMenu = cocos2d::CCMenu::create();
	geode::Layout* layout = geode::RowLayout::create()->setDefaultScaleLimits(.0001f, 1.0f)->setGap(1.5f);
	abridgedControlsMenu->setLayout(layout);

	Utils::addButton("shuffle", menu_selector(SongListLayer::onShuffleButton), abridgedControlsMenu, this);
	Utils::addButton("copy", menu_selector(SongListLayer::onCopyButton), abridgedControlsMenu, this);
	Utils::addButton("prev", menu_selector(SongListLayer::onPreviousButton), abridgedControlsMenu, this);
	if (Utils::getString("buttonMode") != "Classic") Utils::addButton("controls", menu_selector(SongListLayer::onControlsButton), abridgedControlsMenu, this);

	abridgedControlsMenu->setPosition({layerSize.width / 2.f, abridgedControlsMenuYPos});
	abridgedControlsMenu->ignoreAnchorPointForPosition(false);
	abridgedControlsMenu->setContentSize({80.f, 24.f});
	abridgedControlsMenu->updateLayout();
	abridgedControlsMenu->setID("abridged-controls-menu"_spr);
	this->m_mainLayer->addChild(abridgedControlsMenu);

	/*
	if (songManager.getSawbladeCustomSongsFolder()) {
		cocos2d::CCLabelBMFont* customSongsFolderBottom = cocos2d::CCLabelBMFont::create("CUSTOM SONGS FOLDER BY SAWBLADE IS LOADED!!! BUGS REPORTS WILL BE IGNORED.", "chatFont.fnt");
		cocos2d::CCLabelBMFont* customSongsFolderTop = cocos2d::CCLabelBMFont::create("CUSTOM SONGS FOLDER BY SAWBLADE IS LOADED!!! BUGS REPORTS WILL BE IGNORED.", "chatFont.fnt");
		customSongsFolderBottom->limitLabelWidth(this->m_mainLayer->getContentWidth(), 1.f, .0001f);
		customSongsFolderTop->limitLabelWidth(this->m_mainLayer->getContentWidth(), 1.f, .0001f);
		const float originalScale = customSongsFolderBottom->getScale();
		cocos2d::CCSequence* sequenceTop = cocos2d::CCSequence::create(
			cocos2d::CCTintTo::create(0.f, 255,   0,   0),
			cocos2d::CCScaleTo::create(0.f, originalScale * 1.1f),
			cocos2d::CCDelayTime::create(0.25f),
			cocos2d::CCTintTo::create(0.f, 255, 255, 255),
			cocos2d::CCScaleTo::create(0.f, originalScale * 1.0f),
			cocos2d::CCDelayTime::create(0.25f),
			nullptr
		);
		cocos2d::CCSequence* sequenceBottom = cocos2d::CCSequence::create(
			cocos2d::CCTintTo::create(0.f, 255, 255, 255),
			cocos2d::CCScaleTo::create(0.f, originalScale * 1.0f),
			cocos2d::CCDelayTime::create(0.25f),
			cocos2d::CCTintTo::create(0.f, 255,   0,   0),
			cocos2d::CCScaleTo::create(0.f, originalScale * 1.1f),
			cocos2d::CCDelayTime::create(0.25f),
			nullptr
		);
		customSongsFolderBottom->runAction(cocos2d::CCRepeatForever::create(sequenceBottom));
		customSongsFolderTop->runAction(cocos2d::CCRepeatForever::create(sequenceTop));
		this->m_mainLayer->addChildAtPosition(customSongsFolderBottom, geode::Anchor::Bottom, {0, -6.f});
		this->m_mainLayer->addChildAtPosition(customSongsFolderTop, geode::Anchor::Top, {0, 6.f});
	}
	*/

	this->setID("SongListLayer"_spr);
	this->m_bgSprite->setID("background"_spr);
	this->m_closeBtn->setID("close-button"_spr);
	this->m_buttonMenu->setID("close-menu"_spr);

	songManager.resetTowerRepeatCount();

	return true;
}

void SongListLayer::searchSongs(const std::string& queryString) {
	CCNode* contentLayer = this->m_mainLayer->getChildByID("list-of-songs"_spr)->getChildByID("content-layer");
	if (!contentLayer || !contentLayer->getLayout()) return;
	contentLayer->removeAllChildrenWithCleanup(true);
	SongListLayer::addSongsToScrollLayer(static_cast<geode::ScrollLayer*>(contentLayer->getParent()), SongManager::get(), queryString);
}

void SongListLayer::onSettingsButton(CCObject*) {
	this->keyBackClicked();
	geode::openSettingsPopup(geode::Mod::get());
}

void SongListLayer::onShuffleButton(CCObject*) {
	SongControl::shuffleSong();
}
void SongListLayer::onCopyButton(CCObject*) {
	SongControl::copySong();
}
void SongListLayer::onPreviousButton(CCObject*) {
	SongControl::previousSong();
}
void SongListLayer::onControlsButton(CCObject*) {
	this->onClose(nullptr);
	SongControlMenu::create("GJ_square05.png")->show();
}

std::string SongListLayer::generateDisplayName(SongData &songData) {
	std::string displayName = geode::utils::string::replace(songData.fileName, songData.fileExtension, "");

	MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
	const int songID = geode::utils::numFromString<int>(displayName).unwrapOr(-1);
	if (songID > 0 && !songData.isFromConfigOrAltDir) {
		if (SongInfoObject* songInfoObject = mdm->getSongInfoObject(songID)) displayName = Utils::getFormattedNGMLSongName(songInfoObject);
		else displayName = fmt::format("{} - No song info found :(", songID);
	}

	return displayName;
}
