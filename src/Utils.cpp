#include "SongManager.hpp"
#include "ui/PlayingCard.hpp"
#include "Utils.hpp"
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
	int randomIndex = dist(gen);

	return randomIndex;
}

bool Utils::isSupportedFile(const std::string_view path) {
	return !path.empty() && Utils::goodExtension(path);
}

bool Utils::goodExtension(const std::string_view path) {
	return path.ends_with(".mp3") || path.ends_with(".wav") || path.ends_with(".ogg") || path.ends_with(".oga") || path.ends_with(".flac");
}

bool Utils::getBool(const std::string& setting) {
	return geode::Mod::get()->getSettingValue<bool>(setting);
}

std::string Utils::getString(const std::string& setting) {
	return geode::Mod::get()->getSettingValue<std::string>(setting);
}

cocos2d::CCNode* Utils::findCard() {
	return cocos2d::CCDirector::get()->getRunningScene()->getChildByIDRecursive("now-playing"_spr);
}

cocos2d::CCNode* Utils::findCardRemotely() {
	if (const auto gm = GameManager::get()) {
		if (const auto menuLayer = gm->m_menuLayer) {
			if (const auto card = menuLayer->getChildByIDRecursive("now-playing"_spr)) {
				return card;
			}
		}
	}
	return nullptr;
}

void Utils::removeCard() {
	if (const auto card = Utils::findCard()) card->removeMeAndCleanup();
}

void Utils::setNewSong() {
	if (Utils::getBool("playlistMode")) return Utils::playlistModeNewSong();
	FMODAudioEngine::sharedEngine()->m_backgroundMusicChannel->stop();
	SongManager& songManager = SongManager::get();
	const std::string& songToBeStored = songManager.getCurrentSong();
	if (!songToBeStored.empty()) songManager.setPreviousSong(songToBeStored);
	else geode::log::info("no current song found, probably");
	songManager.pickRandomSong();
	if (!songManager.isOverride()) geode::Mod::get()->setSavedValue<std::string>("lastMenuLoop", songManager.getCurrentSong());
	GameManager::sharedState()->playMenuMusic();
}

void Utils::playlistModeNewSong(const bool fromGJBGL) {
	if (VANILLA_GD_MENU_LOOP_DISABLED) return;
	if (!Utils::getBool("playlistMode")) return Utils::setNewSong();
	geode::log::info("attempting to hijack menuloop channel to use playlist mode");
	const auto fmod = FMODAudioEngine::sharedEngine();
	float fmodIsCBrained;
	const FMOD_RESULT fmodResult = fmod->m_backgroundMusicChannel->getVolume(&fmodIsCBrained);
	if (fmod->m_musicVolume <= 0.0f || fmod->getBackgroundMusicVolume() <= 0.0f || fmodIsCBrained <= 0.0f) return geode::log::info(" --- !!! MISSION ABORT !!! ---\n\none of the following was at or below 0.0f:\nfmod->m_musicVolume: {}\nfmod->getBackgroundMusicVolume(): {}\nfmodIsCBrained: {} (with fmodResult {} as int)", fmod->m_musicVolume, fmod->getBackgroundMusicVolume(), fmodIsCBrained, static_cast<int>(fmodResult));
	fmod->m_backgroundMusicChannel->stop();
	SongManager& songManager = SongManager::get();
	if (!fromGJBGL) {
		const std::string& songToBeStored = songManager.getCurrentSong();
		if (!songToBeStored.empty()) songManager.setPreviousSong(songToBeStored);
		else geode::log::info("no current song found, probably");
	}
	songManager.pickRandomSong();
	geode::log::info("is it over?");
	if (songManager.getCalledOnce() || !Utils::getBool("saveSongOnGameClose")) {
		geode::log::info("playing song as normal");
		fmod->playMusic(songManager.getCurrentSong(), true, 1.0f, 1);
		if (!songManager.isOverride()) geode::Mod::get()->setSavedValue<std::string>("lastMenuLoop", songManager.getCurrentSong());
	} else {
		const bool override = songManager.isOverride();
		const std::string& song = override ? songManager.getOverrideSong() : geode::Mod::get()->getSavedValue<std::string>("lastMenuLoop");
		geode::log::info("playing song from {}: {}", override ? "override" : "saved value", song);
		songManager.setCurrentSong(song);
		fmod->playMusic(song, true, 1.0f, 1);
	}
	songManager.setCalledOnce(true);
}

