#pragma once

#include <Geode/loader/SettingNode.hpp>

using namespace geode::prelude;

class MySettingValue : public

SettingValue

{
protected:
	std::string m_placeholder;
public:
	MySettingValue(std::string const& key, std::string const& modID, std::string const& placeholder)
	  : SettingValue(key, modID), m_placeholder(placeholder) {}
	bool load(matjson::Value const& json) override { return true; }
	bool save(matjson::Value& json) const override { return true; }
	SettingNode* createNode(float width) override;
};

class MySettingNode : public SettingNode {
private:
	std::string m_desc = "";
protected:
	void onConfigDirButton(CCObject*) {
		file::openFolder(Mod::get()->getConfigDir());
		#ifndef GEODE_IS_MOBILE
		if (!CCKeyboardDispatcher::get()->getShiftKeyPressed()) return;
		FLAlertLayer::create(
			"Config Directory Opened!",
			m_desc,
			"Aight"
		)->show();
		#endif
	}
	bool init(MySettingValue* value, float width) {
		if (!SettingNode::init(value))
			return false;
		this->setContentSize({ width, 40.f });
		std::string name = Mod::get()->getSettingDefinition(value->getKey())->get<CustomSetting>()->json->get<std::string>("name");
		m_desc = Mod::get()->getSettingDefinition(value->getKey())->get<CustomSetting>()->json->get<std::string>("desc");

		auto theMenu = CCMenu::create();
		auto theLabel = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");

		// copied a bit from viper
		theLabel->setScale(.35f);
		theLabel->limitLabelWidth(300.f, .35f, .25f); // added by Ery. max width is 346.f

		auto theLabelAsAButton = CCMenuItemSpriteExtra::create(theLabel, this, menu_selector(MySettingNode::onConfigDirButton));

		theLabelAsAButton->setPositionX(0);

		theMenu->addChild(theLabelAsAButton);
		theMenu->setPosition(width / 2, 20.f);

		this->addChild(theMenu);

		return true;
	}

public:
	void commit() override {
		this->dispatchCommitted();
	}
	bool hasUncommittedChanges() override {
		return false;
	}
	bool hasNonDefaultValue() override {
		return true;
	}
	void resetToDefault() override {}

	static MySettingNode* create(MySettingValue* value, float width) {
		auto ret = new MySettingNode();
		if (ret && ret->init(value, width)) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
};