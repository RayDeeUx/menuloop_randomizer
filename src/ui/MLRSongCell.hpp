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
	std::string displayName;
	SongType type;
	bool isFromConfigOrAltDir;
	bool isEmpty = false;
};

class MLRSongCell : public cocos2d::CCLayerColor {
public:
	static MLRSongCell* create(const SongData& songData, const bool isEven);
	static MLRSongCell* createEmpty(const bool isEven);
	bool init(const SongData& songData, const bool isEven);
	bool initEmpty(const bool isEven);
	void onPlaySong(CCObject*);
	void checkIfCurrentSong() const;
	void update(float delta);
	SongData m_songData;
	cocos2d::CCLabelBMFont* m_songNameLabel;
	cocos2d::CCLayerColor* m_divider;
	cocos2d::CCMenu* m_menu;
	CCMenuItemSpriteExtra* m_playButton;
};