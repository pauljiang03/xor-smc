#include "xor_smc/Solver.hpp"
#include <iostream>
#include <vector>
#include <random>

using namespace xor_smc;
using namespace std;

void test_counting() {
    Solver solver;
    solver.set_num_variables(3);
    
    std::cout << "Testing formula with 3 free variables (8 solutions)\n";
    
    const int NUM_XORS = 4;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution d(0.5);

    std::cout << "Adding " << NUM_XORS << " XORs\n";
    
    for(int i = 0; i < NUM_XORS; i++) {
        std::vector<Literal> xor_lits;
        
        // Each variable appears exactly once with random polarity
        for(int var = 0; var < 3; var++) {
            bool polarity = d(gen);
            xor_lits.push_back(Literal(var, polarity));
        }
        
        // Random parity
        bool want_odd_parity = d(gen);
        if(want_odd_parity) {
            // Add a constant 1 by negating an odd number of literals
            xor_lits[0] = Literal(xor_lits[0].var_id(), !xor_lits[0].is_positive());
        }
        
        solver.add_xor_clause(xor_lits);
        
        std::cout << "Added XOR: ";
        for(size_t j = 0; j < xor_lits.size(); j++) {
            if(j > 0) std::cout << " ⊕ ";
            if(!xor_lits[j].is_positive()) std::cout << "¬";
            std::cout << "x" << xor_lits[j].var_id();
        }
        std::cout << " = 0\n";
    }

    bool is_sat = solver.solve();
    std::cout << "Result with " << NUM_XORS << " XORs: " 
              << (is_sat ? "SAT" : "UNSAT") << "\n";
}

int main() {
    test_counting();
    return 0;
}