#include <Geode/ui/GeodeUI.hpp>
#include "SongControlMenu.hpp"
#include "SongListLayer.hpp"
#include "../Utils.hpp"
#include "../SongControl.hpp"

#define REST_OF_THE_OWL this->m_songControlsMenu, this
#define DEFAULT_FOOTER_TEXT fmt::format("Hi! Menu Loop Randomizer will never resemble Spotify or its distant cousin EditorMusic. Please respect that. :) [Platform: {}]", Utils::getPlatform())

bool SongControlMenu::setup() {
	this->setTitle("Menu Loop Randomizer - Control Panel");
	this->m_title->setScale(.45f);

	const cocos2d::CCSize layerSize = this->m_mainLayer->getContentSize();
	const float idealWidth = layerSize.width * .95f;
	const float centerStage = layerSize.width / 2.f;
	CCLayer* mainLayer = this->m_mainLayer;
	mainLayer->setID("main-layer"_spr);

	this->m_title->setPositionY(this->m_title->getPositionY() + 2.f);
	this->m_title->setPositionX(mainLayer->getContentWidth() / 2.f);

	this->m_songControlsMenu = cocos2d::CCMenu::create();

	Utils::addButton("shuffle", menu_selector(SongControlMenu::onShuffleButton), REST_OF_THE_OWL);
	Utils::addButton("regen", menu_selector(SongControlMenu::onRegenButton), REST_OF_THE_OWL);
	Utils::addButton("copy", menu_selector(SongControlMenu::onCopyButton), REST_OF_THE_OWL);
	Utils::addButton("blacklist", menu_selector(SongControlMenu::onBlacklistButton), REST_OF_THE_OWL);
	Utils::addButton("favorite", menu_selector(SongControlMenu::onFavoriteButton), REST_OF_THE_OWL);
	Utils::addButton("hold", menu_selector(SongControlMenu::onHoldSongButton), REST_OF_THE_OWL);
	Utils::addButton("prev", menu_selector(SongControlMenu::onPreviousButton), REST_OF_THE_OWL);
	if (!Utils::getBool("loadPlaylistFile") || SongManager::get().getPlaylistIsEmpty()) {
		Utils::addButton("add", menu_selector(SongControlMenu::onAddToPlylstBtn), REST_OF_THE_OWL);
	}

	this->m_songControlsMenu->setPosition({centerStage, 40.f});
	this->m_songControlsMenu->ignoreAnchorPointForPosition(false);
	this->m_songControlsMenu->setContentSize({idealWidth, 32.f});
	this->m_songControlsMenu->setLayout(geode::RowLayout::create()->setGap(5.f)->setDefaultScaleLimits(.0001f, 1.0f));

	SongManager& songManager = SongManager::get();
	this->m_infoMenu = cocos2d::CCMenu::create();
	this->m_infoMenu->setLayout(geode::RowLayout::create()->setAutoScale(false)->setAxis(geode::Axis::Row)->setGap(.0f));
	this->m_infoMenu->setContentSize({24.f * .75f, 23.f * .75f});
	this->m_infoButton = InfoAlertButton::create(
		"Menu Loop Randomizer - Debug/FAQ",
		fmt::format(
			"Platform: <cl>{}</c>\n"
			"Constant Shuffle Mode: <cl>{}</c>\n"
			"Current playlist file: <cl>{}</c> (Loaded: <cl>{}</c>)\n\n"
			"<cy>Q: Why do I see overlapping elements?</c>\n"
			"A: Install <cl>Happy Textures by Alphalaneous</c>.\n<c_>This is a non-negotiable solution.</c>\n\n"
			"<cy>Q: Add</c> <cl>[feature suggestion that makes this mod act like Spotify or EditorMusic, but with the menu loop]</c><cy>!</c>\n"
			"A: <c_>No. Never.</c>",
			Utils::getPlatform(),
			songManager.getConstantShuffleMode() ? "ON" : "OFF",
			songManager.getPlaylistFileName(),
			Utils::getBool("loadPlaylistFile") ? "True" : "False"
		),
		.75f
	);
	this->m_infoMenu->addChildAtPosition(this->m_infoButton, geode::Anchor::Center);
	this->m_infoMenu->setPosition({layerSize - 3.f});
	this->m_infoMenu->setID("control-panel-info-menu"_spr);
	this->m_infoButton->setID("control-panel-info-button"_spr);
	this->m_mainLayer->addChild(m_infoMenu);

	this->m_theTimeoutCorner = cocos2d::CCMenu::create();
	Utils::addButton("settings", menu_selector(SongControlMenu::onSettingsButton), this->m_theTimeoutCorner, this);
	this->m_theTimeoutCorner->setPosition({280.f, this->m_title->getPositionY() - 2.5f});
	this->m_theTimeoutCorner->ignoreAnchorPointForPosition(false);
	this->m_theTimeoutCorner->setContentSize({24.f, 24.f});
	this->m_theTimeoutCorner->setLayout(geode::ColumnLayout::create()->setGap(0.f)->setDefaultScaleLimits(.0001f, 1.0f)->setAutoScale(true));

	this->m_openSongListMenu = cocos2d::CCMenu::create();
	Utils::addButton("playlist", menu_selector(SongControlMenu::onPlaylistButton), this->m_openSongListMenu, this);
	this->m_openSongListMenu->getChildren()->objectAtIndex(0)->setTag(20260105);
	this->m_openSongListMenu->setPosition({280.f, this->m_title->getPositionY() - 51.5f});
	this->m_openSongListMenu->ignoreAnchorPointForPosition(false);
	this->m_openSongListMenu->setContentSize({27.f, 27.f});
	this->m_openSongListMenu->setLayout(geode::RowLayout::create()->setGap(0.f)->setDefaultScaleLimits(.0001f, 1.0f)->setAutoScale(true));

	geode::LoadingSpinner* loadingSpinner = geode::LoadingSpinner::create(15.f);
	loadingSpinner->setVisible(true);
	loadingSpinner->stopAllActions();
	loadingSpinner->setVisible(false);
	loadingSpinner->setID("loading-spinner-because-song-length-calculations"_spr);
	loadingSpinner->setTag(20260104);
	loadingSpinner->runAction(cocos2d::CCRepeatForever::create(cocos2d::CCRotateBy::create(1.f, 360.f)));
	this->m_openSongListMenu->addChildAtPosition(loadingSpinner, geode::Anchor::Center);

	this->m_otherLabel = cocos2d::CCLabelBMFont::create(DEFAULT_FOOTER_TEXT.c_str(), "chatFont.fnt");
	this->m_otherLabel->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	this->m_otherLabel->limitLabelWidth(idealWidth * .95f, 1.0f, .0001f);
	this->m_otherLabel->setPosition({centerStage, 12.5f});

	this->m_headerLabl = cocos2d::CCLabelBMFont::create("Current Song:", "chatFont.fnt");
	this->m_headerLabl->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	this->m_headerLabl->setPosition({centerStage, 107.5f});

	this->b = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
	this->b->ignoreAnchorPointForPosition(false);
	this->b->setContentSize({250.f, 30.f});
	this->b->setAnchorPoint({0.f, .5f});
	this->b->setPosition({12.5f, 80.f});
	this->b->setColor({0, 0, 0});
	this->b->setOpacity(128);

	this->schedule(schedule_selector(SongControlMenu::checkManagerFinished));

	SongControlMenu::updateCurrentLabel();

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
		this->m_mainLayer->addChildAtPosition(customSongsFolderBottom, geode::Anchor::Bottom, {0, -26.f});
		this->m_mainLayer->addChildAtPosition(customSongsFolderTop, geode::Anchor::Top, {0, 26.f});
	}
	*/

	this->setID("SongControlMenu"_spr);
	mainLayer->addChild(this->b);
	mainLayer->addChild(this->m_otherLabel);
	mainLayer->addChild(this->m_headerLabl);
	mainLayer->addChild(this->m_songControlsMenu);
	mainLayer->addChild(this->m_openSongListMenu);
	mainLayer->addChild(this->m_theTimeoutCorner);

	this->b->setID("trans-bg"_spr);
	this->m_title->setID("title"_spr);
	this->m_bgSprite->setID("background"_spr);
	this->m_closeBtn->setID("close-button"_spr);
	this->m_buttonMenu->setID("close-menu"_spr);
	this->m_otherLabel->setID("im-not-spotify"_spr);
	this->m_smallLabel->setID("current-song-label"_spr);
	this->m_headerLabl->setID("current-song-header"_spr);
	this->m_theTimeoutCorner->setID("timeout-corner"_spr);
	this->m_openSongListMenu->setID("song-list-menu"_spr);
	this->m_songControlsMenu->setID("song-controls-menu"_spr);

	this->m_noElasticity = true;

	return true;
}

