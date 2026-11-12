#pragma once
#include <iostream>
#include <vector>



struct Gate {       // 16 Byte
    char type[4];   // 게이트 종류
    int q0;         // 컨트롤 큐비트
    int q1;         // 타겟 큐비트
    float param;    // 파라미터 각도
}; 

class Circuits {			    // 양자 논리 회로
	/*게이트 삽입: 게이트 종류, 타겟 큐비트, 파라미터*/
	const int num_lq;		    // 논리 큐비트 갯수
	std::vector<Gate> gates;    // 실제 회로 게이트들
public:
    Circuits(int num_lq) : num_lq(num_lq) {}

    // 게이트 추가 기능 natual gate set
    void h(int target) {
        std::cout << "execute | h q[" << target << "] <-- ";
        if (target >= 0 && target < num_lq) {
            gates.push_back({ "h", -1, target, 0.0f });
            std::cout << "success\n";
        }
        else {
            std::cout << "fail\n";
        }
    }
    void cx(int control, int target) {
        std::cout << "execute | cx q[" << control << "] q[" << target << "] <-- ";
        if (target >= 0 && control >= 0 && target < num_lq && control < num_lq && target != control) {
            gates.push_back({ "cx", control, target, 0.0f });
            std::cout << "success\n";
        }
        else {
            std::cout << "fail\n";
        }
    }

    // 편의 기능
    void print() const {
        std::cout << "circuit: \n";
        for (int i = 0; i < num_lq; i++) {
            std::cout << 'q' << i << " | ";
            for (const Gate& g : gates) {
                if (g.q0 == -1) {
                    if (g.q1 == i)
                        std::cout << g.type[0] << ' ';
                    else
                        std::cout << "- ";
                }
                else {
                    if (g.q1 == i)
                        std::cout << "x ";
                    else if (g.q0 == i)
                        std::cout << g.type[0] << ' ';
                    else
                        std::cout << "- ";
                }
            }
            std::cout << std::endl;
        }
    }

    
    std::vector<std::vector<int>> getBubbleGrid() {
        std::vector<std::vector<int>> bubble_grid(num_lq);
        for (Gate g : gates) {
            if (g.q0 == -1) continue;
            // 어떤 논리 게이트 cx q0,q1에 대해서 bubble_grid[q0].size() > bubble_grid[q1].size()이라면 부족한 만큼 q1라인에 버블을 넣는다.
            if (bubble_grid[g.q0].size() < bubble_grid[g.q1].size()) {
                bubble_grid[g.q0].insert(bubble_grid[g.q0].end(), bubble_grid[g.q1].size()- bubble_grid[g.q0].size(), -1);
            }
            else {
                bubble_grid[g.q1].insert(bubble_grid[g.q1].end(), bubble_grid[g.q0].size()- bubble_grid[g.q1].size(), -1);
            }
            // 어떤 논리 게이트 cx q0,q1에 대해서 bubble_grid[q0].size() == bubble_grid[q1].size()이라면 바로 집어넣는다.
            bubble_grid[g.q0].push_back(g.q1);
            bubble_grid[g.q1].push_back(g.q0);
        }
        return bubble_grid;
    }
};