// create notif card stuff
void Utils::newNotification(const std::string& notifString, const bool checkSetting) {
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

std::string Utils::composedNotifString(std::string notifString, const std::string& middle, const std::string& suffix) {
	return notifString.append(middle).append(suffix);
}

void Utils::newCardAndDisplayNameFromCurrentSong() {
	SongManager& songManager = SongManager::get();
	const std::string& songFileName = Utils::toNormalizedString(std::filesystem::path(songManager.getCurrentSong()).filename());
	songManager.setCurrentSongDisplayName(songFileName);

	std::string notifString = "";
	if (const std::string& prefix = geode::Mod::get()->getSettingValue<std::string>("customPrefix"); prefix != "[Empty]")
		notifString = fmt::format("{}: ", prefix);

	std::string suffix = "";
	if (songManager.isOverride())
		suffix = " (MLR OVERRIDE)";

	if (songManager.isPreviousSong())
		suffix = " (PREVIOUS SONG)";

	if (Utils::getBool("useCustomSongs"))
		return Utils::newNotification(composedNotifString(notifString, songFileName, suffix), true);

	// in case that the current file selected is the original menuloop, don't gather any info
	if (songManager.isOriginalMenuLoop()) {
		const std::string& robtop = "Original Menu Loop by RobTop";
		songManager.setCurrentSongDisplayName(robtop);
		return Utils::newNotification(composedNotifString(notifString, robtop, suffix), true);
	}

	geode::log::info("attempting to play {}", songFileName);
	// if it's not menuLoop.mp3, then get info
	size_t dotPos = songFileName.find_last_of('.');

	if (dotPos == std::string::npos) {
		geode::log::error("{} was not a valid file name...? [NG/Music Library]", songFileName);
		const std::string& unknown = "Unknown";
		songManager.setCurrentSongDisplayName(unknown);
		return Utils::newNotification(composedNotifString(notifString, unknown, suffix), true);
	}

	std::string songFileNameWithoutExtension = songFileName.substr(0, dotPos);

	geode::Result<int> songFileNameAsID = geode::utils::numFromString<int>(songFileNameWithoutExtension);

	if (songFileNameAsID.isErr()) {
		geode::log::error("{} had an invalid Song ID! [NG/Music Library]", songFileNameWithoutExtension);
		songManager.setCurrentSongDisplayName(songFileNameWithoutExtension);
		return Utils::newNotification(composedNotifString(notifString, fmt::format("Unknown ({})", songFileNameWithoutExtension), suffix), true);
	}

	// sometimes songInfo is nullptr, so improvise
	if (SongInfoObject* songInfo = MusicDownloadManager::sharedState()->getSongInfoObject(songFileNameAsID.unwrap())) {
		// default: "Song Name, Artist, Song ID"
		// fmt::format("{} by {} ({})", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songID);
		const std::string& resultString = Utils::getFormattedNGMLSongName(songInfo);
		songManager.setCurrentSongDisplayName(resultString);
		return Utils::newNotification(composedNotifString(notifString, resultString, suffix), true);
	}
	songManager.setCurrentSongDisplayName(songFileName);
	return Utils::newNotification(composedNotifString(notifString, songFileName, suffix), true);
}

std::string Utils::getFormattedNGMLSongName(SongInfoObject* songInfo) {
	if (Utils::getBool("useCustomSongs")) return Utils::currentCustomSong();
	if (!songInfo) return Utils::toNormalizedString(std::filesystem::path(SongManager::get().getCurrentSong()).filename());
	const std::string& formatSetting = geode::Mod::get()->getSettingValue<std::string>("songFormatNGML");
	const bool isMenuLoopFromNG = songInfo->m_songID == 584131;
	const std::string& robtopSuffix = isMenuLoopFromNG ? " [OOF!]" : "";
	if (formatSetting == "Song Name, Artist, Song ID") return fmt::format("{} by {} ({}){}", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songID, robtopSuffix);
	if (formatSetting == "Song Name + Artist") return fmt::format("{} by {}{}", songInfo->m_songName, songInfo->m_artistName, robtopSuffix);
	if (formatSetting == "Song Name + Song ID") return fmt::format("{} ({}){}", songInfo->m_songName, songInfo->m_songID, robtopSuffix);
	return fmt::format("{}", songInfo->m_songName);
}

void Utils::playlistModePLAndEPL() {
	if (Utils::getBool("playlistMode") && GJBaseGameLayer::get()) return Utils::playlistModeNewSong(true);
}

void Utils::copyCurrentSongName() {
	std::string currentSongName = SongManager::get().getCurrentSong();
	std::smatch match;
	std::smatch geodeMatch;
	std::smatch jukeboxMatch;
	std::string result = "";
	currentSongName = std::regex_replace(currentSongName, std::regex(R"(com\.geode\.launcher\/)"), ""); // android is cring, original is [ "com\.geode\.launcher\/" ]
	currentSongName = fmt::format("/{}", currentSongName); // adding an extra slash to get it working on all possible paths. this is because combo burst does some stuff under the hood i am too scared to look at and i don't want to define more regex than necessary.
	geode::log::info("path after: {}", currentSongName);
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

void Utils::populateVector(const bool customSongs, const std::filesystem::path& path, std::vector<std::string> textFileBlacklist, std::vector<std::string> textFileFavorites) {
	if (geode::utils::string::contains(Utils::toNormalizedString(path), "store_your_disabled_menuloops_here") && geode::utils::string::contains(Utils::toNormalizedString(path), Utils::toNormalizedString(geode::Mod::get()->getConfigDir()))) return;
	const std::filesystem::path configDir = path.string().empty() ? geode::Mod::get()->getConfigDir() : path;
	/*
		if custom songs are enabled search for files in the config dir
		if not, just use the newgrounds songs
		--elnex
	*/

	SongManager& songManager = SongManager::get();

	std::vector<std::string> songManagerBlacklist = songManager.getBlacklist();

	if (auto blacklistPath = std::filesystem::exists(configDir / R"(blacklist.txt)"); blacklistPath && textFileBlacklist.empty()) {
		std::ifstream blacklistFile((configDir / R"(blacklist.txt)"));
		std::string blacklistString;
		while (std::getline(blacklistFile, blacklistString)) {
			if (blacklistString.starts_with('#') || blacklistString.empty()) continue;
			std::string blacklistStringModified = blacklistString;
			if (blacklistStringModified.ends_with(" [MLR] #")) {
				blacklistStringModified = blacklistStringModified.substr(0, blacklistStringModified.find(" # [MLR] Song: "));
			}
			textFileBlacklist.push_back(blacklistStringModified);
		}
		geode::log::info("Finished storing blacklist. size: {}", textFileBlacklist.size());
	}

	std::vector<std::string> songManagerFavorites = songManager.getFavorites();

	if (auto favoritePath = std::filesystem::exists(configDir / R"(favorites.txt)"); favoritePath && textFileFavorites.empty()) {
		std::ifstream favoriteFile((configDir / R"(favorites.txt)"));
		std::string favoriteString;
		while (std::getline(favoriteFile, favoriteString)) {
			if (favoriteString.starts_with('#') || favoriteString.empty()) continue;
			std::string favoriteStringModified = favoriteString;
			if (favoriteStringModified.ends_with(" [MLR] #")) {
				favoriteStringModified = favoriteStringModified.substr(0, favoriteStringModified.find(" # [MLR] Song: "));
			}
			textFileFavorites.push_back(favoriteStringModified);
			songManager.addToFavorites(favoriteStringModified);
		}
		geode::log::info("Finished storing favorites. size: {}", textFileFavorites.size());
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

			geode::log::info("Adding custom song: {}", Utils::toNormalizedString(filePath.filename()));
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
				geode::log::info("Adding FAVORITE custom song: {}", Utils::toNormalizedString(filePath.filename()));
			}
		}
	} else {
		auto downloadManager = MusicDownloadManager::sharedState();

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
		geode::cocos::CCArrayExt<SongInfoObject*> songs = downloadManager->getDownloadedSongs();
		bool qualifiedForOGMenuBlacklist = geode::Mod::get()->getSavedValue<bool>("isEry");
		for (auto song : songs) {
			if (!song) continue;

			std::string songPath = downloadManager->pathForSong(song->m_songID);
			if (!Utils::isSupportedFile(songPath)) continue;

			bool isInTextBlacklist = false;

			for (const std::string& string : textFileBlacklist) {
				if (string.starts_with(songPath)) {
					isInTextBlacklist = true;
					break;
				}
			}

			if (std::ranges::find(songManagerBlacklist, songPath) != songManagerBlacklist.end() || isInTextBlacklist || (qualifiedForOGMenuBlacklist && song->m_songID == 584131)) continue; // apply hardcode blacklist 584131 onto self in light of BS edge case caught by hiimjustin001: https://discord.com/channels/911701438269386882/911702535373475870/1289021323279990795

			geode::log::info("Adding Newgrounds/Music Library song: {}", songPath);
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
				geode::log::info("Adding FAVORITE Newgrounds/Music Library song: {}", Utils::toNormalizedString(songPath));
			}
		}
	}
}

