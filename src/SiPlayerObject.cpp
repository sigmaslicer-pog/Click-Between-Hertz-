#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(SIPlayerObject, PlayerObject) {
    struct Fields {
        double m_yVelAdjustment = 0.0;
        double m_yDispAdjustment = 0.0;
        float m_rotationDelta = 0.0f;
        CCPoint m_preTickPosition = {0.f, 0.f};
        bool m_didWaveSplit = false;
        std::vector<struct { double m_ratio; int m_button; bool m_isPush; }> m_pendingWaveInputs;
    };

    void update(float dt) {
        auto* playLayer = PlayLayer::get();
        if (!playLayer || playLayer->m_playerDied || playLayer->m_isPlatformer) {
            PlayerObject::update(dt);
            return;
        }

        // Fixed: Use .self() instead of *m_fields
        auto& fields = m_fields.self();

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
                // Fixed: Use m_isSecondPlayer
                playLayer->handleButton(waveInput.m_isPush, waveInput.m_button, !this->m_isSecondPlayer);
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

    void updateRotation(float dt) {
        // Fixed: Use .self() instead of *m_fields
        auto& fields = m_fields.self();
        if (fields.m_rotationDelta != 0.0f) {
            PlayerObject::updateRotation(fields.m_rotationDelta);
            fields.m_rotationDelta = 0.0f;
        } else {
            PlayerObject::updateRotation(dt);
        }
    }
};

// Setting change listeners with explicit types to resolve deduction errors
$on_mod(Loaded) {
    auto bindSetting = [](auto& targetVar, const std::string& key) {
        targetVar = Mod::get()->getSettingValue<std::decay_t<decltype(targetVar)>>(key);
        // Fixed: Provide explicit template type to listenForSettingChanges
        Mod::get()->listenForSettingChanges<std::decay_t<decltype(targetVar)>>(key, [ &targetVar ](auto val) {
            targetVar = val;
        });
    };

    // Add your mod setting bindings here if needed
}