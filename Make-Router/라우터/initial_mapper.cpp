/*
* initial_mapper.cpp
* initial_mapper.cpp
* initial_mapper.cpp
* 원래 AQubits_log.cpp 였던 코드를 약간 수정해서 재활용한 버전이다.
* 당연하게도 매핑을 위한 모든 함수와 매핑 함수가 있는 코드이다.
*/

#include <vector>
#include "QCompiler.h"

using namespace std;

double QCompiler::getAffinity(int q_cand) const {
	double power = 1.0;
	double affinity = 0.0;
	for (int t = 0; t < T; t++) {
		if (grid[q_cand][t] == -1) {
			power *= param_alpha;
			continue;
		}
		int q_other = grid[q_cand][t];
		if (l2p[q_other] != -1) {  // 
			affinity += power;
		}
		power *= param_alpha;
	}
	return affinity;
}

double QCompiler::getCost(int best_q, int Q_cand) const {
	double power = 1.0;
	double cost = 0.0;
	// Place best_q onto Q_cand
	for (int t = 0; t < T; t++) {
		if (grid[best_q][t] == -1) {
			power *= param_alpha;
			continue;
		}
		int Q_other = l2p[grid[best_q][t]];
		if (Q_other != -1) {  // Calculate about placed q_other
			cost += power * dist[Q_cand][Q_other];
		}
		power *= param_alpha;
	}
	return cost;
}

// ***** 큐비트 뽑기 기준에 따라 뽑기 *****
// 아직 짝지어지지 않은 논리 큐비트들 중 하나를 뽑는다. --> A(a)가 가장 큰 a를 뽑는다.
int QCompiler::getBestQ() const {
	int best_q = -1;
	double maxaff = -1;
	for (int q_cand = 0; q_cand < num_lq; q_cand++) {
		if (l2p[q_cand] != -1) continue;
		double aff = getAffinity(q_cand);
		if (best_q == -1 || aff > maxaff) {
			best_q = q_cand;
			maxaff = aff;
		}
		else if (aff == maxaff && importance[q_cand] > importance[best_q]) {
			best_q = q_cand;
			maxaff = aff;
		}
	}
	return best_q;
}
// 아직 짝지어지지 않은 물리 큐비트들 중 하나를 뽑는다. --> C(x)가 가장 작은 x를 뽑는다. 
int QCompiler::getBestP(int best_p) {
	int best_Q = -1;
	double min_cost = -1;
	for (int Q_cand = 0; Q_cand < num_pq; Q_cand++) {
		if (p2l[Q_cand] != -1) continue;
		double cost = getCost(best_p, Q_cand);
		if (min_cost == -1 || cost < min_cost) {
			best_Q = Q_cand;
			min_cost = cost;
		}
		else if (cost == min_cost && centrality[Q_cand] < centrality[best_Q]) {
			best_Q = Q_cand;
			min_cost = cost;
		}
	}
	return best_Q;
}

// ***** 특수 케이스 *****
// importance(lq)
int QCompiler::getFirstQ() const {
	int first_q = -1;
	double max_imp = -1;
	for (int q_cand = 0; q_cand < num_lq; q_cand++) {
		if (importance[q_cand] > max_imp) {
			max_imp = importance[q_cand];
			first_q = q_cand;
		}
	}
	return first_q;
}
// centrality()
int QCompiler::getFirstP() const {
	int first_Q = -1;
	int min_center = -1;
	for (int Q_cand = 0; Q_cand < num_pq; Q_cand++) {
		if (min_center == -1 || centrality[Q_cand] < min_center) {
			min_center = centrality[Q_cand];
			first_Q = Q_cand;
		}
	}
	return first_Q;
}

// ***** 본체 *****
// ***** 뽑은 Q와 P를 짝지은 후 저장, 반복 ***** 
vector<int> QCompiler::mapping() {
	p2l.assign(num_pq, -1);
	l2p.assign(num_lq, -1);
	int first_lq = getFirstQ();
	int first_pq = getFirstP();
	doPlace(first_lq, first_pq);
	for (int i = 1; i < num_lq; i++) {
		int best_lq = getBestQ();
		// 논리 큐비트 고르기 실패에요
		if (best_lq == -1) {
			break;
		}
		int best_pq = getBestP(best_lq);
		if (best_pq == -1) {
			// 물리 큐비트 고르기 실패에요
			break;
		}
		doPlace(best_lq, best_pq);
	}

	return l2p;
}

inline void QCompiler::doPlace(int best_lq, int best_pq) {
	p2l[best_pq] = best_lq;
	l2p[best_lq] = best_pq;
}
