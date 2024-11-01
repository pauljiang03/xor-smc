#include "xor_smc/Solver.hpp"
#include <iostream>
#include <vector>

using namespace xor_smc;
using namespace std;

void test_counting() {
    // Should have exactly 8 solutions (2^3)
    Solver solver;
    solver.set_num_variables(3);
    
    std::cout << "Testing formula with 3 free variables (8 solutions)\n";
    
    std::vector<uint32_t> thresholds = {16};  //threshold 16 to get 4 XORs bc log2(16)=4
    std::vector<std::vector<uint32_t>> counting_vars = {{0,1,2}}; 
    std::vector<std::vector<uint32_t>> fixed_vars = {{}};  
    
    std::cout << "Adding 4 XORs...\n\n";
    
    bool result = solver.solve_smc(thresholds, counting_vars, fixed_vars);
    
    std::cout << "\nFinal result: formula has " 
              << (result ? ">=" : "<") << " 16 solutions\n";
}

int main() {
    test_counting();
    return 0;
}