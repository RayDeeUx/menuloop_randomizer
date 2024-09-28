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

bool Utils::isSupportedExtension(std::string path) {
	return !path.empty() && (path.ends_with(".mp3") || path.ends_with(".wav") || path.ends_with(".ogg") || path.ends_with(".oga") || path.ends_with(".flac"));
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
	if (Utils::getBool("playlistMode")) {
		return Utils::playlistModeNewSong();
	}
	FMODAudioEngine::sharedEngine()->m_backgroundMusicChannel->stop();
	SongManager::get().pickRandomSong();
	geode::Mod::get()->setSavedValue<std::string>("lastMenuLoop", SongManager::get().getCurrentSong());
	GameManager::sharedState()->playMenuMusic();
}

void Utils::playlistModeNewSong() {
	if (GameManager::sharedState()->getGameVariable("0122")) return;
	if (!Utils::getBool("playlistMode")) {
		return Utils::setNewSong();
	}
	geode::log::info("attempting to hijack menuloop channel to use playlist mode");
	auto fmod = FMODAudioEngine::sharedEngine();
	fmod->m_backgroundMusicChannel->stop();
	SongManager::get().pickRandomSong();
	geode::log::info("is it over?");
	if (SongManager::get().getCalledOnce() || !Utils::getBool("saveSongOnGameClose")) {
		geode::log::info("playing song as normal");
		fmod->playMusic(SongManager::get().getCurrentSong(), true, 1.0f, 1);
		geode::Mod::get()->setSavedValue<std::string>("lastMenuLoop", SongManager::get().getCurrentSong());
	} else {
		std::string lastSong = geode::Mod::get()->getSavedValue<std::string>("lastMenuLoop");
		geode::log::info("playing song from saved value: {}", lastSong);
		SongManager::get().setCurrentSong(lastSong);
		fmod->playMusic(lastSong, true, 1.0f, 1);
	}
	SongManager::get().setCalledOnce(true);
}

// create notif card stuff
void Utils::makeNewCard(const std::string& notifString) {
	if (auto oldCard = Utils::findCardRemotely()) {
		oldCard->removeMeAndCleanup();
	}
	auto card = PlayingCard::create(notifString);
	auto screenSize = cocos2d::CCDirector::get()->getWinSize();

	card->position.x = screenSize.width / 2.0f;
	card->position.y = screenSize.height;

	auto defaultPos = card->position;
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
	auto songFileName = std::filesystem::path(SongManager::get().getCurrentSong()).filename();

	std::string notifString;
	auto prefix = geode::Mod::get()->getSettingValue<std::string>("customPrefix");
	if (prefix != "[Empty]")
		notifString = fmt::format("{}: ", prefix);

	if (Utils::getBool("useCustomSongs"))
		return Utils::makeNewCard(notifString.append(songFileName.string()));

	// in case that the current file selected is the original menuloop, don't gather any info
	if (SongManager::get().isOriginalMenuLoop())
		return Utils::makeNewCard(notifString.append("Original Menu Loop by RobTop"));

	geode::log::info("attempting to play {}", songFileName.string());
	// if it's not menuLoop.mp3, then get info
	size_t dotPos = songFileName.string().find_last_of('.');

	if (dotPos == std::string::npos) {
		geode::log::error("{} was not a valid file name...? [NG/Music Library]", songFileName.string());
		return Utils::makeNewCard(notifString.append("Unknown"));
	}

	std::string songFileNameAsAtring = songFileName.string().substr(0, dotPos);

	geode::Result<int> songFileNameAsID = geode::utils::numFromString<int>(songFileNameAsAtring);

	if (songFileNameAsID.isErr()) {
		geode::log::error("{} had an invalid Song ID! [NG/Music Library]", songFileNameAsAtring);
		return Utils::makeNewCard(notifString.append(fmt::format("Unknown ({})", songFileNameAsAtring)));
	}

	auto songInfo = MusicDownloadManager::sharedState()->getSongInfoObject(songFileNameAsID.unwrap());

	// sometimes songInfo is nullptr, so improvise
	if (songInfo) {
		// default: "Song Name, Artist, Song ID"
		// fmt::format("{} by {} ({})", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songID);
		std::string resultString = "";
		auto formatSetting = geode::Mod::get()->getSettingValue<std::string>("songFormatNGML");
		if (formatSetting == "Song Name, Artist, Song ID") {
			resultString = fmt::format("{} by {} ({})", songInfo->m_songName, songInfo->m_artistName, songInfo->m_songID);
		} else if (formatSetting == "Song Name + Artist") {
			resultString = fmt::format("{} by {}", songInfo->m_songName, songInfo->m_artistName);
		} else if (formatSetting == "Song Name + Song ID") {
			resultString = fmt::format("{} ({})", songInfo->m_songName, songInfo->m_songID);
		} else {
			resultString = fmt::format("{}", songInfo->m_songName);
		}
		return Utils::makeNewCard(notifString.append(resultString));
	}
	return Utils::makeNewCard(notifString.append(songFileName.string()));
}

