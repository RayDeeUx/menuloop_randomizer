#include "ConstantShuffleModeWarning.hpp"

using namespace geode::prelude;

ConstantShuffleModeWarning* ConstantShuffleModeWarning::create(bool isCringe) {
	ConstantShuffleModeWarning* ret = new ConstantShuffleModeWarning();
	if (ret && ret->init(isCringe)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool ConstantShuffleModeWarning::init(bool isCringe) {
	if (!FLAlertLayer::init(nullptr, "Constant Shuffle Mode Warning",
		fmt::format(
			"{}<cr>Constant Shuffle Mode is ***__semi-unstable__***. It's best if you restart your game after toggling this setting.</c>\n\n"
			"<cy>Known issues:</c>\n"
			"<cy>- Constant Shuffle Mode's current code has not been extensively tested on iOS."
			"<cy>- Constant Shuffle Mode may break when there is only one song for MLR to choose from.",
			isCringe ? "<cy>Geodify's \"Menu Loop\" option is enabled.</c> <cr>Things may break.</c>\n\n" : ""),
		"I Understand", nullptr, 420.f, true, 300.f, 1.f)) return false;
	return true;
}