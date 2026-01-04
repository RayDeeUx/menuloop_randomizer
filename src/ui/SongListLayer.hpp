#pragma once

#include "../SongManager.hpp"
#include "MLRSongCell.hpp"

#define SEARCH_BAR_NODE_ID "song-list-search-bar"_spr

class SongListLayer final : public geode::Popup<> {
public:
	static SongListLayer* create();
	void addSongsToScrollLayer(geode::ScrollLayer* scrollLayer, SongManager& songManager, const std::string& queryString = "");
	bool setup() override;
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
	void onFavoritesOnlyToggle(CCObject*);
	void onSortReverseToggle(CCObject*);
	void onSortABCToggle(CCObject*);
	void onSortDateToggle(CCObject*);
	void onSortLengthToggle(CCObject*);
	void onSortSizeToggle(CCObject*);
	void disableAllSortFiltersThenToggleThenSearch(const std::string_view);
	void toggleSavedValueAndSearch(const std::string_view);
	void keyDown(const cocos2d::enumKeyCodes) override;
	void searchSongs(const std::string&);

	static std::string generateDisplayName(SongData&);
	static bool tallEnough(geode::ScrollLayer*);
	static float determineYPosition(geode::ScrollLayer*);
	static void displayCurrentSongByLimitingPlaceholderLabelWidth(CCTextInputNode* inputNode, const bool updateString = true);

	static bool caseInsensitiveAlphabetical(MLRSongCell* a, MLRSongCell* b, bool reverse);
	static bool fileSize(MLRSongCell* a, MLRSongCell* b, bool reverse);
	static bool dateAdded(MLRSongCell* a, MLRSongCell* b, bool reverse);
	static bool songLength(MLRSongCell* a, MLRSongCell* b, bool reverse);

	static unsigned int getLength(const std::string& path, const bool reverse);

	void displayCurrentSongByLimitingPlaceholderLabelWidthScheduler(float);
};