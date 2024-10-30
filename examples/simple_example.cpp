#include "xor_smc/Solver.hpp"
#include "xor_smc/Formula.hpp"
#include <iostream>

using namespace xor_smc;

void test_case_1() {
    std::cout << "\nTest Case 1: Simple SAT problem (x0 AND x1)\n";
    
    Formula f1;
    f1.add_clause({Literal(0, true)});   // x0
    f1.add_clause({Literal(1, true)});   // x1
    
    Solver solver(0.01);
    bool result = solver.solve(
        f1,             // main formula
        {},            // no counting constraints
        {},            // no thresholds
        2              // number of variables
    );
    
    std::cout << "Expected: SAT\n";
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n";
}

void test_case_2() {
    std::cout << "\nTest Case 2: Simple UNSAT problem (x0 AND NOT x0)\n";
    
    Formula f2;
    f2.add_clause({Literal(0, true)});    // x0
    f2.add_clause({Literal(0, false)});   // NOT x0
    
    Solver solver(0.01);
    bool result = solver.solve(
        f2,
        {},
        {},
        1     // just one variable
    );
    
    std::cout << "Expected: UNSAT\n";
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n";
}

void test_case_3() {
    std::cout << "\nTest Case 3: Simple SMC problem\n";
    
    Formula phi;
    phi.add_clause({Literal(0, true)});  // x0 must be true
    
    Formula count_formula;
    // x0 AND x1 - this should have exactly one satisfying assignment when x0 is true
    count_formula.add_clause({Literal(0, true)});
    count_formula.add_clause({Literal(1, true)});
    
    Solver solver(0.01);
    bool result = solver.solve(
        phi,
        {count_formula},   // one counting constraint
        {1},              // want at least 2^1 = 2 solutions
        2                 // number of variables
    );
    
    std::cout << "Expected: UNSAT (not enough satisfying assignments)\n";
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n";
}

int main() {
    test_case_1();
    test_case_2();
    test_case_3();
    return 0;
}
