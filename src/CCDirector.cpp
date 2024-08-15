#include "Utils.hpp"
#include <Geode/modify/CCDirector.hpp>

using namespace geode::prelude;

class $modify(MenuLoopCCDHook, CCDirector) {
    void fadeOutCardRemotely() {
        if (const auto card = Utils::findCardRemotely()) {
        	card->stopAllActions();
            CCAction* remoteCardRemoval = cocos2d::CCSequence::create(
            	CCEaseOut::create(CCMoveBy::create(.25f, {0, 12}), 1.f),
				CCCallFunc::create(this, callfunc_selector(MenuLoopCCDHook::removeCardRemotely)),
				nullptr
			);
			card->runAction(remoteCardRemoval);
        }
    }
	
    void removeCardRemotely() {
		if (auto card = Utils::findCardRemotely())
			card->removeMeAndCleanup();
	}

	void willSwitchToScene(cocos2d::CCScene* scene) {
    	CCDirector* director = get();
    	CCScene* previousScene = director->getRunningScene();

    	director->willSwitchToScene(scene);

    	if (previousScene->getChildByIDRecursive("MenuLayer"))
    		MenuLoopCCDHook::fadeOutCardRemotely();
    	else if (!scene->getChildByIDRecursive("MenuLayer"))
    	    MenuLoopCCDHook::removeCardRemotely();
    }

	bool pushScene(cocos2d::CCScene* scene) {
    	CCDirector* director = get();
    	CCScene* previousScene = director->getRunningScene();

    	bool result = director->pushScene(scene);

    	if (previousScene->getChildByIDRecursive("MenuLayer"))
    		MenuLoopCCDHook::fadeOutCardRemotely();
    	else if (!scene->getChildByIDRecursive("MenuLayer"))
    	    MenuLoopCCDHook::removeCardRemotely();

    	return result;
    }

	bool replaceScene(cocos2d::CCScene* scene) {
    	CCDirector* director = get();
    	CCScene* previousScene = director->getRunningScene();

    	bool result = director->replaceScene(scene);

    	if (previousScene->getChildByIDRecursive("MenuLayer"))
    		MenuLoopCCDHook::fadeOutCardRemotely();
    	else if (!scene->getChildByIDRecursive("MenuLayer"))
    	    MenuLoopCCDHook::removeCardRemotely();

    	return result;
    }

	void popSceneWithTransition(float p0, cocos2d::PopTransition p1) {
    	CCDirector* director = get();
    	CCScene* previousScene = director->getRunningScene();

    	director->popSceneWithTransition(p0, p1);

    	if (previousScene->getChildByIDRecursive("MenuLayer"))
    		MenuLoopCCDHook::fadeOutCardRemotely();
    	else
    		MenuLoopCCDHook::removeCardRemotely();
    }
};