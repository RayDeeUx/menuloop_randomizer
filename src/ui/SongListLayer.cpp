#include <Geode/ui/GeodeUI.hpp>
#include "SongListLayer.hpp"
#include "MLRSongCell.hpp"
#include "SongControlMenu.hpp"
#include "../Utils.hpp"
#include "../SongControl.hpp"
#include "../SongManager.hpp"
#ifndef GEODE_IS_IOS
#include <miniaudio.h>
#endif

#define SAVED(key) geode::Mod::get()->getSavedValue<bool>(key, false)

#define GET_SEARCH_BAR_NODE this->m_mainLayer->getChildByIDRecursive(SEARCH_BAR_NODE_ID)
#define GET_SEARCH_STRING static_cast<geode::TextInput*>(searchBar)->getString()
#define EMPTY_SEARCH_STRG static_cast<geode::TextInput*>(searchBar)->setString("", false);
#define LIMIT_PLACEHOLDER SongListLayer::displayCurrentSongByLimitingPlaceholderLabelWidth(static_cast<geode::TextInput*>(GET_SEARCH_BAR_NODE)->getInputNode());

#define SONG_SORTING_DISABLED !SongManager::get().getUndefined0Alk1m123TouchPrio() || !Utils::getBool("showSortSongOptions")
#define SEARCH_BAR_DISABLED !SongManager::get().getUndefined0Alk1m123TouchPrio() || !Utils::getBool("showSearchBar")
#define SEARCH_BAR_ENABLED songManager.getUndefined0Alk1m123TouchPrio() && Utils::getBool("showSearchBar")
#define SONG_SORTING_ENABLED songManager.getUndefined0Alk1m123TouchPrio() && Utils::getBool("showSortSongOptions")

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
	const std::vector<std::string>& blacklist = songManager.getBlacklist();
	const std::vector<std::string>& favorites = songManager.getFavorites();
	std::vector<std::string> alreadyAdded {};
	std::vector<MLRSongCell*> cellsToAdd {};

	const bool songListCompactMode = SONG_SORTING_DISABLED ? false : SAVED("songListCompactMode");
	const bool songListFavoritesOnlyMode = SONG_SORTING_DISABLED ? false : SAVED("songListFavoritesOnlyMode");
	const bool reverse = SONG_SORTING_DISABLED ? false : SAVED("songListReverseSort");

	if (SEARCH_BAR_ENABLED) scrollLayer->m_contentLayer->addChild(MLRSongCell::createEmpty(false)); // intentonally blank song cell for padding to go beneath search bar. 36.f units tall

	const std::vector<std::string>& songsVector = songManager.getSongs();
	float desiredContentHeight = SEARCH_BAR_ENABLED ? 36.f : 0.f; // always start with the height of the blank song cell, which is guaranteed to be 36.f units

	for (const std::string& song : songsVector) {
		if (std::ranges::find(alreadyAdded.begin(), alreadyAdded.end(), song) != alreadyAdded.end()) continue;

		SongType songType = SongType::Regular;
		if (std::ranges::find(blacklist.begin(), blacklist.end(), song) != blacklist.end()) songType = SongType::Blacklisted;
		else if (std::ranges::find(favorites.begin(), favorites.end(), song) != favorites.end()) songType = SongType::Favorited;

		if (songListFavoritesOnlyMode && songType != SongType::Favorited) continue;

		const std::filesystem::path& songFilePath = Utils::toProblematicString(song);
		SongData songData = {
			.actualFilePath = Utils::toNormalizedString(songFilePath),
			.fileExtension = Utils::toNormalizedString(songFilePath.extension()),
			.fileName = Utils::toNormalizedString(songFilePath.filename()),
			.type = songType,
			.isFromConfigOrAltDir = Utils::isFromConfigOrAlternateDir(songFilePath.parent_path()),
			.isEmpty = false
		};

		songData.displayName = SongListLayer::generateDisplayName(songData);

		if (SONG_SORTING_ENABLED && SAVED("songListSortSongLength")) songData.songLength = SongListLayer::getLength(songData.actualFilePath, reverse);

		if (SEARCH_BAR_ENABLED && !queryString.empty()) {
			const bool contains = geode::utils::string::contains(geode::utils::string::toLower(songData.displayName), geode::utils::string::toLower(queryString));
			if (SongManager::get().getAdvancedLogs()) {
				geode::log::info("songData.displayName: {}", songData.displayName);
				geode::log::info("queryString: {}", queryString);
				geode::log::info("contains: {}", contains);
				geode::log::info("==============================================");
			}
			if (!contains) continue;
		}

		if (MLRSongCell* songCell = MLRSongCell::create(songData, false, songListCompactMode)) {
			cellsToAdd.push_back(songCell);
			alreadyAdded.push_back(song);
			desiredContentHeight += songCell->getContentHeight();
		}
	}

	if (SONG_SORTING_ENABLED) {
		if (SAVED("songListSortAlphabetically")) {
			std::sort(cellsToAdd.begin(), cellsToAdd.end(), [reverse](MLRSongCell* a, MLRSongCell* b) {
				return SongListLayer::caseInsensitiveAlphabetical(a, b, reverse);
			});
		} else if (SAVED("songListSortFileSize")) {
			std::sort(cellsToAdd.begin(), cellsToAdd.end(), [reverse](MLRSongCell* a, MLRSongCell* b) {
				return SongListLayer::fileSize(a, b, reverse);
			});
		} else if (SAVED("songListSortDateAdded")) {
			std::sort(cellsToAdd.begin(), cellsToAdd.end(), [reverse](MLRSongCell* a, MLRSongCell* b) {
				return SongListLayer::dateAdded(a, b, reverse);
			});
		} else if (SAVED("songListSortSongLength")) {
			std::sort(cellsToAdd.begin(), cellsToAdd.end(), [reverse](MLRSongCell* a, MLRSongCell* b) {
				return SongListLayer::songLength(a, b, reverse);
			});
		} else if (reverse) {
			std::reverse(cellsToAdd.begin(), cellsToAdd.end());
		}
	}

	bool isEven = SEARCH_BAR_ENABLED ? true : false;
	for (MLRSongCell* cell : cellsToAdd) {
		cell->toggleEven(isEven);
		scrollLayer->m_contentLayer->addChild(cell);
		isEven = !isEven;
	}

	scrollLayer->m_contentLayer->updateLayout();
	scrollLayer->m_contentLayer->setContentHeight(desiredContentHeight);

	const bool tallEnough = SongListLayer::tallEnough(scrollLayer);
	scrollLayer->m_disableMovement = !tallEnough;
	if (tallEnough) scrollLayer->scrollToTop();
	else scrollLayer->m_contentLayer->setPositionY(0.f);

	if (SEARCH_BAR_ENABLED && GET_SEARCH_BAR_NODE && queryString.empty()) LIMIT_PLACEHOLDER

	if (CCNode* scrollBar = this->m_mainLayer->getChildByID("song-list-scrollbar"_spr)) scrollBar->setPositionY(SongListLayer::determineYPosition(scrollLayer));
	if (CCNode* scrollShortcuts = this->m_mainLayer->getChildByID("scroll-shortcuts-menu"_spr)) scrollShortcuts->setPositionY(SongListLayer::determineYPosition(scrollLayer));
}

