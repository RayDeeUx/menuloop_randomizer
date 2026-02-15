#pragma once

class SongControlMenu final : public geode::Popup {
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
	cocos2d::extension::CCScale9Sprite*b{};
	cocos2d::CCLayerColor* m_darkProgBar{};
	cocos2d::CCLayerColor* m_currProgBar{};
	cocos2d::CCClippingNode* m_clipNode {};
	float m_time = 0;
	bool setup() override;
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
	void forceSharpCornerIllusion();
	void forceSharpCornerIllusionScheduler(float);
public:
	static SongControlMenu* create();
	void onRegenButton(cocos2d::CCObject*);
	void updateCurrentLabel();
	void onExit() override;
	void keyDown(cocos2d::enumKeyCodes key) override;
	void toggleButtonState(cocos2d::CCNode *playlistButton, const bool isEnabled) const;
};