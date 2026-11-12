#include <iostream>
#include <vector>
#include "Quantum_circuit.h"
using namespace std;


int main() {
    Router qc1;
    vector<pair<int, int>> v = qc1.routing();
    for (pair<int, int> g : v) {
        if (g.first == 0 || g.second == 0)
            cout << "C ";
        else
            cout << "| ";
        if (g.first == 1 || g.second == 1)
            cout << "C ";
        else
            cout << "| ";
        if (g.first == 2 || g.second == 2)
            cout << "C ";
        else
            cout << "| ";
        if (g.first == 3 || g.second == 3)
            cout << "C ";
        else
            cout << "| ";
        if (g.first == 4 || g.second == 4)
            cout << "C ";
        else
            cout << "| ";
        cout << endl;
    }
    return 0;
}
