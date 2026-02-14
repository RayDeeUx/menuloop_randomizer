#include "Utils.hpp"
#include "SongManager.hpp"
#include "ui/PlayingCard.hpp"
#include "ui/SongControlMenu.hpp"
#include "ui/SongListLayer.hpp"
#include <random>
#include <regex>
#include <filesystem>
#include <iostream>
#include <fstream>

static std::regex m_songEffectRegex(R"(.*(?:\\|\/)(\S+)\.(mp3|ogg|wav|flac|oga))", std::regex::optimize | std::regex::icase); // see https://regex101.com/r/CqvIvI/1.
static const std::regex m_geodeAudioRegex(R"(((?!\S+geode)(?:\\|\/)(?:([a-z0-9\-_]+\.[a-z0-9\-_]+)(?:\\|\/))([\S ]+)\.(mp3|ogg|wav|flac|oga))$)", std::regex::optimize | std::regex::icase); // see https://regex101.com/r/0b9rY1/1.
static const std::regex m_possiblyJukeboxRegex(R"(.*.[\\\/]([\S]+))", std::regex::optimize | std::regex::icase); // see https://regex101.com/r/Brmwbs/1.
static constexpr int m_desiredIndexForFileName = 1; // easier place to change index

int Utils::randomIndex(int size) {
	// select a random item from the vector and return the path
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, size - 1);
	return dist(gen);
}

bool Utils::isSupportedFile(const std::string_view path) {
	return !path.empty() && Utils::goodExtension(path);
}

bool Utils::goodExtension(const std::string_view path) {
	const std::string& extension = Utils::toNormalizedString(Utils::toProblematicString(path).extension());

	static const std::array<std::string, 5> absolutelyConfirmed = {".mp3", ".wav", ".ogg", ".oga", ".flac"};
	if (std::ranges::find(absolutelyConfirmed.begin(), absolutelyConfirmed.end(), extension) != absolutelyConfirmed.end()) return true;

	// list of extensions adapted from undefined's editor music mod.
	static const std::array<std::string, 16> extensions = {
		".aiff", ".aif", ".aifc",
		".midi", ".mid",
		".m3u", ".m3u8",
		".wma", ".wmv",
		".asf", ".asx",
		".mod", ".it",
		".mp2", ".pls", ".s3m"
	};

	return std::ranges::find(extensions.begin(), extensions.end(), extension) != extensions.end();
}

bool Utils::getBool(const std::string_view setting) {
	return geode::Mod::get()->getSettingValue<bool>(setting);
}

std::string Utils::getString(const std::string_view setting) {
	return geode::Mod::get()->getSettingValue<std::string>(setting);
}

cocos2d::CCNode* Utils::findCard() {
	if (!cocos2d::CCScene::get()) return nullptr;
	return cocos2d::CCScene::get()->getChildByIDRecursive("now-playing"_spr);
}

cocos2d::CCNode* Utils::findCardRemotely() {
	GameManager* gm = GameManager::get();
	if (!gm) return nullptr;
	MenuLayer* menuLayer = gm->m_menuLayer;
	if (!menuLayer) return nullptr;
	cocos2d::CCNode* card = menuLayer->getChildByIDRecursive("now-playing"_spr);
	if (!card) return nullptr;
	return card;
}

void Utils::removeCard() {
	if (cocos2d::CCNode* card = Utils::findCard()) card->removeMeAndCleanup();
}

void Utils::setNewSong() {
	if (Utils::getBool("playlistMode")) return Utils::constantShuffleModeNewSong();
	FMODAudioEngine::sharedEngine()->m_backgroundMusicChannel->stop();
	SongManager& songManager = SongManager::get();
	const std::string& songToBeStored = songManager.getCurrentSong();
	if (!songToBeStored.empty()) {
		if (Utils::songDataContainsSong(songToBeStored) && Utils::getSongDataOfSong(songToBeStored).type != SongType::Blacklisted) {
			songManager.setPreviousSong(songToBeStored);
		}
	}
	else geode::log::info("no current song found, probably");
	songManager.pickRandomSong();
	Utils::queueUpdateFrontfacingLabelsInSCMAndSLL();
	if (!songManager.isOverride()) geode::Mod::get()->setSavedValue<std::string>("lastMenuLoop", songManager.getCurrentSong());
	GameManager::sharedState()->playMenuMusic();
}

void Utils::constantShuffleModeNewSong() {
	if (VANILLA_GD_MENU_LOOP_DISABLED) return;
	if (!Utils::getBool("playlistMode")) return Utils::setNewSong();
	const auto fmod = FMODAudioEngine::sharedEngine();
	float fmodIsCBrained;
	const FMOD_RESULT fmodResult = fmod->m_backgroundMusicChannel->getVolume(&fmodIsCBrained);
	if (fmod->m_musicVolume <= 0.0f || fmod->getBackgroundMusicVolume() <= 0.0f || fmodIsCBrained <= 0.0f) return geode::log::info(" --- !!! MISSION ABORT !!! ---\n\none of the following was at or below 0.0f:\nfmod->m_musicVolume: {}\nfmod->getBackgroundMusicVolume(): {}\nfmodIsCBrained: {} (with fmodResult {} as int)", fmod->m_musicVolume, fmod->getBackgroundMusicVolume(), fmodIsCBrained, static_cast<int>(fmodResult));
	fmod->m_backgroundMusicChannel->stop();
	SongManager& songManager = SongManager::get();
	const std::string& songToBeStored = songManager.getCurrentSong();
	if (!songToBeStored.empty()) {
		if (Utils::songDataContainsSong(songToBeStored) && Utils::getSongDataOfSong(songToBeStored).type != SongType::Blacklisted) {
			songManager.setPreviousSong(songToBeStored);
		}
	}
	// else geode::log::info("no current song found, probably");
	songManager.pickRandomSong();
	Utils::queueUpdateFrontfacingLabelsInSCMAndSLL();
	if (SongManager::get().getAdvancedLogs()) geode::log::info("is it over?");
	if (songManager.getCalledOnce() || !Utils::getBool("saveSongOnGameClose")) {
		if (SongManager::get().getAdvancedLogs()) geode::log::info("playing song as normal");
		if (!songManager.isOverride()) geode::Mod::get()->setSavedValue<std::string>("lastMenuLoop", songManager.getCurrentSong());
	} else if (songManager.isOverride()) {
		const std::string& song = songManager.getOverrideSong();
		if (SongManager::get().getAdvancedLogs()) geode::log::info("playing song from override: {}", song);
		songManager.setCurrentSong(song);
	}
	GameManager::sharedState()->playMenuMusic();
	songManager.setCalledOnce(true);
}

