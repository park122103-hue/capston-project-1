#pragma once
#include <vector>


struct G {
    int other;
    int id;
};

class QCompiler {
    const int num_lq;
    const int num_pq;
    const int T;
    // pysical circuit - distance(dist)/adjacent nodes(adj)
    std::vector<std::vector<int>> dist;
    std::vector<std::vector<int>> adj;
    // logic circuit - absolute(Glist-top Struct)/relative depth(bubble grid)
    std::vector<std::vector<G>> Glist;
    std::vector<int> top;
    std::vector<std::vector<int>> grid;
    // mapping - pi(l2p)/inverse pi(p2l)
    std::vector<int> l2p;
    std::vector<int> p2l;
public:
    QCompiler ();

    // routing function
    double getCostOH(int Q_self, int Q_cand, double param_alpha = 0.8);

    // relative depth update
    void pull(int q_self, int now);
    void fatch(int q_self);

    // single routing
    void doSWAP(std::vector<std::pair<int, int>>& routed_gate, int& swap_overhead);
    bool doGate(std::vector<std::pair<int, int>>& routed_gate);
    std::vector<std::pair<int, int>> routing();
};
