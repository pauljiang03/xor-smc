#include "xor_smc/CDCLSolver.hpp"
#include <iostream>

using namespace xor_smc;

void test_case_2() {
    std::cout << "\n=== Testing UNSAT case (x0 AND NOT x0) ===\n";
    CDCLSolver solver;
    solver.set_num_variables(1);  // Just one variable x0
    
    // Add the two contradictory clauses
    std::vector<Literal> clause1 = {Literal(0, true)};   // x0
    std::vector<Literal> clause2 = {Literal(0, false)};  // NOT x0
    
    std::cout << "Adding first clause (x0):\n";
    solver.add_clause(clause1);
    
    std::cout << "Adding second clause (NOT x0):\n";
    solver.add_clause(clause2);
    
    std::cout << "Starting solve...\n";
    bool result = solver.solve();
    
    if (result) {
        std::cout << "ERROR: Found SAT when should be UNSAT!\n";
        std::cout << "Final assignment: x0 = " << solver.get_value(0) << "\n";
    } else {
        std::cout << "Correctly found UNSAT\n";
    }
}

int main() {
    xor_smc::CDCLSolver solver;
    solver.set_num_variables(1);
    
    // Add x0 AND NOT x0
    solver.add_clause({xor_smc::Literal(0, true)});    // x0
    solver.add_clause({xor_smc::Literal(0, false)});   // NOT x0
    
    bool result = solver.solve();
    std::cout << "Final result: " << (result ? "SAT" : "UNSAT") << "\n";
    
    return 0;
}