// create notif card stuff
void Utils::newNotification(const std::string& notifString, const bool checkSetting) {
	if (!GameManager::get() || !GameManager::get()->m_menuLayer) return;
	if (cocos2d::CCNode* oldCard = Utils::findCardRemotely()) oldCard->removeMeAndCleanup();
	if (checkSetting && !Utils::getBool("enableNotification")) return;

	auto card = PlayingCard::create(notifString);
	auto screenSize = cocos2d::CCDirector::get()->getWinSize();

	card->position.x = screenSize.width / 2.0f;
	card->position.y = screenSize.height;

	const cocos2d::CCPoint defaultPos = card->position;
	auto posx = defaultPos.x;
	auto posy = defaultPos.y;

	card->setPosition(defaultPos);
	card->setZOrder(200);
	card->setID("now-playing"_spr);

	GameManager::get()->m_menuLayer->addChild(card);

	auto sequence = cocos2d::CCSequence::create(
		cocos2d::CCEaseInOut::create(cocos2d::CCMoveTo::create(1.5f, {posx, posy - 24.0f}), 2.0f),
		cocos2d::CCDelayTime::create(geode::Mod::get()->getSettingValue<double>("notificationTime")),
		cocos2d::CCEaseInOut::create(cocos2d::CCMoveTo::create(1.5f, {posx, posy}), 2.0f),
		nullptr
	);
	card->runAction(sequence);
}

void Utils::composeAndSetCurrentSongDisplayNameOnlyOnLoadOrWhenBlacklistingSongs() {
	SongManager& songManager = SongManager::get();
	const std::filesystem::path& currentSong = Utils::toProblematicString(songManager.getCurrentSong());
	if (songManager.getFinishedCalculatingSongLengths() && !songManager.getSongToSongDataEntries().empty() && songManager.getSongToSongDataEntries().contains(currentSong)) return songManager.setCurrentSongDisplayName(Utils::getSongDataOfCurrentSong().displayName);
	std::string customSongDisplayName = Utils::toNormalizedString(currentSong.stem());
	Utils::sanitizeASCII(customSongDisplayName);
	if (Utils::getBool("useCustomSongs") && songManager.getPlaylistIsEmpty()) return songManager.setCurrentSongDisplayName(customSongDisplayName);
	if (songManager.isOriginalMenuLoop()) return songManager.setCurrentSongDisplayName("Original Menu Loop by RobTop");
	geode::Result<int> songFileNameAsID = geode::utils::numFromString<int>(customSongDisplayName);
	MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
	if (songFileNameAsID.isErr()) {
		if (!songManager.getPlaylistIsEmpty()) return songManager.setCurrentSongDisplayName(customSongDisplayName);
		return songManager.setCurrentSongDisplayName(fmt::format("Unknown ({})", customSongDisplayName));
	}
	const int songID = songFileNameAsID.unwrapOr(-1);
	const bool isNotFromConfigOrAltDir = !Utils::isFromConfigOrAlternateDir(currentSong);
	if (SongInfoObject* songInfo = mdm->getSongInfoObject(songID); songInfo && songID > 0 && isNotFromConfigOrAltDir) return songManager.setCurrentSongDisplayName(Utils::getFormattedNGMLSongName(songInfo, false));
	return songManager.setCurrentSongDisplayName(customSongDisplayName);
}

std::string Utils::composedNotifString(std::string notifString, const std::string& middle, const std::string& suffix) {
	SongManager& songManager = SongManager::get();
	const std::filesystem::path& currentSong = Utils::toProblematicString(songManager.getCurrentSong());
	if (songManager.getFinishedCalculatingSongLengths() && !songManager.getSongToSongDataEntries().empty() && songManager.getSongToSongDataEntries().contains(currentSong)) songManager.setCurrentSongDisplayName(Utils::getSongDataOfCurrentSong().displayName);
	else if (!Utils::getBool("useCustomSongs") || !songManager.getPlaylistIsEmpty()) songManager.setCurrentSongDisplayName(middle);
	Utils::queueUpdateFrontfacingLabelsInSCMAndSLL();
	std::string result = notifString.append(middle).append(suffix);
	return result;
}

void Utils::newCardAndDisplayNameFromCurrentSong() {
	SongManager& songManager = SongManager::get();
	const std::filesystem::path& currentSong = Utils::toProblematicString(songManager.getCurrentSong());
	if (songManager.getFinishedCalculatingSongLengths() && !songManager.getSongToSongDataEntries().empty() && songManager.getSongToSongDataEntries().contains(currentSong)) songManager.setCurrentSongDisplayName(Utils::getSongDataOfCurrentSong().displayName);
	const std::string& songFileName = Utils::toNormalizedString(currentSong.filename());
	const std::string& currentSongRawFileName = Utils::toNormalizedString(currentSong.stem());
	songManager.setCurrentSongDisplayName(songFileName);

	std::string notifString = "";
	if (const std::string& prefix = geode::Mod::get()->getSettingValue<std::string>("customPrefix"); prefix != "[Empty]")
		notifString = fmt::format("{}: ", prefix);

	std::string suffix = "";
	if (songManager.isOverride()) suffix = " (CUSTOM OVERRIDE)";
	else if (songManager.isPreviousSong()) suffix = " (PREVIOUS SONG)";

	if (Utils::getBool("useCustomSongs") && songManager.getPlaylistIsEmpty())
		return Utils::newNotification(Utils::composedNotifString(notifString, currentSongRawFileName, suffix), true);

	// in case that the current file selected is the original menuloop, don't gather any info
	if (songManager.isOriginalMenuLoop())
		return Utils::newNotification(Utils::composedNotifString(notifString, "Original Menu Loop by RobTop", suffix), true);

	geode::log::info("attempting to play {}", songFileName);
	const geode::Result<int> songFileNameAsID = geode::utils::numFromString<int>(currentSongRawFileName);

	if (songFileNameAsID.isErr()) {
		if (!songManager.getPlaylistIsEmpty()) {
			return Utils::newNotification(Utils::composedNotifString(notifString, currentSongRawFileName, suffix), true);
		}
		geode::log::error("{} had an invalid Song ID! [NG/Music Library]", currentSongRawFileName);
		return Utils::newNotification(Utils::composedNotifString(notifString, fmt::format("Unknown ({})", currentSongRawFileName), suffix), true);
	}

	MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
	const int songID = songFileNameAsID.unwrapOr(-1);

	// sometimes songInfo is nullptr, so improvise
	if (SongInfoObject* songInfo = mdm->getSongInfoObject(songID); songInfo && songID > 0 && !Utils::isFromConfigOrAlternateDir(currentSong)) {
		// default: "Song Name, Artist, Song ID"
		// fmt::format("{} by {} ({})", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songID);
		return Utils::newNotification(Utils::composedNotifString(notifString, Utils::getFormattedNGMLSongName(songInfo, false), suffix), true);
	}
	return Utils::newNotification(Utils::composedNotifString(notifString, currentSongRawFileName, suffix), true);
}

