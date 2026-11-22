/*
* 라우터를 위한 각종 함수들과 라우터
*/

#include <vector>
#include "QCompiler.h"

// gate_id에 해당하는 게이트가 pop 되었으므로 pull
// 전부 당기기+Glist에서 당겨오기까지.
void QCompiler::pull(int q_self, int now) {
	if (grid[q_self][now] != -1) return;  // 오류 방지
	if (now + 1 == T) {  // 만약 (비어있는)현재의 격자가 grid의 끝이라면 grid를 갱신한다
		fetch(q_self);
		return;
	}
	// 현재 큐비트 q_self에 대해서 다음 게이트는 cx q_self, q_other이라면
	// 또한 q_self, q_other의 이전 grid가 모두 nop(-1)이라면, 
	int next_gate_id = grid[q_self][now + 1];
	if (next_gate_id == -1) return;
	int q_other = (q_self != qc[next_gate_id].first) ? qc[next_gate_id].first : qc[next_gate_id].second;
	if (grid[q_other][now] != -1) return;
	// 게이트를 당긴다.
	grid[q_self][now] = next_gate_id;
	grid[q_other][now] = next_gate_id;
	grid[q_self][now + 1] = -1;
	grid[q_other][now + 1] = -1;
	pull(q_self, now + 1);
	pull(q_other, now + 1);
}

// q_self의 다음 게이트를 Glist에서 당겨오기
void QCompiler::fetch(int q_self) {
	if (grid[q_self][T - 1] != -1) return;  // 오류 방지
	if (top[q_self] >= Dlist[q_self].size()) return;  // 만약 패치할 것이 Dlist에 남아있지 않으면 return;
	int gate_id = Dlist[q_self][top[q_self]];
	int q_other = (q_self != qc[gate_id].first) ? qc[gate_id].first : qc[gate_id].second;
	if (top[q_other] >= Dlist[q_other].size()) return;  // 만약 패치할 것이 Dlist에 남아있지 않으면 return;
	if (grid[q_other][T - 1] != -1) return;  // 당겨올 수 있는지 검사하고, 
	if (Dlist[q_self][top[q_self]] != Dlist[q_other][top[q_other]]) return;  // 해당 게이트가 아직 대기되어야 한다면 대기한다.
	grid[q_self][T - 1] = gate_id;
	grid[q_other][T - 1] = gate_id;
	top[q_self]++;
	top[q_other]++;
}

double QCompiler::getCostOH(int Q_self, int Q_cand) {
	if (p2l[Q_self] == -1) return 0.0;  // 빈 노드는 큐비트간 거리 계산을 할 필요 없으므로 비용 = 0;
	double power = 1.0;
	double overhead = 0.0;
	int q_self = p2l[Q_self];
	// grid[q_self]에 해당하는 q1에 대해서 순회.
	for (int t = 0; t < T; t++) {
		if (grid[q_self][t] == -1) {
			power *= param_alpha;
			continue;
		}
		int gate_id = grid[q_self][t];
		int Q_other = l2p[(q_self != qc[gate_id].first) ? qc[gate_id].first : qc[gate_id].second];
		if (Q_other != Q_cand) {  // ohter 큐비트와 스왑하는 경우 당연히 거리 변화량은 0이라서 스킵해도 됨
			overhead += power * (dist[Q_cand][Q_other] - dist[Q_self][Q_other]);
		}
		power *= param_alpha;
	}
	return overhead;
}

