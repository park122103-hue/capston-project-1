#include <vector>
#include "Quantum_circuit.h"

QCompiler::QCompiler(int num_lq, int num_pq, const std::vector<std::pair<int, int>>& gates)
 : num_lq(num_lq), num_pq(num_pq) {
    // 
    T = 10;
    param_alpha = 0.8;
    setDist();
    setAdj();
    setGlist(gates);
    setGrid();
    // for mapping
    for (int Q_cand = 0; Q_cand < num_pq; Q_cand++) {  // about physical
        double sum = 0.0;
        for (int Q_other = 0; Q_other < num_pq; Q_other++) {
            sum += dist[Q_cand][Q_other];
        }
        centrality.push_back(sum);
    }
    for (int q_cand = 0; q_cand < num_lq; q_cand++) {  // about logical
        double power = 1.0;
        double sum = 0.0;
        for (int t = 0; t < T; t++) {
            if (grid[q_cand][t] == -1) {
                power *= param_alpha;
                continue;
            }
            int q_other = grid[q_cand][t];
            sum += power;
            power *= param_alpha;
        }
        importance.push_back(sum);
    }
}

void QCompiler::setDist() {

}

void QCompiler::setAdj() {

}

void QCompiler::setGlist(const std::vector<std::pair<int, int>>& gates) {
    top.assign(num_lq, 0);
    Glist.assign(num_lq, std::vector<G>());
    for (int id = 0; id < gates.size(); id++) {
        std::pair<int, int> g = gates[id];
        if (g.first == -1) continue;
        Glist[g.first].push_back({ g.second, id });
        Glist[g.second].push_back({ g.first, id });
    }
}

void QCompiler::setGrid() {
    grid.assign(num_lq, std::vector<int>());
    bool progress;
    do {
        progress = false;
        for (int q_self = 0; q_self < num_lq; q_self++) {
            // Has the Glist scan been completed?
            if (top[q_self] >= Glist[q_self].size()) continue;
            int q_other = Glist[q_self][top[q_self]].other;
            if (top[q_other] >= Glist[q_other].size()) continue;
            // Conditions for not being able to "push"
            if (Glist[q_self][top[q_self]].id != Glist[q_other][top[q_other]].id) continue;
            if (grid[q_self].size() >= T || grid[q_other].size() >= T) continue;
            // push bubble
            if (grid[q_self].size() < grid[q_other].size()) grid[q_self].resize(grid[q_other].size(), -1);
            while (grid[q_other].size() < grid[q_self].size()) grid[q_other].push_back(-1);
            // push gate
            grid[q_self].push_back(q_other);
            grid[q_other].push_back(q_self);
            top[q_self]++;
            top[q_other]++;
            progress = true;
        }
    } while (progress);
    for (int q = 0; q < num_lq; q++) {
        grid[q].resize(T, -1);
    }
}