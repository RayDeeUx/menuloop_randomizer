#include "SongManager.hpp"
#include "Utils.hpp"
#include <random>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/loader/Mod.hpp>

int Utils::randomIndex(int size) {
	// select a random item from the vector and return the path
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, size - 1);
	int randomIndex = dist(gen);

	return randomIndex;
}

uint64_t Utils::stoi(std::string text) {
	// create a new string containing only digits
	std::string digits;
	std::copy_if(text.begin(), text.end(), std::back_inserter(digits), ::isdigit);

	uint64_t result = std::stoi(digits);
	if (!result) return 0;

	// convert the digits string to an integer (or 0 if empty)
	return digits.empty() ? 0 : result;
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
	FMODAudioEngine::sharedEngine()->m_backgroundMusicChannel->stop();
	SongManager::get().pickRandomSong();
	GameManager::sharedState()->playMenuMusic();
}