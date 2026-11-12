#include <iostream>
#include <iomanip>
#include <vector>
#include "AQubits.h"
#include "Circuit.h"
using namespace std;


class Router {
    // 출발 게이트 번호들(degree=0인거)이랑 DAG를 받아다가 싹 비교 돌려야 함.

};


// 편의 기능
void print(const vector<int>& v);
void print(const vector<vector<double>>& v);
void print(const vector<pair<int, int>>& v);
vector<vector<int>> getAdj(const vector<pair<int, int>>& edge, int num_pq);


int main() {
    //가중치와 거리 행렬의 값.v
    //vector<vector<int>> adj = {
    //{ 1 },
    //{ 0, 2 },
    //{ 1, 3 },
    //{ 2, 4 },
    //{ 3 }
    //};
    vector<vector<int>> dist = {
    { 0, 1, 2, 3, 4 },
    { 1, 0, 1, 2, 3 },
    { 2, 1, 0, 1, 2 },
    { 3, 2, 1, 0, 1 },
    { 4, 3, 2, 1, 0 }
    };
    Circuits qc(4);
    for (int i = 0; i < 7; i++) qc.cx(0, 1);
    for (int i = 0; i < 4; i++) qc.cx(0, 2);
    for (int i = 0; i < 1; i++) qc.cx(1, 2);
    for (int i = 0; i < 2; i++) qc.cx(1, 3);
    DAG qc_dag = qc.getDAG();
    vector<vector<double>> weight = qc_dag.getWeight(0.8);
    print(weight);
    Mapper qc_map(dist, weight);
    vector<int> pi = qc_map.mapping();
    print(pi);

    return 0;
}

void print(const vector<int>& v) {
    cout << "f : ";
    for (int x : v) cout << x << ' ';
    cout << endl;
}

void print(const vector<vector<double>>& v) {
    cout << "matrix: \n";
    for (const vector<double>& i : v) {
        for (double j : i) {
            cout << setw(16) << j;
        }
        cout << endl;
    }
}

void print(const vector<pair<int, int>>& v) {
    cout << "overhead: \n";
    for (const pair<int, int>& i : v) {
        cout << "SWAP " << i.first << ", " << i.second << endl;
    }
}

vector<vector<int>> getAdj(const vector<pair<int, int>>& edge, int num_pq) {
    vector<vector<int>> adj(num_pq);
    // 양방향 그래프 생성
    for (pair<int, int> e : edge) {
        adj[e.first].push_back(e.second);
    }
    return adj;
}
