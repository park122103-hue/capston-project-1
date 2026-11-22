#pragma once
#include <vector>

class QCompiler {
public:
    // 불변량
    const int num_pq;
    const std::vector<std::vector<int>> adj;
    const std::vector<std::vector<int>> dist;
    const int num_lq;
    const std::vector<std::pair<int, int>>& qc;
    const std::vector<std::vector<int>> Dlist;
    // 생성자, 함수들.
    QCompiler(int num_lq, int num_pq, const std::vector<std::pair<int, int>>& qc, const std::vector<std::pair<int, int>>& edge);
    void mapping(std::vector<int>& layout);
    void routing(std::vector<std::pair<int, int>>& routing_list, std::vector<std::pair<int, int>>& gate_dist_at_swapstarted, std::vector<std::pair<double, int>>& gate_swapcount_at_swapended);
private:
    // 논리 큐비트
    int T;
    std::vector<int> top;
    std::vector<std::vector<int>> grid;
    // 매핑
    std::vector<int> l2p;
    std::vector<int> p2l;
    // 일회용...
    std::vector<double> importance;  // 논리 큐비트의 중요도(~가중치)
    std::vector<int> centrality;  // 물리 큐비트의 중심도(~거리)
    // @SET 계열
    std::vector<std::vector<int>> setAdj(const std::vector<std::pair<int, int>>& edge);
    std::vector<std::vector<int>> setDist();
    std::vector<std::vector<int>> setDlist(const std::vector<std::pair<int, int>>& qc);
    void setGrid();
    // Damping ratio
    double param_alpha;
    // @MAPPING 계열
    double getAffinity(int q_cand) const;  // 그리드와 감쇠비를 받아 각 큐비트의 중요도를 구한다.
    double getCost(int q_cand, int Q_cand) const;  // 그리드와 감쇠비, dist를 받아 비용을 구한다.
    // 매핑이므로 각각 아직 배치되지 않은 모든 큐비트에 대해서 중요도를 따져본다. 아직 배치되지 않은 모든 노드에 대해서 비용을 따져본다.
    int getBestQ() const;  // best_q = max(unmapped, key=lambda i:(getAffinity(i), importance[i], -i))
    int getBestP(int best_q);  // best_Q = min(unmapped, key=lambda i:(getCost(i), centrality[i], i))
    int getFirstQ() const;  // first_q = max(unmapped, key=lambda i:(importance[i], -i))
    int getFirstP() const;  // first_Q = min(unmapped, key=lambda i:(centrality[i], i))
    // getBestQ 함수를 호출하여 뽑은 논리 큐비트를 getBestP 함수를 호출하여 선택된 물리 노드에 배치한다.
    void doPlace(int best_q, int best_Q);
    // @ROUTING 계열
    double getCostOH(int Q_self, int Q_cand);
    // routing시 호출되는 함수. getCostOH를 호출하여 비용 오버헤드가 작은 쪽으로 스왑한다. 즉시 실행할 수 있는 게이트가 있으면 즉시 실행한다.
    // 라우팅이므로 스왑 대상은 인접 노드에만 한정된다. (adj)
    void doSWAP(std::vector<std::pair<int, int>>& routed_gate, std::vector<std::pair<int, int>>& gate_dist_at_swapstarted, std::vector<std::pair<double, int>>& gate_swapcount_at_swapended);
    bool doGate(std::vector<std::pair<int, int>>& routed_gate, std::vector<std::pair<int, int>>& gate_dist_at_swapstarted, std::vector<std::pair<double, int>>& gate_swapcount_at_swapended);
    // doGate가 되면 pull을 호출하고, pull이 fetch를 호출하는 구조. 그리드를 지속해서 갱신해주는 역할이다.
    void pull(int q_self, int now);
    void fetch(int q_self);
};
