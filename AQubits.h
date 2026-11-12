#pragma once
#include <vector>

//using LQ = int;	//논리 큐빗
//using PQ = int;	//물리 큐빗

class Mapper {
	// 하드웨어 거리 행렬(D)과 회로 상호작용 행렬(W)
	const std::vector<std::vector<int>>& distance;				// 물리 큐비트 거리 행렬
	std::vector<std::vector<double>>& weight;					// 가상 큐비트 가중치 행렬

	// 큐비트 총 개수 (Logical / Physical)
	const int num_lq;											// 총 논리 큐비트 갯수
	const int num_pq;											// 총 물리 큐비트 갯수
	//int bestQ;
	//int bestP;
	//std::vector<int> neer;

	// 사전 계산된 정적 지표 (Importance / Centrality)
	std::vector<double> importance;								// 논리 큐비트의 중요도(~가중치)
	std::vector<int> centrality;								// 물리 큐비트의 중심도(~거리)

	// 양방향 매핑 테이블 (l2p[lq] = pq, p2l[pq] = lq)
	std::vector<int> p2l;										// 물리 큐비트에 대응되는 논리 큐비트 (~p2l[pq]=lq)
	std::vector<int> l2p;										// 논리 큐비트에 대응되는 물리 큐비트 (~l2p[lq]=pq)

	// 내부 탐색 메서드
	double getAffinity(int q_i) const;						// 논리 큐비트의 친밀도(~가중치)
	double getCost(int Q_i) const;			// 최적화될 목적 함수(~가중치, ~거리)
	int getBestQ() const;										// result = max(unmapped, key=lambda i:(getAffinity(i), importance[i], -i))
	int getBestP(int best_lq);							// result = min(unmapped, key=lambda i:(getCost(i), centrality[i], i))
	int getFirstQ() const;										// result = max(unmapped, key=lambda i:(importance[i], -i))
	int getFirstP() const;										// result = min(unmapped, key=lambda i:(centrality[i], i))

	void map(int best_lq, int best_pq);
	// void map(int best_lq, int best_pq, std::vector<int>& neer);
public:
	Mapper(const std::vector<std::vector<int>>& distance, std::vector<std::vector<double>>& weight);
	// mapper
	std::vector<int> mapping();										// result[lq] = pq (Index: Logical, Value: Physical)
	void router();
};