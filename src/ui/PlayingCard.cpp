#include "PlayingCard.hpp"
#include "../Utils.hpp"
#include <regex>

using namespace geode::prelude;

static const std::regex fileExtensionRegex = std::regex(R"(\.(?:mp3|ogg|oga|flac|wav|aiff|aif|aifc|midi|mid|m3u|m3u8|wma|wmv|asf|asx|mod|it|mp2|pls|s3m)$)");
static const std::regex unsupportedCharRegex = std::regex(R"([^[\]+,. !@#$%^&*()'|`~•/:;<>=?\-\w\\])");

PlayingCard* PlayingCard::create(const std::string& output) {
	PlayingCard *ret = new PlayingCard();
	if (ret && ret->init(output)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool PlayingCard::init(const std::string& output) {
	if (!CCNode::init()) return false;

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

	CCLabelBMFont* nowPlayingLabel = nullptr;
	if (output.empty()) nowPlayingLabel = CCLabelBMFont::create("Unknown", fontFile.c_str());
	else if (Utils::getBool("removeSuffix")) nowPlayingLabel = CCLabelBMFont::create(std::regex_replace(output, fileExtensionRegex, "").c_str(), fontFile.c_str());
	else nowPlayingLabel = CCLabelBMFont::create(output.c_str(), fontFile.c_str());

	if (Utils::getBool("replaceUnsupportedChars")) nowPlayingLabel->setString(std::regex_replace(output, unsupportedCharRegex, "?").c_str());

	std::string temp = nowPlayingLabel->getString();
	Utils::sanitizeASCII(temp);
	nowPlayingLabel->setString(temp.c_str());

	CCNode* mainNode = CCNode::create();

	this->setID("now-playing-card"_spr);

	CCScale9Spritegeode::NineSlice* bg = geode::NineSlice::create("black-square.png"_spr);
	bg->setContentSize(cardSize);
	bg->setAnchorPoint({0.5f, 0.0f});

	nowPlayingLabel->limitLabelWidth(cardSize.x - 4.0f, 0.5f, 0.1f);
	nowPlayingLabel->setPositionY(12.0f);

	mainNode->addChild(bg);
	mainNode->addChild(nowPlayingLabel);

	this->addChild(mainNode);

	return true;
}