bool Utils::adjustSongInfoIfJukeboxReplacedIt(SongInfoObject* songInfo) {
	bool shouldReplaceDisplayName = false;
	const std::filesystem::path& targetPath = geode::dirs::getModsSaveDir() / "fleym.nongd" / "manifest" / fmt::format("{}.json", songInfo->m_songID);
	if (!std::filesystem::exists(targetPath)) return shouldReplaceDisplayName;
	auto parsed = geode::utils::file::readJson(targetPath);
	if (parsed.isOk() && parsed.unwrap().contains("default") && parsed.unwrap().get("default").isOk()) {
		auto fooBar = parsed.unwrap().get("default").unwrap().get("name");
		auto barBaz = parsed.unwrap().get("default").unwrap().get("artist");
		if (fooBar.isOk()) songInfo->m_songName = fooBar.unwrap().asString().unwrapOr(songInfo->m_songName);
		if (barBaz.isOk()) songInfo->m_artistName = barBaz.unwrap().asString().unwrapOr(songInfo->m_artistName);
		if (fooBar.isOk() && barBaz.isOk()) shouldReplaceDisplayName = true;
	}
	return shouldReplaceDisplayName;
}

std::string Utils::getFormattedNGMLSongName(SongInfoObject* songInfo, const bool showFullAnyway) {
	SongManager& songManager = SongManager::get();
	const std::filesystem::path& currentSongPath = Utils::toProblematicString(songManager.getCurrentSong());
	if (Utils::getBool("useCustomSongs") && songManager.getPlaylistIsEmpty()) return Utils::currentCustomSong();
	if (!songInfo) return Utils::toNormalizedString(currentSongPath.filename());
	const std::string& formatSetting = geode::Mod::get()->getSettingValue<std::string>("songFormatNGML");
	const bool isMenuLoopFromNG = songInfo->m_songID == 584131;
	const std::string& robtopSuffix = isMenuLoopFromNG ? " [OOF!]" : "";
	Utils::adjustSongInfoIfJukeboxReplacedIt(songInfo);
	std::string displayName = fmt::format("{}", songInfo->m_songName);
	if (showFullAnyway || formatSetting == "Song Name, Artist, Song ID") displayName = fmt::format("{} by {} ({}){}", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songID, robtopSuffix);
	else if (formatSetting == "Song Name + Artist") displayName = fmt::format("{} by {}{}", songInfo->m_songName, songInfo->m_artistName, robtopSuffix);
	else if (formatSetting == "Song Name + Song ID") displayName = fmt::format("{} ({}){}", songInfo->m_songName, songInfo->m_songID, robtopSuffix);
	Utils::sanitizeASCII(displayName);
	return displayName;
}

void Utils::copyCurrentSongName() {
	std::string currentSongName = SongManager::get().getCurrentSong();
	std::smatch match;
	std::smatch geodeMatch;
	std::smatch jukeboxMatch;
	std::string result = "";
	currentSongName = std::regex_replace(currentSongName, std::regex(R"(com\.geode\.launcher\/)"), ""); // android is cring, original is [ "com\.geode\.launcher\/" ]
	currentSongName = fmt::format("/{}", currentSongName); // adding an extra slash to get it working on all possible paths. this is because combo burst does some stuff under the hood i am too scared to look at and i don't want to define more regex than necessary.
	if (SongManager::get().getAdvancedLogs()) geode::log::info("path after: {}", currentSongName);
	if (currentSongName.find("geode") != std::string::npos && (currentSongName.find("mods") != std::string::npos || currentSongName.find("config") != std::string::npos)) {
		if (std::regex_search(currentSongName, geodeMatch, m_geodeAudioRegex)) {
			result = geodeMatch[geodeMatch.size() - 2].str();
			if (std::regex_search(result, jukeboxMatch, m_possiblyJukeboxRegex)) {
				result = jukeboxMatch[jukeboxMatch.size() - 1].str();
			}
		} else {
			result = "Something went wrong...";
		}
	} else if (std::regex_match(currentSongName, match, m_songEffectRegex)) {
		if (std::regex_search(currentSongName, geodeMatch, m_geodeAudioRegex)) {
			result = geodeMatch[geodeMatch.size() - 2].str();
			if (std::regex_search(result, jukeboxMatch, m_possiblyJukeboxRegex)) {
				result = jukeboxMatch[jukeboxMatch.size() - 1].str();
			}
		} else {
			result = match[m_desiredIndexForFileName].str();
		}
	} else {
		result = currentSongName;
	}
	geode::utils::clipboard::write(result);
}

void Utils::loadFromPlaylistFile(const std::filesystem::path& playlistFile) {
	SongManager& songManager = SongManager::get();
	const std::filesystem::path& configDir = geode::Mod::get()->getConfigDir();

	std::vector<std::string> blacklist = Utils::parseBlacklistFile(configDir / R"(blacklist.txt)");
	std::vector<std::string> favorites = Utils::parseFavoritesFile(configDir / R"(favorites.txt)");

	std::ifstream playlistFileStream(playlistFile);
	std::string playlistFileLineString;
	int lineCount = -1;
	while (std::getline(playlistFileStream, playlistFileLineString)) {
		lineCount++;
		if (playlistFileLineString.starts_with('#') || playlistFileLineString.empty()) continue;
		std::string playlistFileLineModified = playlistFileLineString;
		if (playlistFileLineModified.ends_with(" [MLR] #")) {
			playlistFileLineModified = playlistFileLineModified.substr(0, playlistFileLineModified.find(" # [MLR] Song: "));
		}
		if (!std::filesystem::exists(Utils::toProblematicString(playlistFileLineModified))) {
			geode::log::info("unable to find song on line {} of playlistFile {}", lineCount, playlistFile.filename());
			continue;
		}
		const std::string& songPath = Utils::toNormalizedString(playlistFileLineModified);

		if (!Utils::isSupportedFile(songPath)) {
			geode::log::info("song {} is not supported (check file extension!)", songPath);
			continue;
		}
		if (std::ranges::find(blacklist, songPath) != blacklist.end()) {
			if (SongManager::get().getAdvancedLogs()) geode::log::info("song {} is in blacklist.", songPath);
			continue;
		}

		if (SongManager::get().getAdvancedLogs()) geode::log::info("loading song on line {} of playlistFile {}", lineCount, playlistFile.filename());
		songManager.addSong(playlistFileLineModified);
		if (SongManager::get().getAdvancedLogs()) geode::log::info("adding song from line {} of playlistFile {}", lineCount, songPath);
		if (std::ranges::find(favorites, songPath) != favorites.end()) {
			songManager.addSong(playlistFileLineModified);
			geode::log::info("adding song from line {} of playlistFile {} ***AGAIN because favorited***", lineCount, songPath);
		}
	}
	geode::log::info("Finished storing playlist. loaded songs size: {}", songManager.getSongsSize());
	Utils::popualteSongToSongDataMap();
}

