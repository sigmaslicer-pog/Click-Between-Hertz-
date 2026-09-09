#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>

#include "CBHQueue.hpp"
#include "SIPlayerObject.hpp"
#include "SubtickInputs.hpp"
#include "timestamp.hpp"
#include "includes.hpp"

using namespace geode::prelude;
using namespace subtickinputs;

// Define this at the top so the compiler knows it before functions use it
struct PlayerButtonCommandWithRatio {
    PlayerButtonCommand input;
    float ratio;
};

static bool s_isLastTickOfFrame = true;
static bool s_updateJumpCalledP1 = false;
static bool s_updateJumpCalledP2 = false;

static void processInputs(float dt);

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        log::debug("Hello from my MenuLayer::init hook! This layer has {} children.", this->getChildrenCount());

        auto myButton = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),
            this,
            menu_selector(MyMenuLayer::onMyButton)
        );

        auto menu = this->getChildByID("bottom-menu");
        if (menu) {
            menu->addChild(myButton);
            myButton->setID("my-button"_spr);
            menu->updateLayout();
        }

        return true;
    }

    void onMyButton(CCObject*) {
        FLAlertLayer::create("Geode", "Hello from my custom mod!", "OK")->show();
    }
};

class $modify(GJBaseGameLayer) {
    static void onModify(auto& self) {
        (void) self.setHookPriority("GJBaseGameLayer::processQueuedButtons", Priority::Replace);
    }

