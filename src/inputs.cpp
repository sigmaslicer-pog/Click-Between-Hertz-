#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "InputQueue.hpp"

using namespace geode::prelude;

class $modify(SIInputLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        // Push the captured click into your sub-tick queue
        InputQueue::get().push(down, button, isPlayer1);
        
        // Pass it along to the original game function
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};