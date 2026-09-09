#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "InputQueue.hpp"

using namespace geode::prelude;

class $modify(SIPlayerObject, PlayerObject) {
    struct Fields {
        double m_yVelAdjustment = 0.0;
        double m_yDispAdjustment = 0.0;
        CCPoint m_preTickPosition;
        bool m_didWaveSplit = false;
    };

    void player_0(float dt) {
        auto fields = m_fields.self();

        if (fields->m_yVelAdjustment != 0.0) {
            this->m_yVelocity += fields->m_yVelAdjustment;
            fields->m_yVelAdjustment = 0.0;
        }

        fields->m_preTickPosition = this->getPosition();

        // Process any queued sub-tick inputs from InputQueue
        auto& queuedInputs = InputQueue::get().getInputs();
        if (this->m_isDart && !queuedInputs.empty()) {
            for (const auto& input : queuedInputs) {
                if (input.m_isPlayer1 == (this == PlayLayer::get()->m_player1)) {
                    // Handle button state for click between hertz
                    this->pushButton(input.m_button);
                }
            }
            InputQueue::get().clear();
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