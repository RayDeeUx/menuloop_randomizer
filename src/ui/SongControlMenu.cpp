#include "SongControlMenu.hpp"
#include "../SongControl.hpp"
#include "SongListLayer.hpp"
#include "../Utils.hpp"
#include <Geode/ui/GeodeUI.hpp>

#define REST_OF_THE_OWL this->m_songControlsMenu, this
#define REST_OF_THE_CAT this->m_theTimeoutCorner, this
#define REST_OF_THE_DOG this->m_buttonMenu, this

bool SongControlMenu::setup(const std::string&) {
	this->setTitle("Menu Loop Randomizer - Control Panel");
	this->m_title->setScale(.45f);

	const cocos2d::CCSize layerSize = this->m_mainLayer->getContentSize();
	cocos2d::extension::CCScale9Sprite* background = this->m_bgSprite;
	const float idealWidth = layerSize.width * 0.95f;
	const float centerStage = layerSize.width / 2.f;
	CCLayer* mainLayer = this->m_mainLayer;
	mainLayer->setID("main-layer"_spr);

	this->m_title->setPositionY(this->m_title->getPositionY() + 2.f);
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
	if (!geode::Mod::get()->getSettingValue<bool>("loadPlaylistFile") || SongManager::get().getPlaylistIsEmpty()) {
		Utils::addButton("add", menu_selector(SongControlMenu::onAddToPlylstBtn), REST_OF_THE_OWL);
	}

	geode::AxisLayout* layout = geode::RowLayout::create()->setGap(5.f)->setDefaultScaleLimits(.0001f, 1.0f);

	this->m_songControlsMenu->setPosition({centerStage, 40.f});
	this->m_songControlsMenu->ignoreAnchorPointForPosition(false);
	this->m_songControlsMenu->setContentSize({idealWidth, 32.f});
	this->m_songControlsMenu->setLayout(layout);

	this->m_theTimeoutCorner = cocos2d::CCMenu::create();

	Utils::addButton("playlist", menu_selector(SongControlMenu::onPlaylistButton), REST_OF_THE_CAT);
	Utils::addButton("controls", menu_selector(SongControlMenu::onSettingsButton), REST_OF_THE_DOG);

	geode::AxisLayout* layoutTimeout = geode::ColumnLayout::create()->setGap(2.5f)->setDefaultScaleLimits(.0001f, 1.0f)->setAutoScale(true);

	if (CCNode* controlsButton = this->m_buttonMenu->getChildByID("controls-button"_spr)) {
		controlsButton->setPosition({m_mainLayer->getContentSize() - 3.f});
		controlsButton->setScale(.8f);
	}

	this->m_theTimeoutCorner->setPosition({280.f, this->m_title->getPositionY() - 8.5f});
	this->m_theTimeoutCorner->ignoreAnchorPointForPosition(false);
	this->m_theTimeoutCorner->setContentSize({24.f, 36.f});
	this->m_theTimeoutCorner->setLayout(layoutTimeout);

	this->m_otherLabel = cocos2d::CCLabelBMFont::create("Hi! Menu Loop Randomizer will never resemble Spotify or its distant cousin EditorMusic. Please respect that. :)", "chatFont.fnt");
	this->m_otherLabel->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	this->m_otherLabel->limitLabelWidth(idealWidth * .95f, 1.0f, .0001f);
	this->m_otherLabel->setPosition({centerStage, 12.5f});

	this->m_headerLabl = cocos2d::CCLabelBMFont::create("Current Song:", "chatFont.fnt");
	this->m_headerLabl->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	this->m_headerLabl->setPosition({centerStage, 107.5f});

	SongControlMenu::updateCurrentLabel();

	this->b = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
	this->b->ignoreAnchorPointForPosition(false);
	this->b->setPosition({centerStage, 80.f});
	this->b->setContentSize({275.f, 30.f});
	this->b->setColor({0, 0, 0});
	this->b->setOpacity(128);

	this->setID("SongControlMenu"_spr);
	mainLayer->addChild(this->b);
	mainLayer->addChild(this->m_otherLabel);
	mainLayer->addChild(this->m_smallLabel);
	mainLayer->addChild(this->m_headerLabl);
	mainLayer->addChild(this->m_songControlsMenu);
	mainLayer->addChild(this->m_theTimeoutCorner);

	this->b->setID("trans-bg"_spr);
	this->m_title->setID("title"_spr);
	this->m_bgSprite->setID("background"_spr);
	this->m_buttonMenu->setID("close-menu"_spr);
	this->m_otherLabel->setID("im-not-spotify"_spr);
	this->m_smallLabel->setID("current-song-label"_spr);
	this->m_headerLabl->setID("current-song-header"_spr);
	this->m_theTimeoutCorner->setID("timeout-corner"_spr);
	this->m_songControlsMenu->setID("song-controls-menu"_spr);

	return true;
}

SongControlMenu* SongControlMenu::create(const std::string& id) {
	auto ret = new SongControlMenu();
	if (ret->initAnchored(300.f, 150.f, id)) {
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
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onCopyButton(CCObject*) {
	SongControl::copySong();
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onBlacklistButton(CCObject*) {
	SongControl::blacklistSong();
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onFavoriteButton(CCObject*) {
	SongControl::favoriteSong();
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onHoldSongButton(CCObject*) {
	SongControl::holdSong();
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onPreviousButton(CCObject*) {
	SongControl::previousSong();
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onPlaylistButton(CCObject*) {
	SongControlMenu::onClose(nullptr);
	SongListLayer::create()->showLayer(true);
}
void SongControlMenu::onAddToPlylstBtn(CCObject*) {
	if (geode::Mod::get()->getSettingValue<bool>("loadPlaylistFile")) return geode::log::info("tried adding current song to a playlist file. BOOOOOOOOOOOOO");
	SongControl::addSongToPlaylist(SongManager::get().getCurrentSong());
	SongControlMenu::updateCurrentLabel();
}
void SongControlMenu::onSettingsButton(CCObject*) {
	geode::openSettingsPopup(geode::Mod::get());
}
void SongControlMenu::updateCurrentLabel() {
	SongManager& songManager = SongManager::get();
	const std::string& currentSong = songManager.getCurrentSongDisplayName();
	const cocos2d::CCSize layerSize = this->m_mainLayer->getContentSize();
	if (!this->m_smallLabel || !this->m_smallLabel->getParent() || this->m_smallLabel->getParent() != this->m_mainLayer) {
		this->m_smallLabel = cocos2d::CCLabelBMFont::create(currentSong.c_str(), "chatFont.fnt");
	} else this->m_smallLabel->setString(currentSong.c_str(), "chatFont.fnt");
	this->m_smallLabel->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	this->m_smallLabel->limitLabelWidth(layerSize.width * 0.95f * .9f, 1.0f, .0001f);
	this->m_smallLabel->setPosition({layerSize.width / 2.f, 80.f});
	if (!this->m_headerLabl) return;
	if (songManager.isOverride()) this->m_headerLabl->setString("Current Song (Custom Override):");
	else if (songManager.getConstantShuffleMode()) this->m_headerLabl->setString("Current Song (Constant Shuffle Mode):");
	else this->m_headerLabl->setString("Current Song:");
	this->m_headerLabl->limitLabelWidth(this->m_mainLayer->getContentSize().width * .95f * .95f, 1.0f, .0001f);
}