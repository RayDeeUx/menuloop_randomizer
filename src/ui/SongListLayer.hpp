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
	CCContentLayer* getContentLayer() const;
	void onScrollTopButton(CCObject*);
	void onScrollCurButton(CCObject*);
	void onScrollBtmButton(CCObject*);
	void onCompactModeToggle(CCObject*);
	void keyDown(const cocos2d::enumKeyCodes) override;
	void searchSongs(const std::string& queryString);

	static std::string generateDisplayName(SongData& songData);
	static bool tallEnough(geode::ScrollLayer *scrollLayer);
	static float determineYPosition(geode::ScrollLayer* scrollLayer);
};