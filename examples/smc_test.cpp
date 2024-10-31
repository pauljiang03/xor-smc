#include "xor_smc/Solver.hpp"
#include <iostream>
#include <vector>

using namespace xor_smc;

void print_result(const std::string& test_name, bool expected, bool result) {
    std::cout << "\n=== " << test_name << " ===\n";
    std::cout << "Expected: " << (expected ? "SAT" : "UNSAT") << "\n";
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n";
}

void test_simple_and_or() {
    std::cout << "\nTest Case 1: Simple AND Formula - UNSAT case\n";
    
    // f1: (x0 AND x1) - has 1 solution
    std::vector<std::vector<Literal>> f1 = {
        {Literal(0, true)},
        {Literal(1, true)}
    };
    
    // f2: (x2 OR x3) - has 3 solutions
    std::vector<std::vector<Literal>> f2 = {
        {Literal(2, true), Literal(3, true)}
    };
    
    std::vector<uint32_t> q = {1, 1};  // Both need >= 2 solutions
    
    Solver solver(0.01);
    bool result = solver.solve({}, {f1, f2}, q, 4);
    
    print_result("#(x0 AND x1) >= 2 AND #(x2 OR x3) >= 2", false, result);
}

void test_simple_or() {
    std::cout << "\nTest Case 2: Simple OR Formula - SAT case\n";
    
    // f1: (x0 OR x1) - has 3 solutions: (1,0), (0,1), (1,1)
    std::vector<std::vector<Literal>> f1 = {
        {Literal(0, true), Literal(1, true)}
    };
    
    // Need >= 2 solutions (SAT since 3 > 2)
    std::vector<uint32_t> q = {1};  // 2^1 = 2 solutions needed
    
    Solver solver(0.01);
    bool result = solver.solve({}, {f1}, q, 2);
    
    print_result("#(x0 OR x1) >= 2", true, result);
}

void test_two_ors() {
    std::cout << "\nTest Case 3: Two OR Formulas - SAT case\n";
    
    // f1: (x0 OR x1) - has 3 solutions
    std::vector<std::vector<Literal>> f1 = {
        {Literal(0, true), Literal(1, true)}
    };
    
    // f2: (x2 OR x3) - has 3 solutions
    std::vector<std::vector<Literal>> f2 = {
        {Literal(2, true), Literal(3, true)}
    };
    
    // Both need >= 2 solutions (SAT since both have 3 solutions)
    std::vector<uint32_t> q = {1, 1};
    
    Solver solver(0.01);
    bool result = solver.solve({}, {f1, f2}, q, 4);
    
    print_result("#(x0 OR x1) >= 2 AND #(x2 OR x3) >= 2", true, result);
}

int main() {
    test_simple_and_or();  // UNSAT case
    test_simple_or();      // SAT case
    test_two_ors();       // SAT case with multiple constraints
    return 0;
}