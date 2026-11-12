#include <vector>
#include "Quantum_circuit.h"

Router::Router () {}

// 전부 당기기+Glist에서 당겨오기까지.
void Router::pull(int logic_qubit, int now) {
	int next = now + 1;
	if (next == T) {
        fatch_from_Glist(logic_qubit);
        return;
    }
	// 현재 큐비트 q0에 대해서 현재의 게이트는 nop(-1)이고 다음 게이트는 cx q0, q1이라면
	if (qc_Look_ahaed[logic_qubit][now] != -1) return;
	// (다음 진행할 게이트가 cx q0, q1이고) 
	int next_branch = qc_Look_ahaed[logic_qubit][next];
	if (next_branch == -1) return;
	// q1에 대해서도 현재 기준 게이트가 nop(-1)이라면, 
	if (qc_Look_ahaed[next_branch][now] != -1) return;
	// 게이트를 당긴다.
	qc_Look_ahaed[logic_qubit][now] = next_branch;
	qc_Look_ahaed[next_branch][now] = logic_qubit;
	qc_Look_ahaed[logic_qubit][next] = -1;
	qc_Look_ahaed[next_branch][next] = -1;
	pull(logic_qubit, next);
	pull(next_branch, next);
}

// q0의 다음 게이트를 Glist에서 당겨오기
void Router::fatch_from_Glist(int logic_qubit) {
    // 만약 패치할 것이 Glist에 남아있지 않으면 return;
    if (top[logic_qubit] >= Glist[logic_qubit].size()) return;
	// Glist[q0] = { { q1, 0 }, { q2, 1 }, ... }
	int branch_fatched = Glist[logic_qubit][top[logic_qubit]].branch;
    if (top[branch_fatched] >= Glist[logic_qubit].size()) return;
	if (Glist[logic_qubit][top[logic_qubit]].gate_id != Glist[branch_fatched][top[branch_fatched]].gate_id) return;
	if (qc_Look_ahaed[branch_fatched][T-1] != -1) return;
	qc_Look_ahaed[logic_qubit][T-1] = branch_fatched;
	qc_Look_ahaed[branch_fatched][T-1] = logic_qubit;
	top[logic_qubit]++;
	top[branch_fatched]++;
}

double Router::getCostOH(int logic_qubit_A, int physical_qubit_B, double alpha) {
	if (logic_qubit_A == -1) return 0.0;
    int physical_qubit_A = l2p[logic_qubit_A];
	double alpha_t = 1.0;
	double overhead = 0.0;
	// BGbuf[q0]에 해당하는 q1에 대해서 순회.
	for (int t = 0; t < T; t++) {
		int branch_A = qc_Look_ahaed[logic_qubit_A][t];
		if (branch_A != -1 && l2p[branch_A] != physical_qubit_B) {
			double w_t = alpha_t;
			overhead += w_t * (dist[physical_qubit_B][l2p[branch_A]] - dist[physical_qubit_A][l2p[branch_A]]);
		}
		alpha_t *= alpha;
	}
	return overhead;
}

std::vector<std::pair<int, int>> Router::routing() {
    // 결과내기
    std::vector<std::pair<int, int>> routed_gate;
    int swap_count = 0;
	// 라우터
	bool flag = true;
	while (flag) {
		if (!doGate(routed_gate)) doSWAP(routed_gate, swap_count);
		// 다음 실행할 게이트가 하나도 없으면 끝낸다.
		flag = false;
		for (int logic_qubit = 0; logic_qubit < num_lq; logic_qubit++) {
			flag = flag || (qc_Look_ahaed[logic_qubit][0] != -1);
		}
	}
}

void Router::doSWAP(std::vector<std::pair<int, int>>& routed_gate, int& swap_count) {
	int best_QA = -1;
	int best_QB = -1;
	double best_swap_overhead = 1e9;
	// 스왑 대상 큐비트들은 qc_Look_ahaed 쭉 돌면 된다.
	// 가장 오버헤드가 적은 qA->QB 조합을 구한다.
	for (int logic_qubit = 0; logic_qubit < num_lq; logic_qubit++) {
		if (qc_Look_ahaed[logic_qubit][0] == -1) continue;
		for (int QB : adj[l2p[logic_qubit]]) {
			int QA = l2p[logic_qubit];
			int qB = p2l[QB];  // -1 나올 수 있음.
			double swap_overhead = getCostOH(logic_qubit, QB) + getCostOH(qB, QA);
			if (swap_overhead < best_swap_overhead) {
				best_QA = QA;
				best_QB = QB;
				best_swap_overhead = swap_overhead;
			}
		}
	}
	int best_qA = p2l[best_QA];
	int best_qB = p2l[best_QB];
	l2p[best_qA] = best_QB;
	if (best_qB != -1) l2p[best_qB] = best_QA;
	p2l[best_QA] = best_qB;
	p2l[best_QB] = best_qA;
	routed_gate.push_back( { best_QA, best_QB } );
	routed_gate.push_back( { best_QB, best_QA } );
	if (best_qB != -1) routed_gate.push_back( { best_QA, best_QB } );
	swap_count++;
}

bool Router::doGate(std::vector<std::pair<int, int>>& routed_gate) {
	for (int logic_qubit = 0; logic_qubit < num_lq; logic_qubit++) {
		if (qc_Look_ahaed[logic_qubit][0] == -1) continue;
		int branch = qc_Look_ahaed[logic_qubit][0];
		if (dist[l2p[logic_qubit]][l2p[branch]] == 1) {
			qc_Look_ahaed[logic_qubit][0] = -1;
			qc_Look_ahaed[branch][0] = -1;
			pull(logic_qubit, 0);
			pull(branch, 0);
			routed_gate.push_back( { l2p[logic_qubit], l2p[branch] } );
			return true;
		}
	}
	return false;
}