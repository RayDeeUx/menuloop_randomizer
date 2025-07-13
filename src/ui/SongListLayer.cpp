#include "SongListLayer.hpp"
#include "MLRSongCell.hpp"
#include "../Utils.hpp"
#include "../SongManager.hpp"

SongListLayer* SongListLayer::create() {
	auto* ret = new SongListLayer();
	if (ret->init("Menu Loop Randomizer")) {
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
		"Menu Loop Randomizer",
		"<cy>Q: I can't blacklist/favorite songs from here?!</c>\n"
		"A: You should at least <c_>***listen***</c> to a song before making these decisions. "
		"Also, there wasn't enough room to fit those buttons into each row.\n\n"
		"<cy>Q: Why did the MLR control panel just close?!\n"
		"A: Touch priority and Z ordering issues. <cy>(In other words, bugs not worth fixing.)</c>",
		1.f
	);
	infoMenu->addChildAtPosition(infoBtn, geode::Anchor::Center);
	infoMenu->setPosition({437.f, 282.f});
	this->m_mainLayer->addChild(infoMenu);

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
		SongData songData = { Utils::toNormalizedString(songFilePath.filename()), SongType::Regular };
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
}