    void processQueuedButtons(float dt, bool clearInputQueue) {
        if (!useVanilla()) {
            processInputs(dt);
        }

        GJBaseGameLayer::processQueuedButtons(dt, clearInputQueue);
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        s_isLastTickOfFrame = isLastTick;
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(PlayerObject) {
    void updateJump(float dt) {
        auto* playLayer = PlayLayer::get();
        if (playLayer) {
            if (this == playLayer->m_player1)
                s_updateJumpCalledP1 = true;
            else if (this == playLayer->m_player2)
                s_updateJumpCalledP2 = true;
        }
        PlayerObject::updateJump(dt);
    }
};

static double vanillaRoundValue(double value) {
    if (config::velocityUnroundingEnabled) {
        return value;
    } else {
        int integerPart = static_cast<int>(value);
        return std::round((value - integerPart) * 1000.0) / 1000.0 + integerPart;
    }
}

static float getRawDeltaVel(PlayerObject* player, float scaledDt) {
    if (player->m_isDart) return 0.0f;

    if (player->m_isRobot && player->m_maybeIsBoosted && player->m_jumpBuffered &&
        !player->m_touchedPad && player->m_accelerationOrSpeed < 1.5f) {
        return 0.0f;
    }

    bool jumpBuffered = player->m_jumpBuffered;
    bool fallingBugged = player->playerIsFallingBugged();
    bool isMini = player->m_vehicleSize != 1.0f;

    float rawBaseGravity =
        player->isInBasicMode() ? static_cast<float>(player->m_gravity) : 0.958199f;
    float baseGravity = rawBaseGravity * player->m_gravityMod;
    float sizeFactor = isMini ? (player->isFlying() ? 0.85f : 0.8f) : 1.0f;

    float gravPerTick;

    if (player->m_isShip) {
        bool isFalling = player->m_yVelocity * player->flipMod() < 0.0;
        bool forceNegative = player->m_isAccelerating ? isFalling : jumpBuffered;

        float shipYVelFactor = 0.8f;
        if (forceNegative) shipYVelFactor = -1.0f;
        if (!jumpBuffered && !fallingBugged) shipYVelFactor = 1.2f;

        if (player->m_isPlatformer) baseGravity *= 0.8f;

        float effectiveBaseGrav = shipYVelFactor >= 0.0f ? baseGravity : rawBaseGravity;
        float gravCoeff = (jumpBuffered && fallingBugged) ? 0.5f : 0.4f;

        gravPerTick = scaledDt * effectiveBaseGrav * shipYVelFactor * gravCoeff / sizeFactor;
    } else if (player->m_isBird) {
        float ufoYVelFactor = fallingBugged ? 0.8f : 1.2f;
        gravPerTick = scaledDt * baseGravity * ufoYVelFactor * 0.5f / sizeFactor;
    } else if (player->m_isSwing) {
        float swingSizeFactor = isMini ? 0.6f : 0.4f;
        gravPerTick = scaledDt * baseGravity * swingSizeFactor;
    } else {
        float gravCoeff = 1.0f;
        if (player->m_isBall || player->m_isSpider)
            gravCoeff = 0.6f;
        else if (player->m_isRobot)
            gravCoeff = 0.9f;

        gravPerTick = scaledDt * baseGravity * gravCoeff;
    }

    return player->flipMod() * gravPerTick;
}

static double getDeltaVel(PlayerObject* player, float scaledDt) {
    float dV = getRawDeltaVel(player, scaledDt);
    return vanillaRoundValue(player->m_yVelocity - dV) - player->m_yVelocity;
}

static void processInputs(float dt) {
    auto* playLayer = PlayLayer::get();
    if (!playLayer || dt <= 0.0f) return;

    auto& inputQueue = playLayer->m_queuedButtons;

    double tps = 1.0 / dt;
    double inputChecksPerTick = config::inputHz / tps;
    float scaledDt = 60.0f * dt * 0.9f;

    PlayerObject* p1 = playLayer->m_player1;
    PlayerObject* p2 = playLayer->m_player2;
    if (p1 && !p1->isVanillaPlayer()) p1 = nullptr;
    if (p2 && (!playLayer->m_gameState.m_isDualMode || !p2->isVanillaPlayer())) p2 = nullptr;

    for (PlayerObject* player : {p1, p2}) {
        if (!player) continue;
        double& yVelAdjustment = GET_PLAYER_FIELD(player, m_yVelAdjustment);
        player->m_yVelocity += yVelAdjustment;
        yVelAdjustment = 0.0;
    }

    double adjustedYVel1 = 0.0, adjustedYVel2 = 0.0;
    double finalVel1 = p1 ? p1->m_yVelocity : 0.0;
    double finalVel2 = p2 ? p2->m_yVelocity : 0.0;
    double lastRatio1 = 0.0, lastRatio2 = 0.0;

    double tickDuration = dt;
    double tickStartTime = playLayer->m_timestamp;
    double tickEndTime = tickStartTime + tickDuration;

    bool processEntireQueue = s_isLastTickOfFrame;
    size_t processedCount = 0;

    for (auto& input : inputQueue) {
        if (!processEntireQueue && input.m_timestamp >= tickEndTime) break;
        processedCount++;

        PlayerObject* player = input.m_isPlayer2 ? p2 : p1;
        double currentTime = input.m_timestamp;
        double rawRatio = (currentTime - tickStartTime) / tickDuration;
        double ratio = std::clamp(rawRatio, 0.0, 1.0);

        if (!config::instantInputsEnabled) {
            if (inputChecksPerTick > 1.0) {
                ratio = std::floor(ratio * inputChecksPerTick) / inputChecksPerTick;
            } else {
                ratio = 0.0;
            }
        }

        if (player && player->m_isDart && !player->m_isDashing) {
            GET_PLAYER_FIELD(player, m_pendingWaveInputs)
                .push_back({
                    ratio,
                    input.m_isPush,
                    static_cast<int>(input.m_button),
                });
            continue;
        }

        double preVel1 = p1 ? p1->m_yVelocity : 0.0;
        double preDv1 = p1 ? getDeltaVel(p1, scaledDt) : 0.0;
        double preVel2 = p2 ? p2->m_yVelocity : 0.0;
        double preDv2 = p2 ? getDeltaVel(p2, scaledDt) : 0.0;

        if (p1) {
            finalVel1 -= getRawDeltaVel(p1, scaledDt * (ratio - lastRatio1));
            lastRatio1 = ratio;
        }
        if (p2) {
            finalVel2 -= getRawDeltaVel(p2, scaledDt * (ratio - lastRatio2));
            lastRatio2 = ratio;
        }

        s_updateJumpCalledP1 = false;
        s_updateJumpCalledP2 = false;

        playLayer->handleButton(input.m_isPush, static_cast<int>(input.m_button), !input.m_isPlayer2);

        if (p1) {
            if (!s_updateJumpCalledP1) p1->updateJump(0.0f);
            double postVel = p1->m_yVelocity;
            double postDv = getDeltaVel(p1, scaledDt);
            adjustedYVel1 += ratio * ((preVel1 + preDv1) - (postVel + postDv));
            if (postVel != preVel1) finalVel1 = postVel;
        }

        if (p2) {
            if (!s_updateJumpCalledP2) p2->updateJump(0.0f);
            double postVel = p2->m_yVelocity;
            double postDv = getDeltaVel(p2, scaledDt);
            adjustedYVel2 += ratio * ((preVel2 + preDv2) - (postVel + postDv));
            if (postVel != preVel2) finalVel2 = postVel;
        }
    }

    for (PlayerObject* player : {p1, p2}) {
        if (!player) continue;

        double adjustedYVel = (player == p1) ? adjustedYVel1 : adjustedYVel2;
        double& finalVel = (player == p1) ? finalVel1 : finalVel2;
        double lastRatio = (player == p1) ? lastRatio1 : lastRatio2;

        if (player->m_isDart) {
            double waveDt = 60.0 / tps * ((player->m_vehicleSize != 1.0f) ? 2.0 : 1.0);
            GET_PLAYER_FIELD(player, m_yDispAdjustment) = adjustedYVel * waveDt;
        } else {
            GET_PLAYER_FIELD(player, m_yDispAdjustment) = adjustedYVel * scaledDt;
        }

        finalVel -= getRawDeltaVel(player, scaledDt * (1.0 - lastRatio));
        double predictedVanillaVel = vanillaRoundValue(player->m_yVelocity - getRawDeltaVel(player, scaledDt));

        double& yVelAdjustment = GET_PLAYER_FIELD(player, m_yVelAdjustment);
        yVelAdjustment = vanillaRoundValue(finalVel) - predictedVanillaVel;
    }

    inputQueue.erase(inputQueue.begin(), inputQueue.begin() + processedCount);
}

namespace subtickinputs {
    Result<std::vector<PlayerButtonCommandWithRatio>> getInputQueueWithRatios(float dt) {
        auto* playLayer = PlayLayer::get();
        if (!playLayer) return Err("playlayer is null");

        double tps = 1.0 / dt;
        double inputChecksPerTick = config::inputHz / tps;
        double tickDuration = dt;
        double tickStartTime = playLayer->m_timestamp;

        std::vector<PlayerButtonCommandWithRatio> result;

        for (auto& input : playLayer->m_queuedButtons) {
            double currentTime = input.m_timestamp;
            double ratio = std::clamp((currentTime - tickStartTime) / tickDuration, 0.0, 1.0);

            if (!config::instantInputsEnabled) {
                if (inputChecksPerTick > 1.0) {
                    ratio = std::floor(ratio * inputChecksPerTick) / inputChecksPerTick;
                } else {
                    ratio = 0.0;
                }
            }

            result.push_back({input, ratio});
        }

        return Ok(std::move(result));
    }

    Result<void> setInputQueueWithRatios(std::vector<PlayerButtonCommandWithRatio> inputQueue, float dt) {
        auto* playLayer = PlayLayer::get();
        if (!playLayer) return Err("playlayer is null");

        playLayer->m_queuedButtons.clear();
        for (auto& inputWithRatio : inputQueue) {
            auto& inputWithoutRatio = inputWithRatio.input;
            inputWithoutRatio.m_timestamp = playLayer->m_timestamp + inputWithRatio.ratio * dt;
            playLayer->m_queuedButtons.push_back(inputWithoutRatio);
        }
        return Ok();
    }
}