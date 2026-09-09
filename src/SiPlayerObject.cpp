#include <Geode/Geode.hpp>
#include "SIPlayerObject.hpp"
#include "SubtickInputs.hpp"

using namespace geode::prelude;

void SIPlayerObject::setYVelocity(double velocity, int unk) {
    PlayerObject::setYVelocity(velocity, unk);
}

void SIPlayerObject::updateRotation(float dt) {
    PlayerObject::updateRotation(dt);
}

void SIPlayerObject::update(float dt) {
    PlayerObject::update(dt);
}

namespace subtickinputs {
    bool useVanilla() {
        return false; 
    }
}