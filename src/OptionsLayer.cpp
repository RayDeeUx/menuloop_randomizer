#include <Geode/modify/OptionsLayer.hpp>

using namespace geode::prelude;

class $modify(MenuLoopOLHook, OptionsLayer) {
    void customSetup() {
        OptionsLayer::customSetup();

        // add the folder btn to the settings layer
        auto menu = CCMenu::create();

        auto btn = CCMenuItemSpriteExtra::create(
            CircleButtonSprite::create(
                CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png"),
                CircleBaseColor::Pink
            ),
            this,
            menu_selector(MenuLoopOLHook::onSongsBtn)
        );

        btn->setPosition({-144.0f, -60.0f});
        btn->m_scaleMultiplier = 1.1f;
        btn->setID("songs-button"_spr);

        menu->addChild(btn);
        menu->setID("songs-button-menu"_spr);

        m_mainLayer->addChild(menu);
    }

    void onSongsBtn(CCObject *sender) {
        file::openFolder(Mod::get()->getConfigDir());
    }
};