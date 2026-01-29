#pragma once
#include "../SongManager.hpp"

class MLRSongCell final : public cocos2d::CCLayerColor {
public:
	static MLRSongCell* create(const SongData& songData, const bool isEven, const bool isCompact = true);
	static MLRSongCell* createEmpty(const bool);
	bool init(const SongData& songData, const bool isEven, const bool isCompact = true);
	bool initEmpty(const bool);
	void onPlaySong(CCObject*);
	void toggleEven(const bool);
	void updateProgressBar() const;
	void checkIfCurrentSong() const;
	void onSkipBkwdButton(CCObject*);
	void onSkipFwrdButton(CCObject*);
	void checkIfCurrentSongScheduler(float);
	void pressAndHoldScheduler(float);
	SongData m_songData;
	cocos2d::CCLabelBMFont* m_songNameLabel;
	cocos2d::CCLabelBMFont* m_extraInfoLabl;
	cocos2d::CCLayerColor* m_divider;
	cocos2d::CCLayerColor* m_currentB;
	cocos2d::CCLayerColor* m_totalBar;
	cocos2d::CCMenu* m_menu;
	CCMenuItemSpriteExtra* m_playButton;
	CCMenuItemSpriteExtra* m_bkwdButton;
	CCMenuItemSpriteExtra* m_ffwdButton;
	float m_time = 0.f;
};