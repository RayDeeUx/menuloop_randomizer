#include "SongManager.hpp"
#include "Utils.hpp"
#include <Geode/modify/FMODAudioEngine.hpp>

using namespace geode::prelude;

class $modify(MenuLoopFMODHook, FMODAudioEngine) {
	void update(float dt) {
		FMODAudioEngine::update(dt);
		if (!SongManager::get().getConstantShuffleMode()) return;
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		if (GJBaseGameLayer::get() || SongManager::get().isOriginalMenuLoop() || SongManager::get().getSongsSize() < 2) return;
		constexpr int channelNumber = 0;
		bool isPlaying = true;
		unsigned int position = 0;
		FMOD::Sound* sound;
		unsigned int length = 0;
		FMOD::Channel* menuLoopChannelProbably = FMODAudioEngine::get()->getActiveMusicChannel(channelNumber);
		const auto activeSong = FMODAudioEngine::get()->getActiveMusic(channelNumber);
		const auto songManagerSong = SongManager::get().getCurrentSong();
		const bool isSongManagerSong = activeSong == songManagerSong;
		if (!isSongManagerSong) {
			if (Utils::getBool("advancedLogs")) {
				log::info("activeSong: {}", activeSong);
				log::info("songManagerSong: {}", songManagerSong);
			}
			return;
		}
		menuLoopChannelProbably->isPlaying(&isPlaying);
		menuLoopChannelProbably->getPosition(&position, 1);
		menuLoopChannelProbably->getCurrentSound(&sound);
		sound->getLength(&length, 1);
		if (Utils::getBool("advancedLogs")) {
			log::info("isSongManagerSong: {}", isSongManagerSong);
			log::info("position: {}", position);
			log::info("isPlaying: {}", isPlaying);
			log::info("length - 50: {}", length - 50);
			log::info("(length - 50) < position: {}", (length - 50) < position);
			log::info("===========================");
		}
		if ((length - 50) < position) {
			geode::log::info("song is probably finished. Switching songs.");
			Utils::removeCard();
			Utils::constantShuffleModeNewSong();
			Utils::newCardAndDisplayNameFromCurrentSong();
		}
	}
};