void Utils::populateVector(const bool customSongs, const std::filesystem::path& path, std::vector<std::string> textFileBlacklist, std::vector<std::string> textFileFavorites) {
	if (geode::utils::string::contains(Utils::toNormalizedString(path), "store_your_disabled_menuloops_here") && geode::utils::string::contains(Utils::toNormalizedString(path), Utils::toNormalizedString(geode::Mod::get()->getConfigDir()))) return; // avoid recursion --raydeeux

	SongManager& songManager = SongManager::get();

	// impl playlist files
	const std::filesystem::path& playlistFile = geode::Mod::get()->getSettingValue<std::filesystem::path>("playlistFile");
	songManager.setPlaylistFileName();
	if (geode::Mod::get()->getSettingValue<bool>("loadPlaylistFile") && std::filesystem::exists(playlistFile) && playlistFile.extension() == ".txt") {
		bool isPlaylistEmpty = true;
		std::ifstream playlistFileStream(playlistFile);
		std::string playlistFileLineString;
		while (std::getline(playlistFileStream, playlistFileLineString) && isPlaylistEmpty) {
			if (playlistFileLineString.starts_with('#')) continue;
			if (!playlistFileLineString.empty()) isPlaylistEmpty = false;
		}
		songManager.setPlaylistIsEmpty(isPlaylistEmpty);
		if (!isPlaylistEmpty) {
			Utils::loadFromPlaylistFile(playlistFile);
			return geode::log::info("loading from playlist file instead: {}", playlistFile);
		}
		geode::log::info("playlist file {} is empty. load songs w/o playlist.", playlistFile);
	}

	const std::filesystem::path& configDir = Utils::toNormalizedString(path).empty() ? geode::Mod::get()->getConfigDir() : path;
	/*
		if custom songs are enabled search for files in the config dir
		if not, just use the newgrounds songs
		--elnex
	*/

	const std::vector<std::string>& songManagerBlacklist = songManager.getBlacklist();
	const std::filesystem::path& blacklistPath = configDir / R"(blacklist.txt)";
	const bool blacklistPathExists = std::filesystem::exists(blacklistPath);

	if (blacklistPathExists && textFileBlacklist.empty()) {
		textFileBlacklist = Utils::parseBlacklistFile(blacklistPath);
	}

	const std::vector<std::string>& songManagerFavorites = songManager.getFavorites();
	const std::filesystem::path& favoritePath = configDir / R"(favorites.txt)";
	const bool favoritePathExists = std::filesystem::exists(favoritePath);

	if (favoritePathExists && textFileFavorites.empty()) {
		textFileFavorites = Utils::parseFavoritesFile(favoritePath);
	}

	if (customSongs) {
		for (const auto& file : std::filesystem::directory_iterator(configDir)) {
			if (std::filesystem::is_directory(file) && Utils::getBool("searchDeeper")) {
				Utils::populateVector(customSongs, file, textFileBlacklist, textFileFavorites);
				continue;
			}
			if (!std::filesystem::exists(file)) continue;

			const auto& filePath = file.path();
			const std::string& songPath = Utils::toNormalizedString(filePath);
			if (!Utils::isSupportedFile(songPath)) continue;

			bool isInTextBlacklist = false;

			for (const std::string& string : textFileBlacklist) {
				if (string.starts_with(songPath)) {
					isInTextBlacklist = true;
					break;
				}
			}

			if (std::ranges::find(songManagerBlacklist, songPath) != songManagerBlacklist.end() || isInTextBlacklist) continue;

			if (SongManager::get().getAdvancedLogs()) geode::log::info("Adding custom song: {}", Utils::toNormalizedString(filePath.filename()));
			songManager.addSong(songPath);

			bool isInTextFavorites = false;
			for (const std::string& string : textFileFavorites) {
				if (string.starts_with(songPath)) {
					isInTextFavorites = true;
					break;
				}
			}

			if (std::ranges::find(songManagerFavorites, songPath) != songManagerFavorites.end() || isInTextFavorites) {
				songManager.addSong(songPath);
				if (SongManager::get().getAdvancedLogs()) geode::log::info("Adding FAVORITE custom song: {}", Utils::toNormalizedString(filePath.filename()));
			}
		}
	} else {
		MusicDownloadManager* downloadManager = MusicDownloadManager::sharedState();

		// for every downloaded song push it to the m_songs vector --elnex
		/*
		getDownloadedSongs() function call binding for macOS found
		from ninXout (ARM) and hiimjustin000 (Intel + verification)

		remarks:
		- getDownloadedSongs() grabs both music library and newgrounds
		songs lol.
		- only reason it didn't work was because file support was limited to `.mp3`.
		--raydeeux
		*/
		const bool qualifiedForOGMenuBlacklist = geode::Mod::get()->getSavedValue<bool>("isEry");
		for (const auto song : geode::cocos::CCArrayExt<SongInfoObject*>(downloadManager->getDownloadedSongs())) {
			if (!song) continue;
			if (downloadManager->isResourceSong(song->m_songID)) continue; // resource songs not supported!!!

			std::string songPath = downloadManager->pathForSong(song->m_songID);
			if (!Utils::isSupportedFile(songPath)) continue;

			bool isInTextBlacklist = false;

			for (const std::string& string : textFileBlacklist) {
				if (string.starts_with(songPath)) {
					isInTextBlacklist = true;
					break;
				}
			}

			if (std::ranges::find(songManagerBlacklist.begin(), songManagerBlacklist.end(), songPath) != songManagerBlacklist.end() || isInTextBlacklist || (qualifiedForOGMenuBlacklist && song->m_songID == 584131)) continue; // apply hardcode blacklist 584131 onto self in light of BS edge case caught by hiimjustin001: https://discord.com/channels/911701438269386882/911702535373475870/1289021323279990795

			if (SongManager::get().getAdvancedLogs()) geode::log::info("Adding Newgrounds/Music Library song: {}", songPath);
			songManager.addSong(songPath);

			bool isInTextFavorites = false;
			for (const std::string& string : textFileFavorites) {
				if (string.starts_with(songPath)) {
					isInTextFavorites = true;
					break;
				}
			}

			if (std::ranges::find(songManagerFavorites.begin(), songManagerFavorites.end(), songPath) != songManagerFavorites.end() || isInTextFavorites) {
				songManager.addSong(songPath);
				if (SongManager::get().getAdvancedLogs()) geode::log::info("Adding FAVORITE Newgrounds/Music Library song: {}", Utils::toNormalizedString(songPath));
			}
		}
	}

	Utils::popualteSongToSongDataMap();
}

