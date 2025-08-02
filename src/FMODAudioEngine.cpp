#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/FMODAudioEngine.hpp>
#include <regex>

using namespace geode::prelude;

const static std::regex geometryDashRegex = std::regex(R"(^.*(?:(?:Geometry ?Dash.*|com\.geode\.launcher)|config.(?:[\w])*\.(?:[\w])*).(?:.*)\.(?:mp3|ogg|oga|flac|wav)$)");
const static std::regex terribleLoopRegex = std::regex(R"(^[\w]+\.mp3$)");

class $modify(MenuLoopFMODHook, FMODAudioEngine) {
	void playMusic(gd::string path, bool shouldLoop, float fadeInTime, int channel) {
		SongManager& songManager = SongManager::get();
		const bool isLavaChicken = geode::utils::string::contains(static_cast<std::string>(path), "sawblade.steves_lava_chicken");
		songManager.setLavaChicken(isLavaChicken);
		if (isLavaChicken) {
			songManager.setCurrentSong(static_cast<std::string>(path));
			Utils::newCardAndDisplayNameFromCurrentSong();
		}
		if (!Utils::getBool("playlistMode") || isLavaChicken)
			return FMODAudioEngine::get()->playMusic(path, shouldLoop, fadeInTime, channel);
		log::info("Constant Shuffle Mode enabled.\n=== Constant Shuffle Mode DEBUG INFO ===\npath: {}\nshouldLoop: {}\nfadeInTime: {}\nchannel: {}", path, shouldLoop, fadeInTime, channel);
		if (CCScene* scene = CCScene::get(); Utils::getBool("advancedLogs") && scene && scene->getChildren()) {
			for (CCObject* object : CCArrayExt<CCObject*>(scene->getChildren())) {
				const auto node = typeinfo_cast<CCNode*>(object);
				if (node) log::info("there is a CCNode with ID: \"{}\"", node->getID());
			}
		}
		bool desiredShouldLoop = shouldLoop;
		std::string gdStringSucks = path;
		std::smatch smatch;
		if (std::regex_match(gdStringSucks, smatch, terribleLoopRegex)) return log::info("terrible loop detected while Constant Shuffle Mode is active: {}", gdStringSucks);
		const bool isMenuLoop = std::regex_match(gdStringSucks, smatch, geometryDashRegex);
		if (GJBaseGameLayer::get() && !isMenuLoop) return FMODAudioEngine::get()->playMusic(path, desiredShouldLoop, fadeInTime, channel);
		if (fadeInTime == 0 && gdStringSucks == "shop.mp3") return;
		if (shouldLoop && fadeInTime == 1.0f) {
			if (!isMenuLoop && !utils::string::contains(gdStringSucks, "/Library/Caches/")) {
				log::info("non-menu loop found while constant shuffle mode is enabled: {}", gdStringSucks);
				return FMODAudioEngine::get()->playMusic(path, desiredShouldLoop, fadeInTime, channel);
			}
			/*
			if and ONLY if the song being is determined to be:
			- not the original menuLoop.mp3
			- is from Newgrounds, the Music Library, or config directory
			then set loop to false for songmanager function to work properly
			*/
			log::info("menu loop detected.");
			desiredShouldLoop = false;
			// T0D0: maybe ifdef these few lines. it works on macos intel
			if (channel == 0) return log::info("attempted to loop menu music on channel zero! see if on windows or not. aborting early.");
			#ifdef GEODE_IS_WINDOWS
			if (!songManager.getCalledOnce()) songManager.setCalledOnce(true);
			#endif
			return FMODAudioEngine::get()->playMusic(path, desiredShouldLoop, 0.0f, channel);
		}
		FMODAudioEngine::get()->playMusic(path, desiredShouldLoop, fadeInTime, channel);
	}
};
