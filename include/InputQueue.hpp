#pragma once
#include <vector>

struct SimpleInput {
    bool down;
    int button;
    bool player1;
};

class InputQueue {
public:
    static InputQueue& instance() {
        static InputQueue inst;
        return inst;
    }

    std::vector<SimpleInput> queue;

    void add(bool down, int button, bool player1) {
        queue.push_back({down, button, player1});
    }

    void clear() {
        queue.clear();
    }
};