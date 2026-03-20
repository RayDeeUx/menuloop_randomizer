#pragma once

#if GEODE_COMP_GD_VERSION == 22081
class SongControlMenu final : public geode::Popup {
#endif
#if GEODE_COMP_GD_VERSION == 22074
class SongControlMenu final : public geode::Popup<> {
#endif
protected:
	cocos2d::CCLabelBMFont* m_smallLabel{};
	cocos2d::CCLabelBMFont* m_otherLabel{};
	cocos2d::CCLabelBMFont* m_headerLabl{};
	cocos2d::CCLabelBMFont* m_currTimeLb{};
	cocos2d::CCLabelBMFont* m_totlTimeLb{};
	cocos2d::CCMenu* m_songControlsMenu {};
	cocos2d::CCMenu* m_theTimeoutCorner {};
	cocos2d::CCMenu* m_openSongListMenu {};
	cocos2d::CCMenu* m_infoMenu         {};
	cocos2d::CCMenu* m_increDecreMenu   {};
	InfoAlertButton* m_infoButton       {};
	CCMenuItemSpriteExtra* m_ffwdButton {};
	CCMenuItemSpriteExtra* m_bkwdButton {};
	#if GEODE_COMP_GD_VERSION == 22081
	geode::NineSlice* b                 {};
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	cocos2d::extension::CCScale9Sprite*b{};
	#endif
	#if GEODE_COMP_GD_VERSION == 22081
	geode::NineSlice* w                 {};
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	cocos2d::extension::CCScale9Sprite*w{};
	#endif
	cocos2d::CCLayerColor* m_darkProgBar{};
	cocos2d::CCLayerColor* m_currProgBar{};
	cocos2d::CCClippingNode* m_clipNode {};
	FMODLevelVisualizer* m_audieoVisual {};
	geode::TextInput* m_musicVolumeInput{};
	cocos2d::CCLabelBMFont* m_musVolLabl{};
	cocos2d::CCNodeRGBA* m_musicVolCntnr{};
	int m_originalVolume = 0;
	float m_time = 0.f;
	#if GEODE_COMP_GD_VERSION == 22081
	bool init() override;
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	bool setup() override;
	#endif
	void onShuffleButton(cocos2d::CCObject*);
	void onCopyButton(cocos2d::CCObject*);
	void onBlacklistButton(cocos2d::CCObject*);
	void onFavoriteButton(cocos2d::CCObject*);
	void onHoldSongButton(cocos2d::CCObject*);
	void onPreviousButton(cocos2d::CCObject*);
	void onPlaylistButton(cocos2d::CCObject*);
	void onAddToPlylstBtn(cocos2d::CCObject*);
	void onSettingsButton(cocos2d::CCObject*);
	void onSkipBkwdButton(cocos2d::CCObject*);
	void onSkipFwrdButton(cocos2d::CCObject*);
	void checkManagerFinished(float);
	void checkDaSongPositions(float);
	void pressAndHoldScheduler(float);
	void visualizerScheduler(float);
	void forceSharpCornerIllusion();
	void forceSharpCornerIllusionScheduler(float);
	void toggleMenuLayerVisibility();
	void doTheRippleEffectFromOsuLazer();
public:
	bool m_osu = false;
	static SongControlMenu* create();
	void onRegenButton(cocos2d::CCObject*);
	void updateCurrentLabel();
	void onExit() override;
	void toggleOsu();
	void onClose(cocos2d::CCObject*) override;
	#if GEODE_COMP_GD_VERSION == 22081
	void keyDown(cocos2d::enumKeyCodes key, double p1) override;
	#endif
	#if GEODE_COMP_GD_VERSION == 22074
	void keyDown(cocos2d::enumKeyCodes key) override;
	#endif
	void toggleButtonState(cocos2d::CCNode *playlistButton, const bool isEnabled) const;
};
