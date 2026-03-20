#include <Geode/ui/GeodeUI.hpp>
#include "SongControlMenu.hpp"
#include "SongListLayer.hpp"
#include "../Utils.hpp"
#include "../SongControl.hpp"

#define REST_OF_THE_OWL this->m_songControlsMenu, this
#define DEFAULT_FOOTER_TEXT fmt::format("Hi! Menu Loop Randomizer will never resemble Spotify or its distant cousin EditorMusic. Please respect that. :) [Platform: {}]", Utils::getPlatform())
#define CAN_SHOW_PLAYBACK_PROGRESS songManager.getShowPlaybackProgressAndControls() && !songManager.isOverride() && !VANILLA_GD_MENU_LOOP_DISABLED && Utils::songDataContainsSong(songManager.getCurrentSong())

#if GEODE_COMP_GD_VERSION == 22081
bool SongControlMenu::init() {
#endif
#if GEODE_COMP_GD_VERSION == 22074
bool SongControlMenu::setup() {
#endif
	#if GEODE_COMP_GD_VERSION == 22081
	if (!geode::Popup::init(300.f, 150.f, "GJ_square05.png")) return false;
	#endif

	this->setTitle("Menu Loop Randomizer - Control Panel");
	this->m_title->setScale(.45f);

	const cocos2d::CCSize layerSize = this->m_mainLayer->getContentSize();
	const float idealWidth = layerSize.width * .95f;
	const float centerStage = layerSize.width / 2.f;
	cocos2d::CCLayer* mainLayer = this->m_mainLayer;
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


	#if GEODE_COMP_GD_VERSION == 22081
	this->b = geode::NineSlice::create("square02b_001.png");
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	this->b = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
	#endif
	this->b->ignoreAnchorPointForPosition(false);
	this->b->setContentSize({250.f, 30.f});
	this->b->setAnchorPoint({0.f, .5f});
	this->b->setPosition({12.5f, 80.f});
	this->b->setColor({0, 0, 0});
	this->b->setOpacity(128);

	#if GEODE_COMP_GD_VERSION == 22081
	this->w = geode::NineSlice::create("square02b_001.png");
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	this->w = cocos2d::extension::CCScale9Sprite::create("square02b_001.png");
	#endif
	this->w->setContentSize(this->m_mainLayer->getContentSize());
	this->w->setPosition(this->w->getContentSize() / 2.f);
	this->w->ignoreAnchorPointForPosition(false);
	this->w->setAnchorPoint({.5f, .5f});
	this->w->setColor({255, 255, 255});
	this->w->setZOrder(-8124);
	this->w->setOpacity(0);
	this->w->setTag(8124);

	this->m_mainLayer->addChild(this->w);

	this->schedule(schedule_selector(SongControlMenu::checkManagerFinished));

	cocos2d::CCMenu* incrementDecrementMenu = cocos2d::CCMenu::create();
	incrementDecrementMenu->setContentWidth(240.f);
	this->m_ffwdButton = Utils::addButton("skip-bkwd", menu_selector(SongControlMenu::onSkipBkwdButton), incrementDecrementMenu, this, true);
	this->m_bkwdButton = Utils::addButton("skip-fwrd", menu_selector(SongControlMenu::onSkipFwrdButton), incrementDecrementMenu, this, true);
	incrementDecrementMenu->setLayout(geode::RowLayout::create()->setAutoScale(true)->setGap(1000.f)->setDefaultScaleLimits(.0001f, 1.f));
	this->m_increDecreMenu = incrementDecrementMenu;
	// DONT ADD this->m_increDecreMenu AS A CHILD -- SongControlMenu::updateCurrentLabel() HANDLES THAT!

	cocos2d::CCLabelBMFont* currentPosition = cocos2d::CCLabelBMFont::create("PLACEHOLDER", "chatFont.fnt");
	currentPosition->setAnchorPoint({0.f, 0.f});
	this->m_currTimeLb = currentPosition;
	// DONT ADD this->m_currTimeLb AS A CHILD -- SongControlMenu::updateCurrentLabel() HANDLES THAT!

	cocos2d::CCLabelBMFont* totalLength = cocos2d::CCLabelBMFont::create("PLACEHOLDER", "chatFont.fnt");
	totalLength->setAnchorPoint({1.f, 0.f});
	this->m_totlTimeLb = totalLength;
	// DONT ADD this->m_totlTimeLb AS A CHILD -- SongControlMenu::updateCurrentLabel() HANDLES THAT!

	cocos2d::CCLayerColor* currProgBar = cocos2d::CCLayerColor::create({200, 200, 200, 255});
	currProgBar->setContentSize({0.f, 2.5f});
	currProgBar->setAnchorPoint({0.f, 0.f});
	this->m_currProgBar = currProgBar;

	cocos2d::CCLayerColor* darkProgBar = cocos2d::CCLayerColor::create({10, 10, 10, 255});
	darkProgBar->setContentSize({this->b->getContentWidth(), 2.5f});
	darkProgBar->setAnchorPoint({0.f, 0.f});
	darkProgBar->addChild(currProgBar);
	this->m_darkProgBar = darkProgBar;

	cocos2d::CCClippingNode* clippingNode = cocos2d::CCClippingNode::create(darkProgBar);
	clippingNode->setContentSize(darkProgBar->getContentSize());
	clippingNode->setAnchorPoint({0.f, 0.f});
	clippingNode->addChild(darkProgBar);
	this->m_clipNode = clippingNode;

	this->m_audieoVisual = FMODLevelVisualizer::create();
	this->m_audieoVisual->setID("budget-audio-visualizer-inspired-by-osu-first"_spr);
	this->m_audieoVisual->setPosition({50.f, -10.f});
	this->m_audieoVisual->setRotation(90.f);

	for (cocos2d::CCSprite* spriteChild : geode::cocos::CCArrayExt<cocos2d::CCSprite*>(static_cast<cocos2d::CCSpriteBatchNode*>(this->m_audieoVisual->getChildByIndex(0))->m_pChildren)) {
		spriteChild->setOpacity(0);
	}

	this->m_mainLayer->addChild(this->m_audieoVisual);

	this->m_musicVolCntnr = cocos2d::CCNodeRGBA::create();
	this->m_musicVolCntnr->setPosition({this->m_mainLayer->getContentWidth(), this->m_audieoVisual->getPositionY()});
	this->m_musicVolCntnr->setContentSize({150.f, 20.f});
	this->m_musicVolCntnr->setAnchorPoint({1.f, .5f});
	this->m_musicVolCntnr->setScale(.5f);

	this->m_musVolLabl = cocos2d::CCLabelBMFont::create("Music Volume:", "chatFont.fnt");
	this->m_musVolLabl->setOpacity(0);

	this->m_originalVolume = std::clamp<int>(FMODAudioEngine::get()->getBackgroundMusicVolume() * 100.f, 0, 100);
	const std::string& stringifiedVolume = geode::utils::numToString(this->m_originalVolume);
	this->m_musicVolumeInput = geode::TextInput::create(50.f, stringifiedVolume, "chatFont.fnt");
	this->m_musicVolumeInput->setCommonFilter(geode::CommonFilter::Uint);
	this->m_musicVolumeInput->setCallback([this](const std::string& inputString) {
		FMODAudioEngine::get()->setBackgroundMusicVolume(std::clamp<int>(geode::utils::numFromString<int>(inputString).unwrapOr(this->m_originalVolume), 0, 100) / 100.f);
	});
	this->m_musicVolumeInput->setString(stringifiedVolume);

	#if GEODE_COMP_GD_VERSION < 22081
	this->m_musicVolumeInput->getChildByType<cocos2d::extension::CCScale9Sprite>(0)->setOpacity(0);
	#endif
	#if GEODE_COMP_GD_VERSION > 22074
	this->m_musicVolumeInput->getChildByType<geode::NineSlice>(0)->setOpacity(0);
	#endif
	this->m_musicVolumeInput->getInputNode()->m_textField->setOpacity(0);
	this->m_musicVolumeInput->getInputNode()->m_textLabel->setOpacity(0);
	this->m_musicVolumeInput->getInputNode()->m_cursor->setOpacity(0);

	#if GEODE_COMP_GD_VERSION < 22081
	this->m_musicVolumeInput->getChildByType<cocos2d::extension::CCScale9Sprite>(0)->_scale9Image->setID("music-volume-input-background-batch-node"_spr);
	this->m_musicVolumeInput->getChildByType<cocos2d::extension::CCScale9Sprite>(0)->setID("music-volume-input-background"_spr);
	#endif
	#if GEODE_COMP_GD_VERSION > 22074
	this->m_musicVolumeInput->getChildByType<geode::NineSlice>(0)->setID("music-volume-input-background"_spr);
	#endif
	this->m_musicVolumeInput->getInputNode()->m_textField->setID("music-volume-input-text-field"_spr);
	this->m_musicVolumeInput->getInputNode()->m_textLabel->setID("music-volume-input-text-label"_spr);
	this->m_musicVolumeInput->getInputNode()->m_cursor->setID("music-volume-input-text-cursor"_spr);
	this->m_musicVolumeInput->getInputNode()->setID("music-volume-input-node"_spr);

	this->m_musicVolumeInput->setID("music-volume-input"_spr);
	this->m_musicVolCntnr->addChild(this->m_musicVolumeInput);
	this->m_musVolLabl->setID("music-volume-label"_spr);
	this->m_musicVolCntnr->addChild(this->m_musVolLabl);

	this->m_musicVolCntnr->setLayout(geode::RowLayout::create()->setAxisReverse(true));
	this->m_musicVolCntnr->setID("music-volume-container"_spr);
	this->m_mainLayer->addChild(this->m_musicVolCntnr);

	this->schedule(schedule_selector(SongControlMenu::visualizerScheduler));

	SongControlMenu::forceSharpCornerIllusion();
	SongControlMenu::updateCurrentLabel();

	this->m_osu = false;

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
	this->w->setID("ripple-bg"_spr);
	this->m_title->setID("title"_spr);
	this->m_clipNode->setID("clip-node"_spr);
	this->m_bgSprite->setID("background"_spr);
	this->m_closeBtn->setID("close-button"_spr);
	this->m_buttonMenu->setID("close-menu"_spr);
	this->m_otherLabel->setID("im-not-spotify"_spr);
	this->m_darkProgBar->setID("dark-prog-bar"_spr);
	this->m_currProgBar->setID("curr-prog-bar"_spr);
	this->m_totlTimeLb->setID("total-time-label"_spr);
	this->m_smallLabel->setID("current-song-label"_spr);
	this->m_currTimeLb->setID("current-time-label"_spr);
	this->m_headerLabl->setID("current-song-header"_spr);
	this->m_increDecreMenu->setID("incdec-time-menu"_spr);
	this->m_theTimeoutCorner->setID("timeout-corner"_spr);
	this->m_openSongListMenu->setID("song-list-menu"_spr);
	#if GEODE_COMP_GD_VERSION == 22081
	this->b->getBatchNode()->setID("the-darn-scale-9"_spr);
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	this->b->_scale9Image->setID("the-darn-scale-9"_spr);
	#endif
	#if GEODE_COMP_GD_VERSION == 22081
	this->w->getBatchNode()->setID("osu-darn-scale-9"_spr);
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	this->w->_scale9Image->setID("osu-darn-scale-9"_spr);
	#endif
	this->m_songControlsMenu->setID("song-controls-menu"_spr);
	#if GEODE_COMP_GD_VERSION == 22081
	this->m_bgSprite->getBatchNode()->setID("the-less-darned-sprite-9"_spr);
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	this->m_bgSprite->_scale9Image->setID("the-less-darned-sprite-9"_spr);
	#endif

	this->m_noElasticity = true;
	this->m_isInQOLMod = songManager.addingToQOLModRightNow;

	if (!MenuLayer::get() || !cocos2d::CCScene::get() || cocos2d::CCScene::get()->getChildByType<MenuLayer>(0) != MenuLayer::get() || this->m_isInQOLMod) {
		if (this->m_theTimeoutCorner) {
			this->m_theTimeoutCorner->removeMeAndCleanup();
			this->m_theTimeoutCorner = nullptr;
		}
		if (this->m_musicVolumeInput) {
			this->m_musicVolumeInput->removeMeAndCleanup();
			this->m_musicVolumeInput = nullptr;
		}
		if (this->m_musVolLabl) {
			this->m_musVolLabl->removeMeAndCleanup();
			this->m_musVolLabl = nullptr;
		}
		if (this->m_musicVolCntnr) {
			this->m_musicVolCntnr->removeMeAndCleanup();
			this->m_musicVolCntnr = nullptr;
		}
		if (this->m_audieoVisual) {
			this->m_audieoVisual->removeMeAndCleanup();
			this->m_audieoVisual = nullptr;
		}
	}

	if (this->m_isInQOLMod) {
		this->unschedule(schedule_selector(SongControlMenu::visualizerScheduler));
		this->unschedule(schedule_selector(SongControlMenu::checkManagerFinished));
		if (this->m_closeBtn) this->m_closeBtn->removeMeAndCleanup();
		if (this->m_openSongListMenu->getChildByTag(20260105)) {
			static_cast<CCMenuItemSpriteExtra*>(this->m_openSongListMenu->getChildByTag(20260105))->setEnabled(false);
			static_cast<CCMenuItemSpriteExtra*>(this->m_openSongListMenu->getChildByTag(20260105))->setColor(cocos2d::ccColor3B{128, 128, 128});
			static_cast<CCMenuItemSpriteExtra*>(this->m_openSongListMenu->getChildByTag(20260105))->m_pListener = nullptr;
			static_cast<CCMenuItemSpriteExtra*>(this->m_openSongListMenu->getChildByTag(20260105))->m_animationEnabled = false;
		}
		if (this->m_otherLabel) {
			this->m_otherLabel->setString("Hey there! You're currently viewing this menu inside QOLMod. Features are limited here, including the Song List, keybinds, and shortcuts.");
		}
		if (this->m_infoMenu) {
			this->m_infoMenu->removeMeAndCleanup();
			this->m_infoMenu = nullptr;
		}
		this->setOpacity(0);
	}

	return true;
}

SongControlMenu* SongControlMenu::create() {
	SongControlMenu* ret = new SongControlMenu();
	#if GEODE_COMP_GD_VERSION == 22081
	if (ret->init()) {
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	if (ret->initAnchored(300.f, 150.f, "GJ_square05.png")) {
	#endif
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

void SongControlMenu::onExit() {
	Popup::onExit();
}

void SongControlMenu::onClose(cocos2d::CCObject* sender) {
	if (this->m_isInQOLMod) return;
	if (MenuLayer::get() && !MenuLayer::get()->isVisible() && this->m_osu) MenuLayer::get()->setVisible(true);
	Popup::onClose(sender);
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

void SongControlMenu::forceSharpCornerIllusion() {
	if (!this->b) return;

	#if GEODE_COMP_GD_VERSION == 22081
	if (this->b->getBottom()) this->b->getBottom()->setPositionX(0.f);
	if (this->b->getBottom()) this->b->getBottom()->setScaleX(9.375f);
	if (this->b->getBottomLeft()) this->b->getBottomLeft()->setVisible(false);
	if (this->b->getBottomRight()) this->b->getBottomRight()->setVisible(false);
	#endif

	#if GEODE_COMP_GD_VERSION == 22074
	if (this->b->_bottom) this->b->_bottom->setPositionX(0.f);
	if (this->b->_bottom) this->b->_bottom->setScaleX(9.375f);
	if (this->b->_bottomLeft) this->b->_bottomLeft->setVisible(false);
	if (this->b->_bottomRight) this->b->_bottomRight->setVisible(false);
	#endif

	SongManager& songManager = SongManager::get();
	const bool canShowPlaybackProgress = CAN_SHOW_PLAYBACK_PROGRESS;

	if (!this->m_smallLabel) return;
	this->m_smallLabel->setPositionY(canShowPlaybackProgress ? 18.5f : 15.f);
	this->m_smallLabel->limitLabelWidth((this->b->getContentWidth() - 20.f) * (canShowPlaybackProgress ? .85f : 1.f), (canShowPlaybackProgress ? .85f : 1.f), .0001f);
}

void SongControlMenu::forceSharpCornerIllusionScheduler(const float) {
	SongControlMenu::forceSharpCornerIllusion();
}

void SongControlMenu::checkDaSongPositions(float) {
	SongManager& songManager = SongManager::get();
	const bool canShowPlaybackProgress = CAN_SHOW_PLAYBACK_PROGRESS;

	if (this->m_currTimeLb) this->m_currTimeLb->setVisible(canShowPlaybackProgress);
	if (this->m_totlTimeLb) this->m_totlTimeLb->setVisible(canShowPlaybackProgress);
	if (this->m_currProgBar) this->m_currProgBar->setVisible(canShowPlaybackProgress);
	if (this->m_darkProgBar) this->m_darkProgBar->setVisible(canShowPlaybackProgress);
	if (this->m_increDecreMenu) this->m_increDecreMenu->setVisible(canShowPlaybackProgress);
	if (this->m_clipNode) this->m_clipNode->setVisible(canShowPlaybackProgress);

	if (!canShowPlaybackProgress) return;
	if (!this->m_currTimeLb || !this->m_totlTimeLb || !this->m_currProgBar || !this->m_darkProgBar) return;

	FMODAudioEngine* fmod = FMODAudioEngine::get();
	const std::string& currSong = songManager.getCurrentSong();
	if (fmod->getActiveMusic(0) != currSong) return;

	const int fullLength = fmod->getMusicLengthMS(0);
	const int lastPosition = songManager.getLastMenuLoopPosition();

	this->m_currTimeLb->setString(fmt::format("{}:{:02}", ((lastPosition / 1000) / 60), ((lastPosition / 1000) % 60)).c_str());
	this->m_totlTimeLb->setString(fmt::format("{}:{:02}", ((fullLength / 1000) / 60), ((fullLength / 1000) % 60)).c_str());

	this->m_currProgBar->setContentWidth(std::clamp<float>((1.f * lastPosition) / (1.f * std::max<int>(fullLength, fmod->getMusicLengthMS(0))), 0.f, 1.f) * this->m_darkProgBar->getContentWidth());
}

void SongControlMenu::pressAndHoldScheduler(float dt) {
	m_time += dt;
	if (!this->m_ffwdButton || !this->m_bkwdButton) {
		m_time = 0.f;
		return;
	}

	if (m_time < .5f) return;

	if (this->m_ffwdButton->isSelected() && this->m_ffwdButton->isVisible() && this->m_ffwdButton->isEnabled() && this->m_ffwdButton->m_pfnSelector && this->m_ffwdButton->m_pListener) (this->m_ffwdButton->m_pListener->*this->m_ffwdButton->m_pfnSelector)(this->m_ffwdButton);
	else if (this->m_bkwdButton->isSelected() && this->m_bkwdButton->isVisible() && this->m_bkwdButton->isEnabled() && this->m_bkwdButton->m_pfnSelector && this->m_bkwdButton->m_pListener) (this->m_bkwdButton->m_pListener->*this->m_bkwdButton->m_pfnSelector)(this->m_bkwdButton);
	else m_time = 0;
}

void SongControlMenu::visualizerScheduler(float dt) {
	FMODAudioEngine* fmod = FMODAudioEngine::get();
	if (!this->m_audieoVisual || !fmod) return;
	if (!fmod->m_metering) fmod->enableMetering();
	this->m_audieoVisual->updateVisualizer(fmod->m_pulse1, 0.f, dt); // no idea how to fill in the second param lol
}

void SongControlMenu::onShuffleButton(cocos2d::CCObject*) {
	SongControl::shuffleSong();
	SongControlMenu::updateCurrentLabel();
}

void SongControlMenu::onRegenButton(cocos2d::CCObject*) {
	SongControl::regenSong();
	SongControlMenu::updateCurrentLabel();
}

void SongControlMenu::onCopyButton(cocos2d::CCObject*) {
	SongControl::copySong();
	SongControlMenu::updateCurrentLabel();
}

void SongControlMenu::onBlacklistButton(cocos2d::CCObject*) {
	SongControl::blacklistSong();
	SongControlMenu::updateCurrentLabel();
}

void SongControlMenu::onFavoriteButton(cocos2d::CCObject*) {
	SongControl::favoriteSong();
	SongControlMenu::updateCurrentLabel();
}

void SongControlMenu::onHoldSongButton(cocos2d::CCObject*) {
	SongControl::holdSong();
	SongControlMenu::updateCurrentLabel();
}

void SongControlMenu::onPreviousButton(cocos2d::CCObject*) {
	SongControl::previousSong();
	SongControlMenu::updateCurrentLabel();
}

void SongControlMenu::onPlaylistButton(cocos2d::CCObject*) {
	if (this->m_isInQOLMod) return;
	if (!SongManager::get().getFinishedCalculatingSongLengths()) {
		FLAlertLayer* alert = FLAlertLayer::create("Hold on a sec!", "Menu Loop Randomizer is still calculating song lengths to set up the Song List menu. Hang tight, and try again later.", "I Understand");
		alert->m_noElasticity = true;
		alert->show();
		return;
	}
	if (MenuLayer::get() && !MenuLayer::get()->isVisible() && this->m_osu) MenuLayer::get()->setVisible(true);
	SongControlMenu::onClose(nullptr);
	SongListLayer::create()->show();
}

void SongControlMenu::onAddToPlylstBtn(cocos2d::CCObject*) {
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

void SongControlMenu::onSettingsButton(cocos2d::CCObject*) {
	geode::openSettingsPopup(geode::Mod::get());
	if (this->m_infoMenu) this->m_infoMenu->setScale(0.f);
	if (this->m_infoButton) {
		this->m_infoButton->setScale(0.f);
		this->m_infoButton->setEnabled(false);
		this->m_infoButton->m_pfnSelector = nullptr;
	}
}

void SongControlMenu::onSkipBkwdButton(cocos2d::CCObject*) {
	SongControl::skipBackward();
}

void SongControlMenu::onSkipFwrdButton(cocos2d::CCObject*) {
	SongControl::skipForward();
}

void SongControlMenu::updateCurrentLabel() {
	SongManager& songManager = SongManager::get();
	songManager.resetTowerRepeatCount();
	const std::string& currentSong = !songManager.getFinishedCalculatingSongLengths() || !Utils::songDataContainsSong(songManager.getCurrentSong()) ? songManager.getCurrentSongDisplayName() : Utils::getSongDataOfCurrentSong().fullDisplayNameForControlPanelAndSongList;
	if (!this->m_smallLabel || !this->m_smallLabel->getParent() || this->m_smallLabel->getParent() != this->b) {
		this->m_smallLabel = cocos2d::CCLabelBMFont::create(currentSong.c_str(), "chatFont.fnt");
		this->b->addChildAtPosition(this->m_smallLabel, geode::Anchor::Center, {0.f, 3.5f});
		this->b->addChildAtPosition(this->m_increDecreMenu, geode::Anchor::Center);
		this->b->addChildAtPosition(this->m_currTimeLb, geode::Anchor::BottomLeft, {27.5f, 6.f});
		this->b->addChildAtPosition(this->m_totlTimeLb, geode::Anchor::BottomRight, {-27.5f, 6.f});
		this->b->addChildAtPosition(this->m_clipNode, geode::Anchor::BottomLeft);
		this->m_currTimeLb->setScale(.35f);
		this->m_totlTimeLb->setScale(.35f);
		this->schedule(schedule_selector(SongControlMenu::checkDaSongPositions), 2.f / 60.f);
		this->schedule(schedule_selector(SongControlMenu::forceSharpCornerIllusionScheduler));
		if (this->m_ffwdButton && this->m_bkwdButton) {
			this->schedule(schedule_selector(SongControlMenu::pressAndHoldScheduler), .125f);
		}
	} else this->m_smallLabel->setString(currentSong.c_str(), "chatFont.fnt");
	this->m_smallLabel->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	this->m_currTimeLb->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	this->m_totlTimeLb->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
	if (!this->m_headerLabl) return;
	if (songManager.isOverride()) this->m_headerLabl->setString("Current Song (Custom Override):");
	else if (songManager.getConstantShuffleMode()) this->m_headerLabl->setString("Current Song (Constant Shuffle Mode):");
	else this->m_headerLabl->setString("Current Song:");
	if (!songManager.getPlaylistIsEmpty() && Utils::getBool("loadPlaylistFile") && !songManager.isOverride()) {
		const std::string& newHeaderString = geode::utils::string::replace(this->m_headerLabl->getString(), "Song", fmt::format("Song from {}", songManager.getPlaylistFileName()));
		this->m_headerLabl->setString(newHeaderString.c_str());
	}
	this->m_headerLabl->limitLabelWidth(this->m_mainLayer->getContentSize().width * .95f * .95f, 1.0f, .0001f);
	if (!Utils::getBool("songIndicatorsInControlPanel")) {
		this->m_smallLabel->setSkewX(0.f);
		this->m_smallLabel->setColor({255, 255, 255});
		return;
	}
	if (!Utils::songDataContainsSong(songManager.getCurrentSong())) return;
	SongData& songData = Utils::getSongDataOfCurrentSong();
	const SongType songType = songData.type;
	this->m_smallLabel->setSkewX(0.f);
	this->m_smallLabel->setColor({255, 255, 255});
	if (songData.isInNonVanillaNGMLSongLocation && !songData.isFromConfigOrAltDir && songData.couldPossiblyExistInMusicDownloadManager && !songData.isFromJukeboxDirectory) this->m_smallLabel->setSkewX(10.f);
	if (songType == SongType::Favorited) this->m_smallLabel->setColor({255, 175, 0});
	else if (songType == SongType::Blacklisted) this->m_smallLabel->setColor({0, 0, 0});
}

void SongControlMenu::toggleMenuLayerVisibility() {
	MenuLayer::get()->setVisible(!this->m_osu);
}

void SongControlMenu::doTheRippleEffectFromOsuLazer() {
	if (!this->w) return;
	this->w->stopAllActions();
	this->w->setScale(1.f);
	if (this->m_osu) {
		this->w->setOpacity(0);
		this->w->setVisible(false);
	} else {
		this->w->setOpacity(255);
		this->w->setVisible(true);
		// easing types and easing values taken from https://github.com/ppy/osu/blob/master/osu.Game/Screens/Menu/OsuLogo.cs#L429 under the MIT license, but they're just silly numbers at the end of the day
		this->w->runAction(
			cocos2d::CCSequence::create(
				cocos2d::CCSpawn::create(
					cocos2d::CCEaseIn::create(cocos2d::CCScaleTo::create(.25f, 1.12f), .5f),
					cocos2d::CCEaseIn::create(cocos2d::CCFadeOut::create(.25f), .5f),
					nullptr
				),
				cocos2d::CCScaleTo::create(0.f, 1.f),
				nullptr
			)
		);
	}
}

void SongControlMenu::toggleOsu() {
	if (this->m_isInQOLMod) return;
	if (!MenuLayer::get() || !cocos2d::CCScene::get() || cocos2d::CCScene::get()->getChildByType<MenuLayer>(0) != MenuLayer::get()) return;
	this->m_osu = !this->m_osu;
	this->stopAllActions();
	this->m_mainLayer->stopAllActions();
	if (this->m_musVolLabl) this->m_musVolLabl->stopAllActions();
	#if GEODE_COMP_GD_VERSION < 22081
	if (this->m_musicVolumeInput && this->m_musicVolumeInput->getChildByType<cocos2d::extension::CCScale9Sprite>(0)) this->m_musicVolumeInput->getChildByType<cocos2d::extension::CCScale9Sprite>(0)->stopAllActions();
	#endif
	#if GEODE_COMP_GD_VERSION > 22074
	if (this->m_musicVolumeInput && this->m_musicVolumeInput->getChildByType<geode::NineSlice>(0)) this->m_musicVolumeInput->getChildByType<geode::NineSlice>(0)->stopAllActions();
	#endif
	if (this->m_musicVolumeInput && this->m_musicVolumeInput->getInputNode()->m_textField) this->m_musicVolumeInput->getInputNode()->m_textField->stopAllActions();
	if (this->m_musicVolumeInput && this->m_musicVolumeInput->getInputNode()->m_textLabel) this->m_musicVolumeInput->getInputNode()->m_textLabel->stopAllActions();
	if (this->m_musicVolumeInput && this->m_musicVolumeInput->getInputNode()->m_cursor) this->m_musicVolumeInput->getInputNode()->m_cursor->stopAllActions();
	for (cocos2d::CCSprite* spriteChild : geode::cocos::CCArrayExt<cocos2d::CCSprite*>(this->m_audieoVisual->getChildByIndex(0)->m_pChildren)) {
		if (!spriteChild) continue;
		spriteChild->stopAllActions();
	}
	// easing types and easing values taken from https://github.com/ppy/osu/blob/master/osu.Game/Screens/Menu/ButtonSystem.cs#L476 and https://github.com/ppy/osu/blob/master/osu.Game/Screens/Menu/ButtonSystem.cs#L495 under the MIT license, but they're just silly numbers at the end of the day
	if (this->m_osu) {
		this->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.8f, 255)));
		if (this->m_musVolLabl) this->m_musVolLabl->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.8f, 255)));
		for (cocos2d::CCSprite* spriteChild : geode::cocos::CCArrayExt<cocos2d::CCSprite*>(this->m_audieoVisual->getChildByIndex(0)->m_pChildren)) {
			if (!spriteChild) continue;
			spriteChild->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.8f, spriteChild->getTag() < 100 ? 100 : 255)));
		}
		#if GEODE_COMP_GD_VERSION < 22081
		if (this->m_musicVolumeInput && this->m_musicVolumeInput->getChildByType<cocos2d::extension::CCScale9Sprite>(0)) this->m_musicVolumeInput->getChildByType<cocos2d::extension::CCScale9Sprite>(0)->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.8f, 255)));
		#endif
		#if GEODE_COMP_GD_VERSION > 22074
		if (this->m_musicVolumeInput && this->m_musicVolumeInput->getChildByType<geode::NineSlice>(0)) this->m_musicVolumeInput->getChildByType<geode::NineSlice>(0)->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.8f, 255)));
		#endif
		if (this->m_musicVolumeInput && this->m_musicVolumeInput->getInputNode()->m_textField) this->m_musicVolumeInput->getInputNode()->m_textField->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.8f, 255)));
		if (this->m_musicVolumeInput && this->m_musicVolumeInput->getInputNode()->m_textLabel) this->m_musicVolumeInput->getInputNode()->m_textLabel->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.8f, 255)));
		if (this->m_musicVolumeInput && this->m_musicVolumeInput->getInputNode()->m_cursor) this->m_musicVolumeInput->getInputNode()->m_cursor->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.8f, 255)));
		this->m_mainLayer->runAction(
			cocos2d::CCSequence::create(
				cocos2d::CCCallFunc::create(this, callfunc_selector(SongControlMenu::doTheRippleEffectFromOsuLazer)),
				cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(.8f, 1.5f)),
				cocos2d::CCCallFunc::create(this, callfunc_selector(SongControlMenu::toggleMenuLayerVisibility)),
				nullptr
			)
		);
	} else {
		this->runAction(cocos2d::CCEaseIn::create(cocos2d::CCFadeTo::create(.2f, 105), .5f));
		if (this->m_musVolLabl) this->m_musVolLabl->runAction(cocos2d::CCEaseIn::create(cocos2d::CCFadeTo::create(.2f, 0), .5f));
		for (cocos2d::CCSprite* spriteChild : geode::cocos::CCArrayExt<cocos2d::CCSprite*>(this->m_audieoVisual->getChildByIndex(0)->m_pChildren)) {
			if (!spriteChild) continue;
			spriteChild->runAction(cocos2d::CCEaseIn::create(cocos2d::CCFadeTo::create(.2f, 0), .5f));
		}
		#if GEODE_COMP_GD_VERSION < 22081
		if (this->m_musicVolumeInput && this->m_musicVolumeInput->getChildByType<cocos2d::extension::CCScale9Sprite>(0)) this->m_musicVolumeInput->getChildByType<cocos2d::extension::CCScale9Sprite>(0)->runAction(cocos2d::CCEaseIn::create(cocos2d::CCFadeTo::create(.2f, 0), .5f));
		#endif
		#if GEODE_COMP_GD_VERSION > 22074
		if (this->m_musicVolumeInput && this->m_musicVolumeInput->getChildByType<geode::NineSlice>(0)) this->m_musicVolumeInput->getChildByType<geode::NineSlice>(0)->runAction(cocos2d::CCEaseIn::create(cocos2d::CCFadeTo::create(.2f, 0), .5f));
		#endif
		if (this->m_musicVolumeInput && this->m_musicVolumeInput->getInputNode()->m_textField) this->m_musicVolumeInput->getInputNode()->m_textField->runAction(cocos2d::CCEaseIn::create(cocos2d::CCFadeTo::create(.2f, 0), .5f));
		if (this->m_musicVolumeInput && this->m_musicVolumeInput->getInputNode()->m_textLabel) this->m_musicVolumeInput->getInputNode()->m_textLabel->runAction(cocos2d::CCEaseIn::create(cocos2d::CCFadeTo::create(.2f, 0), .5f));
		if (this->m_musicVolumeInput && this->m_musicVolumeInput->getInputNode()->m_cursor) this->m_musicVolumeInput->getInputNode()->m_cursor->runAction(cocos2d::CCEaseIn::create(cocos2d::CCFadeTo::create(.2f, 0), .5f));
		this->m_mainLayer->runAction(
			cocos2d::CCSequence::create(
				cocos2d::CCCallFunc::create(this, callfunc_selector(SongControlMenu::toggleMenuLayerVisibility)),
				cocos2d::CCEaseIn::create(cocos2d::CCScaleTo::create(.2f, 1.f), 1.f),
				cocos2d::CCCallFunc::create(this, callfunc_selector(SongControlMenu::doTheRippleEffectFromOsuLazer)),
				nullptr
			)
		);
	}
}

