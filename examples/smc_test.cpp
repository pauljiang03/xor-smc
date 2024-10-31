#include "xor_smc/Solver.hpp"
#include <iostream>
#include <vector>

using namespace xor_smc;

void print_result(const std::string& test_name, bool expected, bool result) {
    std::cout << "\n=== " << test_name << " ===\n";
    std::cout << "Expected: " << (expected ? "SAT" : "UNSAT") << "\n";
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n";
}

void test_simple_xor_counting() {
    std::cout << "\nTest Case 1: XOR counting on simple OR formula\n";
    
    // We'll count solutions to (x1 OR x2)
    std::vector<std::vector<Literal>> f1 = {
        {Literal(1, true), Literal(2, true)}  // x1 OR x2
    };
    
    // Main formula: x0 must be true
    std::vector<std::vector<Literal>> phi = {
        {Literal(0, true)}
    };
    
    // Solutions to (x1 OR x2) are:
    // (1,0), (0,1), (1,1) - total: 3 solutions
    // Add q=1 XOR constraint (should remain SAT as 3 >= 2^1)
    std::vector<uint32_t> q = {1};
    
    Solver solver(0.01);  // 1% error probability
    bool result = solver.solve(phi, {f1}, q, 3);
    
    print_result("x0 AND #(x1 OR x2) >= 2", true, result);
}

void test_and_counting() {
    std::cout << "\nTest Case 2: XOR counting on AND formula\n";
    
    // Count solutions to (x1 AND x2)
    std::vector<std::vector<Literal>> f1 = {
        {Literal(1, true)},  // x1
        {Literal(2, true)}   // x2
    };
    
    // Main formula: x0 must be true
    std::vector<std::vector<Literal>> phi = {
        {Literal(0, true)}
    };
    
    // Solutions to (x1 AND x2) are:
    // Just (1,1) - total: 1 solution
    // Add q=1 XOR constraint (should be UNSAT as 1 < 2^1)
    std::vector<uint32_t> q = {1};
    
    Solver solver(0.01);
    bool result = solver.solve(phi, {f1}, q, 3);
    
    print_result("x0 AND #(x1 AND x2) >= 2", false, result);
}

void test_multiple_xor_constraints() {
    std::cout << "\nTest Case 3: Multiple XOR constraints\n";
    
    // Count solutions to (x1 OR x2 OR x3)
    std::vector<std::vector<Literal>> f1 = {
        {Literal(1, true), Literal(2, true), Literal(3, true)}
    };
    
    // Main formula: None (true)
    std::vector<std::vector<Literal>> phi;
    
    // Solutions to (x1 OR x2 OR x3):
    // 7 solutions (all except 0,0,0)
    // Add q=2 XOR constraints (should be SAT as 7 >= 2^2)
    std::vector<uint32_t> q = {2};
    
    Solver solver(0.01);
    bool result = solver.solve(phi, {f1}, q, 4);
    
    print_result("#(x1 OR x2 OR x3) >= 4", true, result);
}

void test_multiple_formulas() {
    std::cout << "\nTest Case 4: Multiple counting formulas\n";
    
    // Count solutions to two formulas:
    // f1: (x1 OR x2)
    // f2: (x3 AND x4)
    std::vector<std::vector<Literal>> f1 = {
        {Literal(1, true), Literal(2, true)}
    };
    std::vector<std::vector<Literal>> f2 = {
        {Literal(3, true)},
        {Literal(4, true)}
    };
    
    // Main formula: x0
    std::vector<std::vector<Literal>> phi = {
        {Literal(0, true)}
    };
    
    // f1 has 3 solutions, f2 has 1 solution
    // q = {1, 1} means we need:
    // f1 >= 2 solutions (SAT)
    // f2 >= 2 solutions (UNSAT)
    std::vector<uint32_t> q = {1, 1};
    
    Solver solver(0.01);
    bool result = solver.solve(phi, {f1, f2}, q, 5);
    
    print_result("x0 AND #(x1 OR x2) >= 2 AND #(x3 AND x4) >= 2", false, result);
}

int main() {
    test_simple_xor_counting();
    test_and_counting();
    test_multiple_xor_constraints();
    test_multiple_formulas();
    return 0;
}