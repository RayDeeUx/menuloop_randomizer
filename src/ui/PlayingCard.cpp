#include "PlayingCard.hpp"
#include "../Utils.hpp"
#include <regex>

using namespace geode::prelude;

const static std::regex fileExtensionRegex = std::regex(R"(\.(?:mp3|ogg|oga|flac|wav)$)");
const static std::regex quotationMarkRegex = std::regex(R"(")");

PlayingCard *PlayingCard::create(std::string output) {
	PlayingCard *ret = new PlayingCard();
	if (ret && ret->init(output)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool PlayingCard::init(std::string output) {
	if (!CCNode::init())
		return false;

	if (output.empty())
		output = "Unknown";
	else if (Utils::getBool("removeSuffix"))
		output = std::regex_replace(output, fileExtensionRegex, "");

	output = std::regex_replace(output, quotationMarkRegex, "''");

	auto mainNode = CCNode::create();

	this->setID("now-playing-card"_spr);

	auto bg = CCScale9Sprite::create("black-square.png"_spr);
	bg->setContentSize(cardSize);
	bg->setAnchorPoint({0.5f, 0.0f});

	std::string fontFile = "gjFont17.fnt";
	int font = Mod::get()->getSettingValue<int64_t>("notificationFont");
	if (font == 0) {
		fontFile = "bigFont.fnt";
	} else if (font == -1) {
		fontFile = "goldFont.fnt";
	} else if (font == -2) {
		fontFile = "chatFont.fnt";
	} else if (font != 17) {
		fontFile = fmt::format("gjFont{:02d}.fnt", font);
	}

	auto nowPlayingLabel = CCLabelBMFont::create(output.c_str(), fontFile.c_str());
	nowPlayingLabel->limitLabelWidth(cardSize.x - 4.0f, 0.5f, 0.1f);
	nowPlayingLabel->setPositionY(12.0f);

	mainNode->addChild(bg);
	mainNode->addChild(nowPlayingLabel);

	this->addChild(mainNode);

	return true;
}