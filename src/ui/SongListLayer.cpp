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

bool SongListLayer::setup(const std::string&) {
	this->m_noElasticity = true;
	this->setTitle("Menu Loop Randomizer - Your Songs");
	this->m_title->setID("song-list-title"_spr);

	const cocos2d::CCSize layerSize = this->m_mainLayer->getContentSize();
	cocos2d::extension::CCScale9Sprite* background = this->m_bgSprite;
	CCLayer* mainLayer = this->m_mainLayer;
	mainLayer->setID("main-layer"_spr);

	background->initWithFile("GJ_square01.png");
	background->setContentSize(layerSize);

	cocos2d::CCMenu* infoMenu = cocos2d::CCMenu::create();
	infoMenu->setLayout(
		geode::RowLayout::create()
			->setAutoScale(false)
			->setAxis(geode::Axis::Row)
			->setGap(.0f)
	);
	infoMenu->setContentSize({24.f, 23.f});
	InfoAlertButton* infoBtn = InfoAlertButton::create(
		"Menu Loop Randomizer - Help/FAQ",
		"Play button = switch songs.\n"
		"Yellow = <cy>favorited</c> song. Green = <cg>current</c> song.\n"
		"Buttons at the bottom let you <cl>shuffle to a new song</c>,\n"
		"<cl>copy the current song's name</c>, <cl>go back one song</c>,\n"
		"and <cy>(if on \"Reduced\" Button Mode)</c> <cl>reopen the control panel</c>.\n\n"
		"<cy>Q: I can't blacklist or favorite songs from here?!</c>\n"
		"A: <c_>***Listen***</c> to them first.\n\n"
		"<cy>Q: Add a search bar!</c>\n"
		"A: <c_>No. Never. Learn how to scroll through a list.</c>",
		1.f
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

	geode::ScrollLayer* scrollLayer = geode::ScrollLayer::create({356, 220});
	scrollLayer->m_contentLayer->setLayout(
		geode::ColumnLayout::create()
			->setAxisReverse(true)
			->setAxisAlignment(geode::AxisAlignment::End)
			->setAutoGrowAxis(220.f)
			->setGap(.0f)
	);

	SongManager& songManager = SongManager::get();
	const std::vector<std::string> blacklist = songManager.getBlacklist();
	const std::vector<std::string> favorites = songManager.getFavorites();
	const std::vector<std::string> songs = songManager.getSongs();
	std::vector<std::string> alreadyAdded {};

	bool isEven = false;

	for (const std::string& song : songs) {
		if (std::ranges::find(alreadyAdded.begin(), alreadyAdded.end(), song) != alreadyAdded.end()) continue;

		std::filesystem::path songFilePath = song;
		SongData songData = {
			Utils::toNormalizedString(songFilePath),
			Utils::toNormalizedString(songFilePath.extension()),
			Utils::toNormalizedString(songFilePath.filename()), SongType::Regular
		};
		if (std::ranges::find(blacklist.begin(), blacklist.end(), song) != blacklist.end()) songData.type = SongType::Blacklisted;
		else if (std::ranges::find(favorites.begin(), favorites.end(), song) != favorites.end()) songData.type = SongType::Favorited;
		alreadyAdded.push_back(song);

		scrollLayer->m_contentLayer->addChild(MLRSongCell::create(songData, isEven));
		isEven = !isEven;
	}

	scrollLayer->m_contentLayer->updateLayout();
	scrollLayer->scrollToTop();

	scrollLayer->setID("list-of-songs"_spr);
	scrollLayer->ignoreAnchorPointForPosition(false);
	this->m_mainLayer->addChildAtPosition(scrollLayer, geode::Anchor::Center);

	geode::ListBorders* listBorder = geode::ListBorders::create();
	listBorder->setContentSize(scrollLayer->getContentSize());
	listBorder->ignoreAnchorPointForPosition(false);
	listBorder->setID("songs-list-border"_spr);
	this->m_mainLayer->addChildAtPosition(listBorder, geode::Anchor::Center);

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

	this->setID("SongListLayer"_spr);
	this->m_bgSprite->setID("background"_spr);
	this->m_closeBtn->setID("close-button"_spr);
	this->m_buttonMenu->setID("close-menu"_spr);

	songManager.resetTowerRepeatCount();

	return true;
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