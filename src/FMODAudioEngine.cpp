#include "Utils.hpp"
#include <Geode/modify/FMODAudioEngine.hpp>
#include <regex>

using namespace geode::prelude;

const static std::regex geometryDashRegex = std::regex(R"(^.*(?:Geometry ?Dash.*|config.(?:[\w])*\.(?:[\w])*).(?:.*)\.(?:mp3|ogg|oga|flac|wav)$)");

class $modify(MenuLoopFMODHook, FMODAudioEngine) {
	void playMusic(gd::string path, bool shouldLoop, float fadeInTime, int channel) {
		bool desiredShouldLoop = shouldLoop;
		std::string gdStringSucks = path;
		std::smatch smatch;
		bool isMenuLoop = std::regex_match(gdStringSucks, smatch, geometryDashRegex);
		if (GJBaseGameLayer::get() && !isMenuLoop) {
			return FMODAudioEngine::get()->playMusic(path, desiredShouldLoop, fadeInTime, channel);
		}
		if (Utils::getBool("playlistMode") && shouldLoop && fadeInTime == 1.0f) {
			log::info("playlist mode enabled.");
			log::info("\npath: {}\nshouldLoop: {}\nfadeInTime: {}\nchannel: {}", path, shouldLoop, fadeInTime, channel);
			if (!isMenuLoop) {
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
			return FMODAudioEngine::get()->playMusic(path, desiredShouldLoop, 0.0f, channel);
		}
		FMODAudioEngine::get()->playMusic(path, desiredShouldLoop, fadeInTime, channel);
	}
};