#if GEODE_COMP_GD_VERSION == 22081
void SongControlMenu::keyDown(const cocos2d::enumKeyCodes key, double p1) {
#endif
#if GEODE_COMP_GD_VERSION == 22074
void SongControlMenu::keyDown(const cocos2d::enumKeyCodes key) {
#endif
	SongManager& songManager = SongManager::get();
	if (songManager.getOsu() && (key == cocos2d::KEY_O || key == cocos2d::KEY_Enter)) {
		SongControlMenu::toggleOsu();
		return;
	}
	#ifdef GEODE_IS_DESKTOP
	if (songManager.getYoutubeAndVLCKeyboardShortcutsControlPanel()) {
		cocos2d::CCKeyboardDispatcher* cckd = cocos2d::CCKeyboardDispatcher::get();
		const bool isShift = cckd->getShiftKeyPressed();
		const bool isCmd = GEODE_MACOS(cckd->getCommandKeyPressed()) GEODE_WINDOWS(false);
		const bool macOSCtrl = GEODE_MACOS(cckd->getControlKeyPressed()) GEODE_WINDOWS(false);
		const bool windowsCtrl = GEODE_MACOS(false) GEODE_WINDOWS(cckd->getControlKeyPressed());
		const bool isAlt = GEODE_MACOS(cckd->getControlKeyPressed()) GEODE_WINDOWS(cckd->getAltKeyPressed());
		if (key == cocos2d::KEY_Zero || key == cocos2d::KEY_One || key == cocos2d::KEY_Two || key == cocos2d::KEY_Three || key == cocos2d::KEY_Four || key == cocos2d::KEY_Five || key == cocos2d::KEY_Six || key == cocos2d::KEY_Seven || key == cocos2d::KEY_Eight || key == cocos2d::KEY_Nine) {
			return SongControl::setSongPercentage(10 * (static_cast<int>(key) - static_cast<int>(cocos2d::KEY_Zero)));
		}
		if (key == cocos2d::KEY_NumPad0 || key == cocos2d::KEY_NumPad1 || key == cocos2d::KEY_NumPad2 || key == cocos2d::KEY_NumPad3 || key == cocos2d::KEY_NumPad4 || key == cocos2d::KEY_NumPad5 || key == cocos2d::KEY_NumPad6 || key == cocos2d::KEY_NumPad7 || key == cocos2d::KEY_NumPad8 || key == cocos2d::KEY_NumPad9) {
			return SongControl::setSongPercentage(10 * (static_cast<int>(key) - static_cast<int>(cocos2d::KEY_NumPad0)));
		}
		if ((windowsCtrl || isCmd) && key == cocos2d::KEY_R) {
			return SongControl::setSongPercentage(0);
		}
		if (((macOSCtrl || windowsCtrl) && key == cocos2d::KEY_S) || (isShift && key == cocos2d::KEY_N) || ((key == cocos2d::KEY_ArrowRight || key == cocos2d::KEY_Right) && (windowsCtrl || isCmd))) {
			return SongControl::shuffleSong();
		}
		if ((isShift && key == cocos2d::KEY_P) || ((key == cocos2d::KEY_ArrowLeft || key == cocos2d::KEY_Left) && (windowsCtrl || isCmd))) {
			return SongControl::previousSong();
		}
		if (key == cocos2d::KEY_B && isShift && isAlt) {
			return SongControlMenu::onFavoriteButton(nullptr);
		}
		if ((key == cocos2d::KEY_K && (windowsCtrl || isCmd)) && songManager.getFinishedCalculatingSongLengths()) {
			return SongControlMenu::onPlaylistButton(nullptr);
		}
	}
	#endif
	if (CAN_SHOW_PLAYBACK_PROGRESS) {
		if (key == cocos2d::KEY_Right || key == cocos2d::KEY_ArrowRight || key == cocos2d::KEY_L) return SongControl::skipForward();
		if (key == cocos2d::KEY_Left || key == cocos2d::KEY_ArrowLeft || key == cocos2d::KEY_J) return SongControl::skipBackward();
	}
	if (key == cocos2d::enumKeyCodes::KEY_Escape) {
		if (MenuLayer::get() && !MenuLayer::get()->isVisible() && this->m_osu) MenuLayer::get()->setVisible(true);
		return this->onClose(nullptr);
	}
	if (key == cocos2d::enumKeyCodes::KEY_Space) return;
	#if GEODE_COMP_GD_VERSION == 22081
	return FLAlertLayer::keyDown(key, p1);
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	return FLAlertLayer::keyDown(key);
	#endif
}