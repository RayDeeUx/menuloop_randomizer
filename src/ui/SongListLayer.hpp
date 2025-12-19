#pragma once

#include "../SongManager.hpp"
#include "MLRSongCell.hpp"

class SongListLayer final : public geode::Popup<const std::string&> {
public:
	static SongListLayer* create(const std::string&);
	void addSongsToScrollLayer(geode::ScrollLayer* scrollLayer, SongManager& songManager, const std::string& queryString = "");
	bool setup(const std::string&) override;
	void onSettingsButton(CCObject*);
	void onShuffleButton(CCObject*);
	void onCopyButton(CCObject*);
	void onPreviousButton(CCObject*);
	void onControlsButton(CCObject*);
	void SongListLayer::keyDown(cocos2d::enumKeyCodes);
	void searchSongs(const std::string& queryString);

	static std::string generateDisplayName(SongData& songData);
};