bool SongListLayer::setup(const std::string&) {
	this->setUserObject("user95401.scrollbar_everywhere/scrollbar", cocos2d::CCBool::create(true)); // fuck off, user95401.
	this->m_noElasticity = true;

	SongManager& songManager = SongManager::get();
	const int songCount = songManager.getSongsSize();
	this->setTitle(fmt::format("Menu Loop Randomizer - Your {} Song{}", songCount, (songCount == 1 ? "" : "s")));
	this->m_title->setID("song-list-title"_spr);
	this->m_title->limitLabelWidth(320.f, 1.f, .0001f);

	const cocos2d::CCSize layerSize = this->m_mainLayer->getContentSize();
	CCLayer* mainLayer = this->m_mainLayer;
	mainLayer->setID("main-layer"_spr);

	cocos2d::extension::CCScale9Sprite* newBG = cocos2d::extension::CCScale9Sprite::create("GJ_square02.png");
	newBG->ignoreAnchorPointForPosition(this->m_bgSprite->isIgnoreAnchorPointForPosition());
	newBG->setPosition(this->m_bgSprite->getPosition());
	newBG->setContentSize(layerSize);
	this->m_mainLayer->addChild(newBG);
	this->m_bgSprite->removeMeAndCleanup();
	this->m_bgSprite = newBG;

	geode::ScrollLayer* scrollLayer = geode::ScrollLayer::create({356.f, 220.f});
	scrollLayer->m_contentLayer->setLayout(geode::ColumnLayout::create()->setAxisReverse(true)->setAxisAlignment(geode::AxisAlignment::End)->setAutoGrowAxis(std::make_optional<float>(220.f))->setGap(.0f));
	scrollLayer->ignoreAnchorPointForPosition(false);
	scrollLayer->setID("list-of-songs"_spr);
	this->m_mainLayer->addChildAtPosition(scrollLayer, geode::Anchor::Center);

	SongListLayer::addSongsToScrollLayer(scrollLayer, songManager);

	if (SEARCH_BAR_ENABLED) {
		// search code UI graciously provided by hiimjasmine00
		cocos2d::CCMenu* searchBarMenu = cocos2d::CCMenu::create();
		searchBarMenu->setContentSize({350.f, 35.f});
		searchBarMenu->setPosition({35.f, 218.f});
		searchBarMenu->setID("song-list-search-menu"_spr);
		this->m_mainLayer->addChild(searchBarMenu);

		CCLayerColor* searchBackground = CCLayerColor::create({194, 114, 62, 255}, 350.f, 35.f);
		searchBackground->setID("song-list-search-background"_spr);
		searchBarMenu->addChild(searchBackground);

		CCMenuItemSpriteExtra* searchButton = geode::cocos::CCMenuItemExt::createSpriteExtraWithFrameName("gj_findBtn_001.png", 0.7f, [this](auto) {
			CCNode* searchBar = GET_SEARCH_BAR_NODE;
			if (!searchBar || (searchBar->getTag() == -1 && GET_SEARCH_STRING.empty())) return;
			const std::string& queryString = GET_SEARCH_STRING;
			searchBar->setTag(queryString.empty() ? -1 : 12202025);
			SongListLayer::searchSongs(queryString);
		});
		searchButton->setPosition({302.f, 17.f});
		searchButton->setID("song-list-search-button"_spr);
		searchBarMenu->addChild(searchButton);

		CCMenuItemSpriteExtra* clearButton = geode::cocos::CCMenuItemExt::createSpriteExtraWithFrameName("GJ_editHSVBtn2_001.png", 0.7f, [this](auto) {
			CCNode* searchBar = GET_SEARCH_BAR_NODE;
			if (!searchBar || (searchBar->getTag() == -1 && GET_SEARCH_STRING.empty())) return;
			EMPTY_SEARCH_STRG
			searchBar->setTag(-1);
			SongListLayer::searchSongs("");
		});
		clearButton->setPosition({330.f, 17.f});
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

		geode::TextInput* searchBar = geode::TextInput::create(370.f, fmt::format("Search... (Current Song: {})", songManager.getCurrentSongDisplayName()));
		searchBar->setCommonFilter(geode::CommonFilter::Any);
		searchBar->setTextAlign(geode::TextInputAlign::Left);
		searchBar->setPosition({145.f, 17.f});
		searchBar->setScale(.75f);
		searchBar->setID(SEARCH_BAR_NODE_ID);
		searchBarMenu->addChild(searchBar);

		CCTextInputNode* inputNode = searchBar->getInputNode();
		inputNode->setLabelPlaceholderScale(.5f);
		inputNode->setMaxLabelScale(.5f);
		inputNode->setID("song-list-search-bar-input-node"_spr);

		SongListLayer::displayCurrentSongByLimitingPlaceholderLabelWidth(inputNode);

		CCLayerColor* searchBarDivider = CCLayerColor::create({0, 0, 0, 127});
		searchBarDivider->setContentSize({350.f, .5f});
		searchBarDivider->setAnchorPoint({0.f, 0.f});
		searchBarDivider->setID("song-list-search-divider"_spr);
		searchBarMenu->addChild(searchBarDivider);
	}

	geode::ListBorders* listBorder = geode::ListBorders::create();
	listBorder->setSpriteFrames("GJ_commentTop2_001.png", "GJ_commentSide2_001.png");
	listBorder->setContentSize(scrollLayer->getContentSize());
	listBorder->ignoreAnchorPointForPosition(false);
	listBorder->setID("songs-list-border"_spr);
	this->m_mainLayer->addChildAtPosition(listBorder, geode::Anchor::Center);

	if (Utils::getBool("showScrollingShortcuts")) {
		cocos2d::CCMenu* scrollShortcutsMenu = cocos2d::CCMenu::create();
		scrollShortcutsMenu->setLayout(geode::ColumnLayout::create()->setDefaultScaleLimits(.0001f, 1.0f)->setGap(600.f)->setAxisReverse(true)->setAutoScale(true));
		Utils::addButton("scroll-top", menu_selector(SongListLayer::onScrollTopButton), scrollShortcutsMenu, this, true);
		Utils::addButton("scroll-cur", menu_selector(SongListLayer::onScrollCurButton), scrollShortcutsMenu, this, true);
		Utils::addButton("scroll-btm", menu_selector(SongListLayer::onScrollBtmButton), scrollShortcutsMenu, this, true);

		scrollShortcutsMenu->setPosition({405.f, SongListLayer::determineYPosition(scrollLayer)});
		scrollShortcutsMenu->ignoreAnchorPointForPosition(false);
		scrollShortcutsMenu->setContentHeight(220.f);
		scrollShortcutsMenu->updateLayout();
		scrollShortcutsMenu->setID("scroll-shortcuts-menu"_spr);
		this->m_mainLayer->addChild(scrollShortcutsMenu);
	}

	geode::Scrollbar* scrollBar = geode::Scrollbar::create(scrollLayer);
	scrollBar->setPosition({scrollLayer->getPositionX() + (scrollLayer->getContentWidth() / 2.f) + 5.f, SongListLayer::determineYPosition(scrollLayer)});
	scrollBar->setID("song-list-scrollbar"_spr);
	this->m_mainLayer->addChild(scrollBar);

	if (SONG_SORTING_ENABLED) {
		cocos2d::CCMenu* viewModeMenu = cocos2d::CCMenu::create();

		Utils::addViewModeToggle(SAVED("songListCompactMode"), "GJ_smallModeIcon_001.png", "compact-mode", menu_selector(SongListLayer::onCompactModeToggle), viewModeMenu, this);
		Utils::addViewModeToggle(SAVED("songListFavoritesOnlyMode"), "favorites.png"_spr, "favorites-only", menu_selector(SongListLayer::onFavoritesOnlyToggle), viewModeMenu, this);
		Utils::addViewModeToggle(SAVED("songListReverseSort"), "reverse.png"_spr, "reverse-list", menu_selector(SongListLayer::onSortReverseToggle), viewModeMenu, this);
		Utils::addViewModeToggle(SAVED("songListSortAlphabetically"), "abc.png"_spr, "alphabetical", menu_selector(SongListLayer::onSortABCToggle), viewModeMenu, this);
		Utils::addViewModeToggle(SAVED("songListSortDateAdded"), "dates.png"_spr, "date-added", menu_selector(SongListLayer::onSortDateToggle), viewModeMenu, this);
		if (Utils::getBool("showSortSongLength")) Utils::addViewModeToggle(SAVED("songListSortSongLength"), "length.png"_spr, "song-length", menu_selector(SongListLayer::onSortLengthToggle), viewModeMenu, this);
		Utils::addViewModeToggle(SAVED("songListSortFileSize"), "size.png"_spr, "song-size", menu_selector(SongListLayer::onSortSizeToggle), viewModeMenu, this);

		viewModeMenu->setContentHeight(viewModeMenu->getChildrenCount() * 30.f);
		viewModeMenu->ignoreAnchorPointForPosition(false);
		viewModeMenu->setPosition({19.f, scrollLayer->getPositionY()});
		viewModeMenu->setLayout(geode::ColumnLayout::create()->setAxisReverse(true));
		viewModeMenu->setID("view-mode-menu"_spr);
		this->m_mainLayer->addChild(viewModeMenu);
	}

	cocos2d::CCMenu* infoMenu = cocos2d::CCMenu::create();
	infoMenu->setContentSize({24.f * .75f, 23.f * .75f});
	InfoAlertButton* infoBtn = InfoAlertButton::create(
		"Menu Loop Randomizer - Help/FAQ",
		"Yellow = <cy>favorited</c> song. Green = <cg>current</c> song.\n"
		"Italicized = NG/ML song from unexpected path. Gradient = Jukebox (NONG).\n"
		"You can <cl>shuffle songs</c>, <cl>copy the current song's name</c>,\n"
		"<cl>go back one song</c>, and <cl>reopen the control panel</c>.\n"
		"<cy>Q: I'm seeing numbers for most of my songs?!</c>\n"
		"A: Someone probably goofed up, which is out of my reach.\n"
		"<cy>Q: Make this mod act more closely to Spotify!</c>\n"
		"A: Someone probably goofed up, which is out of my reach.\n"
		"<cy>Q: Where is the search bar and song sorting options?\n"
		"A: <c_>Please install \"Better Touch Prio\" by alk1m123 and undefined0.</c>",
		.75f
	);
	infoMenu->addChildAtPosition(infoBtn, geode::Anchor::Center);
	infoMenu->setPosition({layerSize - 3.f});
	infoMenu->setLayout(geode::RowLayout::create()->setAutoScale(false)->setAxis(geode::Axis::Row)->setGap(.0f));
	infoMenu->setID("songlayerlist-info-menu"_spr);
	infoBtn->setID("songlayerlist-info-button"_spr);
	this->m_mainLayer->addChild(infoMenu);

	cocos2d::CCMenu* settingsMenu = cocos2d::CCMenu::create();
	settingsMenu->setLayout(geode::RowLayout::create()->setAutoScale(true)->setAxis(geode::Axis::Row)->setGap(.0f));
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

	Utils::addButton("shuffle", menu_selector(SongListLayer::onShuffleButton), abridgedControlsMenu, this);
	Utils::addButton("copy", menu_selector(SongListLayer::onCopyButton), abridgedControlsMenu, this);
	Utils::addButton("prev", menu_selector(SongListLayer::onPreviousButton), abridgedControlsMenu, this);
	if (Utils::getString("buttonMode") != "Classic") Utils::addButton("controls", menu_selector(SongListLayer::onControlsButton), abridgedControlsMenu, this);

	abridgedControlsMenu->setPosition({layerSize.width / 2.f, abridgedControlsMenuYPos});
	abridgedControlsMenu->ignoreAnchorPointForPosition(false);
	abridgedControlsMenu->setContentSize({80.f, 24.f});
	abridgedControlsMenu->setLayout(geode::RowLayout::create()->setDefaultScaleLimits(.0001f, 1.0f)->setGap(1.5f));
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

	if (SEARCH_BAR_ENABLED) this->scheduleUpdate();

	return true;
}

