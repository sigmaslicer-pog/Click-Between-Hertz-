#pragma once
#include <vector>

struct CBHInput {
    bool down;
    int button;
    bool player1;
};

class CBHQueue {
public:
    static CBHQueue& get() {
        static CBHQueue instance;
        return instance;
    }

    std::vector<CBHInput> m_list;

    void push(bool down, int button, bool player1) {
        m_list.push_back({down, button, player1});
    }

    void clear() {
        m_list.clear();
    }
};