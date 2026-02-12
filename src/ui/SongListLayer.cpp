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

#define GET_SEARCH_BAR_NODE this->m_searchBar
#define GET_SEARCH_STRING m_searchBar->getString()
#define EMPTY_SEARCH_STRG m_searchBar->setString("", false);
#define LIMIT_PLACEHOLDER SongListLayer::displayCurrentSongByLimitingPlaceholderLabelWidth(this->m_searchBar->getInputNode());

#define SONG_SORTING_DISABLED !SongManager::get().getUndefined0Alk1m123TouchPrio() || !Utils::getBool("showSortSongOptions")
#define SEARCH_BAR_DISABLED !SongManager::get().getUndefined0Alk1m123TouchPrio() || !Utils::getBool("showSearchBar")
#define SEARCH_BAR_ENABLED songManager.getUndefined0Alk1m123TouchPrio() && Utils::getBool("showSearchBar")
#define SONG_SORTING_ENABLED songManager.getUndefined0Alk1m123TouchPrio() && Utils::getBool("showSortSongOptions")

SongListLayer* SongListLayer::create() {
	SongListLayer* ret = new SongListLayer();
	if (ret->initAnchored(420.f, 290.f, "GJ_square02.png")) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

void SongListLayer::addSongsToScrollLayer(geode::ScrollLayer* scrollLayer, SongManager& songManager, const std::string& queryString) {
	std::vector<std::string_view> alreadyAdded {};
	std::vector<MLRSongCell*> cellsToAdd {};

	const bool songListCompactMode = SONG_SORTING_DISABLED ? false : SAVED("songListCompactMode");
	const bool songListFavoritesOnlyMode = SONG_SORTING_DISABLED ? false : SAVED("songListFavoritesOnlyMode");
	const bool reverse = SONG_SORTING_DISABLED ? false : SAVED("songListReverseSort");

	if (SEARCH_BAR_ENABLED) scrollLayer->m_contentLayer->addChild(MLRSongCell::createEmpty(false)); // intentonally blank song cell for padding to go beneath search bar. 36.f units tall

	const std::vector<std::string>& songsVector = songManager.getSongs();
	float desiredContentHeight = SEARCH_BAR_ENABLED ? 36.f : 0.f; // always start with the height of the blank song cell, which is guaranteed to be 36.f units

	const SongToSongData& songToSongData = songManager.getSongToSongDataEntries();
	std::error_code ec, ed;

	const std::vector<std::string>& blacklist = songManager.getBlacklist();
	const std::vector<std::string>& favorites = songManager.getFavorites();

	MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
	const std::string& dummyJukeboxPath = Utils::toNormalizedString(geode::dirs::getModsSaveDir() / "fleym.nongd" / "nongs");
	const std::string& hypotheticalPathWhereMDMSongsAreStored = Utils::toNormalizedString(Utils::toProblematicString(mdm->pathForSong(593059)).parent_path()); // this song ID in particular is banned from newgrounds

	for (const std::string_view song : songsVector) {
		if (std::ranges::find(alreadyAdded.begin(), alreadyAdded.end(), song) != alreadyAdded.end()) continue;

		SongData songData {};
		bool songDataFromTheMap = false;
		const std::filesystem::path& songFilePath = Utils::toProblematicString(song);
		auto songDataIterator = songToSongData.find(songFilePath);
		if (songDataIterator != songToSongData.end()) {
			songData = songDataIterator->second;
			songDataFromTheMap = true;
		} else {
			SongType songType = SongType::Regular;
			if (std::ranges::find(blacklist.begin(), blacklist.end(), song) != blacklist.end()) songType = SongType::Blacklisted;
			else if (std::ranges::find(favorites.begin(), favorites.end(), song) != favorites.end()) songType = SongType::Favorited;

			const std::filesystem::path& theirPath = Utils::toProblematicString(song);
			const std::filesystem::path& theirParentPath = theirPath.parent_path();
			const int songID = geode::utils::numFromString<int>(Utils::toNormalizedString(theirPath.stem())).unwrapOr(-1);

			std::uintmax_t fileSize = std::filesystem::file_size(theirPath, ec);
			std::filesystem::file_time_type fileTime = std::filesystem::last_write_time(theirPath, ed);
			const bool isFromConfigOrAltDirWithoutMDMCheck = Utils::isFromConfigOrAlternateDir(theirParentPath);

			SongInfoObject* songInfoObject = mdm->getSongInfoObject(songID);
			const bool isInNonVanillaNGMLSongLocation = songInfoObject && !geode::utils::string::contains(hypotheticalPathWhereMDMSongsAreStored, Utils::toNormalizedString(theirParentPath));

			songData = {
				.actualFilePath = std::string(song),
				.fileExtension = Utils::toNormalizedString(theirPath.extension()),
				.fileName = Utils::toNormalizedString(theirPath.filename()),
				.type = songType, .songFileSize = ec ? std::numeric_limits<std::uintmax_t>::max() : fileSize,
				.songWriteTime = ed ? std::filesystem::file_time_type::min() : fileTime,
				.hashedPath = std::hash<std::string>{}(std::string(song)),
				.isFromConfigOrAltDir = isFromConfigOrAltDirWithoutMDMCheck,
				.couldPossiblyExistInMusicDownloadManager = songID > -1 && songInfoObject,
				.isInNonVanillaNGMLSongLocation = isInNonVanillaNGMLSongLocation,
				.isFromJukeboxDirectory = geode::utils::string::contains(std::string(song), dummyJukeboxPath),
				.isEmpty = false
			};
			if (!isFromConfigOrAltDirWithoutMDMCheck && songData.couldPossiblyExistInMusicDownloadManager && songInfoObject && std::filesystem::exists(geode::dirs::getModsSaveDir() / "fleym.nongd" / "manifest" / fmt::format("{}.json", songID)) && Utils::adjustSongInfoIfJukeboxReplacedIt(songInfoObject)) {
				songData.displayName = Utils::getFormattedNGMLSongName(songInfoObject);
			}
			songData.displayName = SongListLayer::generateDisplayName(songData);

			std::chrono::system_clock::time_point timepoint = std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(songData.songWriteTime.time_since_epoch()));
			auto datepoint = floor<std::chrono::days>(floor<std::chrono::seconds>(timepoint));
			const std::chrono::year_month_day yearMonthDate{std::chrono::sys_days{datepoint}};
			songData.dateTimeText = fmt::format("{} {:02}, {:04}", months[static_cast<unsigned>(yearMonthDate.month()) - 1], static_cast<unsigned>(yearMonthDate.day()), static_cast<int>(yearMonthDate.year()));
			songData.extraInfoText = fmt::format("{} | {:.2f} MB | {}", songData.fileExtension, songData.songFileSize / 1000000.f, songData.dateTimeText).c_str();
		}

		if (songData.type == SongType::Blacklisted) continue;
		if (songListFavoritesOnlyMode && songData.type != SongType::Favorited) continue;

		if (SONG_SORTING_ENABLED && SAVED("songListSortSongLength") && !songDataFromTheMap) songData.songLength = SongListLayer::getLength(songData.actualFilePath, reverse);

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
		} else if (SAVED("songListSortFileExtn")) {
			std::sort(cellsToAdd.begin(), cellsToAdd.end(), [reverse](MLRSongCell* a, MLRSongCell* b) {
				return SongListLayer::fileExtn(a, b, reverse);
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

	scrollLayer->m_contentLayer->addChild(MLRSongCell::createEmpty(true)); // intentonally blank song cell for padding for progress bar
	desiredContentHeight += 3.5f;

	scrollLayer->m_contentLayer->updateLayout();
	scrollLayer->m_contentLayer->setContentHeight(desiredContentHeight);

	const bool tallEnough = SongListLayer::tallEnough(scrollLayer);
	scrollLayer->m_disableMovement = !tallEnough;
	if (tallEnough) scrollLayer->scrollToTop();
	else scrollLayer->m_contentLayer->setPositionY(0.f);

	if (SEARCH_BAR_ENABLED && GET_SEARCH_BAR_NODE && queryString.empty()) LIMIT_PLACEHOLDER

	if (this->m_scrollBar) this->m_scrollBar->setPositionY(SongListLayer::determineYPosition(scrollLayer));
	if (this->m_scrollShortcuts) this->m_scrollShortcuts->setPositionY(SongListLayer::determineYPosition(scrollLayer));
}

bool SongListLayer::setup() {
	this->setUserObject("user95401.scrollbar_everywhere/scrollbar", cocos2d::CCBool::create(true)); // fuck off, user95401.
	this->m_noElasticity = true;

	SongManager& songManager = SongManager::get();
	const int favorites = std::clamp<int>(songManager.getFavorites().size(), 0, std::numeric_limits<int>::max());
	const int songCount = std::clamp<int>(songManager.getSongsSize() - favorites, 0, std::numeric_limits<int>::max());;
	this->setTitle(fmt::format("Menu Loop Randomizer - Your {} Song{} ({} Favorite{})", songCount, (songCount == 1 ? "" : "s"), favorites, (favorites == 1 ? "" : "s")));
	this->m_title->setID("song-list-title"_spr);
	this->m_title->limitLabelWidth(320.f, 1.f, .0001f);

	const cocos2d::CCSize layerSize = this->m_mainLayer->getContentSize();
	CCLayer* mainLayer = this->m_mainLayer;
	mainLayer->setID("main-layer"_spr);

	this->m_scrollLayer = geode::ScrollLayer::create({356.f, 220.f});
	this->m_scrollLayer->m_contentLayer->setLayout(geode::ColumnLayout::create()->setAxisReverse(true)->setAxisAlignment(geode::AxisAlignment::End)->setAutoGrowAxis(std::make_optional<float>(220.f))->setGap(.0f));
	this->m_scrollLayer->ignoreAnchorPointForPosition(false);
	this->m_scrollLayer->setID("list-of-songs"_spr);
	this->m_mainLayer->addChildAtPosition(this->m_scrollLayer, geode::Anchor::Center);

	SongListLayer::addSongsToScrollLayer(this->m_scrollLayer, songManager);

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

		cocos2d::CCSprite* fakeDeleteIcon = geode::EditorButtonSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png", .7f, geode::EditorBaseColor::DarkGray, geode::EditorBaseSize::Normal);
		fakeDeleteIcon->setScale(.7f);
		fakeDeleteIcon->setID("fake-delete-sprite"_spr);

		CCMenuItemSpriteExtra* clearButton = geode::cocos::CCMenuItemExt::createSpriteExtra(fakeDeleteIcon, [this](auto) {
			CCNode* searchBar = GET_SEARCH_BAR_NODE;
			if (!searchBar || (searchBar->getTag() == -1 && GET_SEARCH_STRING.empty())) return;
			EMPTY_SEARCH_STRG
			searchBar->setTag(-1);
			SongListLayer::searchSongs("");
		});
		clearButton->setPosition({330.f, 17.f});
		clearButton->setID("song-list-clear-button"_spr);
		searchBarMenu->addChild(clearButton);

		this->m_searchBar = geode::TextInput::create(370.f, fmt::format("Search... (Current Song: {})", songManager.getCurrentSongDisplayName()));
		this->m_searchBar->setCommonFilter(geode::CommonFilter::Any);
		this->m_searchBar->setTextAlign(geode::TextInputAlign::Left);
		this->m_searchBar->setPosition({145.f, 17.f});
		this->m_searchBar->setScale(.75f);
		this->m_searchBar->setID(SEARCH_BAR_NODE_ID);
		searchBarMenu->addChild(this->m_searchBar);

		CCTextInputNode* inputNode = this->m_searchBar->getInputNode();
		inputNode->setLabelPlaceholderScale(.5f);
		inputNode->setMaxLabelScale(.5f);
		inputNode->setID("song-list-search-bar-input-node"_spr);

		CCLayerColor* searchBarDivider = CCLayerColor::create({0, 0, 0, 127});
		searchBarDivider->setContentSize({350.f, .5f});
		searchBarDivider->setAnchorPoint({0.f, 0.f});
		searchBarDivider->setID("song-list-search-divider"_spr);
		searchBarMenu->addChild(searchBarDivider);
	}

	geode::ListBorders* listBorder = geode::ListBorders::create();
	listBorder->setSpriteFrames("GJ_commentTop2_001.png", "GJ_commentSide2_001.png");
	listBorder->setContentSize(this->m_scrollLayer->getContentSize());
	listBorder->ignoreAnchorPointForPosition(false);
	listBorder->setID("songs-list-border"_spr);
	this->m_mainLayer->addChildAtPosition(listBorder, geode::Anchor::Center);

	if (Utils::getBool("showScrollingShortcuts")) {
		this->m_scrollShortcuts = cocos2d::CCMenu::create();
		this->m_scrollShortcuts->setLayout(geode::ColumnLayout::create()->setDefaultScaleLimits(.0001f, 1.0f)->setGap(600.f)->setAxisReverse(true)->setAutoScale(true));
		Utils::addButton("scroll-top", menu_selector(SongListLayer::onScrollTopButton), this->m_scrollShortcuts, this, true);
		Utils::addButton("scroll-cur", menu_selector(SongListLayer::onScrollCurButton), this->m_scrollShortcuts, this, true);
		Utils::addButton("scroll-btm", menu_selector(SongListLayer::onScrollBtmButton), this->m_scrollShortcuts, this, true);

		this->m_scrollShortcuts->setPosition({405.f, SongListLayer::determineYPosition(this->m_scrollLayer)});
		this->m_scrollShortcuts->ignoreAnchorPointForPosition(false);
		this->m_scrollShortcuts->setContentHeight(220.f);
		this->m_scrollShortcuts->updateLayout();
		this->m_scrollShortcuts->setID("scroll-shortcuts-menu"_spr);
		this->m_mainLayer->addChild(this->m_scrollShortcuts);
	}

	this->m_scrollBar = geode::Scrollbar::create(this->m_scrollLayer);
	this->m_scrollBar->setPosition({this->m_scrollLayer->getPositionX() + (this->m_scrollLayer->getContentWidth() / 2.f) + 5.f, SongListLayer::determineYPosition(this->m_scrollLayer)});
	this->m_scrollBar->setID("song-list-scrollbar"_spr);
	this->m_mainLayer->addChild(this->m_scrollBar);

	if (SONG_SORTING_ENABLED) {
		this->m_viewFiltersMenu = cocos2d::CCMenu::create();

		this->m_songListCompactMode = Utils::addViewModeToggle(SAVED("songListCompactMode"), "GJ_smallModeIcon_001.png", "compact-mode", menu_selector(SongListLayer::onCompactModeToggle), this->m_viewFiltersMenu, this);
		this->m_songListFavoritesOnlyMode = Utils::addViewModeToggle(SAVED("songListFavoritesOnlyMode"), "favorites.png"_spr, "favorites-only", menu_selector(SongListLayer::onFavoritesOnlyToggle), this->m_viewFiltersMenu, this);
		this->m_songListReverseSort = Utils::addViewModeToggle(SAVED("songListReverseSort"), "reverse.png"_spr, "reverse-list", menu_selector(SongListLayer::onSortReverseToggle), this->m_viewFiltersMenu, this);

		cocos2d::CCSprite* separator = cocos2d::CCSprite::createWithSpriteFrameName("gridLine03_001.png");
		separator->setOpacity(128);
		separator->setID("sort-filter-separator"_spr);
		this->m_viewFiltersMenu->addChild(separator);

		this->m_songListSortAlphabetically = Utils::addViewModeToggle(SAVED("songListSortAlphabetically"), "abc.png"_spr, "alphabetical", menu_selector(SongListLayer::onSortABCToggle), this->m_viewFiltersMenu, this);
		this->m_songListSortDateAdded = Utils::addViewModeToggle(SAVED("songListSortDateAdded"), "dates.png"_spr, "date-added", menu_selector(SongListLayer::onSortDateToggle), this->m_viewFiltersMenu, this);
		if (Utils::getBool("showSortSongLength")) this->m_songListSortSongLength = Utils::addViewModeToggle(SAVED("songListSortSongLength"), "length.png"_spr, "song-length", menu_selector(SongListLayer::onSortLengthToggle), this->m_viewFiltersMenu, this);
		this->m_songListSortFileSize = Utils::addViewModeToggle(SAVED("songListSortFileSize"), "size.png"_spr, "song-size", menu_selector(SongListLayer::onSortSizeToggle), this->m_viewFiltersMenu, this);
		this->m_songListSortFileExtn = Utils::addViewModeToggle(SAVED("songListSortFileExtn"), "xtension.png"_spr, "file-extension", menu_selector(SongListLayer::onSortExtnToggle), this->m_viewFiltersMenu, this);

		this->m_viewFiltersMenu->setContentHeight(240.f);
		this->m_viewFiltersMenu->ignoreAnchorPointForPosition(false);
		this->m_viewFiltersMenu->setPosition({19.f, this->m_scrollLayer->getPositionY()});
		this->m_viewFiltersMenu->setLayout(geode::ColumnLayout::create()->setAxisReverse(true));
		this->m_viewFiltersMenu->setID("view-mode-menu"_spr);
		this->m_mainLayer->addChild(this->m_viewFiltersMenu);
	}

	cocos2d::CCMenu* infoMenu = cocos2d::CCMenu::create();
	infoMenu->setContentSize({24.f * .75f, 23.f * .75f});
	InfoAlertButton* infoBtn = InfoAlertButton::create(
		"Menu Loop Randomizer - Help/FAQ",
		"Gold = <cy>favorited</c>. Green = <cg>current</c>.\n"
		"Italics = NG/ML song stored in an unexpected place.\n"
		"Dark gradients OR random letters/numbers = Jukebox/NONG, <c_>OR</c> someone else goofed up.\n"
		"Controls: <cl>shuffle</c>, <cl>copy name</c>, <cl>previous song</c>, <cl>control panel</c>.\n\n"
		"<cy>Q: Make this mod act more closely to Spotify!</c>\n"
		"A: <c_>No. Never. Spotify's UI/UX sets BAD precedents.</c>\n\n"
		"<cy>Q: Where is the search bar/song sorting menu?</c>\n"
		"A: <cg>Install \"Better Touch Prio\" by alk1m123; check your settings.</c>",
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
	CCMenuItemSpriteExtra* settings = Utils::addButton("settings", menu_selector(SongListLayer::onSettingsButton), settingsMenu, this);
	settings->setPosition(settingsMenu->getContentSize() / 2.f);
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

	const float sllWidth = this->m_scrollLayer->getContentWidth();
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

	if (SEARCH_BAR_ENABLED && GET_SEARCH_BAR_NODE) {
		SongListLayer::displayCurrentSongByLimitingPlaceholderLabelWidth(this->m_searchBar->getInputNode());
		this->schedule(schedule_selector(SongListLayer::displayCurrentSongByLimitingPlaceholderLabelWidthScheduler), 0.f);
	}

	if (Utils::getBool("autoScrollToCurrentSong") && !VANILLA_GD_MENU_LOOP_DISABLED && !songManager.isOverride()) SongListLayer::scrollToCurrentSong();

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
}

void SongListLayer::onCopyButton(CCObject*) {
	SongControl::copySong();
}

void SongListLayer::onPreviousButton(CCObject*) {
	SongControl::previousSong();
}

void SongListLayer::onControlsButton(CCObject*) {
	this->onClose(nullptr);
	SongControlMenu::create()->show();
}

CCContentLayer* SongListLayer::getContentLayer() const {
	if (this->m_scrollLayer) return this->m_scrollLayer->m_contentLayer;
	return nullptr;
}

void SongListLayer::onScrollTopButton(CCObject*) {
	if (!Utils::getBool("showScrollingShortcuts")) return;
	CCContentLayer* contentLayer = SongListLayer::getContentLayer();
	if (!contentLayer || !this->m_scrollLayer || !SongListLayer::tallEnough(this->m_scrollLayer)) return; // cmon bruh it's in plain sight lol
	contentLayer->setPositionY((contentLayer->getContentHeight() * -1.f) + contentLayer->getParent()->getContentHeight());
}

void SongListLayer::scrollToCurrentSong() {
	CCContentLayer* contentLayer = SongListLayer::getContentLayer();
	if (!contentLayer || !this->m_scrollLayer || !SongListLayer::tallEnough(this->m_scrollLayer)) return;
	// cmon bruh it's in plain sight lol
	CCNode* currentCell = contentLayer->getChildByTag(12192025);
	cocos2d::CCKeyboardDispatcher* cckd = cocos2d::CCKeyboardDispatcher::get();
	const float theAbsolluteTop = contentLayer->getContentHeight() * -1.f + contentLayer->getParent()->getContentHeight();
	const float centerOrCurrent = (cckd->getShiftKeyPressed() && (GEODE_MOBILE(true) GEODE_MACOS(!cckd->getControlKeyPressed()) GEODE_WINDOWS(!cckd->getAltKeyPressed())) || !currentCell) ? ((contentLayer->getContentHeight() + contentLayer->getParent()->getContentHeight()) * -1.f * .5f) : ((currentCell->getPositionY() * -1.f) - (contentLayer->getParent()->getContentHeight() / 2.f) + 20.f);
	const float desiredPosition = centerOrCurrent + contentLayer->getParent()->getContentHeight();
	if (desiredPosition > 0.f) contentLayer->setPositionY(0.f);
	else if (desiredPosition < theAbsolluteTop) contentLayer->setPositionY(theAbsolluteTop);
	else contentLayer->setPositionY(desiredPosition);
}

void SongListLayer::onScrollCurButton(CCObject*) {
	if (!Utils::getBool("showScrollingShortcuts")) return;
	SongListLayer::scrollToCurrentSong();
}

void SongListLayer::onScrollBtmButton(CCObject*) {
	if (!Utils::getBool("showScrollingShortcuts") || !this->m_scrollLayer) return;
	CCContentLayer* contentLayer = SongListLayer::getContentLayer();
	if (!contentLayer || !SongListLayer::tallEnough(this->m_scrollLayer)) return; // cmon bruh it's in plain sight lol
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

void SongListLayer::onSortExtnToggle(CCObject*) {
	if (SONG_SORTING_DISABLED) return;
	SongListLayer::disableAllSortFiltersThenToggleThenSearch("songListSortFileExtn");
}

void SongListLayer::handleMutuallyExclusiveSortToggle(const std::string_view savedValueKeyToMatch, const std::string_view nodeID, const std::string_view savedValueKey, cocos2d::CCNode *viewModeMenu, const bool originalSavedValue) {
	geode::Mod::get()->setSavedValue<bool>(savedValueKeyToMatch, false);
	if (const auto toggler = static_cast<CCMenuItemToggler*>(viewModeMenu->getChildByID(nodeID)); toggler) {
		toggler->toggle(false);
		if (savedValueKey == savedValueKeyToMatch) toggler->toggle(originalSavedValue);
	}
}

void SongListLayer::disableAllSortFiltersThenToggleThenSearch(const std::string_view savedValueKey) {
	if (SONG_SORTING_DISABLED) return;
	cocos2d::CCNode* viewModeMenu = this->m_viewFiltersMenu;
	if (!viewModeMenu) return;
	const bool originalSavedValue = SAVED(savedValueKey);
	SongListLayer::handleMutuallyExclusiveSortToggle("songListSortAlphabetically", "alphabetical-button"_spr, savedValueKey, viewModeMenu, originalSavedValue);
	SongListLayer::handleMutuallyExclusiveSortToggle("songListSortDateAdded", "date-added-button"_spr, savedValueKey, viewModeMenu, originalSavedValue);
	SongListLayer::handleMutuallyExclusiveSortToggle("songListSortSongLength", "song-length-button"_spr, savedValueKey, viewModeMenu, originalSavedValue);
	SongListLayer::handleMutuallyExclusiveSortToggle("songListSortFileSize", "song-size-button"_spr, savedValueKey, viewModeMenu, originalSavedValue);
	SongListLayer::handleMutuallyExclusiveSortToggle("songListSortFileExtn", "file-extension-button"_spr, savedValueKey, viewModeMenu, originalSavedValue);
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
	SongManager& songManager = SongManager::get();
	#ifdef GEODE_IS_DESKTOP
	if (songManager.getYoutubeAndVLCKeyboardShortcutsSongList()) {
		cocos2d::CCKeyboardDispatcher* cckd = cocos2d::CCKeyboardDispatcher::get();
		const bool isShift = cckd->getShiftKeyPressed();
		const bool isCmd = GEODE_MACOS(cckd->getCommandKeyPressed()) GEODE_WINDOWS(false);
		const bool macOSCtrl = GEODE_MACOS(cckd->getControlKeyPressed()) GEODE_WINDOWS(false);
		const bool windowsCtrl = GEODE_MACOS(false) GEODE_WINDOWS(cckd->getControlKeyPressed());
		const bool isAlt = GEODE_MACOS(cckd->getControlKeyPressed()) GEODE_WINDOWS(cckd->getAltKeyPressed());
		if (key == cocos2d::KEY_Zero || key == cocos2d::KEY_One || key == cocos2d::KEY_Two || key == cocos2d::KEY_Three || key == cocos2d::KEY_Four || key == cocos2d::KEY_Five || key == cocos2d::KEY_Six || key == cocos2d::KEY_Seven || key == cocos2d::KEY_Eight || key == cocos2d::KEY_Nine) {
			if (!isShift && !isCmd && !windowsCtrl) return SongControl::setSongPercentage(10 * (static_cast<int>(key) - static_cast<int>(cocos2d::KEY_Zero)));
			if (isShift && (windowsCtrl || macOSCtrl)) {
				if (key == cocos2d::KEY_One && this->m_songListCompactMode) this->m_songListCompactMode->activate();
				if (key == cocos2d::KEY_Two && this->m_songListFavoritesOnlyMode) this->m_songListFavoritesOnlyMode->activate();
				if (key == cocos2d::KEY_Three && this->m_songListReverseSort) this->m_songListReverseSort->activate();
				if (key == cocos2d::KEY_Four && this->m_songListSortAlphabetically) this->m_songListSortAlphabetically->activate();
				if (key == cocos2d::KEY_Five && this->m_songListSortDateAdded) this->m_songListSortDateAdded->activate();
				if (key == cocos2d::KEY_Six && this->m_songListSortSongLength) this->m_songListSortSongLength->activate();
				if (key == cocos2d::KEY_Seven && this->m_songListSortFileSize) this->m_songListSortFileSize->activate();
				if (key == cocos2d::KEY_Eight && this->m_songListSortFileExtn) this->m_songListSortFileExtn->activate();
			}
		}
		if (key == cocos2d::KEY_NumPad0 || key == cocos2d::KEY_NumPad1 || key == cocos2d::KEY_NumPad2 || key == cocos2d::KEY_NumPad3 || key == cocos2d::KEY_NumPad4 || key == cocos2d::KEY_NumPad5 || key == cocos2d::KEY_NumPad6 || key == cocos2d::KEY_NumPad7 || key == cocos2d::KEY_NumPad8 || key == cocos2d::KEY_NumPad9) {
			if (!isShift && !macOSCtrl && !windowsCtrl) return SongControl::setSongPercentage(10 * (static_cast<int>(key) - static_cast<int>(cocos2d::KEY_NumPad0)));
			if (isShift && (windowsCtrl || macOSCtrl)) {
				if (key == cocos2d::KEY_NumPad1 && this->m_songListCompactMode) this->m_songListCompactMode->activate();
				if (key == cocos2d::KEY_NumPad2 && this->m_songListFavoritesOnlyMode) this->m_songListFavoritesOnlyMode->activate();
				if (key == cocos2d::KEY_NumPad3 && this->m_songListReverseSort) this->m_songListReverseSort->activate();
				if (key == cocos2d::KEY_NumPad4 && this->m_songListSortAlphabetically) this->m_songListSortAlphabetically->activate();
				if (key == cocos2d::KEY_NumPad5 && this->m_songListSortDateAdded) this->m_songListSortDateAdded->activate();
				if (key == cocos2d::KEY_NumPad6 && this->m_songListSortSongLength) this->m_songListSortSongLength->activate();
				if (key == cocos2d::KEY_NumPad7 && this->m_songListSortFileSize) this->m_songListSortFileSize->activate();
				if (key == cocos2d::KEY_NumPad8 && this->m_songListSortFileExtn) this->m_songListSortFileExtn->activate();
			}
		}
		if ((windowsCtrl || isCmd) && key == cocos2d::KEY_R) {
			return SongControl::setSongPercentage(0);
		}
		if (((macOSCtrl || windowsCtrl) && key == cocos2d::KEY_S) || ((key == cocos2d::KEY_ArrowRight || key == cocos2d::KEY_Right) && (windowsCtrl || isCmd))) {
			return SongControl::shuffleSong();
		}
		if ((isShift && key == cocos2d::KEY_P) || ((key == cocos2d::KEY_ArrowLeft || key == cocos2d::KEY_Left) && (windowsCtrl || isCmd))) {
			return SongControl::previousSong();
		}
		if (key == cocos2d::KEY_B && isShift && isAlt) {
			return SongControl::favoriteSong();
		}
		if (key == cocos2d::KEY_J && isShift && isAlt) {
			return SongListLayer::scrollToCurrentSong();
		}
		if (((key == cocos2d::KEY_K || key == cocos2d::KEY_H) && (windowsCtrl || isCmd)) && songManager.getFinishedCalculatingSongLengths()) {
			return SongListLayer::onControlsButton(nullptr);
		}
		if (GET_SEARCH_BAR_NODE && (key == cocos2d::KEY_F && (isCmd || windowsCtrl))) {
			if (this->m_searchBar->getInputNode()->m_cursor->isVisible()) return;
			this->m_searchBar->focus();
			return geode::Loader::get()->queueInMainThread([sb = geode::Ref(this->m_searchBar)] {
				if (sb) sb->setString("", false);
			});
		}
	}
	#endif
	if (songManager.getShowPlaybackControlsSongList()) {
		if (key == cocos2d::KEY_Right || key == cocos2d::KEY_ArrowRight || key == cocos2d::KEY_L) return SongControl::skipForward();
		if (key == cocos2d::KEY_Left || key == cocos2d::KEY_ArrowLeft || key == cocos2d::KEY_J) return SongControl::skipBackward();
	}
	// this is fine since searchbar swallows delete (macos)/backspace (all other platforms) key inputs first
	if (SEARCH_BAR_DISABLED) {
		// code taken directly from geode::Popup keyDown impl as of dec 19 2025
		if (key == cocos2d::KEY_Escape) return this->onClose(nullptr);
		if (key == cocos2d::KEY_Space) return;
		return FLAlertLayer::keyDown(key);
	}
	CCNode* searchBar = GET_SEARCH_BAR_NODE;
	if (key == cocos2d::KEY_Escape) {
		if (!searchBar) return this->onClose(nullptr);
		if (this->m_searchBar->getInputNode()->m_cursor->isVisible()) return this->m_searchBar->getInputNode()->onClickTrackNode(false);
		return this->onClose(nullptr);
	}
	if (!searchBar || (GET_SEARCH_STRING.empty() && searchBar->getTag() == -1)) return;
	if (key == cocos2d::KEY_Delete || key == cocos2d::KEY_Backspace) return EMPTY_SEARCH_STRG; // clear search query before re-populating
	if (key != cocos2d::KEY_Enter) return;
	const std::string& queryString = GET_SEARCH_STRING;
	searchBar->setTag(queryString.empty() ? -1 : 12202025);
	SongListLayer::searchSongs(queryString);
}

std::string SongListLayer::generateDisplayName(SongData& songData) {
	if (!songData.displayName.empty()) return songData.displayName;

	std::string displayName = Utils::toNormalizedString(Utils::toProblematicString(songData.actualFilePath).stem());
	const int songID = geode::utils::numFromString<int>(displayName).unwrapOr(-1);
	if (songID > 0 && songData.couldPossiblyExistInMusicDownloadManager) {
		MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
		if (SongInfoObject* songInfoObject = mdm->getSongInfoObject(songID)) displayName = Utils::getFormattedNGMLSongName(songInfoObject);
	}

	Utils::sanitizeASCII(displayName);
	return displayName;
}

bool SongListLayer::tallEnough(geode::ScrollLayer* scrollLayer) {
	if (!scrollLayer || !scrollLayer->m_contentLayer) return false;
	return scrollLayer->m_contentLayer->getContentHeight() > scrollLayer->getContentHeight();
}

float SongListLayer::determineYPosition(geode::ScrollLayer* scrollLayer) {
	return scrollLayer && SongListLayer::tallEnough(scrollLayer) ? 145.f : 99999.f;
}

void SongListLayer::displayCurrentSongByLimitingPlaceholderLabelWidth(CCTextInputNode* inputNode, const bool updateString) {
	if (SEARCH_BAR_DISABLED || !inputNode || (inputNode->m_selected && !inputNode->getString().empty())) return;
	cocos2d::CCLabelBMFont* placeholderLabelMaybe = static_cast<cocos2d::CCLabelBMFont*>(inputNode->getChildByTag(12242025));
	if (!placeholderLabelMaybe) placeholderLabelMaybe = inputNode->m_textLabel;
	if (!placeholderLabelMaybe) placeholderLabelMaybe = inputNode->getChildByType<cocos2d::CCLabelBMFont>(0);
	if (!placeholderLabelMaybe || placeholderLabelMaybe->getColor() != cocos2d::ccColor3B{150, 150, 150}) return;
	if (updateString) placeholderLabelMaybe->setString(fmt::format("Search... (Current Song: {})", SongManager::get().getCurrentSongDisplayName()).c_str());
	if (placeholderLabelMaybe->isVisible()) placeholderLabelMaybe->limitLabelWidth(350.f, .5f, .0001f);
	placeholderLabelMaybe->setTag(12242025);
}

bool SongListLayer::caseInsensitiveAlphabetical(MLRSongCell* a, MLRSongCell* b, const bool reverse) {
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

bool SongListLayer::fileSize(MLRSongCell* a, MLRSongCell* b, const bool reverse) {
	if (SONG_SORTING_DISABLED) return false;
	const std::uintmax_t fileSizeA = a->m_songData.songFileSize;
	const std::uintmax_t fileSizeB = b->m_songData.songFileSize;
	if (fileSizeA < fileSizeB) return !reverse;
	if (fileSizeA > fileSizeB) return reverse;
	return a->m_songData.actualFilePath < b->m_songData.actualFilePath;
}

bool SongListLayer::fileExtn(MLRSongCell* a, MLRSongCell* b, const bool reverse) {
	if (SONG_SORTING_DISABLED) return false;
	const std::string& cleanedUpA = a->m_songData.fileExtension;
	const std::string& cleanedUpB = b->m_songData.fileExtension;
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
	return SongListLayer::caseInsensitiveAlphabetical(a, b, reverse);
}

bool SongListLayer::dateAdded(MLRSongCell* a, MLRSongCell* b, const bool reverse) {
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

bool SongListLayer::songLength(MLRSongCell* a, MLRSongCell* b, const bool reverse) {
	if (SONG_SORTING_DISABLED) return false;
	const unsigned int extreme = reverse ? std::numeric_limits<unsigned int>::min() : std::numeric_limits<unsigned int>::max();
	if (a->m_songData.songLength == extreme || b->m_songData.songLength == extreme) {
		if (a->m_songData.songLength == extreme && b->m_songData.songLength == extreme) return a->m_songData.actualFilePath < b->m_songData.actualFilePath;
		if (a->m_songData.songLength == extreme) return reverse;
		if (b->m_songData.songLength == extreme) return !reverse;
	}
	if (a->m_songData.songLength < b->m_songData.songLength) return !reverse;
	if (a->m_songData.songLength > b->m_songData.songLength) return reverse;
	return a->m_songData.actualFilePath < b->m_songData.actualFilePath;
}

unsigned int SongListLayer::useFMODToGetLength(const std::string& path, const unsigned int extreme) {
	FMOD::System* sys = FMODAudioEngine::get()->m_system;
	if (!sys) return extreme;
	unsigned int temp = extreme;
	FMOD::Sound* sound;
	const FMOD_RESULT resultSoundA = sys->createSound(path.c_str(), FMOD_OPENONLY, nullptr, &sound);
	if (sound && resultSoundA == FMOD_OK) {
		sound->getLength(&temp, FMOD_TIMEUNIT_MS);
		sound->release();
		return temp;
	}
	return extreme;
}

unsigned int SongListLayer::getLength(const std::string& path, const bool reverse) {
	const unsigned int extreme = reverse ? std::numeric_limits<unsigned int>::min() : std::numeric_limits<unsigned int>::max();
	#ifndef GEODE_IS_IOS
	if (!Utils::getBool("useMiniaudioForLength")) return SongListLayer::useFMODToGetLength(path, extreme);

	ma_decoder decoder;
	if (ma_decoder_init_file(path.c_str(), nullptr, &decoder) != MA_SUCCESS) return SongListLayer::useFMODToGetLength(path, extreme);

	ma_uint64 frames = 0;
	if (ma_decoder_get_length_in_pcm_frames(&decoder, &frames) != MA_SUCCESS) {
		ma_decoder_uninit(&decoder);
		return SongListLayer::useFMODToGetLength(path, extreme);
	}

	const ma_uint32 sampleRate = decoder.outputSampleRate;
	ma_decoder_uninit(&decoder);

	if (sampleRate == 0) return extreme;
	return 1000 * static_cast<unsigned int>(static_cast<double>(frames) / static_cast<double>(sampleRate));
	#else
	return SongListLayer::useFMODToGetLength(path, extreme);
	#endif
}

void SongListLayer::displayCurrentSongByLimitingPlaceholderLabelWidthScheduler(float) {
	if (SEARCH_BAR_DISABLED || !GET_SEARCH_BAR_NODE) return;
	SongListLayer::displayCurrentSongByLimitingPlaceholderLabelWidth(this->m_searchBar->getInputNode());
}