void SongListLayer::searchSongs(const std::string& queryString) {
	if (SONG_SORTING_DISABLED && SEARCH_BAR_DISABLED) return;
	CCNode* contentLayer = SongListLayer::getContentLayer();
	if (!contentLayer || !contentLayer->getLayout()) return;
	contentLayer->removeAllChildrenWithCleanup(true);
	SongListLayer::addSongsToScrollLayer(static_cast<geode::ScrollLayer*>(contentLayer->getParent()), SongManager::get(), queryString);
}

void SongListLayer::onSettingsButton(CCObject*) {
	this->onClose(nullptr);
	geode::openSettingsPopup(geode::Mod::get());
}

void SongListLayer::onShuffleButton(CCObject*) {
	SongControl::shuffleSong();
	if (GET_SEARCH_BAR_NODE) LIMIT_PLACEHOLDER
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

CCContentLayer* SongListLayer::getContentLayer() const {
	CCNode* scrollLayer = this->m_mainLayer->getChildByID("list-of-songs"_spr);
	if (scrollLayer) return static_cast<geode::ScrollLayer*>(scrollLayer)->m_contentLayer;
	return nullptr;
}

void SongListLayer::onScrollTopButton(CCObject*) {
	if (!Utils::getBool("showScrollingShortcuts")) return;
	CCContentLayer* contentLayer = SongListLayer::getContentLayer();
	if (!contentLayer || !SongListLayer::tallEnough(static_cast<geode::ScrollLayer*>(contentLayer->getParent()))) return; // cmon bruh it's in plain sight lol
	contentLayer->setPositionY((contentLayer->getContentHeight() * -1.f) + contentLayer->getParent()->getContentHeight());
}

void SongListLayer::onScrollCurButton(CCObject*) {
	if (!Utils::getBool("showScrollingShortcuts")) return;
	CCContentLayer* contentLayer = SongListLayer::getContentLayer();
	if (!contentLayer || !SongListLayer::tallEnough(static_cast<geode::ScrollLayer*>(contentLayer->getParent()))) return; // cmon bruh it's in plain sight lol
	CCNode* currentCell = contentLayer->getChildByTag(12192025);
	const float theAbsolluteTop = contentLayer->getContentHeight() * -1.f + contentLayer->getParent()->getContentHeight();
	const float centerOrCurrent = (cocos2d::CCKeyboardDispatcher::get()->getShiftKeyPressed() || !currentCell) ? ((contentLayer->getContentHeight() + contentLayer->getParent()->getContentHeight()) * -1.f * .5f) : ((currentCell->getPositionY() * -1.f) - (contentLayer->getParent()->getContentHeight() / 2.f) + 20.f);
	const float desiredPosition = centerOrCurrent + contentLayer->getParent()->getContentHeight();
	if (desiredPosition > 0.f) contentLayer->setPositionY(0.f);
	else if (desiredPosition < theAbsolluteTop) contentLayer->setPositionY(theAbsolluteTop);
	else contentLayer->setPositionY(desiredPosition);
}

void SongListLayer::onScrollBtmButton(CCObject*) {
	if (!Utils::getBool("showScrollingShortcuts")) return;
	CCContentLayer* contentLayer = SongListLayer::getContentLayer();
	if (!contentLayer || !SongListLayer::tallEnough(static_cast<geode::ScrollLayer*>(contentLayer->getParent()))) return; // cmon bruh it's in plain sight lol
	contentLayer->setPositionY(0.f);
}

void SongListLayer::onCompactModeToggle(CCObject*) {
	if (SONG_SORTING_DISABLED) return;
	SongListLayer::toggleSavedValueAndSearch("songListCompactMode");
}

void SongListLayer::onFavoritesOnlyToggle(CCObject*) {
	if (SONG_SORTING_DISABLED) return;
	SongListLayer::toggleSavedValueAndSearch("songListFavoritesOnlyMode");
}

void SongListLayer::onSortReverseToggle(CCObject*) {
	if (SONG_SORTING_DISABLED) return;
	SongListLayer::toggleSavedValueAndSearch("songListReverseSort");
}

void SongListLayer::onSortABCToggle(CCObject*) {
	if (SONG_SORTING_DISABLED) return;
	SongListLayer::disableAllSortFiltersThenToggleThenSearch("songListSortAlphabetically");
}

void SongListLayer::onSortDateToggle(CCObject*) {
	if (SONG_SORTING_DISABLED) return;
	SongListLayer::disableAllSortFiltersThenToggleThenSearch("songListSortDateAdded");
}

void SongListLayer::onSortLengthToggle(CCObject*) {
	if (SONG_SORTING_DISABLED) return;
	SongListLayer::disableAllSortFiltersThenToggleThenSearch("songListSortSongLength");
}

void SongListLayer::onSortSizeToggle(CCObject*) {
	if (SONG_SORTING_DISABLED) return;
	SongListLayer::disableAllSortFiltersThenToggleThenSearch("songListSortFileSize");
}

void SongListLayer::disableAllSortFiltersThenToggleThenSearch(const std::string_view savedValueKey) {
	if (SONG_SORTING_DISABLED) return;
	cocos2d::CCNode* viewModeMenu = this->m_mainLayer->getChildByID("view-mode-menu"_spr);
	if (!viewModeMenu) return;
	const bool originalSavedValue = SAVED(savedValueKey);
	geode::Mod::get()->setSavedValue<bool>("songListSortAlphabetically", false);
	geode::Mod::get()->setSavedValue<bool>("songListSortSongLength", false);
	geode::Mod::get()->setSavedValue<bool>("songListSortDateAdded", false);
	geode::Mod::get()->setSavedValue<bool>("songListSortFileSize", false);
	if (const auto toggler = static_cast<CCMenuItemToggler*>(viewModeMenu->getChildByID("alphabetical-button"_spr)); toggler) {
		toggler->toggle(false);
		if (savedValueKey == "songListSortAlphabetically") toggler->toggle(originalSavedValue);
	}
	if (const auto toggler = static_cast<CCMenuItemToggler*>(viewModeMenu->getChildByID("date-added-button"_spr)); toggler) {
		toggler->toggle(false);
		if (savedValueKey == "songListSortDateAdded") toggler->toggle(originalSavedValue);
	}
	if (const auto toggler = static_cast<CCMenuItemToggler*>(viewModeMenu->getChildByID("song-length-button"_spr)); toggler) {
		toggler->toggle(false);
		if (savedValueKey == "songListSortSongLength") toggler->toggle(originalSavedValue);
	}
	if (const auto toggler = static_cast<CCMenuItemToggler*>(viewModeMenu->getChildByID("song-size-button"_spr)); toggler) {
		toggler->toggle(false);
		if (savedValueKey == "songListSortFileSize") toggler->toggle(originalSavedValue);
	}
	geode::Mod::get()->setSavedValue<bool>(savedValueKey, !originalSavedValue);
	CCNode* searchBar = GET_SEARCH_BAR_NODE;
	SongListLayer::searchSongs(!searchBar ? "" : GET_SEARCH_STRING);
}

void SongListLayer::toggleSavedValueAndSearch(const std::string_view savedValueKey) {
	if (SONG_SORTING_DISABLED) return;
	const bool originalSavedValue = SAVED(savedValueKey);
	geode::Mod::get()->setSavedValue<bool>(savedValueKey, !originalSavedValue);
	CCNode* searchBar = GET_SEARCH_BAR_NODE;
	SongListLayer::searchSongs(!searchBar ? "" : GET_SEARCH_STRING);
}

void SongListLayer::keyDown(const cocos2d::enumKeyCodes key) {
	// this is fine since searchbar swallows delete (macos)/backspace (all other platforms) key inputs first
	if (SEARCH_BAR_DISABLED || (key != cocos2d::KEY_Enter && key != cocos2d::KEY_Delete && key != cocos2d::KEY_Backspace)) {
		// code taken directly from geode::Popup keyDown impl as of dec 19 2025
		if (key == cocos2d::enumKeyCodes::KEY_Escape) return this->onClose(nullptr);
		if (key == cocos2d::enumKeyCodes::KEY_Space) return;
		return FLAlertLayer::keyDown(key);
	}
	CCNode* searchBar = GET_SEARCH_BAR_NODE;
	if (!searchBar || (GET_SEARCH_STRING.empty() && searchBar->getTag() == -1)) return;
	if (key != cocos2d::KEY_Enter) EMPTY_SEARCH_STRG // clear search query before re-populating
	const std::string& queryString = GET_SEARCH_STRING;
	searchBar->setTag(queryString.empty() ? -1 : 12202025);
	SongListLayer::searchSongs(queryString);
}

std::string SongListLayer::generateDisplayName(SongData& songData) {
	if (!songData.displayName.empty()) return songData.displayName;

	const std::string& displayName = geode::utils::string::replace(songData.fileName, songData.fileExtension, "");
	const int songID = geode::utils::numFromString<int>(displayName).unwrapOr(-1);
	if (songID > 0 && !songData.isFromConfigOrAltDir) {
		MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
		if (SongInfoObject* songInfoObject = mdm->getSongInfoObject(songID)) return Utils::getFormattedNGMLSongName(songInfoObject);
		return fmt::format("{} - No song info found :(", songID);
	}

	return displayName;
}

bool SongListLayer::tallEnough(geode::ScrollLayer* scrollLayer) {
	return scrollLayer->m_contentLayer->getContentHeight() > scrollLayer->getContentHeight();
}

float SongListLayer::determineYPosition(geode::ScrollLayer* scrollLayer) {
	return SongListLayer::tallEnough(scrollLayer) ? 145.f : 99999.f;
}

void SongListLayer::displayCurrentSongByLimitingPlaceholderLabelWidth(CCTextInputNode* inputNode, const bool updateString) {
	if (SEARCH_BAR_DISABLED) return;
	if (!inputNode) return;
	cocos2d::CCLabelBMFont* placeholderLabelMaybe = static_cast<cocos2d::CCLabelBMFont*>(inputNode->getChildByTag(12242025));
	if (!placeholderLabelMaybe) placeholderLabelMaybe = inputNode->getChildByType<cocos2d::CCLabelBMFont>(0);
	if (!placeholderLabelMaybe || placeholderLabelMaybe->getColor() != cocos2d::ccColor3B{150, 150, 150}) return;
	if (updateString) placeholderLabelMaybe->setString(fmt::format("Search... (Current Song: {})", SongManager::get().getCurrentSongDisplayName()).c_str());
	placeholderLabelMaybe->limitLabelWidth(350.f, 1.f, .0001f);
	placeholderLabelMaybe->setTag(12242025);
}

bool SongListLayer::caseInsensitiveAlphabetical(MLRSongCell* a, MLRSongCell* b, const bool reverse = false) {
	if (SONG_SORTING_DISABLED) return false;
	const std::string& cleanedUpA = SongListLayer::generateDisplayName(a->m_songData);
	const std::string& cleanedUpB = SongListLayer::generateDisplayName(b->m_songData);
	auto it1 = cleanedUpA.begin(), it2 = cleanedUpB.begin();
	while (it1 != cleanedUpA.end() && it2 != cleanedUpB.end()) {
		unsigned char c1 = static_cast<unsigned char>(*it1++);
		unsigned char c2 = static_cast<unsigned char>(*it2++);
		char lc1 = static_cast<char>(std::tolower(c1));
		char lc2 = static_cast<char>(std::tolower(c2));
		if (lc1 < lc2) return !reverse;
		if (lc1 > lc2) return reverse;
	}
	if (cleanedUpA.size() < cleanedUpB.size()) return !reverse;
	if (cleanedUpA.size() > cleanedUpB.size()) return reverse;
	return false;
}

bool SongListLayer::fileSize(MLRSongCell* a, MLRSongCell* b, const bool reverse = false) {
	if (SONG_SORTING_DISABLED) return false;
	std::error_code ec;
	std::uintmax_t fileSizeA = std::filesystem::file_size(Utils::toProblematicString(a->m_songData.actualFilePath), ec);
	fileSizeA = ec ? std::numeric_limits<std::uintmax_t>::max() : fileSizeA;
	std::uintmax_t fileSizeB = std::filesystem::file_size(Utils::toProblematicString(b->m_songData.actualFilePath), ec);
	fileSizeB = ec ? std::numeric_limits<std::uintmax_t>::max() : fileSizeB;
	if (fileSizeA < fileSizeB) return !reverse;
	if (fileSizeA > fileSizeB) return reverse;
	return a->m_songData.actualFilePath < b->m_songData.actualFilePath;
}

bool SongListLayer::dateAdded(MLRSongCell* a, MLRSongCell* b, const bool reverse = false) {
	if (SONG_SORTING_DISABLED) return false;
	std::error_code ea, eb;
	auto ta = std::filesystem::last_write_time(Utils::toProblematicString(a->m_songData.actualFilePath), ea);
	auto tb = std::filesystem::last_write_time(Utils::toProblematicString(b->m_songData.actualFilePath), eb);
	if (ea || eb) {
		if (ea && eb) return a->m_songData.actualFilePath < b->m_songData.actualFilePath;
		if (ea) return true;
		if (eb) return false;
	}
	if (reverse) return tb < ta;
	return ta < tb;
}

bool SongListLayer::songLength(MLRSongCell* a, MLRSongCell* b, const bool reverse = false) {
	if (SONG_SORTING_DISABLED) return false;
	if (a->m_songData.songLength < b->m_songData.songLength) return !reverse;
	if (a->m_songData.songLength > b->m_songData.songLength) return reverse;
	return a->m_songData.actualFilePath < b->m_songData.actualFilePath;
}

unsigned int SongListLayer::getLength(const std::string& path, const bool reverse) {
	const unsigned int extreme = reverse ? std::numeric_limits<unsigned int>::min() : std::numeric_limits<unsigned int>::max();
	if (SONG_SORTING_DISABLED) return extreme;
	#ifndef GEODE_IS_IOS
	ma_decoder decoder;
	if (ma_decoder_init_file(path.c_str(), nullptr, &decoder) != MA_SUCCESS) return extreme;

	ma_uint64 frames = 0;
	if (ma_decoder_get_length_in_pcm_frames(&decoder, &frames) != MA_SUCCESS) {
		ma_decoder_uninit(&decoder);
		return extreme;
	}

	const ma_uint32 sampleRate = decoder.outputSampleRate;
	ma_decoder_uninit(&decoder);

	if (sampleRate == 0) return extreme;
	return static_cast<unsigned int>(static_cast<double>(frames) / static_cast<double>(sampleRate));
	#else
	FMOD::System* sys = FMODAudioEngine::get()->m_system;
	if (!sys) return extreme;
	unsigned int temp = extreme;
	FMOD::Sound* sound;
	FMOD_RESULT resultSoundA = sys->createSound(path.c_str(), FMOD_OPENONLY | FMOD_2D, nullptr, &sound);
	if (sound && resultSoundA == FMOD_OK) {
		sound->getLength(&temp, FMOD_TIMEUNIT_MS);
		sound->release();
		return temp;
	}
	return extreme;
	#endif
}

void SongListLayer::update(float) {
	if (SEARCH_BAR_DISABLED) return;
	SongListLayer::displayCurrentSongByLimitingPlaceholderLabelWidth(static_cast<geode::TextInput*>(GET_SEARCH_BAR_NODE)->getInputNode());
}