void Utils::popualteSongToSongDataMap() {
	SongManager& songManager = SongManager::get();

	SongToSongData& songToSongData = songManager.getSongToSongDataEntries();
	if (!songToSongData.empty()) songToSongData.clear();

	const std::vector<std::string>& blacklist = songManager.getBlacklist();
	const std::vector<std::string>& favorites = songManager.getFavorites();
	std::vector<std::string> tempKeys = {};
	std::error_code ec, ed;

	MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
	const std::string& dummyJukeboxPath = Utils::toNormalizedString(geode::dirs::getModsSaveDir() / "fleym.nongd" / "nongs");
	const std::string& hypotheticalPathWhereMDMSongsAreStored = Utils::toNormalizedString(Utils::toProblematicString(mdm->pathForSong(593059)).parent_path()); // this song ID in particular is banned from newgrounds

	for (const std::string_view song : songManager.getSongs()) {
		SongType songType = SongType::Regular;
		if (std::ranges::find(blacklist.begin(), blacklist.end(), song) != blacklist.end()) songType = SongType::Blacklisted;
		else if (std::ranges::find(favorites.begin(), favorites.end(), song) != favorites.end()) songType = SongType::Favorited;

		const std::filesystem::path& theirPath = Utils::toProblematicString(song);
		const std::filesystem::path& theirParentPath = theirPath.parent_path();
		const int songID = geode::utils::numFromString<int>(Utils::toNormalizedString(theirPath.stem())).unwrapOr(-1);

		std::uintmax_t fileSize = std::filesystem::file_size(theirPath, ec);
		std::filesystem::file_time_type fileTime = std::filesystem::last_write_time(theirPath, ed);
		const bool isFromConfigOrAltDirWithoutMDMCheck = Utils::isFromConfigOrAlternateDir(theirParentPath);

		SongInfoObject* songInfoObject = mdm->getSongInfoObject(songID);
		const bool isInNonVanillaNGMLSongLocation = songInfoObject && !geode::utils::string::contains(hypotheticalPathWhereMDMSongsAreStored, Utils::toNormalizedString(theirParentPath));

		SongData songData = {
			.actualFilePath = std::string(song),
			.fileExtension = Utils::toNormalizedString(theirPath.extension()),
			.fileName = Utils::toNormalizedString(theirPath.filename()),
			.type = songType,
			.songFileSize = ec ? std::numeric_limits<std::uintmax_t>::max() : fileSize,
			.hashedPath = std::hash<std::string>{}(std::string(song)),
			.songWriteTime = ed ? std::filesystem::file_time_type::min() : fileTime,
			.isFromConfigOrAltDir = isFromConfigOrAltDirWithoutMDMCheck,
			.couldPossiblyExistInMusicDownloadManager = songID > -1 && songInfoObject,
			.isInNonVanillaNGMLSongLocation = isInNonVanillaNGMLSongLocation,
			.isFromJukeboxDirectory = geode::utils::string::contains(std::string(song), dummyJukeboxPath),
			.isEmpty = false
		};
		if (!isFromConfigOrAltDirWithoutMDMCheck && songData.couldPossiblyExistInMusicDownloadManager && songInfoObject && std::filesystem::exists(geode::dirs::getModsSaveDir() / "fleym.nongd" / "manifest" / fmt::format("{}.json", songID)) && Utils::adjustSongInfoIfJukeboxReplacedIt(songInfoObject)) {
			songData.displayName = Utils::getFormattedNGMLSongName(songInfoObject, false);
			songData.songID = songInfoObject->m_songID;
		}
		songData.displayName = SongListLayer::generateDisplayName(songData);

		std::chrono::system_clock::time_point timepoint = std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(songData.songWriteTime.time_since_epoch()));
		auto datepoint = floor<std::chrono::days>(floor<std::chrono::seconds>(timepoint));
		const std::chrono::year_month_day yearMonthDate{std::chrono::sys_days{datepoint}};
		songData.dateTimeText = fmt::format("{} {:02}, {:04}", months[static_cast<unsigned>(yearMonthDate.month()) - 1], static_cast<unsigned>(yearMonthDate.day()), static_cast<int>(yearMonthDate.year()));
		songData.extraInfoText = fmt::format("{} | {:.2f} MB | {}", songData.fileExtension, songData.songFileSize / 1000000.f, songData.dateTimeText).c_str();

		songToSongData.emplace(theirPath, songData);
		tempKeys.push_back(songData.actualFilePath);
	}

	songManager.setFinishedCalculatingSongLengths(false);
	if (songManager.getUndefined0Alk1m123TouchPrio()) {
		std::thread([]() {
			for (const std::string_view song : SongManager::get().getSongs()) {
				const auto iterator = SongManager::get().getSongToSongDataEntries().find(Utils::toProblematicString(song));
				if (iterator == SongManager::get().getSongToSongDataEntries().end()) continue;
				auto& [unused, songData] = *iterator;
				songData.songLength = SongListLayer::getLength(std::string(song), std::numeric_limits<unsigned int>::max());
				songData.extraInfoText = fmt::format("{} | {:.2f} sec | {:.2f} MB | {}", songData.fileExtension, songData.songLength / 1000.f, songData.songFileSize / 1000000.f, songData.dateTimeText).c_str();
			}
			SongManager::get().setFinishedCalculatingSongLengths(true);
	   }).detach();
	} else songManager.setFinishedCalculatingSongLengths(true);
}

std::vector<std::string> Utils::parseBlacklistFile(const std::filesystem::path& blacklistFilePath) {
	std::vector<std::string> blacklistVector {};
	std::ifstream blacklistFile(blacklistFilePath);
	std::string blacklistString;
	while (std::getline(blacklistFile, blacklistString)) {
		if (blacklistString.starts_with('#') || blacklistString.empty()) continue;
		std::string blacklistStringModified = blacklistString;
		if (blacklistStringModified.ends_with(" [MLR] #")) {
			blacklistStringModified = blacklistStringModified.substr(0, blacklistStringModified.find(" # [MLR] Song: "));
		}
		blacklistVector.push_back(blacklistStringModified);
	}
	if (SongManager::get().getAdvancedLogs()) geode::log::info("Finished storing blacklist. size: {}", blacklistVector.size());
	return blacklistVector;
}

std::vector<std::string> Utils::parseFavoritesFile(const std::filesystem::path& favoritesFilePath) {
	SongManager& songManager = SongManager::get();
	std::vector<std::string> favoritesVector {};
	std::ifstream favoriteFile(favoritesFilePath);
	std::string favoriteString;
	while (std::getline(favoriteFile, favoriteString)) {
		if (favoriteString.starts_with('#') || favoriteString.empty()) continue;
		std::string favoriteStringModified = favoriteString;
		if (favoriteStringModified.ends_with(" [MLR] #")) {
			favoriteStringModified = favoriteStringModified.substr(0, favoriteStringModified.find(" # [MLR] Song: "));
		}
		favoritesVector.push_back(favoriteStringModified);
		songManager.addToFavorites(favoriteStringModified);
	}
	if (SongManager::get().getAdvancedLogs()) geode::log::info("Finished storing favorites. size: {}", favoritesVector.size());
	return favoritesVector;
}

