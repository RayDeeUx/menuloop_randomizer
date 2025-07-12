#pragma once

enum class SongType {
	Regular,
	Favorited,
	Blacklisted
};

struct SongData {
	std::string fileName;
	SongType type;
};

class MLRSongCell : public cocos2d::CCLayerColor {
public:
	static MLRSongCell* create(const SongData& songData, const bool isEven);
	bool init(const SongData& userData, const bool isEven);
	SongData m_SongData;
};