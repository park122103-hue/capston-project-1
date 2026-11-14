#pragma once
#include <vector>


struct G {
    int other;
    int id;
};

// l2p, p2l, Glist, dist, adj ...
class Router {
    std::vector<std::vector<G>> Glist = {
        { { 1, 0 }, { 2, 1 }, { 1, 2 } },
        { { 0, 0 }, { 0, 2 }, { 2, 4 } },
        { { 0, 1 }, { 3, 3 }, { 1, 4 } },
        { { 2, 3 } }
    };
    std::vector<int> top = { 3, 3, 3, 1 };

    std::vector<std::vector<int>> adj = {
        { 1 },
        { 0, 2, 3 },
        { 1 },
        { 1, 4 },
        { 3 }
    };
    std::vector<std::vector<int>> dist = {
        { 0, 1, 2, 2, 3 },
        { 1, 0, 1, 1, 2 },
        { 2, 1, 0, 2, 3 },
        { 2, 1, 2, 0, 1 },
        { 3, 2, 3, 1, 0 }
    };

    std::vector<int> l2p = { 0, 1, 2, 3 };
    std::vector<int> p2l = { 0, 1, 2, 3, -1 };

    const int num_lq = 4;
    const int T = 10;
    std::vector<std::vector<int>> grid = {
        { 1, 2, 1, -1, -1, -1, -1, -1, -1, -1 },
        { 0, -1, 0, 2, -1, -1, -1, -1, -1, -1 },
        { -1, 0, 3, 1, -1, -1, -1, -1, -1, -1 },
        { -1, -1, 2, -1, -1, -1, -1, -1, -1, -1 }
    };
public:
    Router ();
    void pull(int q_self, int now);
    void fatch(int q_self);
    double getCostOH(int Q_self, int Q_cand, double param_alpha = 0.8);
    std::vector<std::pair<int, int>> routing();
    void doSWAP(std::vector<std::pair<int, int>>& routed_gate, int& swap_overhead);
    bool doGate(std::vector<std::pair<int, int>>& routed_gate);
};
