#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/FMODAudioEngine.hpp>

using namespace geode::prelude;

// i dont trust the geode userbase enough but i also dont want to spam their logs so
#define SECS_BETWEEN_LOGS 15
float stupidAccmulatorTooLazyToPutElsewhere = 0;

class $modify(MenuLoopFMODHook, FMODAudioEngine) {
	void update(float dt) {
		FMODAudioEngine::update(dt);
		if (GJBaseGameLayer::get() || VANILLA_GD_MENU_LOOP_DISABLED) {
			if (stupidAccmulatorTooLazyToPutElsewhere != 0) stupidAccmulatorTooLazyToPutElsewhere = 0;
			return;
		}

		if (SongManager::get().getAdvancedLogs()) stupidAccmulatorTooLazyToPutElsewhere += dt;
		else if (stupidAccmulatorTooLazyToPutElsewhere != 0) stupidAccmulatorTooLazyToPutElsewhere = 0;

		SongManager& songManager = SongManager::get();
		constexpr int channelNumber = 0;

		FMOD::Channel* menuLoopChannelProbably = FMODAudioEngine::get()->getActiveMusicChannel(channelNumber);
		const auto activeSong = FMODAudioEngine::get()->getActiveMusic(channelNumber);
		const auto songManagerSong = songManager.getCurrentSong();
		const bool isSongManagerSong = activeSong == songManagerSong;
		if (!isSongManagerSong) {
			if (!SongManager::get().getAdvancedLogs()) return;
			if (stupidAccmulatorTooLazyToPutElsewhere < SECS_BETWEEN_LOGS) return;
			log::info("activeSong: {}", activeSong);
			log::info("songManagerSong: {}", songManagerSong);
			stupidAccmulatorTooLazyToPutElsewhere = 0;
			return;
		}

		unsigned int position = 0;
		menuLoopChannelProbably->getPosition(&position, 1);
		songManager.setLastMenuLoopPosition(position); // so the problem with trying to move this logic to GJBGL::init() is that by the time GJBGL::init() is called, FMOD's menu loop track has already been stopped and its position resets to 0. hence why we are here instead.

		if (!songManager.getConstantShuffleMode() || songManager.isOverride()) return;
		if (GJBaseGameLayer::get() || songManager.isOriginalMenuLoop() || songManager.getSongsSize() < 2) return;

		bool isPlaying = true;
		FMOD::Sound* sound;
		unsigned int length = 0;
		menuLoopChannelProbably->getCurrentSound(&sound);
		menuLoopChannelProbably->isPlaying(&isPlaying);
		sound->getLength(&length, 1);

		if (SongManager::get().getAdvancedLogs() && stupidAccmulatorTooLazyToPutElsewhere > SECS_BETWEEN_LOGS) {
			log::info("============= [MLR CONSTANT SHUFFLE TRACKER LOGS] =============");
			log::info("isSongManagerSong: {}", isSongManagerSong);
			log::info("position: {}", position);
			log::info("isPlaying: {}", isPlaying);
			log::info("length - 100: {}", length - 100);
			log::info("(length - 100) < position: {}", (length - 100) < position);
			log::info("============= [MLR CONSTANT SHUFFLE TRACKER LOGS] =============");
			stupidAccmulatorTooLazyToPutElsewhere = 0;
		}

		if ((length - 100) < position) {
			geode::log::info("song is probably finished. Switching songs.");
			Utils::removeCard();
			Utils::constantShuffleModeNewSong();
			Utils::newCardAndDisplayNameFromCurrentSong();
		}
	}
};
