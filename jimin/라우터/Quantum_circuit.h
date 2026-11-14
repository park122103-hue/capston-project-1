#pragma once
#include <vector>


struct G {
    int other;
    int id;
};

class QCompiler {
    const int num_lq;
    const int num_pq;
    int T;
    // pysical circuit - distance(dist)/adjacent nodes(adj)
    std::vector<std::vector<int>> dist;
    std::vector<std::vector<int>> adj;
    // logic circuit - absolute(Glist-top Struct)/relative depth(bubble grid)
    std::vector<int> top;
    std::vector<std::vector<G>> Glist;
    std::vector<std::vector<int>> grid;
    // mapping - pi(l2p)/inverse pi(p2l)
    std::vector<int> l2p;
    std::vector<int> p2l;
	// 사전 계산된 정적 지표 (Importance / Centrality)
	std::vector<double> importance;  // 논리 큐비트의 중요도(~가중치)
	std::vector<int> centrality;  // 물리 큐비트의 중심도(~거리)
    // For constructor
    void setDist();
    void setAdj();
    void setGlist(const std::vector<std::pair<int, int>>& gates);
    void setGrid();
    // Damping ratio
    double param_alpha = 0.8;
public:
    QCompiler(int num_lq, int num_pq, const std::vector<std::pair<int, int>>& gates);

	// 내부 탐색 메서드
	double getAffinity(int q_cand) const;  // 논리 큐비트의 친밀도(~가중치)
	double getCost(int q_cand, int Q_cand) const;  // 최적화될 목적 함수(~가중치, ~거리)
	int getBestQ() const;  // result = max(unmapped, key=lambda i:(getAffinity(i), importance[i], -i))
	int getBestP(int best_p);  // result = min(unmapped, key=lambda i:(getCost(i), centrality[i], i))
	int getFirstQ() const;  // result = max(unmapped, key=lambda i:(importance[i], -i))
	int getFirstP() const;  // result = min(unmapped, key=lambda i:(centrality[i], i))

	void doPlace(int best_q, int best_Q);
    std::vector<int> mapping();

    // routing function
    double getCostOH(int Q_self, int Q_cand);

    // relative depth update
    void pull(int q_self, int now);
    void fatch(int q_self);

    // single routing
    void doSWAP(std::vector<std::pair<int, int>>& routed_gate, int& swap_overhead);
    bool doGate(std::vector<std::pair<int, int>>& routed_gate);
    std::vector<std::pair<int, int>> routing();
};
