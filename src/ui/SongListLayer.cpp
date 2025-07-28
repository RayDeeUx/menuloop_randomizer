#include <Geode/ui/GeodeUI.hpp>
#include "SongListLayer.hpp"
#include "MLRSongCell.hpp"
#include "../Utils.hpp"
#include "../SongManager.hpp"

SongListLayer* SongListLayer::create(const std::string& id) {
	auto* ret = new SongListLayer();
	if (ret->initAnchored(420.f, 300.f, id)) {
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

	background->initWithFile("GJ_square05.png");
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
		"Menu Loop Randomizer - FAQ",
		"<cy>Q: I can't blacklist/favorite songs from here?!</c>\n"
		"A: You should at least <c_>***listen***</c> to a song before making these decisions. "
		"Also, there wasn't enough room\nto fit those buttons into each row.\n\n"
		"<cy>Q: Why did the MLR control panel just close?!</c>\n"
		"A: Touch priority and Z ordering issues.\n<cy>(In other words, bugs not worth fixing.)</c>\n\n"
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
	settingsMenu->setID("songlayerlist-controls-menu"_spr);
	Utils::addButton("settings", menu_selector(SongListLayer::onSettingsButton), settingsMenu, this);
	if (CCMenuItemSpriteExtra* button = settingsMenu->getChildByType<CCMenuItemSpriteExtra>(0)) button->setPosition(settingsMenu->getContentSize() / 2.f);
	settingsMenu->setPosition({400.f, this->m_title->getPositionY()});
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
	constexpr float topRowYPos = 28.5f;
	constexpr float bottomRowYPos = 17.5f;
	constexpr float arbitraryPaddingValue = 10.f;

	this->m_mainLayer->addChildAtPosition(currentPlaylistLabel, geode::Anchor::BottomLeft, {(layerSize.width - sllWidth) + arbitraryPaddingValue + offset, bottomRowYPos});
	this->m_mainLayer->addChildAtPosition(loadPlaylistFileLabel, geode::Anchor::BottomLeft, {(layerSize.width - sllWidth) + arbitraryPaddingValue + offset, topRowYPos});
	this->m_mainLayer->addChildAtPosition(constantShuffleModeLabel, geode::Anchor::BottomLeft, {sllWidth - offset - arbitraryPaddingValue, bottomRowYPos});
	this->m_mainLayer->addChildAtPosition(platformLabel, geode::Anchor::BottomLeft, {sllWidth - offset - arbitraryPaddingValue, topRowYPos});

	return true;
}

void SongListLayer::onSettingsButton(CCObject*) {
	this->keyBackClicked();
	geode::openSettingsPopup(geode::Mod::get());
}