SongInfoObject* Utils::getSongInfoObject() {
	if (Utils::getBool("useCustomSongs")) return nullptr;
	SongManager& songManager = SongManager::get();
	if (songManager.isOriginalMenuLoop()) return nullptr;

	std::string songFileName = toNormalizedString(std::filesystem::path(songManager.getCurrentSong()).filename());

	// if it's not menuLoop.mp3, then get info
	size_t dotPos = songFileName.find_last_of('.');

	if (dotPos == std::string::npos) return nullptr;

	std::string songFileNameAsAtring = songFileName.substr(0, dotPos);

	geode::Result<int> songFileNameAsID = geode::utils::numFromString<int>(songFileNameAsAtring);

	if (songFileNameAsID.isErr()) return nullptr;

	return MusicDownloadManager::sharedState()->getSongInfoObject(songFileNameAsID.unwrap());
}

std::string Utils::getSongName() {
	const auto songInfo = Utils::getSongInfoObject();
	if (!songInfo) return "";
	std::string ret = songInfo->m_songName;
	#ifndef __APPLE__
	if (geode::utils::string::contains(ret, "nong")) {
		std::string instructions = "open File Explorer and visit AppData/Local/GeometryDash/geode/mods/fleym.nongd/nongs";
		#ifdef GEODE_IS_ANDROID
		instructions = "open your file manager [preferably ZArchiver, otherwise you would need to close Geode/Geometry Dash] and visit Android/media/com.geode.launcher/save/geode/mods/fleym.nongd/nongs";
		#endif
		return fmt::format("{} (To find this song, {})", ret, instructions);
	}
	#endif
	return ret;
}

