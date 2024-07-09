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