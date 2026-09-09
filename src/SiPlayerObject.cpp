#include <Geode/Geode.hpp>
#include "SIPlayerObject.hpp"
#include "SubtickInputs.hpp"

using namespace geode::prelude;

void SIPlayerObject::setYVelocity(double velocity, int unk) {
    // PlayerObject::setYVelocity expects (double velocity, int type)
    PlayerObject::setYVelocity(velocity, unk);
}

void SIPlayerObject::updateRotation(float dt) {
    PlayerObject::updateRotation(dt);
}

namespace subtickinputs {
    bool useVanilla() {
        // Return false to use subtick processing, or true for vanilla behavior
        return false; 
    }
}