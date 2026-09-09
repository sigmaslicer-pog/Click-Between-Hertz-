#pragma once
#include <vector>

struct QueuedInput {
    bool m_down;
    int m_button;
    bool m_isPlayer1;
    double m_ratio;
};

class InputQueue {
private:
    std::vector<QueuedInput> m_inputs;

public:
    static InputQueue& get() {
        static InputQueue instance;
        return instance;
    }

    void push(bool down, int button, bool isPlayer1, double ratio = 1.0) {
        m_inputs.push_back({down, button, isPlayer1, ratio});
    }

    std::vector<QueuedInput>& getInputs() {
        return m_inputs;
    }

    void clear() {
        m_inputs.clear();
    }
};