void QCompiler::routing(std::vector<std::pair<int, int>>& routing_list, std::vector<std::pair<int, int>>& gate_dist_at_swapstarted, std::vector<std::pair<double, int>>& gate_swapcount_at_swapended) {
	// 라우터
	bool flag = true;
	while (flag) {
		// ================ 성능평가 ===============
		for (int q_self = 0; q_self < num_lq; q_self++) {
			if (grid[q_self][0] != -1 && gate_dist_at_swapstarted[grid[q_self][0]].first == 0) {  // 라우팅 하기 전 임박 게이트들 중 체크 안된 것 있는지 본다.
				int q0 = qc[grid[q_self][0]].first;
				int q1 = qc[grid[q_self][0]].second;
				gate_dist_at_swapstarted[grid[q_self][0]].first = dist[l2p[q0]][l2p[q1]];  // 임박 게이트간 거리를 저장하고
				gate_dist_at_swapstarted[grid[q_self][0]].second = routing_list.size();  // 임박 게이트의 첫 등록의 인덱스(routing_list)를 저장한다.
			}
		}
		// ================ 성능평가 ===============
		if (!doGate(routing_list, gate_dist_at_swapstarted, gate_swapcount_at_swapended)) {
			// ================ 성능평가 ===============
			int count = 0;
			std::vector<bool> checked(num_lq, false);
			for (int q_self = 0; q_self < num_lq; q_self++) {
				if (!checked[q_self] && grid[q_self][0] != -1) {
					count++;
					checked[qc[grid[q_self][0]].first] = true;
					checked[qc[grid[q_self][0]].second] = true;
				}
			}
			checked.assign(num_lq, false);
			for (int q_self = 0; q_self < num_lq; q_self++) {
				if (!checked[q_self] && grid[q_self][0] != -1) {
					gate_swapcount_at_swapended[grid[q_self][0]].first += 1.0 / count;  // 스왑 수는 공평하게 분배한다.
					checked[qc[grid[q_self][0]].first] = true;
					checked[qc[grid[q_self][0]].second] = true;
				}
			}
			// ================ 성능평가 ===============
			doSWAP(routing_list, gate_dist_at_swapstarted, gate_swapcount_at_swapended);
		}
		// 다음 실행할 게이트가 하나도 없으면 끝낸다.
		flag = false;
		for (int q_self = 0; q_self < num_lq; q_self++) {
			flag = flag || (grid[q_self][0] != -1);
		}
	}
}

void QCompiler::doSWAP(std::vector<std::pair<int, int>>& routing_list, std::vector<std::pair<int, int>>& gate_dist_at_swapstarted, std::vector<std::pair<double, int>>& gate_swapcount_at_swapended) {
	int best_Q_self = -1;
	int best_Q_cand = -1;
	double best_cost_overhead = 1e9;
	// 스왑 대상 큐비트들은 grid 쭉 돌면 된다.
	// 가장 오버헤드가 적은 Q_self->Q_cendi 조합을 구한다.
	for (int q_self = 0; q_self < num_lq; q_self++) {
		if (grid[q_self][0] == -1) continue;
		int Q_self = l2p[q_self];
		for (int Q_cand : adj[Q_self]) {
			double cost_overhead = getCostOH(Q_self, Q_cand) + getCostOH(Q_cand, Q_self);
			if (cost_overhead < best_cost_overhead) {
				best_Q_self = Q_self;
				best_Q_cand = Q_cand;
				best_cost_overhead = cost_overhead;
			}
		}
	}
	if (best_Q_self == -1 || best_Q_cand == -1) return;
	int q_self = p2l[best_Q_self];
	int q_cand = p2l[best_Q_cand];

	routing_list.push_back({ best_Q_self, best_Q_cand });
	routing_list.push_back({ best_Q_cand, best_Q_self });
	routing_list.push_back({ best_Q_self, best_Q_cand });
	// 그리고 실제로 스왑하기.
	if (q_self != -1) l2p[q_self] = best_Q_cand;
	if (q_cand != -1) l2p[q_cand] = best_Q_self;
	p2l[best_Q_self] = q_cand;
	p2l[best_Q_cand] = q_self;
}

bool QCompiler::doGate(std::vector<std::pair<int, int>>& routing_list, std::vector<std::pair<int, int>>& gate_dist_at_swapstarted, std::vector<std::pair<double, int>>& gate_swapcount_at_swapended) {
	for (int q_self = 0; q_self < num_lq; q_self++) {
		if (grid[q_self][0] == -1) continue;
		int gate_id = grid[q_self][0];
		int q_control = qc[gate_id].first;
		int q_target = qc[gate_id].second;
		if (dist[l2p[q_control]][l2p[q_target]] == 1) {

			// ================ 성능평가 ===============
			gate_swapcount_at_swapended[grid[q_self][0]].second = routing_list.size();  // q_self에 해당하는 게이트가 라우팅 된다면 라우팅 끝나는 시간을 기록한다.
			// ================ 성능평가 ===============

			grid[q_control][0] = -1;
			grid[q_target][0] = -1;
			pull(q_control, 0);
			pull(q_target, 0);
			routing_list.push_back({ l2p[q_control], l2p[q_target] });
			return true;
		}
	}
	return false;
}
