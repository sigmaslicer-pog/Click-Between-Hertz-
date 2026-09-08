#include <Geode/Geode.hpp>
#include "SIPlayerObject.hpp"
#include "SubtickInputs.hpp"

using namespace geode::prelude;
using namespace subtickinputs;

namespace subtickinputs {
    bool useVanilla() {
        auto* playLayer = PlayLayer::get();
        return !playLayer || !config::modEnabled || playLayer->m_playerDied || playLayer->m_isPlatformer;
    }

    void toggleVelocityUnroundingPatches(bool enable) {
        config::velocityUnroundingEnabled = enable;
    }
}

void SIPlayerObject::update(float dt) {
    auto* playLayer = PlayLayer::get();
    if (!playLayer || useVanilla()) {
        PlayerObject::update(dt);
        return;
    }

    auto& fields = *m_fields;

    if (fields.m_yVelAdjustment != 0.0) {
        this->m_yVelocity += fields.m_yVelAdjustment;
        fields.m_yVelAdjustment = 0.0;
    }

    fields.m_preTickPosition = this->getPosition();

    if (this->m_isDart && !fields.m_pendingWaveInputs.empty()) {
        for (const auto& waveInput : fields.m_pendingWaveInputs) {
            float subDt = dt * static_cast<float>(waveInput.m_ratio);
            if (subDt > 0.0f) {
                PlayerObject::update(subDt);
            }
            playLayer->handleButton(waveInput.m_isPush, waveInput.m_button, !this->m_isPlayer2);
        }
        fields.m_pendingWaveInputs.clear();
        fields.m_didWaveSplit = true;
    } else {
        fields.m_didWaveSplit = false;
        PlayerObject::update(dt);
    }

    if (fields.m_yDispAdjustment != 0.0) {
        CCPoint pos = this->getPosition();
        pos.y += static_cast<float>(fields.m_yDispAdjustment);
        this->setPosition(pos);
        fields.m_yDispAdjustment = 0.0;
    }
}

void SIPlayerObject::updateRotation(float dt) {
    auto& fields = *m_fields;
    if (fields.m_rotationDelta != 0.0f) {
        PlayerObject::updateRotation(fields.m_rotationDelta);
        fields.m_rotationDelta = 0.0f;
    } else {
        PlayerObject::updateRotation(dt);
    }
}

void SIPlayerObject::setYVelocity(double velocity, int type) {
    PlayerObject::setYVelocity(velocity, type);
}

void SIPlayerObject::processPlayerTick(float dt) {
    this->update(dt);
}

$on_mod(Loaded) {
    auto bindSetting = [](auto& targetVar, const std::string& key) {
        targetVar = Mod::get()->getSettingValue<std::decay_t<decltype(targetVar)>>(key);
        listenForSettingChanges(key, [&](std::decay_t<decltype(targetVar)> val) {
            targetVar = val;
        });
    };

    bindSetting(config::modEnabled, "mod-enabled");
    bindSetting(config::inputHz, "input-hz");
    bindSetting(config::instantInputsEnabled, "instant-inputs");
    bindSetting(config::velocityUnroundingEnabled, "velocity-unrounding");
    bindSetting(config::debugModeEnabled, "debug-mode");

    log::info("Superb Input Precision loaded successfully!");
}