void Utils::playlistModePLAndEPL() {
	if (!Utils::getBool("playlistMode")) { return; }
	auto gjbgl = GJBaseGameLayer::get();
	if (gjbgl) { return; }
	Utils::playlistModeNewSong();
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
			blacklist.push_back(blacklistString);
			geode::log::info("adding to blacklist: {}", blacklistString);
		}
		geode::log::info("Finished storing blacklist. size: {}", blacklist.size());
	}

	if (customSongs) {
		for (auto file : std::filesystem::directory_iterator(configDir)) {
			if (!std::filesystem::exists(file)) continue;

			const auto& filePath = file.path();
			auto filePathString = filePath.string();

			bool isInTextBlacklist = false;

			for (std::string string : blacklist) {
				if (string.starts_with(filePathString)) {
					isInTextBlacklist = true;
					break;
				}
			}

			if (!Utils::isSupportedExtension(filePathString) || std::ranges::find(otherBlacklist, filePathString) != otherBlacklist.end() || isInTextBlacklist) continue;

			geode::log::debug("Adding custom song: {}", filePath.filename().string());
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
		for (auto song : songs) {
			if (!song) continue;

			std::string songPath = downloadManager->pathForSong(song->m_songID);

			bool isInTextBlacklist = false;

			for (std::string string : blacklist) {
				if (string.starts_with(songPath)) {
					isInTextBlacklist = true;
					break;
				}
			}

			if (!Utils::isSupportedExtension(songPath) || std::ranges::find(otherBlacklist, songPath) != otherBlacklist.end() || isInTextBlacklist) continue;

			geode::log::debug("Adding Newgrounds/Music Library song: {}", songPath);
			SongManager::get().addSong(songPath);
		}
		// same thing as NG but for music library as well --ninXout
		// SPOILER: IT DOESN'T WORK CROSSPLATFORM (android specifically)! --raydeeux
		/*
		std::filesystem::path musicLibrarySongs = dirs::getGeodeDir().parent_path() / "Resources" / "songs";
		if (!std::filesystem::exists(musicLibrarySongs)) return;
		for (const std::filesystem::path& dirEntry : std::filesystem::recursive_directory_iterator(musicLibrarySongs)) {
			if (!std::filesystem::exists(dirEntry)) continue;

			std::string songPath = dirEntry.string();

			if (!Utils::isSupportedExtension(songPath)) continue;

			log::debug("Adding Music Library song: {}", songPath);
			songManager.addSong(songPath);
		}
		*/
	}
}

SongInfoObject* Utils::getSongInfoObject() {
	if (Utils::getBool("useCustomSongs")) return nullptr;
	if (SongManager::get().isOriginalMenuLoop()) return nullptr;

	auto songFileName = std::filesystem::path(SongManager::get().getCurrentSong()).filename();

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
	return songInfo->m_songName;
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
	return std::filesystem::path(SongManager::get().getCurrentSong()).filename().string();
}