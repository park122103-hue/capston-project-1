#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include "QCompiler.h"

#include <fstream>
#include <string>
#include <cstdio>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

void parser(int& num_lq, int& num_pq, vector<pair<int, int>>& qc, vector<pair<int, int>>& edge) {
    qc.clear(); edge.clear();
    num_lq = 0; num_pq = 0;

    std::ifstream f("devices/default_tokyo.json");
    if (!f) { cerr << "JSON 파일 열기 실패!\n"; return; }
    json j;
    f >> j;

    num_pq = j["count_physical_qubit"];
    for (auto& e : j["list_qubit_edge"]) {
        edge.push_back({ (int)e[0], (int)e[1] });
    }

    std::ifstream file("benchmarks/sym9_193.qasm");
    if (!file) { cerr << "QASM 파일 열기 실패!\n"; return; }
    std::string line;
    while (std::getline(file, line)) {
        int q1, q2; char end_char;
        if (std::sscanf(line.c_str(), " cx q[%d] , q[%d] %c", &q1, &q2, &end_char) == 3 && end_char == ';') {
            qc.push_back({ q1, q2 });
            num_lq = std::max(num_lq, std::max(q1, q2) + 1);
        }
    }
}

int main() {
    // 1. 테스트 회로 게이트
    // 2. 5큐비트 T자형 칩 커플링 간선
    int num_lq, num_pq;
    vector<pair<int, int>> qc, edge;

    parser(num_lq, num_pq, qc, edge);
    cout << num_lq << " " << num_pq << endl;

    // 3. 컴파일러 객체 생성 (4 논리 큐비트, 5 물리 노드)
    QCompiler qc1(num_lq, num_pq, qc, edge);

    // 4. 라우팅 실행
    vector<int> layout;
    qc1.mapping(layout);
    vector<pair<int, int>> routing_list;
    vector<pair<int, int>> gate_dist_at_swapstarted(qc.size(), { 0, -1 });  // first가 거리, second가 routing_list의 인덱스(회로 깊이)
    vector<pair<double, int>> gate_swapcount_at_swapended(qc.size(), { 0, -1 });  // first가 스왑수, second가 routing_list의 인덱스(회로 깊이)
    qc1.routing(routing_list, gate_dist_at_swapstarted, gate_swapcount_at_swapended);
    cout << "SWAP OVERHEAD: " << (routing_list.size() - qc.size())/ 3 << endl;

    std::ofstream out("result.csv");
    // 1. 첫 줄: 열 이름(헤더)
    out << "gate_id,dist_minus_1,started,swapcount,ended\n";
    // 2. 게이트별 한 줄씩 출력
    int q_self = 0;
    int q_other = 1;
    for (int i = 0; i < (int)qc.size(); i++) {
        if ((qc[i].first == q_self && qc[i].second == q_other) || (qc[i].first == q_other && qc[i].second == q_self)) {
            out << i << ","
                << (gate_dist_at_swapstarted[i].first - 1) << ","
                << gate_dist_at_swapstarted[i].second << ","
                << gate_swapcount_at_swapended[i].first << ","
                << gate_swapcount_at_swapended[i].second << "\n";
        }
    }
    out.close();

    std::ofstream out2("result_dist.csv");
    out2 << "index";
    for (int a = 0; a < num_lq; a++) {
        for (int b = a + 1; b < num_lq; b++) {
            out2 << "," << a << "/" << b;
        }
    }
    out2 << "\n";
    vector<int> tamp_l2p = layout;
    vector<int> tamp_p2l(qc1.num_pq, -1);
    for (int q = 0; q < qc1.num_lq; q++) {
        tamp_p2l[tamp_l2p[q]] = q;
    }
    int swap_flag = 0;
    for (int i = 0; i < routing_list.size(); i++) {
        int Q_control = routing_list[i].first;
        int Q_target = routing_list[i].second;
        int q_control = tamp_p2l[Q_control];
        int q_target = tamp_p2l[Q_target];
        // SWAP이라면
        if (i > 1 && swap_flag == 0) {
            if (routing_list[i - 1].first == routing_list[i].second && routing_list[i - 1].second == routing_list[i].first
                && routing_list[i - 2].first == routing_list[i].first && routing_list[i - 2].second == routing_list[i].second) {
                tamp_p2l[Q_control] = q_target;
                tamp_p2l[Q_target] = q_control;
                if (q_control != -1) tamp_l2p[q_control] = Q_target;
                if (q_target != -1) tamp_l2p[q_target] = Q_control;
                swap_flag = 2;
            }
        }
        else if (swap_flag > 0) {
            swap_flag--;
        }
        out2 << i;

        for (int a = 0; a < num_lq; a++) {
            for (int b = a + 1; b < num_lq; b++) {
                out2 << "," << qc1.dist[tamp_l2p[a]][tamp_l2p[b]];
            }
        }
        out2 << "\n";
    }
    out2.close();
    return 0;
}
