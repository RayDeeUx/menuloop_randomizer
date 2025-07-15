#include "ConstantShuffleModeWarning.hpp"

using namespace geode::prelude;

ConstantShuffleModeWarning* ConstantShuffleModeWarning::create(bool isCringe) {
	auto *ret = new ConstantShuffleModeWarning();
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
			"{}<cr>This feature is ***very*** unstable. It's best if you restart your game after toggling this setting.</c>\n\n"
			"<cy>Known issues (usually for non-Windows users):</c>\n"
			"<cy>- Songs may stop changing while/after viewing Geode UI, the Tower, the icon kit, or the shops.</c>\n"
			"<cy>- Constant Shuffle Mode disables loops from the Tower, all shops, and all vaults. "
			"(Not much of a loss; those loops get tiring after a while.)</c>\n"
			"<cy>- Constant Shuffle Mode may break when there is only one song for MLR to choose from.",
			isCringe ? "<cy>Geodify's \"Menu Loop\" option is enabled.</c> <cr>Things may break.</c>\n\n" : ""),
		"I Understand", nullptr, 420.f, true, 300.f, 1.f)) return false;
	return true;
}