#include "Utils.hpp"
#include <Geode/modify/CCDirector.hpp>

using namespace geode::prelude;

class $modify(MenuLoopCCDHook, CCDirector) {
	void willSwitchToScene(cocos2d::CCScene* scene) {
		CCDirector* director = get();
		CCScene* previousScene = director->getRunningScene();

		director->willSwitchToScene(scene);

		if (previousScene->getChildByIDRecursive("MenuLayer"))
			Utils::removeCurrentNotif();
	}

	bool pushScene(cocos2d::CCScene* scene) {
		CCDirector* director = get();
		CCScene* previousScene = director->getRunningScene();

		bool result = director->pushScene(scene);

		if (previousScene->getChildByIDRecursive("MenuLayer"))
			Utils::removeCurrentNotif();

		return result;
	}

	void popSceneWithTransition(float p0, cocos2d::PopTransition p1) {
		CCDirector* director = get();
		CCScene* previousScene = director->getRunningScene();

		director->popSceneWithTransition(p0, p1);

		if (previousScene->getChildByIDRecursive("MenuLayer"))
			Utils::removeCurrentNotif();
	}
};