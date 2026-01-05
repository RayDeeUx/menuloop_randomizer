#pragma once

class SongControlMenu final : public geode::Popup<> {
protected:
	cocos2d::CCLabelBMFont* m_smallLabel{};
	cocos2d::CCLabelBMFont* m_otherLabel{};
	cocos2d::CCLabelBMFont* m_headerLabl{};
	cocos2d::CCMenu* m_songControlsMenu {};
	cocos2d::CCMenu* m_theTimeoutCorner {};
	cocos2d::CCMenu* m_openSongListMenu {};
	cocos2d::CCMenu* m_infoMenu         {};
	InfoAlertButton* m_infoButton       {};
	cocos2d::extension::CCScale9Sprite*b{};
	bool setup() override;
	void onShuffleButton(CCObject*);
	void onCopyButton(CCObject*);
	void onBlacklistButton(CCObject*);
	void onFavoriteButton(CCObject*);
	void onHoldSongButton(CCObject*);
	void onPreviousButton(CCObject*);
	void onPlaylistButton(CCObject*);
	void onAddToPlylstBtn(CCObject*);
	void onSettingsButton(CCObject*);
	void checkManagerFinished(float);
public:
	static SongControlMenu* create();
	void onRegenButton(CCObject*);
	void updateCurrentLabel();
	void onExit() override;
	void toggleButtonState(cocos2d::CCNode *playlistButton, const bool isEnabled) const;
};