void Utils::refreshTheVector() {
	const bool useCustomSongs = Utils::getBool("useCustomSongs");
	Utils::populateVector(useCustomSongs);
	const std::filesystem::path& additionalFolder = useCustomSongs ? geode::Mod::get()->getSettingValue<std::filesystem::path>("additionalFolder") : "";
	if (useCustomSongs && !Utils::toNormalizedString(additionalFolder).empty() && !geode::utils::string::contains(Utils::toNormalizedString(additionalFolder), Utils::toNormalizedString(geode::Mod::get()->getConfigDir()))) Utils::populateVector(useCustomSongs, additionalFolder);
}

void Utils::showFreezeWarning() {
	if (!cocos2d::CCScene::get()) return;
	if (!SongManager::get().getFinishedCalculatingSongLengths()) {
		FLAlertLayer* alert = FLAlertLayer::create("Sorry About That!", "Menu Loop Randomizer still needed to finish song length calculations from your previous set of songs before you changed your settings.\n\n<cy>Don't panic--the freezing was intended, and it's over now.</c>\n\n<cl>(You might see multiple copies of this message depending on how many settings you changed.)</c>", "I Understand");
		alert->setTag(20260106);
		alert->setUserObject("the-freezing-is-part-of-the-mod"_spr, cocos2d::CCBool::create(true));
		alert->m_noElasticity = true;
		alert->show();
	}
}

void Utils::resetSongManagerRefreshVectorSetNewSongBecause(const std::string_view reasonUsuallySettingName) {
	// make sure m_songs is empty, we don't want to make a mess here --elnexreal
	SongManager& songManager = SongManager::get();
	if (!songManager.getFinishedCalculatingSongLengths()) Utils::showFreezeWarning();
	while (!songManager.getFinishedCalculatingSongLengths()) {} // look i'm out of ideas ok i cant "undo" someone's settings value change and i sure as hell don't want to terminate a detached thread
	songManager.clearSongs();
	songManager.resetHeldSong();
	songManager.resetPreviousSong();

	/*
		for every custom song file, push its path to m_songs
		if they're ng songs also push the path bc we're going to use getPathForSong
		--elnexreal
	*/
	if (SongManager::get().getAdvancedLogs()) geode::log::info("repopulating vector from setting {} change", reasonUsuallySettingName);
	Utils::refreshTheVector();
	if (SongManager::get().getAdvancedLogs()) geode::log::info("finished refreshing the vector");

	// change the song when you click apply, stoi will not like custom names. --elnexreal

	Utils::setNewSong();
	Utils::queueUpdateFrontfacingLabelsInSCMAndSLL();
	if (Utils::getBool("enableNotification")) Utils::newCardAndDisplayNameFromCurrentSong();
}

bool Utils::isFromConfigOrAlternateDir(const std::filesystem::path& parentPath) {
	const geode::Mod* mod = geode::Mod::get();
	const std::string& altDirString = Utils::toNormalizedString(mod->getSettingValue<std::filesystem::path>("additionalFolder"));
	const bool isFromConfigDir = geode::utils::string::contains(Utils::toNormalizedString(parentPath), Utils::toNormalizedString(mod->getConfigDir()));
	const bool isFromAlternateDir = !altDirString.empty() && geode::utils::string::contains(Utils::toNormalizedString(parentPath), altDirString);
	return isFromConfigDir || isFromAlternateDir;
}

SongInfoObject* Utils::getSongInfoObject() {
	SongManager& songManager = SongManager::get();
	if (Utils::getBool("useCustomSongs") && songManager.getPlaylistIsEmpty()) return nullptr;
	if (songManager.isOriginalMenuLoop()) return nullptr;

	MusicDownloadManager* mdm = MusicDownloadManager::sharedState();
	const std::filesystem::path& currentSongFilePath = Utils::toProblematicString(songManager.getCurrentSong());

	if (Utils::isFromConfigOrAlternateDir(currentSongFilePath.parent_path())) return nullptr;

	const geode::Result<int> songFileNameAsID = geode::utils::numFromString<int>(Utils::toNormalizedString(currentSongFilePath.stem()));
	if (songFileNameAsID.isErr()) return nullptr;

	SongInfoObject* songInfo = mdm->getSongInfoObject(songFileNameAsID.unwrapOr(-1));
	Utils::adjustSongInfoIfJukeboxReplacedIt(songInfo);
	return songInfo;
}

std::string Utils::getSongName() {
	const SongInfoObject* songInfo = Utils::getSongInfoObject();
	if (!songInfo) return "";
	std::string ret = songInfo->m_songName;
	if (geode::utils::string::contains(ret, "nong")) {
		const std::string& platform = Utils::getPlatform();
		std::string instructions = "you might need some extra help";
		if (geode::utils::string::startsWith(platform, "Android")) {
			instructions = "open your file manager [preferably ZArchiver, otherwise you would need to close Geode/Geometry Dash] and visit Android/media/com.geode.launcher/save/geode/mods/fleym.nongd/nongs";
		} else if (geode::utils::string::startsWith(platform, "Windows")) {
			instructions = "open File Explorer and visit AppData/Local/GeometryDash/geode/mods/fleym.nongd/nongs";
		} else if (geode::utils::string::startsWith(platform, "macOS")) {
			instructions = "open Finder and visit ~/Library/Application Support/GeometryDash/geode/mods/fleym.nongd/nongs";
		} else if (geode::utils::string::startsWith(platform, "iOS")) {
			instructions = "maybe start with the Files app? There's no knowing what the setup on your iOS device is like, to be fair...";
		}
		return fmt::format("{} (To find this song, {})", ret, instructions);
	}
	return ret;
}

std::string Utils::getSongArtist() {
	SongInfoObject* songInfo = Utils::getSongInfoObject();
	if (!songInfo) return "";
	return songInfo->m_artistName;
}

int Utils::getSongID() {
	SongInfoObject* songInfo = Utils::getSongInfoObject();
	if (!songInfo) return -1;
	return songInfo->m_songID;
}

std::string Utils::currentCustomSong() {
	SongManager& songManager = SongManager::get();
	if (!Utils::getBool("useCustomSongs") || !songManager.getPlaylistIsEmpty()) return Utils::getSongName();
	if (songManager.isOriginalMenuLoop()) return "";
	return Utils::toNormalizedString(Utils::toProblematicString(songManager.getCurrentSong()).filename());
}

