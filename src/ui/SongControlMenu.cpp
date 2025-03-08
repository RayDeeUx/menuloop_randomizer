#include "SongControlMenu.hpp"
#include "../SongControl.hpp"
#include "../Utils.hpp"
#include <Geode/ui/GeodeUI.hpp>

#define REST_OF_THE_OWL this->m_songControlsMenu, this

bool SongControlMenu::setup(const std::string& id) {
	this->setTitle("Menu Loop Randomizer");

	const cocos2d::CCSize layerSize = this->m_mainLayer->getContentSize();
	cocos2d::extension::CCScale9Sprite* background = this->m_bgSprite;
	const float idealWidth = layerSize.width * 0.95f;
	const float centerStage = layerSize.width / 2.f;
	CCLayer* mainLayer = this->m_mainLayer;
	mainLayer->setID("main-layer"_spr);

	this->m_title->setPositionX(mainLayer->getContentWidth() / 2.f);
	background->initWithFile("GJ_square05.png");
	background->setContentSize(layerSize);

	this->m_songControlsMenu = cocos2d::CCMenu::create();

	Utils::addButton("shuffle", menu_selector(SongControlMenu::onShuffleButton), REST_OF_THE_OWL);
	Utils::addButton("regen", menu_selector(SongControlMenu::onRegenButton), REST_OF_THE_OWL);
	Utils::addButton("copy", menu_selector(SongControlMenu::onCopyButton), REST_OF_THE_OWL);
	Utils::addButton("blacklist", menu_selector(SongControlMenu::onBlacklistButton), REST_OF_THE_OWL);
	Utils::addButton("favorite", menu_selector(SongControlMenu::onFavoriteButton), REST_OF_THE_OWL);
	Utils::addButton("hold", menu_selector(SongControlMenu::onHoldSongButton), REST_OF_THE_OWL);
	Utils::addButton("prev", menu_selector(SongControlMenu::onPreviousButton), REST_OF_THE_OWL);
	Utils::addButton("controls", menu_selector(SongControlMenu::onSettingsButton), REST_OF_THE_OWL);

	geode::AxisLayout* layout = geode::RowLayout::create()->setGap(5.f)->setDefaultScaleLimits(.0001f, 1.0f);

	this->m_songControlsMenu->setPosition({centerStage, layerSize.height / 2.f});
	this->m_songControlsMenu->ignoreAnchorPointForPosition(false);
	this->m_songControlsMenu->setContentSize({idealWidth, 32.f});
	this->m_songControlsMenu->setLayout(layout);

	SongControlMenu::updateCurrentLabel();

	this->m_otherLabel = cocos2d::CCLabelBMFont::create("Tip of the Day: Menu Loop Randomizer will never be designed to be a Spotify replacement, or anything like its distant cousin Editor Music. Please keep this in mind if you have feedback on MLR. :)", "chatFont.fnt");
	this->m_otherLabel->setPositionY((layerSize.height / 4.f) - (this->m_smallLabel->getContentHeight() / 2.f) - (this->m_otherLabel->getContentHeight() / 2.f) + 2.5f);
	this->m_otherLabel->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	this->m_otherLabel->limitLabelWidth(idealWidth * .95f, 1.0f, .0001f);
	this->m_otherLabel->setPositionX(centerStage);

	this->setID("SongControlMenu"_spr);
	mainLayer->addChild(this->m_otherLabel);
	mainLayer->addChild(this->m_smallLabel);
	mainLayer->addChild(this->m_songControlsMenu);
	this->m_otherLabel->setID("im-not-spotify"_spr);
	this->m_smallLabel->setID("current-song-label"_spr);
	this->m_songControlsMenu->setID("song-controls-menu"_spr);

	return true;
}

SongControlMenu* SongControlMenu::create(const std::string& id) {
	auto ret = new SongControlMenu;
	if (ret->initAnchored(300.f, 100.f, id)) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

void SongControlMenu::onExit() {
	Popup::onExit();
}

void SongControlMenu::onShuffleButton(CCObject*) {
	SongControl::shuffleSong();
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onRegenButton(CCObject*) {
	SongControl::regenSong();
}
void SongControlMenu::onCopyButton(CCObject*) {
	SongControl::copySong();
}
void SongControlMenu::onBlacklistButton(CCObject*) {
	SongControl::blacklistSong();
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onFavoriteButton(CCObject*) {
	SongControl::favoriteSong();
}
void SongControlMenu::onHoldSongButton(CCObject*) {
	SongControl::holdSong();
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onPreviousButton(CCObject*) {
	SongControl::previousSong();
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onSettingsButton(CCObject*) { geode::openSettingsPopup(geode::Mod::get()); }
void SongControlMenu::updateCurrentLabel() {
	const std::string& currentSong = fmt::format("Current Song: {}", SongManager::get().getCurrentSongDisplayName());
	const cocos2d::CCSize layerSize = this->m_mainLayer->getContentSize();
	const float idealWidth = layerSize.width * 0.95f;
	const float centerStage = layerSize.width / 2.f;
	if (!this->m_smallLabel || !this->m_smallLabel->getParent() || this->m_smallLabel->getParent() != this->m_mainLayer) {
		this->m_smallLabel = cocos2d::CCLabelBMFont::create(currentSong.c_str(), "chatFont.fnt");
	} else this->m_smallLabel->setString(currentSong.c_str(), "chatFont.fnt");
	this->m_smallLabel->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	this->m_smallLabel->setPosition({centerStage, (layerSize.height / 4.f) - 2.5f});
	this->m_smallLabel->limitLabelWidth(idealWidth * .9f, 1.0f, .0001f);
}