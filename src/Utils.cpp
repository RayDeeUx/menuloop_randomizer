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
static const int m_desiredIndexForFileName = 1; // easier place to change index

int Utils::randomIndex(int size) {
	// select a random item from the vector and return the path
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, size - 1);
	int randomIndex = dist(gen);

	return randomIndex;
}

bool Utils::isSupportedFile(const std::string_view path) {
	return std::filesystem::exists(path) && !path.empty() && (path.ends_with(".mp3") || path.ends_with(".wav") || path.ends_with(".ogg") || path.ends_with(".oga") || path.ends_with(".flac"));
}

bool Utils::getBool(std::string setting) {
	return geode::Mod::get()->getSettingValue<bool>(setting);
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
	if (auto card = Utils::findCard())
		card->removeMeAndCleanup();
}

void Utils::setNewSong() {
	if (Utils::getBool("playlistMode")) return Utils::playlistModeNewSong();
	SongManager& songManager = SongManager::get();
	FMODAudioEngine::sharedEngine()->m_backgroundMusicChannel->stop();
	songManager.pickRandomSong();
	if (!songManager.isOverride()) geode::Mod::get()->setSavedValue<std::string>("lastMenuLoop", songManager.getCurrentSong());
	GameManager::sharedState()->playMenuMusic();
}

void Utils::playlistModeNewSong() {
	if (GameManager::sharedState()->getGameVariable("0122")) return;
	if (!Utils::getBool("playlistMode")) return Utils::setNewSong();
	geode::log::info("attempting to hijack menuloop channel to use playlist mode");
	const auto fmod = FMODAudioEngine::sharedEngine();
	float fmodIsCBrained;
	const FMOD_RESULT fmodResult = fmod->m_backgroundMusicChannel->getVolume(&fmodIsCBrained);
	if (fmod->m_musicVolume <= 0.0f || fmod->getBackgroundMusicVolume() <= 0.0f || fmodIsCBrained <= 0.0f) return geode::log::info(" --- !!! MISSION ABORT !!! ---\n\none of the following was at or below 0.0f:\nfmod->m_musicVolume: {}\nfmod->getBackgroundMusicVolume(): {}\nfmodIsCBrained: {} (with fmodResult {} as int)", fmod->m_musicVolume, fmod->getBackgroundMusicVolume(), fmodIsCBrained, static_cast<int>(fmodResult));
	fmod->m_backgroundMusicChannel->stop();
	SongManager& songManager = SongManager::get();
	songManager.pickRandomSong();
	geode::log::info("is it over?");
	if (songManager.getCalledOnce() || !Utils::getBool("saveSongOnGameClose")) {
		geode::log::info("playing song as normal");
		fmod->playMusic(songManager.getCurrentSong(), true, 1.0f, 1);
		if (!songManager.isOverride()) geode::Mod::get()->setSavedValue<std::string>("lastMenuLoop", songManager.getCurrentSong());
	} else {
		std::string lastSong = geode::Mod::get()->getSavedValue<std::string>("lastMenuLoop");
		geode::log::info("playing song from saved value: {}", lastSong);
		songManager.setCurrentSong(lastSong);
		fmod->playMusic(lastSong, true, 1.0f, 1);
	}
	songManager.setCalledOnce(true);
}

