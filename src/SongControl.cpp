#include "SongControl.hpp"
#include "Utils.hpp"
#include <Geode/ui/GeodeUI.hpp>

namespace SongControl {
	void woahThereBuddy(const std::string& reason) {
		geode::createQuickPopup(
			"Menu Loop Randomizer", reason,
			"Never Mind", "Open Mod Settings",
			[](FLAlertLayer*, bool openConfig) {
				if (!openConfig) return;
				openSettingsPopup(geode::Mod::get());
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
			Utils::newCardAndDisplayNameFromCurrentSong();
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

		const std::string& currentSong = songManager.getCurrentSong();

		if (const auto songManagerBlacklist = songManager.getBlacklist(); std::ranges::find(songManagerBlacklist, currentSong) != songManagerBlacklist.end()) return SongControl::woahThereBuddy("You've already blacklisted this song. Double-check your <cl>blacklist.txt</c> again.");
		if (const auto songManagerFavorites = songManager.getFavorites(); std::ranges::find(songManagerFavorites, currentSong) != songManagerFavorites.end()) return SongControl::woahThereBuddy("You've already favorited this song! Double-check your <cl>favorites.txt</c> again.");

		geode::log::info("blacklisting: {}", currentSong);

		const bool useCustomSongs = Utils::getBool("useCustomSongs");
		const std::string& songName = Utils::getSongName();
		const std::string& songArtist = Utils::getSongArtist();
		const std::string& customSong = Utils::currentCustomSong();
		const int songID = Utils::getSongID();
		const std::string& toWriteToFile = useCustomSongs ? currentSong : fmt::format("{} # [MLR] Song: {} by {} [MLR] #", currentSong, songName, songArtist);

		Utils::writeToFile(toWriteToFile, BLACKLIST_FILE);

		songManager.addToBlacklist();
		if (!Utils::getBool("dangerousBlacklisting")) {
			songManager.clearSongs();
			geode::log::info("repopulating vector from blacklisting current song");
			Utils::refreshTheVector();
		} else {
			geode::log::info("dangerousBlacklisting is active. added {} to blacklist, removing it from current queue", currentSong);
			geode::log::info("original size: {}", songManager.getSongsSize());
			songManager.removeSong(currentSong);
			geode::log::info("updated size: {}", songManager.getSongsSize());
		}

		if (!Utils::getBool("playlistMode")) Utils::setNewSong();
		else Utils::constantShuffleModeNewSong();

		if (!Utils::getBool("enableNotification")) return;

		if (songManager.isOriginalMenuLoop()) {
			if (useCustomSongs && !customSong.empty()) return Utils::newNotification(fmt::format("Blacklisted {}. Have fun with the original menu loop. :)", customSong));
			if (!useCustomSongs && !songName.empty()) return Utils::newNotification(fmt::format("Blacklisted {}. Have fun with the original menu loop. :)", songName));
		}

		if (!useCustomSongs) return Utils::newNotification(fmt::format("Blacklisted {} by {} ({}), now playing {}.", songName, songArtist, songID, Utils::getSongName()));
		if (!customSong.empty()) return Utils::newNotification(fmt::format("Blacklisted {}, now playing {}.", customSong, Utils::currentCustomSong()));

		Utils::queueUpdateSCMLabel();
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
	void shuffleSong(SongManager& songManager) {
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

		const std::filesystem::path playlistFilePath = geode::Mod::get()->getSettingValue<std::filesystem::path>("playlistFile");
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