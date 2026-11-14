/*
rule 1:
	짝지어지지 않은 논리 큐비트는 unmapped_lq로 표시한다.
	짝지어진 논리 큐비트는 mapped_lq로 표시한다.
	짝지어지지 않은 물리 큐비트는 unmapped_pq로 표시한다.
	짝지어진 물리 큐비트는 mapped_pq로 표시한다.
rule 2:
	바깥 for문의 인자가 배열의 row number, 안쪽 for문의 인자가 column number이다.
	for:i {for:j {v[i][j]}}
rule 3:
	f(best_pq)=y라고 하고 싶다면 x를 바깥 for문의 인자로 한다.
*/

#include <vector>
#include "AQubits.h"

using namespace std;
// ***** 초기 세팅 ******
Mapper::Mapper(const vector<vector<int>>& distance, vector<vector<double>>& weight)
	: distance(distance), weight(weight), num_lq(weight.size()), num_pq(distance.size()), p2l(num_pq, -1), l2p(num_lq, -1) {
	// 1. 논리 큐비트 중요도(W 합)
	for (int Q_i = 0; Q_i < num_lq; Q_i++) {		//최초 선언이므로 모두 짝지어지지 않은 논리 큐비트이다.
		double Im_i = 0;
		for (int Q_j = 0; Q_j < num_lq; Q_j++) Im_i += weight[Q_i][Q_j];
		importance.push_back(Im_i);
	}
	// 2. [추가] 물리 큐비트 중심도 (D 합 - 작을수록 중앙!)
	for (int P_i = 0; P_i < num_pq; P_i++) {		//최초 선언이므로 모두 짝지어지지 않은 물리 큐비트이다.
		int Sum_i = 0;
		for (int P_j = 0; P_j < num_pq; P_j++) Sum_i += distance[P_i][P_j];
		centrality.push_back(Sum_i);
	}
	//물리 큐빗과 논리 큐빗이 1개 이상 존재해야 하며, 
	//dist크기 또한 물리 큐빗의 갯수와 같아야 하고, 
	//dist는 대칭행렬이여야 하며, 
	//weight 또한 대칭행렬이여야 한다.
	//그리고 물리 큐빗이 논리 큐빗보다 많아야 한다.
}

// ***** 큐비트 뽑기 기준 *****
// A(best_lq) : 모든 mapped_lq에 대해 unmapped_lq와 mapped_lq사이의 W(unmapped_lq, mapped_lq)의 합.
// 짝지어지진 부분에 대해서만 구간 합을 구하면 된다.
double Mapper::getAffinity(int q_i) const {
	double Affinity_i = 0;
	for (int Q_j = 0; Q_j < num_lq; Q_j++) {							// 계산 복잡도를 줄이기 위해 논리 큐비트에 대해서 순회한다.
		if (l2p[Q_j] != -1) {														// 짝지어진 부분에 대해서만 실행한다.
			//W(qubit, mapped_qubit(i))를 합한다.
			Affinity_i += weight[q_i][Q_j];
		}
	}
	return Affinity_i;
}

//q가 놓일 p를 구할 때 사용되는 목적 함수: C. 논리 큐비트M회 연산.
// C(best_pq) : 모든 mapped_pq에 대해 unmapped_pq와 mapped_pq사이의 D(best_pq, mapped_pq)*W(best_lq, mapped_lq)가중치의 합.
double Mapper::getCost(int Q_i) const {	//물리 큐비트 노드에 대해
	double result = 0;
	for (int Q_j = 0; Q_j < num_lq; Q_j++) {							// 계산 복잡도를 줄이기 위해 논리 큐비트에 대해서 순회한다.
		if (l2p[Q_j] != -1) {														// 짝지어진 부분에 대해서만 실행한다.
			//D(node, i)와 W(mapped_qubit(node), mapped_qubit(i))를 곱한다.
			result += weight[Q_i][Q_j] * distance[l2p[Q_i]][l2p[Q_j]];
		}
	}
	return result;
}

// ***** 큐비트 뽑기 기준에 따라 뽑기 *****
// 아직 짝지어지지 않은 논리 큐비트들 중 하나를 뽑는다. --> A(a)가 가장 큰 a를 뽑는다.
int Mapper::getBestQ() const {
	int result = -1;
	double maxaff = -1;
	for (int unmapped_lq = 0; unmapped_lq < num_lq; unmapped_lq++) {
		if (l2p[unmapped_lq] != -1) continue;
		double aff = getAffinity(unmapped_lq);
		if (result == -1 || aff > maxaff) {
			result = unmapped_lq;
			maxaff = aff;
		}
		else if (aff == maxaff && importance[unmapped_lq] > importance[result]) {
			result = unmapped_lq;
			maxaff = aff;
		}
	}
	return result;
}
// 아직 짝지어지지 않은 물리 큐비트들 중 하나를 뽑는다. --> C(x)가 가장 작은 x를 뽑는다. 
int Mapper::getBestP(int best_lq) {
	int result = -1;
	double mincost = -1;
	for (int unmapped_pq = 0; unmapped_pq < num_pq; unmapped_pq++) {
		if (p2l[unmapped_pq] != -1) continue;
		int tmp_lq = p2l[unmapped_pq];
		int tmp_pq = l2p[best_lq];
		p2l[unmapped_pq] = best_lq;
		l2p[best_lq] = unmapped_pq;
		int cost = getCost(best_lq);
		if (mincost == -1 || cost < mincost) {
			result = unmapped_pq;
			mincost = cost;
		}
		else if (cost == mincost && centrality[unmapped_pq] < centrality[result]) {
			result = unmapped_pq;
			mincost = cost;
		}
		p2l[unmapped_pq] = tmp_lq; 
		l2p[best_lq] = tmp_pq;
	}
	return result;
}

// ***** 특수 케이스 *****
// importance(lq)
int Mapper::getFirstQ() const {
	int result = -1;
	double max_imp = -1;
	for (int unmapped_lq = 0; unmapped_lq < num_lq; unmapped_lq++) {
		if (importance[unmapped_lq] > max_imp) {
			max_imp = importance[unmapped_lq];
			result = unmapped_lq;
		}
	}
	return result;
}
// centrality()
int Mapper::getFirstP() const {
	int result = -1;
	int min_center = -1;
	for (int unmapped_pq = 0; unmapped_pq < num_pq; unmapped_pq++) {
		if (min_center == -1 || centrality[unmapped_pq] < min_center) {
			min_center = centrality[unmapped_pq];
			result = unmapped_pq;
		}
	}
	return result;
}

// ***** 본체 *****
// ***** 뽑은 Q와 P를 짝지은 후 저장, 반복 ***** 
vector<int> Mapper::mapping() {
	p2l.assign(num_pq, -1);
	l2p.assign(num_lq, -1);
	int first_lq = getFirstQ();
	int first_pq = getFirstP();
	map(first_lq, first_pq);
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
		map(best_lq, best_pq);
	}

	return l2p;
}

//neer넣기 전 밑밥
void Mapper::map(int best_lq, int best_pq) {
	p2l[best_pq] = best_lq;
	l2p[best_lq] = best_pq;
}
