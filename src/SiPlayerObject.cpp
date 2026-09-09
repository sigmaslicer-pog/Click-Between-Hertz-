#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "../CBHQueue.hpp"

using namespace geode::prelude;

class $modify(SIPlayerObject, PlayerObject) {
    struct Fields {
        double m_yVelAdjustment = 0.0;
        double m_yDispAdjustment = 0.0;
        CCPoint m_preTickPosition;
        bool m_didWaveSplit = false;
    };

    void update(float dt) {
        auto fields = m_fields.self();

        if (fields->m_yVelAdjustment != 0.0) {
            this->m_yVelocity += fields->m_yVelAdjustment;
            fields->m_yVelAdjustment = 0.0;
        }

        fields->m_preTickPosition = this->getPosition();

        auto& inputs = CBHQueue::get().m_list;
        if (this->m_isDart && !inputs.empty()) {
            for (const auto& input : inputs) {
                if (input.player1 == (this == PlayLayer::get()->m_player1)) {
                    this->pushButton(static_cast<PlayerButton>(input.button));
                }
            }
            CBHQueue::get().clear();
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

        PlayerObject::update(dt);
    }
};