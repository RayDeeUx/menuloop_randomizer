#include "SongControl.hpp"
#include "Utils.hpp"
#include <Geode/ui/GeodeUI.hpp>

namespace SongControl {
	void woahThereBuddy(const std::string& reason) {
		const bool isCustomSongsReason = geode::utils::string::contains(reason, "Custom Songs Folder by Sawblade");
		const std::string& btnTwo = !isCustomSongsReason ? "Open Mod Settings" : "Manage Custom Songs Folder Mod";
		geode::createQuickPopup(
			"Menu Loop Randomizer", reason,
			"Never Mind", btnTwo.c_str(),
			[isCustomSongsReason](FLAlertLayer*, bool openConfig) {
				if (!openConfig) return;
				if (!isCustomSongsReason) openSettingsPopup(geode::Mod::get());
				openInfoPopup(geode::Loader::get()->getLoadedMod("sawblade.custom_song_folder"));
			}
		);
	}
	void previousSong(SongManager& songManager) {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		if (songManager.isOverride()) return SongControl::woahThereBuddy("You're currently playing a menu loop <cy>override</c>. Double-check your settings again.");
		if (songManager.songSizeIsBad()) return SongControl::woahThereBuddy("You don't have enough songs available to do this. Visit the config directory through the mod settings and try again.");
		if (songManager.isPreviousSong()) {
			if (Utils::getBool("enableNotification")) return Utils::newNotification("You're already playing the previous song! :)");
			return FLAlertLayer::create("Menu Loop Randomizer", "You're already playing the previous song! <cl>:)</c>", "Close")->show();
		}
		const std::string& previousSong = songManager.getPreviousSong();
		if (previousSong.empty()) {
			if (Utils::getBool("enableNotification")) return Utils::newNotification("There's no previous song to go back to! :(");
			return FLAlertLayer::create("Menu Loop Randomizer", "There's no previous song to go back to! <cl>:(</c>", "Close")->show();
		}
		FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
		songManager.setCurrentSong(previousSong);
		if (Utils::getBool("playlistMode")) FMODAudioEngine::get()->playMusic(songManager.getCurrentSong(), true, 1.0f, 1);
		else GameManager::sharedState()->playMenuMusic();
		Utils::newCardAndDisplayNameFromCurrentSong();
	}
	void holdSong(SongManager& songManager) {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		if (songManager.isOverride()) return SongControl::woahThereBuddy("You're currently playing a menu loop <cy>override</c>. Double-check your settings again.");
		if (songManager.songSizeIsBad()) return SongControl::woahThereBuddy("You don't have enough songs available to do this. Visit the config directory through the mod settings and try again.");
		const std::string& formerHeldSong = songManager.getHeldSong();
		const std::string& currentSong = songManager.getCurrentSong();
		if (currentSong == formerHeldSong) {
			if (Utils::getBool("enableNotification")) return Utils::newNotification("You're already holding that song! :D");
			return FLAlertLayer::create("Menu Loop Randomizer", "You're already holding that song! <cl>:D</c>", "Close")->show();
		}
		songManager.setHeldSong(currentSong);
		if (!formerHeldSong.empty()) {
			FMODAudioEngine::get()->m_backgroundMusicChannel->stop();
			songManager.setCurrentSong(formerHeldSong);
			if (Utils::getBool("playlistMode")) FMODAudioEngine::get()->playMusic(songManager.getCurrentSong(), true, 1.0f, 1);
			else GameManager::sharedState()->playMenuMusic();
			return Utils::newCardAndDisplayNameFromCurrentSong();
		}
		if (!Utils::getBool("playlistMode")) Utils::setNewSong();
		else Utils::constantShuffleModeNewSong();
		Utils::newCardAndDisplayNameFromCurrentSong();
	}
	void favoriteSong(SongManager& songManager) {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;

		if (songManager.isOriginalMenuLoop()) return SongControl::woahThereBuddy("There's nothing to favorite! Double-check your config folder again.");
		if (songManager.isOverride()) return SongControl::woahThereBuddy("You're trying to favorite your own <cy>override</c>. Double-check your settings again.");

		const std::string& currentSong = songManager.getCurrentSong();

		if (const auto songManagerFavorites = songManager.getFavorites(); std::ranges::find(songManagerFavorites, currentSong) != songManagerFavorites.end()) return Utils::newNotification("You've already favorited this song! :D");
		if (const auto songManagerBlacklist = songManager.getBlacklist(); std::ranges::find(songManagerBlacklist, currentSong) != songManagerBlacklist.end()) return SongControl::woahThereBuddy("You've already blacklisted this song. Double-check your <cl>blacklist.txt</c> again.");

		geode::log::info("favoriting: {}", currentSong);

		const bool useCustomSongs = Utils::getBool("useCustomSongs");
		const int songID = Utils::getSongID();
		if (songID > 0 && songManager.getSawbladeCustomSongsFolder()) return SongControl::woahThereBuddy("Custom Songs Folder by Sawblade is currently loaded! Disable it to favorite this song.");

		const std::string& songName = Utils::getSongName();
		const std::string& songArtist = Utils::getSongArtist();
		const std::string& customSong = Utils::currentCustomSong();
		const std::string& toWriteToFile = useCustomSongs ? currentSong : fmt::format("{} # [MLR] Song: {} by {} [MLR] #", currentSong, songName, songArtist);

		Utils::writeToFile(toWriteToFile, FAVORITES_FILE);

		songManager.addToFavorites();
		songManager.addSong(currentSong);

		if (!Utils::getBool("enableNotification")) return;
		if (!useCustomSongs) return Utils::newNotification(fmt::format("Favorited {} by {} ({})!", songName, songArtist, songID));
		if (!customSong.empty()) return Utils::newNotification(fmt::format("Favorited {}!", customSong));
	}
	void blacklistSong(SongManager& songManager) {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;

		if (songManager.isOriginalMenuLoop()) return SongControl::woahThereBuddy("There's nothing to blacklist! Open Menu Loop Randomizer's config directory and edit its <cj>blacklist.txt</c> file to bring back some songs.");
		if (songManager.isOverride()) return SongControl::woahThereBuddy("You're trying to blacklist your own <cy>override</c>. Double-check your settings again.");

		const std::string& songBeingBlacklisted = songManager.getCurrentSong();

		if (const auto songManagerBlacklist = songManager.getBlacklist(); std::ranges::find(songManagerBlacklist, songBeingBlacklisted) != songManagerBlacklist.end()) return SongControl::woahThereBuddy("You've already blacklisted this song. Double-check your <cl>blacklist.txt</c> again.");
		if (const auto songManagerFavorites = songManager.getFavorites(); std::ranges::find(songManagerFavorites, songBeingBlacklisted) != songManagerFavorites.end()) return SongControl::woahThereBuddy("You've already favorited this song! Double-check your <cl>favorites.txt</c> again.");

		geode::log::info("blacklisting: {}", songBeingBlacklisted);

		const bool useCustomSongs = Utils::getBool("useCustomSongs");
		const int songID = Utils::getSongID();
		if (songID > 0 && songManager.getSawbladeCustomSongsFolder()) return SongControl::woahThereBuddy("Custom Songs Folder by Sawblade is currently loaded! Disable it to blacklist this song.");

		const std::string& songName = Utils::getSongName();
		const std::string& songArtist = Utils::getSongArtist();
		const std::string& customSong = Utils::currentCustomSong();
		const std::string& toWriteToFile = useCustomSongs ? songBeingBlacklisted : fmt::format("{} # [MLR] Song: {} by {} [MLR] #", songBeingBlacklisted, songName, songArtist);

		Utils::writeToFile(toWriteToFile, BLACKLIST_FILE);

		songManager.addToBlacklist();
		if (!Utils::getBool("dangerousBlacklisting")) {
			songManager.clearSongs();
			geode::log::info("repopulating vector from blacklisting current song");
			Utils::refreshTheVector();
		} else {
			geode::log::info("dangerousBlacklisting is active. added {} to blacklist, removing it from current queue", songBeingBlacklisted);
			geode::log::info("original size: {}", songManager.getSongsSize());
			songManager.removeSong(songBeingBlacklisted);
			geode::log::info("updated size: {}", songManager.getSongsSize());
		}

		if (!Utils::getBool("playlistMode")) Utils::setNewSong();
		else Utils::constantShuffleModeNewSong();

		Utils::composeAndSetCurrentSongDisplayNameOnlyWhenBlacklistingSongs();
		Utils::queueUpdateSCMLabel();

		if (songManager.isOriginalMenuLoop()) {
			if (useCustomSongs && !customSong.empty()) return Utils::newNotification(fmt::format("Blacklisted {}. Have fun with the original menu loop. :)", customSong));
			if (!useCustomSongs && !songName.empty()) return Utils::newNotification(fmt::format("Blacklisted {}. Have fun with the original menu loop. :)", songName));
		}

		if (!useCustomSongs) {
			if (songID != -1) return Utils::newNotification(fmt::format("Blacklisted {} by {} ({}), now playing {}.", songName, songArtist, songID, Utils::getSongName()));
			return Utils::newNotification(fmt::format("Blacklisted {}, now playing {}.", Utils::toNormalizedString(std::filesystem::path(songBeingBlacklisted).filename()), Utils::getSongName()));
		}
		if (!customSong.empty()) return Utils::newNotification(fmt::format("Blacklisted {}, now playing {}.", customSong, Utils::currentCustomSong()));
	}
	void copySong() {
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;
		Utils::copyCurrentSongName();
		geode::Notification::create("[MLR] Current song name copied!", geode::NotificationIcon::None, geode::Mod::get()->getSettingValue<double>("notificationTime"));
	}
	void regenSong() {
		Utils::removeCard();
		if (VANILLA_GD_MENU_LOOP_DISABLED || !Utils::getBool("enableNotification")) return;
		Utils::newCardAndDisplayNameFromCurrentSong();
	}
	void shuffleSong(const SongManager& songManager) {
		Utils::removeCard();
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;

		if (songManager.isOriginalMenuLoop()) {
			geode::log::info("repopulating vector from shuffling song while playing original GD menuloop");
			Utils::refreshTheVector();
		}

		Utils::setNewSong();

		Utils::newCardAndDisplayNameFromCurrentSong();
	}
	void addSongToPlaylist(const std::string& songPath) {
		SongManager& songManager = SongManager::get();
		if (VANILLA_GD_MENU_LOOP_DISABLED) return;

		if (songManager.isOriginalMenuLoop()) return SongControl::woahThereBuddy("There's nothing to add to your playlist! Double-check your config folder again.");
		if (songManager.isOverride()) return SongControl::woahThereBuddy("You're trying to add your own <cy>override</c> to your playlist. Double-check your settings again.");

		if (Utils::getBool("loadPlaylistFile") && !songManager.getPlaylistIsEmpty()) {
			return geode::Notification::create(
				"You've already loaded an MLR playlist file!",
				geode::NotificationIcon::Error, 10.f
			)->show();
		}

		const std::filesystem::path& songAsPath = Utils::toProblematicString(songManager.getCurrentSong());
		if (const geode::Result<int> result = geode::utils::numFromString<int>(geode::utils::string::replace(Utils::toNormalizedString(songAsPath.filename()), Utils::toNormalizedString(songAsPath.extension()), "")); result.isOk()) {
			const int songID = result.unwrapOr(-1);
			if (songID > 0 && songManager.getSawbladeCustomSongsFolder()) return SongControl::woahThereBuddy("Custom Songs Folder by Sawblade is currently loaded! Disable it to add this song to your MLR playlist.");
			MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
			SongInfoObject* songInfoObject = mdm->getSongInfoObject(songID);
			if (songInfoObject && mdm->isResourceSong(songID)) {
				songManager.incrementTowerRepeatCount();
				std::string displayString = fmt::format("{} can't be in a playlist! ", songInfoObject->m_songName);
				if (const int repeats = songManager.getTowerRepeatCount(); repeats > 1) displayString += fmt::format("Touch grass {} times instead.", repeats);
				else displayString += "It's a resource song!";
				return geode::Notification::create(
					displayString,
					geode::NotificationIcon::Error, 5.f
				)->show();
			}
			songManager.resetTowerRepeatCount();
		} else songManager.resetTowerRepeatCount();

		const std::filesystem::path& playlistFilePath = geode::Mod::get()->getSettingValue<std::filesystem::path>("playlistFile");
		if (playlistFilePath.string().empty() || playlistFilePath.extension() != ".txt" || !Utils::notFavoritesNorBlacklist(playlistFilePath)) {
			return geode::Notification::create(
				"Invalid text file selected as your MLR playlist file!",
				geode::NotificationIcon::Error, 5.f
			)->show();
		}

		geode::log::info("adding {} to {} ({})", songPath, playlistFilePath.filename(), playlistFilePath);

		const bool useCustomSongs = Utils::getBool("useCustomSongs");

		const std::string& songName = Utils::getSongName();
		const std::string& songArtist = Utils::getSongArtist();
		const std::string& toWriteToFile = useCustomSongs ? songPath : fmt::format("{} # [MLR] Song: {} by {} | Platform: {} [MLR] #", songPath, songName, songArtist, Utils::getPlatform());

		Utils::writeToFile(toWriteToFile, playlistFilePath);

		geode::Notification::create(
			fmt::format("Successfully added song to playlist file {}!", playlistFilePath.filename()),
			geode::NotificationIcon::Success, 5.f
		)->show();
	}
}