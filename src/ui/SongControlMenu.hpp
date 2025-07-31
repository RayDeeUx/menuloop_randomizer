#pragma once

class SongControlMenu final : public geode::Popup<const std::string&> {
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
	bool setup(std::string const& id) override;
	void onShuffleButton(CCObject*);
	void onCopyButton(CCObject*);
	void onBlacklistButton(CCObject*);
	void onFavoriteButton(CCObject*);
	void onHoldSongButton(CCObject*);
	void onPreviousButton(CCObject*);
	void onPlaylistButton(CCObject*);
	void onAddToPlylstBtn(CCObject*);
	void onSettingsButton(CCObject*);
public:
	static SongControlMenu* create(const std::string&);
	void onRegenButton(CCObject*);
	void updateCurrentLabel();
	void onExit() override;
};