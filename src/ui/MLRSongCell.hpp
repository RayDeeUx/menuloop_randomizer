#pragma once
#include "../SongManager.hpp"
#include "../Utils.hpp"

class MLRSongCell : public cocos2d::CCLayerColor {
public:
	static MLRSongCell* create(const SongData& songData, const bool isEven, const bool isCompact = true);
	static MLRSongCell* createEmpty(const bool);
	bool init(const SongData& songData, const bool isEven, const bool isCompact = true);
	bool initEmpty(const bool);
	void onPlaySong(CCObject*);
	void checkIfCurrentSong() const;
	void toggleEven(const bool);
	void update(float) override;
	SongData m_songData;
	cocos2d::CCLabelBMFont* m_songNameLabel;
	cocos2d::CCLabelBMFont* m_extraInfoLabl;
	cocos2d::CCLayerColor* m_divider;
	cocos2d::CCMenu* m_menu;
	CCMenuItemSpriteExtra* m_playButton;
};