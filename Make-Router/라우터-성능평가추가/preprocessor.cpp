/*
* input:
* 논리 큐빗 수(num_lq)
* 물리 큐빗 수(num_pq)
* cx gate의 큐빗 쌍들(ex. qc = { {q0,q1}, {q0,q2}, ... })
* 물리 노드의 간선(ex.edge = { {0,1}, {1,0}, {1,2}, {2,1}, ... }
* =================================
* data_preprocessor.cpp
* data_preprocessor.cpp
* data_preprocessor.cpp
* 해당 인풋을 그대로 매핑과 라우팅에 쓸 수 없다.
* 매핑과 라우팅에는 인접 노드 리스트(adj)와 거리 행렬(dist),
* 게이트 리스트(DAG 등)가 필요하다.
* 또한 중심도라던가 중요도는 미리 계산해 둘 수 있는 것들이므로 이것 또한 미리 계산해서 배열에 저장해 두었다.
* 이러한 전처리 작업을 인풋이 들어왔을 때 QCompiler 객체를 생성하며 즉시 한다.
*/

#include <vector>
#include <queue>
#include "QCompiler.h"

QCompiler::QCompiler(int num_lq, int num_pq, const std::vector<std::pair<int, int>>& qc, const std::vector<std::pair<int, int>>& edge)
    : num_pq(num_pq), adj(setAdj(edge)), dist(setDist()), num_lq(num_lq), qc(qc), Dlist(setDlist(qc)) {
    T = 10;
    param_alpha = 0.2;
    setGrid();
    // for mapping
    l2p.assign(num_lq, -1);
    p2l.assign(num_pq, -1);
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



std::vector<std::vector<int>> QCompiler::setAdj(const std::vector<std::pair<int, int>>& edge) {
    std::vector<std::vector<int>> v(num_pq, std::vector<int>());
    for (std::pair<int, int> e : edge) {
        v[e.first].push_back(e.second);
        v[e.second].push_back(e.first); // 양방향
    }
    return v;
}

std::vector<std::vector<int>> QCompiler::setDist() {
    std::vector<std::vector<int>> x(num_pq, std::vector<int>(num_pq, -1));
    // 거리를 구하는 BFS 시작...
    for (int i = 0; i < num_pq; i++) {
        std::queue<int> q;
        // 시작
        x[i][i] = 0;
        q.push(i);
        // 계속
        while (!q.empty()) {
            int v = q.front();
            q.pop();

            for (const int w : adj[v]) {
                if (x[i][w] != -1) continue;
                x[i][w] = x[i][v] + 1;
                q.push(w);
            }
        }
    }
    return x;
}

std::vector<std::vector<int>> QCompiler::setDlist(const std::vector<std::pair<int, int>>& qc) {
    std::vector<std::vector<int>> v(num_lq, std::vector<int>());
    for (int id = 0; id < qc.size(); id++) {
        std::pair<int, int> gate = qc[id];
        if (gate.first == -1) continue;
        v[gate.first].push_back(id);
        v[gate.second].push_back(id);
    }
    return v;
}

void QCompiler::setGrid() {
    top.assign(num_lq, 0);
    grid.assign(num_lq, std::vector<int>());
    bool progress;
    do {
        progress = false;
        for (int q_self = 0; q_self < num_lq; q_self++) {
            // Has the Dlist scan been completed?
            if (top[q_self] >= Dlist[q_self].size()) continue;
            int gate_id = Dlist[q_self][top[q_self]];
            int q_control = qc[gate_id].first;
            int q_target = qc[gate_id].second;
            if (top[q_control] >= Dlist[q_control].size() || top[q_target] >= Dlist[q_target].size()) continue;
            // Conditions for not being able to "push"
            if (Dlist[q_control][top[q_control]] != gate_id || Dlist[q_target][top[q_target]] != gate_id) continue;
            if (grid[q_control].size() >= T || grid[q_target].size() >= T) continue;
            // push bubble
            if (grid[q_control].size() < grid[q_target].size()) grid[q_control].resize(grid[q_target].size(), -1);
            if (grid[q_control].size() > grid[q_target].size()) grid[q_target].resize(grid[q_control].size(), -1);
            // push gate
            grid[q_control].push_back(gate_id);
            grid[q_target].push_back(gate_id);
            top[q_control]++;
            top[q_target]++;
            progress = true;
        }
    } while (progress);
    for (int q = 0; q < num_lq; q++) {
        grid[q].resize(T, -1);
    }
}
