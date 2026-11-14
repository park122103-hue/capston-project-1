#include <vector>
#include "Quantum_circuit.h"

Router::Router () {}

// 전부 당기기+Glist에서 당겨오기까지.
void Router::pull(int q_self, int now) {
	if (grid[q_self][now] != -1) return;  // 오류 방지
	if (now + 1 == T) {  // 만약 (비어있는)현재의 격자가 grid의 끝이라면 grid를 갱신한다
        fatch(q_self);
        return;
    }
	// 현재 큐비트 q_self에 대해서 다음 게이트는 cx q_self, q_other이라면
	// 또한 q_self, q_other의 이전 grid가 모두 nop(-1)이라면, 
	int q_other = grid[q_self][now + 1];
	if (q_other == -1) return;
	if (grid[q_other][now] != -1) return;
	// 게이트를 당긴다.
	grid[q_self][now] = q_other;
	grid[q_other][now] = q_self;
	grid[q_self][now + 1] = -1;
	grid[q_other][now + 1] = -1;
	pull(q_self, now + 1);
	pull(q_other, now + 1);
}

// q_self의 다음 게이트를 Glist에서 당겨오기
void Router::fatch(int q_self) {
	if (grid[q_self][T - 1] != -1) return;  // 오류 방지
    if (top[q_self] >= Glist[q_self].size()) return;  // 만약 패치할 것이 Glist에 남아있지 않으면 return;
	int q_other = Glist[q_self][top[q_self]].other;
    if (top[q_other] >= Glist[q_other].size()) return;  // 만약 패치할 것이 Glist에 남아있지 않으면 return;
	if (grid[q_other][T - 1] != -1) return;  // 당겨올 수 있는지 검사하고, 
	if (Glist[q_self][top[q_self]].id != Glist[q_other][top[q_other]].id) return;  // 해당 게이트가 아직 대기되어야 한다면 대기한다.
	grid[q_self][T - 1] = q_other;
	grid[q_other][T - 1] = q_self;
	top[q_self]++;
	top[q_other]++;
}

double Router::getCostOH(int Q_self, int Q_cand, double param_alpha) {
	if (p2l[Q_self] == -1) return 0.0;  // 빈 노드는 큐비트간 거리 계산을 할 필요 없으므로 비용 = 0;
	double power = 1.0;
	double overhead = 0.0;
	int q_self = p2l[Q_self];
	// BGbuf[q0]에 해당하는 q1에 대해서 순회.
	for (int t = 0; t < T; t++) {
		if (grid[p2l[Q_self]][t] == -1) {
			power *= param_alpha;
			continue;
		}
		int Q_other = l2p[grid[q_self][t]];
		if (p2l[Q_other] != -1 && Q_other != Q_cand) {  // ohter 큐비트와 스왑하는 경우 당연히 거리 변화량은 0이라서 스킵해도 됨
			overhead += power * (dist[Q_cand][Q_other] - dist[Q_self][Q_other]);
		}
		power *= param_alpha;
	}
	return overhead;
}

std::vector<std::pair<int, int>> Router::routing() {
    // 결과내기
    std::vector<std::pair<int, int>> routed_gate;
    int swap_overhead = 0;
	// 라우터
	bool flag = true;
	while (flag) {
		if (!doGate(routed_gate)) doSWAP(routed_gate, swap_overhead);
		// 다음 실행할 게이트가 하나도 없으면 끝낸다.
		flag = false;
		for (int q_self = 0; q_self < num_lq; q_self++) {
			flag = flag || (grid[q_self][0] != -1);
		}
	}
	return routed_gate;
}

void Router::doSWAP(std::vector<std::pair<int, int>>& routed_gate, int& swap_overhead) {
	int best_Q_self = -1;
	int best_Q_cand = -1;
	double best_swap_overhead = 1e9;
	// 스왑 대상 큐비트들은 grid 쭉 돌면 된다.
	// 가장 오버헤드가 적은 Q_self->Q_cendi 조합을 구한다.
	for (int q_self = 0; q_self < num_lq; q_self++) {
		if (grid[q_self][0] == -1) continue;
		int Q_self = l2p[q_self];
		for (int Q_cand : adj[Q_self]) {
			double swap_overhead = getCostOH(Q_self, Q_cand) + getCostOH(Q_cand, Q_self);
			if (swap_overhead < best_swap_overhead) {
				best_Q_self = Q_self;
				best_Q_cand = Q_cand;
				best_swap_overhead = swap_overhead;
			}
		}
	}
	if (best_Q_self == -1 || best_Q_cand == -1) return;
	int q_self = p2l[best_Q_self];
	int q_cand = p2l[best_Q_cand];
	if (q_self != -1) l2p[q_self] = best_Q_cand;
	if (q_cand != -1) l2p[q_cand] = best_Q_self;
	p2l[best_Q_self] = q_cand;
	p2l[best_Q_cand] = q_self;
	routed_gate.push_back( { best_Q_self, best_Q_cand } );
	routed_gate.push_back( { best_Q_cand, best_Q_self } );
	if (q_self != -1 && q_cand != -1) routed_gate.push_back( { best_Q_self, best_Q_cand } );
	swap_overhead++;
}

bool Router::doGate(std::vector<std::pair<int, int>>& routed_gate) {
	for (int q_self = 0; q_self < num_lq; q_self++) {
		if (grid[q_self][0] == -1) continue;
		int q_other = grid[q_self][0];
		if (dist[l2p[q_self]][l2p[q_other]] == 1) {
			grid[q_self][0] = -1;
			grid[q_other][0] = -1;
			pull(q_self, 0);
			pull(q_other, 0);
			routed_gate.push_back( { l2p[q_self], l2p[q_other] } );
			return true;
		}
	}
	return false;
}
