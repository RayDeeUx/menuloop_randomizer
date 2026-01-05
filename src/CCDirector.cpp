#include "Utils.hpp"
#include <Geode/modify/CCDirector.hpp>

using namespace geode::prelude;

class $modify(MenuLoopCCDHook, CCDirector) {
	void willSwitchToScene(cocos2d::CCScene* scene) {
		CCDirector* director = CCDirector::get();
		CCScene* previousScene = director->getRunningScene();

		director->willSwitchToScene(scene);

		if (previousScene->getChildByType<MenuLayer>(0)) Utils::fadeOutCardRemotely(Utils::findCardRemotely());
		else if (!scene->getChildByType<MenuLayer>(0)) Utils::removeCardRemotely(Utils::findCardRemotely());
	}

	bool pushScene(cocos2d::CCScene* scene) {
		CCDirector* director = CCDirector::get();
		CCScene* previousScene = director->getRunningScene();

		bool result = director->pushScene(scene);

		if (previousScene->getChildByType<MenuLayer>(0)) Utils::fadeOutCardRemotely(Utils::findCardRemotely());
		else if (!scene->getChildByType<MenuLayer>(0)) Utils::removeCardRemotely(Utils::findCardRemotely());

		return result;
	}

	bool replaceScene(cocos2d::CCScene* scene) {
		CCDirector* director = CCDirector::get();
		CCScene* previousScene = director->getRunningScene();

		bool result = director->replaceScene(scene);

		if (previousScene->getChildByType<MenuLayer>(0)) Utils::fadeOutCardRemotely(Utils::findCardRemotely());
		else if (!scene->getChildByType<MenuLayer>(0)) Utils::removeCardRemotely(Utils::findCardRemotely());

		return result;
	}

	void popSceneWithTransition(float p0, cocos2d::PopTransition p1) {
		CCDirector* director = CCDirector::get();
		CCScene* previousScene = director->getRunningScene();

		director->popSceneWithTransition(p0, p1);

		if (previousScene->getChildByType<MenuLayer>(0)) Utils::fadeOutCardRemotely(Utils::findCardRemotely());
		else Utils::removeCardRemotely(Utils::findCardRemotely());
	}
};