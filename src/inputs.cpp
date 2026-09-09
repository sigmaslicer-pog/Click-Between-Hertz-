#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../include/InputQueue.hpp"

using namespace geode::prelude;

class $modify(SIInputLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        InputQueue::get().push(down, button, isPlayer1);
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};
#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "InputQueue.hpp"

using namespace geode::prelude;

class $modify(SIInputLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        InputQueue::instance().add(down, button, isPlayer1);
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};