std::string Utils::getSongArtist() {
	const auto songInfo = Utils::getSongInfoObject();
	if (!songInfo) return "";
	return songInfo->m_artistName;
}

int Utils::getSongID() {
	const auto songInfo = Utils::getSongInfoObject();
	if (!songInfo) return -1;
	return songInfo->m_songID;
}

std::string Utils::currentCustomSong() {
	if (!Utils::getBool("useCustomSongs")) return Utils::getSongName();
	SongManager& songManager = SongManager::get();
	if (songManager.isOriginalMenuLoop()) return "";
	return Utils::toNormalizedString(std::filesystem::path(songManager.getCurrentSong()).filename());
}

std::string Utils::toNormalizedString(const std::filesystem::path& path) {
	#ifdef GEODE_IS_WINDOWS
	return geode::utils::string::wideToUtf8(path.wstring());
	#else
	return path.string();
	#endif
}

void Utils::fadeOutCardRemotely(cocos2d::CCNode* card) {
	if (!card) return;
	card->stopAllActions();
	card->runAction(cocos2d::CCSequence::create(
		cocos2d::CCEaseOut::create(cocos2d::CCMoveBy::create(.25f, {0, 24}), 1.f),
		nullptr
	));
}

void Utils::removeCardRemotely(cocos2d::CCNode* card) {
	if (!card) return;
	card->removeMeAndCleanup();
}

void Utils::addButton(const std::string& name, const cocos2d::SEL_MenuHandler function, cocos2d::CCMenu* menu, cocos2d::CCNode* target) {
	if (!menu || !target || name.empty()) return;

	CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(
		geode::CircleButtonSprite::create(cocos2d::CCSprite::create(fmt::format("{}-btn-sprite.png"_spr, name).c_str())),
		target, function
	);
	btn->setID(fmt::format("{}-button"_spr, name));

	if (cocos2d::CCSprite* cocosSprite = btn->getChildByType<geode::CircleButtonSprite>(0)->getChildByType<cocos2d::CCSprite>(0)) {
		if (name == "favorite" || name == "prev") cocosSprite->setScale(.5f);
		else if (name == "controls" || name == "blacklist") cocosSprite->setScale(.4f);
	}

	menu->addChild(btn);
	menu->updateLayout();
}

void Utils::writeToFile(const std::string& toWriteToFile, std::filesystem::path fileForWriting) {
	if (!std::filesystem::exists(fileForWriting)) return geode::log::info("error finding {}!", fileForWriting);
	std::ofstream fileOutput;
	fileOutput.open(fileForWriting, std::ios_base::app);
	fileOutput << std::endl << toWriteToFile;
	fileOutput.close();
}