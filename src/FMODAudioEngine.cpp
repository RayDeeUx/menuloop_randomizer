#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/FMODAudioEngine.hpp>
#include <regex>

using namespace geode::prelude;

const static std::regex geometryDashRegex = std::regex(R"(^.*(?:(?:Geometry ?Dash.*|com\.geode\.launcher)|config.(?:[\w])*\.(?:[\w])*).(?:.*)\.(?:mp3|ogg|oga|flac|wav)$)");
const static std::regex terribleLoopRegex = std::regex(R"(^[\w]+\.mp3$)");

class $modify(MenuLoopFMODHook, FMODAudioEngine) {
	void update(float dt) {
		FMODAudioEngine::update(dt);
		if (!SongManager::get().getConstantShuffleMode()) return;
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		if (GJBaseGameLayer::get() || SongManager::get().isOriginalMenuLoop() || SongManager::get().getSongsSize() < 2) return;
		constexpr int channelNumber = 0;
		FMOD::Channel* menuLoopChannelProbably = FMODAudioEngine::get()->getActiveMusicChannel(0);
		const bool isSongManagerSong = FMODAudioEngine::get()->getActiveMusic(channelNumber) == SongManager::get().getCurrentSong();
		log::info("isSongManagerSong: {}", isSongManagerSong);
		// unsigned int position = 0;
		bool isPlaying = true;
		unsigned int position = 0;
		FMOD::Sound* sound;
		unsigned int length = 0;
		menuLoopChannelProbably->isPlaying(&isPlaying);
		menuLoopChannelProbably->getPosition(&position, 1);
		menuLoopChannelProbably->getCurrentSound(&sound);
		sound->getLength(&length, 1);
		log::info("position: {}", position);
		log::info("isPlaying: {}", isPlaying);
		log::info("length - 50: {}", length - 50);
		log::info("(length - 50) < position: {}", (length - 50) < position);
		log::info("===========================");
		if ((length - 50) < position) {
			geode::log::info("song is probably finished. Switching songs.");
			Utils::removeCard();
			Utils::constantShuffleModeNewSong();
			Utils::newCardAndDisplayNameFromCurrentSong();
		}
	}
};
