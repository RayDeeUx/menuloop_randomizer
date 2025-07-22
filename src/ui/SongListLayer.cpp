#include <Geode/ui/GeodeUI.hpp>
#include "SongListLayer.hpp"
#include "MLRSongCell.hpp"
#include "../Utils.hpp"
#include "../SongManager.hpp"

SongListLayer* SongListLayer::create() {
	auto* ret = new SongListLayer();
	if (ret->init("Menu Loop Randomizer - Your Songs")) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

void SongListLayer::customSetup() {
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
	infoMenu->setPosition({437.f, 282.f});
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
	Utils::addButton("controls", menu_selector(SongListLayer::onSettingsButton), settingsMenu, this);
	if (CCMenuItemSpriteExtra* button = settingsMenu->getChildByType<CCMenuItemSpriteExtra>(0)) button->setPosition(settingsMenu->getContentSize() / 2.f);
	settingsMenu->setPosition({75.f, 282.f});
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

	m_listLayer->addChild(scrollLayer);
}

void SongListLayer::showLayer(const bool instant) {
	GJDropDownLayer::showLayer(instant);
	this->removeFromParentAndCleanup(false);
	cocos2d::CCScene::get()->addChild(this);

	this->setID("SongListLayer"_spr);
	m_listLayer->setID("list-of-songs-layer"_spr);

	CCNode* titleLabel = m_listLayer->getChildByID("title");
	if (!titleLabel) return;

	SongManager& songManager = SongManager::get();

	const std::string& playlistFileName = songManager.getPlaylistIsEmpty() ? "None" : songManager.getPlaylistFileName();
	cocos2d::CCLabelBMFont* currentPlaylistLabel = cocos2d::CCLabelBMFont::create(fmt::format("Playlist: {}", playlistFileName).c_str(), "bigFont.fnt");
	currentPlaylistLabel->setPosition({83.5f, 0.f});
	currentPlaylistLabel->limitLabelWidth(279.f * .45f, 1.f, .0001f);
	currentPlaylistLabel->setZOrder(titleLabel->getZOrder());

	const std::string& loadPlaylistFileInstead = Utils::getBool("loadPlaylistFile") ? "ON" : "OFF";
	cocos2d::CCLabelBMFont* loadPlaylistFileLabel = cocos2d::CCLabelBMFont::create(fmt::format("Load Playlist File: {}", loadPlaylistFileInstead).c_str(), "bigFont.fnt");
	loadPlaylistFileLabel->setPosition({83.5f, -11.f});
	loadPlaylistFileLabel->limitLabelWidth(279.f * .45f, 1.f, .0001f);
	loadPlaylistFileLabel->setZOrder(titleLabel->getZOrder());

	const std::string& isConstantShuffleMode = songManager.getConstantShuffleMode() ? "ON" : "OFF";
	cocos2d::CCLabelBMFont* constantShuffleModeLabel = cocos2d::CCLabelBMFont::create(fmt::format("Constant Shuffle Mode: {}", isConstantShuffleMode).c_str(), "bigFont.fnt");
	constantShuffleModeLabel->setPosition({272.5f, 0.f});
	constantShuffleModeLabel->limitLabelWidth(279.f * .45f, 1.f, .0001f);
	constantShuffleModeLabel->setZOrder(titleLabel->getZOrder());

	cocos2d::CCLabelBMFont* platformLabel = cocos2d::CCLabelBMFont::create(fmt::format("Platform: {}", Utils::getPlatform()).c_str(), "bigFont.fnt");
	platformLabel->setPosition({272.5f, -11.f});
	platformLabel->limitLabelWidth(279.f * .45f, 1.f, .0001f);
	platformLabel->setZOrder(titleLabel->getZOrder());

	m_listLayer->addChild(currentPlaylistLabel);
	m_listLayer->addChild(loadPlaylistFileLabel);
	m_listLayer->addChild(constantShuffleModeLabel);
	m_listLayer->addChild(platformLabel);
}

void SongListLayer::onSettingsButton(CCObject*) {
	this->keyBackClicked();
	geode::openSettingsPopup(geode::Mod::get());
}