SongControlMenu* SongControlMenu::create() {
	SongControlMenu* ret = new SongControlMenu();
	if (ret->initAnchored(300.f, 150.f, "GJ_square05.png")) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

void SongControlMenu::onExit() {
	Popup::onExit();
}

void SongControlMenu::toggleButtonState(cocos2d::CCNode* playlistButton, const bool isEnabled) const {
	if (!playlistButton || !geode::cast::typeinfo_cast<CCMenuItemSpriteExtra*>(playlistButton)) return;
	if (isEnabled == static_cast<CCMenuItemSpriteExtra*>(playlistButton)->isEnabled()) return;
	static_cast<CCMenuItemSpriteExtra*>(playlistButton)->setEnabled(isEnabled);
	static_cast<CCMenuItemSpriteExtra*>(playlistButton)->setColor(isEnabled ? cocos2d::ccColor3B{255, 255, 255} : cocos2d::ccColor3B{128, 128, 128});
	if (this->m_otherLabel) this->m_otherLabel->setString(isEnabled ? DEFAULT_FOOTER_TEXT.c_str() : "Hey there! Menu Loop Randomizer is finishing some things to set up the Song List. Hang tight!");
	if (this->m_otherLabel) this->m_otherLabel->limitLabelWidth(this->m_mainLayer->getContentWidth() * .95f * .95f, 1.0f, .0001f);
	if (this->m_openSongListMenu) if (CCNode* spinner = this->m_openSongListMenu->getChildByTag(20260104); spinner) static_cast<geode::LoadingSpinner*>(spinner)->setVisible(!isEnabled);
}

void SongControlMenu::checkManagerFinished(float) {
	SongControlMenu::toggleButtonState(this->m_openSongListMenu->getChildByTag(20260105), SongManager::get().getFinishedCalculatingSongLengths());
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
	if (!SongManager::get().getFinishedCalculatingSongLengths()) {
		FLAlertLayer* alert = FLAlertLayer::create("Hold on a sec!", "Menu Loop Randomizer is still calculating song lengths to set up the Song List menu. Hang tight, and try again later.", "I Understand");
		alert->m_noElasticity = true;
		alert->show();
		return;
	}
	SongControlMenu::onClose(nullptr);
	SongListLayer::create()->show();
}

void SongControlMenu::onAddToPlylstBtn(CCObject*) {
	SongManager& songManager = SongManager::get();
	if (Utils::getBool("loadPlaylistFile") && !songManager.getPlaylistIsEmpty()) {
		geode::log::info("tried adding current song to a playlist file. BOOOOOOOOOOOOO");
		return geode::Notification::create(
			"You've already loaded an MLR playlist file!",
			geode::NotificationIcon::Error, 10.f
		)->show();
	}
	SongControl::addSongToPlaylist(songManager.getCurrentSong());
	SongControlMenu::updateCurrentLabel();
}

void SongControlMenu::onSettingsButton(CCObject*) {
	geode::openSettingsPopup(geode::Mod::get());
	if (this->m_infoMenu) this->m_infoMenu->setScale(0.f);
	if (this->m_infoButton) {
		this->m_infoButton->setScale(0.f);
		this->m_infoButton->setEnabled(false);
		this->m_infoButton->m_pfnSelector = nullptr;
	}
}

void SongControlMenu::updateCurrentLabel() {
	SongManager& songManager = SongManager::get();
	songManager.resetTowerRepeatCount();
	const std::string& currentSong = songManager.getCurrentSongDisplayName();
	if (!this->m_smallLabel || !this->m_smallLabel->getParent() || this->m_smallLabel->getParent() != this->b) {
		this->m_smallLabel = cocos2d::CCLabelBMFont::create(currentSong.c_str(), "chatFont.fnt");
		this->b->addChildAtPosition(this->m_smallLabel, geode::Anchor::Center/*, {0.f, 2.5f}*/);
	} else this->m_smallLabel->setString(currentSong.c_str(), "chatFont.fnt");
	this->m_smallLabel->limitLabelWidth((this->b->getContentWidth() - 20.f) * .85f, 1.0f, .0001f);
	this->m_smallLabel->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	if (!this->m_headerLabl) return;
	if (songManager.isOverride()) this->m_headerLabl->setString("Current Song (Custom Override):");
	else if (songManager.getConstantShuffleMode()) this->m_headerLabl->setString("Current Song (Constant Shuffle Mode):");
	else this->m_headerLabl->setString("Current Song:");
	if (!songManager.getPlaylistIsEmpty() && Utils::getBool("loadPlaylistFile") && !songManager.isOverride()) {
		const std::string& newHeaderString = geode::utils::string::replace(this->m_headerLabl->getString(), "Song", fmt::format("Song from {}", songManager.getPlaylistFileName()));
		this->m_headerLabl->setString(newHeaderString.c_str());
	}
	this->m_headerLabl->limitLabelWidth(this->m_mainLayer->getContentSize().width * .95f * .95f, 1.0f, .0001f);
}