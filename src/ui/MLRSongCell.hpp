#pragma once

enum class SongType {
	Regular,
	Favorited,
	Blacklisted
};

struct SongData {
	std::string actualFilePath;
	std::string fileExtension;
	std::string fileName;
	SongType type;
	bool isFromConfigOrAltDir;
};

class MLRSongCell : public cocos2d::CCLayerColor {
public:
	static MLRSongCell* create(const SongData& songData, const bool isEven);
	bool init(const SongData& songData, const bool isEven);
	void onPlaySong(CCObject*);
	void update(float delta);
	SongData m_songData;
	cocos2d::CCLabelBMFont* m_songNameLabel;
	cocos2d::CCLayerColor* m_divider;
	cocos2d::CCMenu* m_menu;
	CCMenuItemSpriteExtra* m_playButton;
};