std::string Utils::toNormalizedString(const std::filesystem::path& path) {
	return geode::utils::string::pathToString(path);
}

std::filesystem::path Utils::toProblematicString(const std::string_view path) {
	return std::filesystem::path(path);
}

void Utils::fadeOutCardRemotely(cocos2d::CCNode* card) {
	if (!card) return;
	card->stopAllActions();
	card->runAction(cocos2d::CCSequence::create(cocos2d::CCEaseOut::create(cocos2d::CCMoveBy::create(.25f, {0, 24}), 1.f), nullptr));
}

void Utils::removeCardRemotely(cocos2d::CCNode* card) {
	if (card) card->removeMeAndCleanup();
}

void Utils::queueUpdateFrontfacingLabelsInSCMAndSLL() {
	if (SongControlMenu* scm = cocos2d::CCScene::get()->getChildByType<SongControlMenu>(0); scm) geode::Loader::get()->queueInMainThread([scm] { scm->updateCurrentLabel(); });
	else if (SongListLayer* sll = cocos2d::CCScene::get()->getChildByType<SongListLayer>(0); SongManager::get().getUndefined0Alk1m123TouchPrio() && sll && sll->m_searchBar) geode::Loader::get()->queueInMainThread([sll] { sll->displayCurrentSongByLimitingPlaceholderLabelWidth(sll->m_searchBar->getInputNode(), false); });
}

SongData& Utils::getSongDataOfCurrentSong() {
	return Utils::getSongDataOfSong(SongManager::get().getCurrentSong());
}

SongData& Utils::getSongDataOfSong(const std::string_view song) {
	SongManager& songManager = SongManager::get();
	return static_cast<SongData&>(songManager.getSongToSongDataEntries().find(Utils::toProblematicString(song))->second);
}

SongData& Utils::getSongDataOfSongPath(const std::filesystem::path& song) {
	return static_cast<SongData&>(SongManager::get().getSongToSongDataEntries().find(song)->second);
}

bool Utils::songDataContainsSong(const std::string_view song) {
	return SongManager::get().getSongToSongDataEntries().contains(Utils::toProblematicString(song));
}

bool Utils::songDataContainsSongPath(const std::filesystem::path& song) {
	return SongManager::get().getSongToSongDataEntries().contains(song);
}

CCMenuItemSpriteExtra* Utils::addButton(const std::string& name, const cocos2d::SEL_MenuHandler function, cocos2d::CCMenu* menu, cocos2d::CCNode* target, const bool dontAddBG) {
	if (!menu || !target || name.empty()) return nullptr;

	cocos2d::CCSprite* btnSprite = cocos2d::CCSprite::create(fmt::format("{}-btn-sprite.png"_spr, name).c_str());
	btnSprite->setID(fmt::format("{}-button-sprite"_spr, name));

	CCMenuItemSpriteExtra* btn;

	if (dontAddBG) {
		btn = CCMenuItemSpriteExtra::create(btnSprite, target, function);
	} else {
		geode::CircleButtonSprite* circleButtonSprite = geode::CircleButtonSprite::create(btnSprite);
		circleButtonSprite->setID(fmt::format("{}-circle-button-sprite"_spr, name));
		btn = CCMenuItemSpriteExtra::create(circleButtonSprite, target, function);
	}
	btn->setID(fmt::format("{}-button"_spr, name));

	if (!dontAddBG) {
		if (name == "favorite" || name == "prev" || name == "playlist" || name == "add") btnSprite->setScale(.5f);
		else if (name == "controls" || name == "blacklist" || name == "settings") btnSprite->setScale(.4f);
	}

	menu->addChild(btn);
	if (menu->getLayout()) menu->updateLayout();
	return btn;
}

CCMenuItemToggler* Utils::addViewModeToggle(const bool relevantBoolean, const std::string &toggleIcon, const std::string_view nodeID, const cocos2d::SEL_MenuHandler function, cocos2d::CCMenu *menu, cocos2d::CCNode *target, const bool dontAddBG) {
	cocos2d::CCSprite* smallModeIconSpriteOne = cocos2d::CCSprite::createWithSpriteFrameName(toggleIcon.c_str());
	cocos2d::CCSprite* smallModeIconSpriteTwo = cocos2d::CCSprite::createWithSpriteFrameName(toggleIcon.c_str());
	smallModeIconSpriteOne->setID(fmt::format("{}-sprite"_spr, nodeID));
	smallModeIconSpriteTwo->setID(fmt::format("{}-sprite"_spr, nodeID));

	ButtonSprite* spriteOneButtonSprite = ButtonSprite::create(smallModeIconSpriteOne, 30, 30, 30.f, 1.f, false);
	ButtonSprite* spriteTwoButtonSprite = ButtonSprite::create(smallModeIconSpriteTwo, 30, 30, 30.f, 1.f, false);
	spriteOneButtonSprite->updateBGImage("GJ_button_01.png");
	spriteTwoButtonSprite->updateBGImage("GJ_button_02.png");
	spriteOneButtonSprite->setScale(.5f);
	spriteTwoButtonSprite->setScale(.5f);
	spriteOneButtonSprite->setID(fmt::format("{}-off-button-sprite"_spr, nodeID));
	spriteTwoButtonSprite->setID(fmt::format("{}-on-button-sprite"_spr, nodeID));
	spriteOneButtonSprite->m_BGSprite->setID(fmt::format("{}-off-button-background-sprite"_spr, nodeID));
	spriteTwoButtonSprite->m_BGSprite->setID(fmt::format("{}-on-button-background-sprite"_spr, nodeID));
	spriteOneButtonSprite->m_BGSprite->_scale9Image->setID(fmt::format("{}-off-button-background-sprite-scale-nine"_spr, nodeID));
	spriteTwoButtonSprite->m_BGSprite->_scale9Image->setID(fmt::format("{}-on-button-background-sprite-scale-nine"_spr, nodeID));

	if (dontAddBG) {
		spriteOneButtonSprite->setCascadeOpacityEnabled(false);
		spriteTwoButtonSprite->setCascadeOpacityEnabled(false);
		spriteOneButtonSprite->setOpacity(0);
		spriteTwoButtonSprite->setOpacity(0);
	}

	CCMenuItemToggler* toggle = CCMenuItemToggler::create(spriteOneButtonSprite, spriteTwoButtonSprite, target, function);
	toggle->m_offButton->setID(fmt::format("{}-off-sprite"_spr, nodeID));
	toggle->m_onButton->setID(fmt::format("{}-on-sprite"_spr, nodeID));
	toggle->toggle(relevantBoolean);
	toggle->setID(fmt::format("{}-button"_spr, nodeID));

	menu->addChild(toggle);
	if (menu->getLayout()) menu->updateLayout();

	return toggle;
}

