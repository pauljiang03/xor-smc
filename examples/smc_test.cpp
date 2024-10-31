#include "xor_smc/Solver.hpp"
#include <iostream>
#include <vector>

using namespace xor_smc;

void print_result(const std::string& test_name, bool expected, bool result) {
    std::cout << test_name << " - Expected: " << (expected ? "SAT" : "UNSAT") 
              << ", Got: " << (result ? "SAT" : "UNSAT") 
              << (expected == result ? " ✓" : " ✗") << "\n";
}

void run_tests() {
    Solver solver(0.01);
    
    // Test 1: Empty formula should be SAT
    std::vector<std::vector<Literal>> empty_formula;
    print_result("Empty formula >= 2 solutions", 
                true, 
                solver.solve({}, {empty_formula}, {1}, 1));

    // Test 2: Single variable formula x0 should be UNSAT for >=2
    std::vector<std::vector<Literal>> single_var = {
        {Literal(0, true)}
    };
    print_result("Single variable >= 2 solutions", 
                false, 
                solver.solve({}, {single_var}, {1}, 1));

    // Test 3: (x0 OR x1) should be SAT for >=2
    std::vector<std::vector<Literal>> simple_or = {
        {Literal(0, true), Literal(1, true)}
    };
    print_result("x0 OR x1 >= 2 solutions", 
                true, 
                solver.solve({}, {simple_or}, {1}, 2));

    // Test 4: (x0 AND x1) should be UNSAT for >=2
    std::vector<std::vector<Literal>> simple_and = {
        {Literal(0, true)},
        {Literal(1, true)}
    };
    print_result("x0 AND x1 >= 2 solutions", 
                false, 
                solver.solve({}, {simple_and}, {1}, 2));

    // Test 5: (x0 OR x1 OR x2) should be SAT for >=4
    std::vector<std::vector<Literal>> three_or = {
        {Literal(0, true), Literal(1, true), Literal(2, true)}
    };
    print_result("x0 OR x1 OR x2 >= 4 solutions", 
                true, 
                solver.solve({}, {three_or}, {2}, 3));

    // Test 6: (x0 OR x1 OR x2) should be UNSAT for >=8
    print_result("x0 OR x1 OR x2 >= 8 solutions", 
                false, 
                solver.solve({}, {three_or}, {3}, 3));

    // Test 7: Multiple independent OR clauses should be SAT
    std::vector<std::vector<Literal>> or1 = {{Literal(0, true), Literal(1, true)}};
    std::vector<std::vector<Literal>> or2 = {{Literal(2, true), Literal(3, true)}};
    std::vector<uint32_t> q = {1, 1};  // Both need >= 2 solutions
    print_result("(x0 OR x1)>=2 AND (x2 OR x3)>=2", 
                true, 
                solver.solve({}, {or1, or2}, q, 4));

    // Test 8: Mixed AND/OR with main formula
    std::vector<std::vector<Literal>> main_formula = {{Literal(0, true)}};
    std::vector<std::vector<Literal>> and_formula = {
        {Literal(1, true)}, {Literal(2, true)}
    };
    std::vector<std::vector<Literal>> or_formula = {
        {Literal(3, true), Literal(4, true)}
    };
    print_result("x0 AND #(x1 AND x2)>=2 AND #(x3 OR x4)>=2", 
                false, 
                solver.solve(main_formula, {and_formula, or_formula}, {1, 1}, 5));

    // Test 9: Requires all solutions (1<<n)
    std::vector<std::vector<Literal>> all_solutions = {
        {Literal(0, true), Literal(0, false)}  // Tautology clause
    };
    print_result("Tautology >= 2 solutions", 
                true, 
                solver.solve({}, {all_solutions}, {1}, 1));
}

void test_simple_or() {
    Solver solver(0.01);
    
    // x0 OR x1 - exactly 3 solutions: (1,0), (0,1), (1,1)
    std::vector<std::vector<Literal>> or2 = {
        {Literal(0, true), Literal(1, true)}
    };
    
    std::cout << "\nTesting x0 OR x1 >= 2 solutions (should be SAT)\n";
    bool result = solver.solve({}, {or2}, {1}, 2);
    print_result("x0 OR x1 >= 2", true, result);
    
    std::cout << "\nTesting x0 OR x1 >= 4 solutions (should be UNSAT)\n";
    result = solver.solve({}, {or2}, {2}, 2);
    print_result("x0 OR x1 >= 4", false, result);
}

int main() {
    test_simple_or();
    //run_tests();
    return 0;
}