#include <iostream>
#include <vector>
#include "QCompiler.h"
using namespace std;

int main() {
    // 1. 테스트 회로 게이트
    vector<pair<int, int>> qc = {
        {0, 1}, {0, 2}, {1, 2}, {2, 3}, {0, 1}
    };
    // 2. 5큐비트 T자형 칩 커플링 간선
    vector<pair<int, int>> edge = {
        {0, 1}, {1, 2}, {1, 3}, {3, 4}
    };

    // 3. 컴파일러 객체 생성 (4 논리 큐비트, 5 물리 노드)
    QCompiler qc1(4, 5, qc, edge);

    // 4. 라우팅 실행
    qc1.mapping();
    vector<pair<int, int>> v = qc1.routing();
    cout << "Total routed operations: " << v.size() << endl;

    for (pair<int, int> g : v) {
        for (int q = 0; q < 5; q++) {
            if (g.first == q || g.second == q) cout << "C ";
            else cout << "| ";
        }
        cout << endl;
    }
    return 0;
}
