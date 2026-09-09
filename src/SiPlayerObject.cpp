#include <Geode/Geode.hpp>
#include "SIPlayerObject.hpp"
#include "SubtickInputs.hpp"

using namespace geode::prelude;

// Implement SIPlayerObject methods that were causing linker errors
void SIPlayerObject::setYVelocity(double velocity, int unk) {
    // Call the base PlayerObject method or custom logic
    PlayerObject::setYVelocity(static_cast<float>(velocity));
}

void SIPlayerObject::updateRotation(float dt) {
    PlayerObject::updateRotation(dt);
}

// Implement useVanilla configuration check
namespace subtickinputs {
    bool useVanilla() {
        // Return your mod's configuration setting for whether vanilla behavior is forced
        // (Adjust this based on how your config struct is structured)
        return config::useVanillaEnabled; 
    }
}