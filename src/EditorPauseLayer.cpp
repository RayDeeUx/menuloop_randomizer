#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/TextArea.hpp>
#ifndef GEODE_IS_ANDROID
#include <Geode/utils/Cocos.hpp>
#endif

using namespace geode::prelude;

class $modify(MenuLoopEPLHook, EditorPauseLayer) {
	struct Fields {
		SongManager &songManager = SongManager::get();
	};
	#ifndef __APPLE__
	void onExitEditor(CCObject *sender) {
		if (Utils::getBool("randomizeWhenExitingEditor"))
			m_fields->songManager.pickRandomSong();

		EditorPauseLayer::onExitEditor(sender);
	}
	#else
	/*
	this section is for macOS (both intel and ARM). remarks:
	- don't hook onSaveAndPlay; that goes to playlayer
	- don't hook onSave or the FLAlertLayer from it; that does not exit the editor
	- can't hook onExitEditor for macOS, due to aggressive inlining from robtop/appleclang
		(yes, nin. i know the address exists; justin found those addresses for me.
		but i keep getting the same song five times in a row if i hook onExitEditor
		for macos, and i don't think any of us have the same RNG seed as Dream.)
		[for the record, i had 29 possible audio files total while doing this.
		1/29 = 0.0344827586207. (1/29)^5 = 4.87539727785E-8 = 1/20511149.
		let that sink in for a moment.]
	-- raydeeux
	*/
	void onSaveAndExit(CCObject *sender) {
		if (Utils::getBool("randomizeWhenExitingEditor"))
			m_fields->songManager.pickRandomSong();

		EditorPauseLayer::onSaveAndExit(sender);
	}
	void FLAlert_Clicked(FLAlertLayer* p0, bool btnTwo) {
		bool isQualifedAlert = false;
		// determine if the FLAlertLayer being clicked on is the one from onExitNoSave
		/*
		hooking FLAlertLayer::init() and then storing its desc param is also an option,
		but i'm not sure what this mod's stance is with global variables.
		consider this overkill, but it gets the job done.

		i wanted to use getChildOfType to reduce the line count but ran into one of those
		C-Tidy/Clang red squiggly lines about typeinfo_cast or whatever and got worried,
		so all you get is this.

		yep, nested forloops and typeinfo_cast calls for days.

		if anyone has a shorter solution that still hooks this function, go ahead.

		for reference, unformatted FLAlertLayer main text is:
		R"(Exit without saving? All unsaved changes will be lost!)"
		-- raydeeux
		*/
		auto tArea = p0->m_mainLayer->getChildByIDRecursive("content-text-area");
		if (auto textArea = typeinfo_cast<TextArea*>(tArea)) {
			for (auto node : CCArrayExt<CCNode*>(textArea->getChildren())) {
				if (typeinfo_cast<MultilineBitmapFont*>(node)) {
					for (auto nodeTwo : CCArrayExt<CCNode*>(node->getChildren())) {
						if (auto label = typeinfo_cast<CCLabelBMFont*>(nodeTwo)) {
							auto labelString = std::string(label->getString());
							isQualifedAlert = labelString == R"(Exit without saving? All unsaved changes will be lost!)";
							log::debug("labelString: {}", labelString); // log::debug calls since that's kinda this mod's thing
							break;
						}
					}
				}
			}
		}

		log::debug("isQualifedAlert: {}", isQualifedAlert); // log::debug calls since that's kinda this mod's thing
		log::debug("btnTwo: {}", btnTwo); // log::debug calls since that's kinda this mod's thing

		if (Utils::getBool("randomizeWhenExitingEditor") && isQualifedAlert && btnTwo)
			m_fields->songManager.pickRandomSong();

		EditorPauseLayer::FLAlert_Clicked(p0, btnTwo);
	}
	#endif
};