// create notif card stuff
void Utils::makeNewCard(const std::string& notifString) {
	if (cocos2d::CCNode* oldCard = Utils::findCardRemotely()) oldCard->removeMeAndCleanup();

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

void Utils::generateNotification() {
	SongManager& songManager = SongManager::get();
	std::string songFileName = Utils::toNormalizedString(std::filesystem::path(songManager.getCurrentSong()).filename());

	std::string notifString;
	auto prefix = geode::Mod::get()->getSettingValue<std::string>("customPrefix");
	if (prefix != "[Empty]")
		notifString = fmt::format("{}: ", prefix);

	if (songManager.isOverride())
		return Utils::makeNewCard(notifString.append(fmt::format("{} (MLR OVERRIDE)", songFileName)));

	if (Utils::getBool("useCustomSongs"))
		return Utils::makeNewCard(notifString.append(songFileName));

	// in case that the current file selected is the original menuloop, don't gather any info
	if (SongManager::get().isOriginalMenuLoop())
		return Utils::makeNewCard(notifString.append("Original Menu Loop by RobTop"));

	geode::log::info("attempting to play {}", songFileName);
	// if it's not menuLoop.mp3, then get info
	size_t dotPos = songFileName.find_last_of('.');

	if (dotPos == std::string::npos) {
		geode::log::error("{} was not a valid file name...? [NG/Music Library]", songFileName);
		return Utils::makeNewCard(notifString.append("Unknown"));
	}

	std::string songFileNameWithoutExtension = songFileName.substr(0, dotPos);

	geode::Result<int> songFileNameAsID = geode::utils::numFromString<int>(songFileNameWithoutExtension);

	if (songFileNameAsID.isErr()) {
		geode::log::error("{} had an invalid Song ID! [NG/Music Library]", songFileNameWithoutExtension);
		return Utils::makeNewCard(notifString.append(fmt::format("Unknown ({})", songFileNameWithoutExtension)));
	}

	auto songInfo = MusicDownloadManager::sharedState()->getSongInfoObject(songFileNameAsID.unwrap());

	// sometimes songInfo is nullptr, so improvise
	if (songInfo) {
		// default: "Song Name, Artist, Song ID"
		// fmt::format("{} by {} ({})", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songID);
		std::string resultString = "";
		auto formatSetting = geode::Mod::get()->getSettingValue<std::string>("songFormatNGML");
		bool isMenuLoopFromNG = songInfo->m_songID == 584131;
		if (formatSetting == "Song Name, Artist, Song ID") {
			resultString = fmt::format("{} by {} ({})", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songID);
		} else if (formatSetting == "Song Name + Artist") {
			resultString = fmt::format("{} by {}", songInfo->m_songName, songInfo->m_artistName);
		} else if (formatSetting == "Song Name + Song ID") {
			resultString = fmt::format("{} ({})", songInfo->m_songName, songInfo->m_songID);
		} else {
			resultString = fmt::format("{}", songInfo->m_songName);
		}
		if (isMenuLoopFromNG) resultString = resultString.append(" [OOF!]");
		return Utils::makeNewCard(notifString.append(resultString));
	}
	return Utils::makeNewCard(notifString.append(songFileName));
}

void Utils::playlistModePLAndEPL() {
	if (Utils::getBool("playlistMode") && GJBaseGameLayer::get()) return Utils::playlistModeNewSong();
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

void Utils::populateVector(bool customSongs) {
	auto configDir = geode::Mod::get()->getConfigDir();
	/*
		if custom songs are enabled search for files in the config dir
		if not, just use the newgrounds songs
		--elnex
	*/

	std::vector<std::string> blacklist;
	std::vector<std::string> otherBlacklist = SongManager::get().getBlacklist();

	if (auto blacklistPath = std::filesystem::exists(configDir / R"(blacklist.txt)")) {
		std::ifstream blacklistFile((configDir / R"(blacklist.txt)"));
		std::string blacklistString;
		while (std::getline(blacklistFile, blacklistString)) {
			if (blacklistString.starts_with('#') || blacklistString.empty()) continue;
			std::string blacklistStringModified = blacklistString;
			if (blacklistStringModified.ends_with(" [MLR] #")) {
				blacklistStringModified = blacklistStringModified.substr(0, blacklistStringModified.find(" # [MLR] Song: "));
			}
			blacklist.push_back(blacklistStringModified);
			geode::log::info("adding to blacklist: {}", blacklistStringModified);
		}
		geode::log::info("Finished storing blacklist. size: {}", blacklist.size());
	}

	if (customSongs) {
		for (auto file : std::filesystem::directory_iterator(configDir)) {
			if (!std::filesystem::exists(file)) continue;

			const auto& filePath = file.path();
			auto filePathString = Utils::toNormalizedString(filePath);

			bool isInTextBlacklist = false;

			for (const std::string& string : blacklist) {
				if (string.starts_with(filePathString)) {
					isInTextBlacklist = true;
					break;
				}
			}

			if (!Utils::isSupportedFile(filePathString) || std::ranges::find(otherBlacklist, filePathString) != otherBlacklist.end() || isInTextBlacklist) continue;

			geode::log::info("Adding custom song: {}", Utils::toNormalizedString(filePath.filename()));
			SongManager::get().addSong(filePathString);
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

			bool isInTextBlacklist = false;

			for (const std::string& string : blacklist) {
				if (string.starts_with(songPath)) {
					isInTextBlacklist = true;
					break;
				}
			}

			if (!Utils::isSupportedFile(songPath) || std::ranges::find(otherBlacklist, songPath) != otherBlacklist.end() || isInTextBlacklist || (qualifiedForOGMenuBlacklist && song->m_songID == 584131)) continue; // apply hardcode blacklist 584131 onto self in light of BS edge case caught by hiimjustin001: https://discord.com/channels/911701438269386882/911702535373475870/1289021323279990795

			geode::log::info("Adding Newgrounds/Music Library song: {}", songPath);
			SongManager::get().addSong(songPath);
		}
		// same thing as NG but for music library as well --ninXout
		// SPOILER: IT DOESN'T WORK CROSSPLATFORM (android specifically)! --raydeeux
		/*
		std::filesystem::path musicLibrarySongs = dirs::getGeodeDir().parent_path() / "Resources" / "songs";
		if (!std::filesystem::exists(musicLibrarySongs)) return;
		for (const std::filesystem::path& dirEntry : std::filesystem::recursive_directory_iterator(musicLibrarySongs)) {
			if (!std::filesystem::exists(dirEntry)) continue;

			std::string songPath = Utils::toNormalizedString(dirEntry);

			if (!Utils::isSupportedExtension(songPath)) continue;

			log::info("Adding Music Library song: {}", songPath);
			songManager.addSong(songPath);
		}
		*/
	}
}

SongInfoObject* Utils::getSongInfoObject() {
	if (Utils::getBool("useCustomSongs")) return nullptr;
	if (SongManager::get().isOriginalMenuLoop()) return nullptr;

	std::string songFileName = toNormalizedString(std::filesystem::path(SongManager::get().getCurrentSong()).filename());

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
	if (SongManager::get().isOriginalMenuLoop()) return "";
	return Utils::toNormalizedString(std::filesystem::path(SongManager::get().getCurrentSong()).filename());
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