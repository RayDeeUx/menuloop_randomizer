#pragma once

class SongListLayer : GJDropDownLayer {
public:
	static SongListLayer* create();
	virtual void customSetup();
	virtual void showLayer(const bool instant);
	void onSettingsButton(CCObject*);
};