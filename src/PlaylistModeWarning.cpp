#include "PlaylistModeWarning.hpp"

using namespace geode::prelude;

PlaylistModeWarning* PlaylistModeWarning::create(bool isCringe) {
	auto *ret = new PlaylistModeWarning();
	if (ret && ret->init(isCringe)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool PlaylistModeWarning::init(bool isCringe) {
	if (!FLAlertLayer::init(nullptr, "Playlist Mode Warning",
		fmt::format("{}<cr>This feature is ***very*** unstable. It's best if you restart your game after toggling this setting.</c>\n\n<cy>Known issues (usually for non-Windows users):</c>\n<cy>- Songs may stop changing while/after viewing Geode UI, the Tower, the icon kit, or the shops.</c>\n<cy>- Playlist Mode disables loops from the Tower, all shops, and all vaults. (Not much of a loss; those loops get tiring after a while.)</c>",
			isCringe ? "<cy>Geodify's \"Menu Loop\" option is enabled.</c> <cr></c>\n\n" : "").c_str(),
		"I Understand", nullptr, 420.f, true, 300.f, 1.f)) return false;
	return true;
}