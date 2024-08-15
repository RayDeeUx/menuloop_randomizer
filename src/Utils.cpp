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

int Utils::stoi(std::string text) {
	// create a new string containing only digits
	std::string digits;
	std::copy_if(text.begin(), text.end(), std::back_inserter(digits), ::isdigit);

	// convert the digits string to an integer (or 0 if empty)
	return digits.empty() ? 0 : std::stoi(digits);
}

bool Utils::isSupportedExtension(std::string path) {
	return !path.empty() && (path.ends_with(".mp3") || path.ends_with(".wav") || path.ends_with(".ogg") || path.ends_with(".oga") || path.ends_with(".flac"));
}

bool Utils::getBool(std::string setting) {
	return geode::Mod::get()->getSettingValue<bool>(setting);
}

// TODO FIX

void Utils::removeCurrentNotif() {
	if (auto card = cocos2d::CCDirector::get()->getRunningScene()->getChildByIDRecursive("now-playing"_spr)) {
		cocos2d::CCAction* fadeOutSequence = cocos2d::CCSequence::create(
			cocos2d::CCFadeOut::create(geode::Mod::get()->getSettingValue<double>("notificationTime") / 4),
			cocos2d::CCDelayTime::create(.125f),
			cocos2d::CCCallFunc::create(callfunc_selector(Utils::removeCard)),
			nullptr
		);
		card->runAction(fadeOutSequence);
	}
}

void Utils::removeCard(cocos2d::CCObject* sender) {
	if (auto card = cocos2d::CCDirector::get()->getRunningScene()->getChildByIDRecursive("now-playing"_spr))
		card->removeMeAndCleanup();
}

#include "SongManager.hpp"

void Utils::setNewSong() {
	FMODAudioEngine::sharedEngine()->m_backgroundMusicChannel->stop();
	SongManager::get().pickRandomSong();
	GameManager::sharedState()->playMenuMusic();
}