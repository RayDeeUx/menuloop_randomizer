#ifdef GEODE_IS_WINDOWS
#include "SongManager.hpp"
#endif
#include "Utils.hpp"
#include <Geode/modify/FMODAudioEngine.hpp>
#include <regex>

using namespace geode::prelude;

const static std::regex geometryDashRegex = std::regex(R"(^.*(?:(?:Geometry ?Dash.*|com\.geode\.launcher)|config.(?:[\w])*\.(?:[\w])*).(?:.*)\.(?:mp3|ogg|oga|flac|wav)$)");
const static std::regex terribleLoopRegex = std::regex(R"(^[\w]+\.mp3$)");

class $modify(MenuLoopFMODHook, FMODAudioEngine) {
	void playMusic(gd::string path, bool shouldLoop, float fadeInTime, int channel) {
		if (!Utils::getBool("playlistMode"))
			return FMODAudioEngine::get()->playMusic(path, shouldLoop, fadeInTime, channel);
		log::info("playlist mode enabled.\n=== PLAYLIST MODE DEBUG INFO ===\npath: {}\nshouldLoop: {}\nfadeInTime: {}\nchannel: {}", path, shouldLoop, fadeInTime, channel);
		if (auto currentScene = CCDirector::get()->getRunningScene()) {
			for (auto object : CCArrayExt<CCObject*>(currentScene->getChildren())) {
				if (auto node = typeinfo_cast<CCNode*>(object)) {
					log::info("there is a CCNode with ID: \"{}\"", node->getID());
				}
			}
		}
		bool desiredShouldLoop = shouldLoop;
		std::string gdStringSucks = path;
		std::smatch smatch;
		if (std::regex_match(gdStringSucks, smatch, terribleLoopRegex))
			return log::info("terrible loop detected while playlist mode is active: {}", gdStringSucks);
		bool isMenuLoop = std::regex_match(gdStringSucks, smatch, geometryDashRegex);
		if (GJBaseGameLayer::get() && !isMenuLoop)
			return FMODAudioEngine::get()->playMusic(path, desiredShouldLoop, fadeInTime, channel);
		if (fadeInTime == 0 && gdStringSucks == "shop.mp3") return;
		if (shouldLoop && fadeInTime == 1.0f) {
			if (!isMenuLoop && !utils::string::contains(gdStringSucks, "/Library/Caches/")) {
				log::info("non-menu loop found while playlist mode is enabled: {}", gdStringSucks);
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
			if (channel == 0) {
				log::info("attempted to loop menu music on channel zero! see if on windows or not. aborting early.");
				return;
			}
			#ifdef GEODE_IS_WINDOWS
			if (!SongManager::get().getCalledOnce()) SongManager::get().setCalledOnce(true);
			#endif
			return FMODAudioEngine::get()->playMusic(path, desiredShouldLoop, 0.0f, channel);
		}
		FMODAudioEngine::get()->playMusic(path, desiredShouldLoop, fadeInTime, channel);
	}
};