void Utils::showMDPopup(const std::string_view title, const std::string& bodyText, const int tag, const std::string_view nodeID) {
	geode::MDPopup* popup = geode::MDPopup::create(fmt::format("MLR {}: A Warning", title), bodyText, "I Understand");
	popup->setID(fmt::format("{}-warning"_spr, nodeID));
	popup->setTag(tag);
	popup->m_noElasticity = true;
	popup->show();
}

bool Utils::notFavoritesNorBlacklist(const std::filesystem::path& filePath) {
	return (filePath != (geode::Mod::get()->getConfigDir() / "favorites.txt")) && (filePath != (geode::Mod::get()->getConfigDir() / "blacklist.txt"));
}

std::string Utils::getPlatform() {
	#if defined(GEODE_IS_IOS)
	return geode::Loader::get()->isPatchless() ? "iOS (JIT-less)" : "iOS (JIT)";
	#elif defined(GEODE_IS_ANDROID32)
	return "Android (32-bit)";
	#elif defined(GEODE_IS_ANDROID64)
	return "Android (64-bit)";
	#elif defined(GEODE_IS_WINDOWS)
	return "Windows (hopefully 64-bit)";
	#elif defined(GEODE_IS_ARM_MAC)
	return "macOS (Apple Silicon/ARM)";
	#elif defined(GEODE_IS_INTEL_MAC)
	return "macOS (Intel)";
	#else
	return "[an unknown platform]";
	#endif
}

std::string Utils::generatePlatformWarning() {
	const std::string& platform = Utils::getPlatform();
	return fmt::format("This playlist file was created using {}. ***__YOU REALLY SHOULD NOT BE SHARING THIS FILE WITH OTHERS__***, but if you have to, only share this file with other MLR users on {}.", platform, platform);
}

void Utils::writeToFile(const std::string& toWriteToFile, const std::filesystem::path& fileForWriting) {
	if (!std::filesystem::exists(fileForWriting) && Utils::notFavoritesNorBlacklist(fileForWriting)) {
		const std::string& platformWarning = Utils::generatePlatformWarning();
		const std::string& playlistSpecificIntro = fmt::format(R"(# Welcome to a Menu Loop Randomizer playlist file!
# This playlist is named "{}".
# Each line that doesn't start with a "#" will be treated as a song file for the playlist.
# All lines that start with a "#" are ignored. This means you can remove a song from the playlist by adding "#" next to it.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# Reports of any bugs or crashes caused by incorrectly formatted lines (those that don't start with "#") will be ignored. Lines that do not start with "#" are always treated as song files by MLR.
# {}
# BUGS/CRASHES FROM FAILING TO FOLLOW THIS BASIC INSTRUCTION WILL BE IGNORED. ALL FILE PATHS ARE SPECIFIC TO YOUR DEVICE ***__ONLY__***. DO ***__NOT__*** DELETE THESE LINES!!!!!!!!!!
# {}
# BUGS/CRASHES FROM FAILING TO FOLLOW THIS BASIC INSTRUCTION WILL BE IGNORED. ALL FILE PATHS ARE SPECIFIC TO YOUR DEVICE ***__ONLY__***. DO ***__NOT__*** DELETE THESE LINES!!!!!!!!!!
# {}
# BUGS/CRASHES FROM FAILING TO FOLLOW THIS BASIC INSTRUCTION WILL BE IGNORED. ALL FILE PATHS ARE SPECIFIC TO YOUR DEVICE ***__ONLY__***. DO ***__NOT__*** DELETE THESE LINES!!!!!!!!!!
# {}
# BUGS/CRASHES FROM FAILING TO FOLLOW THIS BASIC INSTRUCTION WILL BE IGNORED. ALL FILE PATHS ARE SPECIFIC TO YOUR DEVICE ***__ONLY__***. DO ***__NOT__*** DELETE THESE LINES!!!!!!!!!!
# {}
# BUGS/CRASHES FROM FAILING TO FOLLOW THIS BASIC INSTRUCTION WILL BE IGNORED. ALL FILE PATHS ARE SPECIFIC TO YOUR DEVICE ***__ONLY__***. DO ***__NOT__*** DELETE THESE LINES!!!!!!!!!!
# {}
# BUGS/CRASHES FROM FAILING TO FOLLOW THIS BASIC INSTRUCTION WILL BE IGNORED. ALL FILE PATHS ARE SPECIFIC TO YOUR DEVICE ***__ONLY__***. DO ***__NOT__*** DELETE THESE LINES!!!!!!!!!!
# {}
# BUGS/CRASHES FROM FAILING TO FOLLOW THIS BASIC INSTRUCTION WILL BE IGNORED. ALL FILE PATHS ARE SPECIFIC TO YOUR DEVICE ***__ONLY__***. DO ***__NOT__*** DELETE THESE LINES!!!!!!!!!!
# {}
# BUGS/CRASHES FROM FAILING TO FOLLOW THIS BASIC INSTRUCTION WILL BE IGNORED. ALL FILE PATHS ARE SPECIFIC TO YOUR DEVICE ***__ONLY__***. DO ***__NOT__*** DELETE THESE LINES!!!!!!!!!!
# {}
# BUGS/CRASHES FROM FAILING TO FOLLOW THIS BASIC INSTRUCTION WILL BE IGNORED. ALL FILE PATHS ARE SPECIFIC TO YOUR DEVICE ***__ONLY__***. DO ***__NOT__*** DELETE THESE LINES!!!!!!!!!!
# {}
# BUGS/CRASHES FROM FAILING TO FOLLOW THIS BASIC INSTRUCTION WILL BE IGNORED. ALL FILE PATHS ARE SPECIFIC TO YOUR DEVICE ***__ONLY__***. DO ***__NOT__*** DELETE THESE LINES!!!!!!!!!!
# --RayDeeUx)", fileForWriting.filename(),
platformWarning, platformWarning,
platformWarning, platformWarning,
platformWarning, platformWarning,
platformWarning, platformWarning,
platformWarning, platformWarning);
		auto result = geode::utils::file::writeString(fileForWriting, playlistSpecificIntro);
		if (result.isErr()) return geode::log::error("Error writing to {}", fileForWriting);
	} else if (!std::filesystem::exists(fileForWriting)) return geode::log::info("error finding {}!", fileForWriting);
	std::ofstream fileOutput;
	fileOutput.open(fileForWriting, std::ios_base::app);
	if (geode::utils::string::endsWith(toWriteToFile, " [MLR] #") && SongManager::get().getSawbladeCustomSongsFolder()) {
		fileOutput << std::endl << geode::utils::string::replace(toWriteToFile, " [MLR] #", " (Note: This song was added to this file while using a non-vanilla custom songs folder location.) [MLR] #");
	} else {
		fileOutput << std::endl << toWriteToFile;
	}
	fileOutput.close();
}