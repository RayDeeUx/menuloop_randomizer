#pragma once

#include "../SongManager.hpp"
#include "MLRSongCell.hpp"

#define SEARCH_BAR_NODE_ID "song-list-search-bar"_spr

class SongListLayer final : public geode::Popup {
protected:
	CCMenuItemToggler* m_songListCompactMode {};
    CCMenuItemToggler* m_songListFavoritesOnlyMode {};
    CCMenuItemToggler* m_songListReverseSort {};
    CCMenuItemToggler* m_songListSortAlphabetically {};
    CCMenuItemToggler* m_songListSortDateAdded {};
    CCMenuItemToggler* m_songListSortSongLength {};
    CCMenuItemToggler* m_songListSortFileSize {};
    CCMenuItemToggler* m_songListSortFileExtn {};
	cocos2d::CCMenu* m_scrollShortcuts {};
	cocos2d::CCMenu* m_viewFiltersMenu {};
	geode::ScrollLayer* m_scrollLayer {};
	geode::Scrollbar* m_scrollBar {};
private:
	float m_scaleFactor = 1.f;
	float m_tallestPoint = -0.f;
public:
	static SongListLayer* create();
	void addSongsToScrollLayer(geode::ScrollLayer* scrollLayer, SongManager& songManager, const std::string& queryString = "");
	void updateSongCountAndFavoritesCount(SongManager& songManager);
	bool setup() override;
	void onSettingsButton(cocos2d::CCObject*);
	void onShuffleButton(cocos2d::CCObject*);
	void onCopyButton(cocos2d::CCObject*);
	void onPreviousButton(cocos2d::CCObject*);
	void onControlsButton(cocos2d::CCObject*);
	CCContentLayer* getContentLayer() const;
	void onScrollTopButton(cocos2d::CCObject*);
	void onScrollCurButton(cocos2d::CCObject*);
	void onScrollBtmButton(cocos2d::CCObject*);
	void onCompactModeToggle(cocos2d::CCObject*);
	void onFavoritesOnlyToggle(cocos2d::CCObject*);
	void onSortReverseToggle(cocos2d::CCObject*);
	void onSortABCToggle(cocos2d::CCObject*);
	void onSortDateToggle(cocos2d::CCObject*);
	void onSortLengthToggle(cocos2d::CCObject*);
	void onSortSizeToggle(cocos2d::CCObject*);
	void onSortExtnToggle(cocos2d::CCObject*);

	static void handleMutuallyExclusiveSortToggle(CCMenuItemToggler* toggle, const std::string_view savedValueKeyToMatch, const std::string_view savedValueKey, const bool originalSavedValue);
	void disableAllSortFiltersThenToggleThenSearch(const std::string_view);
	void toggleSavedValueAndSearch(const std::string_view);
	void scrollToCurrentSong();

	void checkPosition(const float);
	void keyDown(const cocos2d::enumKeyCodes) override;
	void keyUp(const cocos2d::enumKeyCodes) override;
	void searchSongs(const std::string&);

	static std::string generateDisplayName(SongData&);
	static bool tallEnough(geode::ScrollLayer*);
	static float determineYPosition(geode::ScrollLayer*);
	static void displayCurrentSongByLimitingPlaceholderLabelWidth(CCTextInputNode* inputNode, const bool updateString = true);

	static bool caseInsensitiveAlphabetical(MLRSongCell* a, MLRSongCell* b, bool reverse);
	static bool fileSize(MLRSongCell* a, MLRSongCell* b, bool reverse);
	static bool fileExtn(MLRSongCell* a, MLRSongCell* b, bool reverse);
	static bool dateAdded(MLRSongCell* a, MLRSongCell* b, bool reverse);
	static bool songLength(MLRSongCell* a, MLRSongCell* b, bool reverse);

	static unsigned int useFMODToGetLength(const std::string& path, const unsigned int extreme);
	static unsigned int getLength(const std::string& path, const bool reverse);

	void displayCurrentSongByLimitingPlaceholderLabelWidthScheduler(float);

	geode::TextInput* m_searchBar {};
};