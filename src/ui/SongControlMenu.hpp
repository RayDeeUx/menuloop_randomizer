#pragma once

class SongControlMenu final : public geode::Popup<const std::string&> {
protected:
	cocos2d::CCLabelBMFont* m_smallLabel{};
	cocos2d::CCLabelBMFont* m_otherLabel{};
	cocos2d::CCMenu* m_songControlsMenu {};
	bool setup(std::string const& id) override;
	void onShuffleButton(CCObject*);
	void onRegenButton(CCObject*);
	void onCopyButton(CCObject*);
	void onBlacklistButton(CCObject*);
	void onFavoriteButton(CCObject*);
	void onHoldSongButton(CCObject*);
	void onPreviousButton(CCObject*);
	void onSettingsButton(CCObject*);
	void updateCurrentLabel();
public:
	static SongControlMenu* create(const std::string& id);
	void onExit() override;
};