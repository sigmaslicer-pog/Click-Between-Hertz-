#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

struct WaveInput {
    double m_ratio;
    int m_button;
    bool m_isPush;
};

class $modify(SIPlayerObject, PlayerObject) {
    struct Fields {
        double m_yVelAdjustment = 0.0;
        double m_yDispAdjustment = 0.0;
        CCPoint m_preTickPosition;
        std::vector<WaveInput> m_pendingWaveInputs;
        bool m_didWaveSplit = false;
    };

    void player_0(float dt) {
        auto fields = m_fields.self();

        if (fields->m_yVelAdjustment != 0.0) {
            this->m_yVelocity += fields->m_yVelAdjustment;
            fields->m_yVelAdjustment = 0.0;
        }

        fields->m_preTickPosition = this->getPosition();

        // Handle sub-tick wave / click inputs if any are queued
        if (this->m_isDart && !fields->m_pendingWaveInputs.empty()) {
            for (const auto& waveInput : fields->m_pendingWaveInputs) {
                // Apply sub-tick button push/release state based on the ratio
                this->pushButton(waveInput.m_button);
                // Add your custom delta time or sub-tick progression here
            }
            fields->m_pendingWaveInputs.clear();
            fields->m_didWaveSplit = true;
        } else {
            fields->m_didWaveSplit = false;
        }

        if (fields->m_yDispAdjustment != 0.0) {
            CCPoint pos = this->getPosition();
            pos.y += static_cast<float>(fields->m_yDispAdjustment);
            this->setPosition(pos);
            fields->m_yDispAdjustment = 0.0;
        }

        PlayerObject::player_0(dt);
    }
};