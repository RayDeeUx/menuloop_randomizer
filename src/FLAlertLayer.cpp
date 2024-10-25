#include <Geode/modify/FLAlertLayer.hpp>
#include "Utils.hpp"

#define PREFERRED_HOOK_PRIO (-2123456789)

using namespace geode::prelude;

class $modify(MyFLAlertLayer, FLAlertLayer) {
	static void onModify(auto & self)
	{
		(void) self.setHookPriority("FLAlertLayer::init", PREFERRED_HOOK_PRIO);
	}
	bool init(FLAlertLayerProtocol* delegate, char const* title, gd::string desc, char const* btn1, char const* btn2, float width, bool scroll, float height, float textScale) {
		const std::string& titleAsString = title;
		const std::string& descAsString = desc;
		if (!utils::string::contains(titleAsString, "Playlist Mode") && !utils::string::contains(descAsString, "Playlist Mode") && !utils::string::contains(descAsString, "Original idea by TheKevinner.") && !utils::string::contains(descAsString, "Known issues")) {
			return FLAlertLayer::init(delegate, title, desc, btn1, btn2, width, scroll, height, textScale);
		}
		return FLAlertLayer::init(delegate, title, desc, btn1, btn2, 420.f, true, 320.f, 1.0f);
	}
};