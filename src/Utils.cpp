#include "SongManager.hpp"
#include "ui/PlayingCard.hpp"
#include "Utils.hpp"
#include <random>

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
	GameManager::sharedState()->playMenuMusic();
}

void Utils::playlistModeNewSong() {
	if (!Utils::getBool("playlistMode")) {
		return Utils::setNewSong();
	}
	geode::log::info("attempting to hijack menuloop channel to use playlist mode");
	FMODAudioEngine::sharedEngine()->m_backgroundMusicChannel->stop();
	SongManager::get().pickRandomSong();
	geode::log::info("is it over?");
	FMODAudioEngine::get()->playMusic(SongManager::get().getCurrentSong(), true, 1.0f, 1);
}

// create notif card stuff
void Utils::makeNewCard(std::string notifString) {
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