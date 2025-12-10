#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/FMODAudioEngine.hpp>

using namespace geode::prelude;

class $modify(MenuLoopFMODHook, FMODAudioEngine) {
	void update(float dt) {
		FMODAudioEngine::update(dt);
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		SongManager& songManager = SongManager::get();
		constexpr int channelNumber = 0;
		bool isPlaying = true;
		unsigned int position = 0;
		FMOD::Sound* sound;
		unsigned int length = 0;
		FMOD::Channel* menuLoopChannelProbably = FMODAudioEngine::get()->getActiveMusicChannel(channelNumber);
		const auto activeSong = FMODAudioEngine::get()->getActiveMusic(channelNumber);
		const auto songManagerSong = songManager.getCurrentSong();
		const bool isSongManagerSong = activeSong == songManagerSong;
		if (!isSongManagerSong) {
			if (SongManager::get().getAdvancedLogs()) {
				log::info("activeSong: {}", activeSong);
				log::info("songManagerSong: {}", songManagerSong);
			}
			return;
		}
		menuLoopChannelProbably->isPlaying(&isPlaying);
		menuLoopChannelProbably->getPosition(&position, 1);
		menuLoopChannelProbably->getCurrentSound(&sound);
		songManager.setLastMenuLoopPosition(position); // so the problem with trying to move this logic to GJBGL::init() is that by the time GJBGL::init() is called, FMOD's menu loop track has already been stopped and its position resets to 0. hence why we are here instead.
		if (!songManager.getConstantShuffleMode() || songManager.isOverride()) return;
		if (GJBaseGameLayer::get() || songManager.isOriginalMenuLoop() || songManager.getSongsSize() < 2) return;
		sound->getLength(&length, 1);
		if (SongManager::get().getAdvancedLogs()) {
			log::info("isSongManagerSong: {}", isSongManagerSong);
			log::info("position: {}", position);
			log::info("isPlaying: {}", isPlaying);
			log::info("length - 100: {}", length - 100);
			log::info("(length - 100) < position: {}", (length - 100) < position);
			log::info("===========================");
		}
		if ((length - 100) < position) {
			geode::log::info("song is probably finished. Switching songs.");
			Utils::removeCard();
			Utils::constantShuffleModeNewSong();
			Utils::newCardAndDisplayNameFromCurrentSong();
		}
	}
};
