#pragma once

class SongListLayer final : public geode::Popup<const std::string&> {
public:
	static SongListLayer* create(const std::string&);
	bool setup(const std::string&) override;
	void onSettingsButton(CCObject*);
	void onShuffleButton(CCObject*);
	void onCopyButton(CCObject*);
	void onPreviousButton(CCObject*);
	void onControlsButton(CCObject*);
	void searchSongs(const std::string& queryString) const;
};