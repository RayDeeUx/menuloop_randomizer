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
	return path.ends_with(".mp3") || path.ends_with(".wav") || path.ends_with(".ogg") || path.ends_with(".oga") || path.ends_with(".flac");
}

bool Utils::getBool(std::string setting) {
	return geode::Mod::get()->getSettingValue<bool>(setting);
}

#include "SongManager.hpp"

void Utils::setNewSong() {
	FMODAudioEngine::sharedEngine()->m_backgroundMusicChannel->stop();
	SongManager::get().pickRandomSong();
	GameManager::sharedState()->playMenuMusic();
}