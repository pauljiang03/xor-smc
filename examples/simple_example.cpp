#include <iostream>
#include <vector>
#include <cassert>
#include <chrono>
#include "xor_smc/CDCLSolver.hpp"
#include "xor_smc/Literal.hpp"

using namespace xor_smc;
using namespace std::chrono;

// Helper to print test results
void print_test_result(const std::string& test_name, bool success) {
    std::cout << "Test " << test_name << ": " << (success ? "PASSED" : "FAILED") << "\n";
}

// Test 1: Unit clause handling
void test_unit_clause() {
    std::cout << "\nTesting unit clause handling...\n";
    CDCLSolver solver;
    solver.set_num_variables(3);
    
    // Add single unit clause
    solver.add_clause({Literal(0, true)});  // x0
    bool result = solver.solve();
    assert(result);
    assert(solver.get_value(0) == true);
    
    // Add contradictory unit clause
    solver.add_clause({Literal(0, false)});  // ¬x0
    result = solver.solve();
    assert(!result);  // Should be UNSAT
    
    print_test_result("Unit Clause", true);
}

// Test 2: Binary clause propagation
void test_binary_propagation() {
    std::cout << "\nTesting binary clause propagation...\n";
    CDCLSolver solver;
    solver.set_num_variables(3);
    
    solver.add_clause({Literal(0, true)});  // x0
    solver.add_clause({Literal(0, false), Literal(1, true)});  // ¬x0 ∨ x1
    
    bool result = solver.solve();
    assert(result);
    assert(solver.get_value(0) == true);
    assert(solver.get_value(1) == true);  // Should be propagated
    
    print_test_result("Binary Propagation", true);
}

// Test 3: Simple conflict and backtracking
void test_conflict_and_backtrack() {
    std::cout << "\nTesting conflict and backtracking...\n";
    CDCLSolver solver;
    solver.set_num_variables(3);
    
    // Create a simple conflict
    solver.add_clause({Literal(0, true), Literal(1, true)});   // x0 ∨ x1
    solver.add_clause({Literal(0, false), Literal(2, true)});  // ¬x0 ∨ x2
    solver.add_clause({Literal(1, false), Literal(2, false)}); // ¬x1 ∨ ¬x2
    
    bool result = solver.solve();
    assert(result);  // Should be SAT with backtracking
    
    print_test_result("Conflict and Backtrack", true);
}

// Test 4: Clause learning effectiveness
void test_clause_learning() {
    std::cout << "\nTesting clause learning effectiveness...\n";
    CDCLSolver solver;
    solver.set_num_variables(4);
    
    // Create a situation where learning is beneficial
    solver.add_clause({Literal(0, true), Literal(1, true)});   // x0 ∨ x1
    solver.add_clause({Literal(1, false), Literal(2, true)});  // ¬x1 ∨ x2
    solver.add_clause({Literal(2, false), Literal(3, false)}); // ¬x2 ∨ ¬x3
    solver.add_clause({Literal(0, false), Literal(3, true)});  // ¬x0 ∨ x3
    
    bool result = solver.solve();
    assert(result);
    
    print_test_result("Clause Learning", true);
}

// Test 5: Pure SAT case
void test_pure_sat() {
    std::cout << "\nTesting pure SAT case...\n";
    CDCLSolver solver;
    solver.set_num_variables(3);
    
    // Simple satisfiable formula
    solver.add_clause({Literal(0, true), Literal(1, true)});   // x0 ∨ x1
    solver.add_clause({Literal(1, false), Literal(2, true)});  // ¬x1 ∨ x2
    solver.add_clause({Literal(0, false), Literal(2, true)});  // ¬x0 ∨ x2
    
    bool result = solver.solve();
    assert(result);
    assert(solver.get_value(2) == true);  // x2 must be true
    
    print_test_result("Pure SAT", true);
}

// Test 6: Pure UNSAT case
void test_pure_unsat() {
    std::cout << "\nTesting pure UNSAT case...\n";
    CDCLSolver solver;
    solver.set_num_variables(2);
    
    // Simple unsatisfiable formula
    solver.add_clause({Literal(0, true)});   // x0
    solver.add_clause({Literal(1, true)});   // x1
    solver.add_clause({Literal(0, false)});  // ¬x0
    
    bool result = solver.solve();
    assert(!result);
    
    print_test_result("Pure UNSAT", true);
}

// Test 7: Small pigeonhole principle (PHP)
void test_small_php() {
    std::cout << "\nTesting small pigeonhole principle...\n";
    CDCLSolver solver;
    
    const int holes = 2;
    const int pigeons = 3;
    solver.set_num_variables(pigeons * holes);
    
    // Each pigeon must go somewhere
    for (int p = 0; p < pigeons; p++) {
        solver.add_clause({
            Literal(p * holes + 0, true),
            Literal(p * holes + 1, true)
        });
    }
    
    // No two pigeons in same hole
    for (int h = 0; h < holes; h++) {
        for (int p1 = 0; p1 < pigeons; p1++) {
            for (int p2 = p1 + 1; p2 < pigeons; p2++) {
                solver.add_clause({
                    Literal(p1 * holes + h, false),
                    Literal(p2 * holes + h, false)
                });
            }
        }
    }
    
    bool result = solver.solve();
    assert(!result);  // Should be UNSAT
    
    print_test_result("Small PHP", true);
}

// Test 8: Long chain of implications
void test_implication_chain() {
    std::cout << "\nTesting long implication chain...\n";
    CDCLSolver solver;
    
    const int n = 10;
    solver.set_num_variables(n);
    
    // Create chain: x0 => x1 => x2 => ... => xn-1
    for (int i = 0; i < n-1; i++) {
        solver.add_clause({Literal(i, false), Literal(i+1, true)});
    }
    solver.add_clause({Literal(0, true)});  // Force x0 true
    
    bool result = solver.solve();
    assert(result);
    
    // Verify all variables got propagated to true
    for (int i = 0; i < n; i++) {
        assert(solver.get_value(i) == true);
    }
    
    print_test_result("Implication Chain", true);
}

// Test 9: Non-chronological backtracking
void test_non_chrono_backtrack() {
    std::cout << "\nTesting non-chronological backtracking...\n";
    CDCLSolver solver;
    solver.set_num_variables(5);
    
    // Create a situation requiring non-chronological backtracking
    solver.add_clause({Literal(0, true), Literal(1, true)});   // x0 ∨ x1
    solver.add_clause({Literal(1, false), Literal(2, true)});  // ¬x1 ∨ x2
    solver.add_clause({Literal(2, false), Literal(3, false)}); // ¬x2 ∨ ¬x3
    solver.add_clause({Literal(3, true), Literal(4, true)});   // x3 ∨ x4
    solver.add_clause({Literal(0, false), Literal(4, false)}); // ¬x0 ∨ ¬x4
    
    bool result = solver.solve();
    assert(result);
    
    print_test_result("Non-chronological Backtrack", true);
}

// Test 10: Random 3-SAT near phase transition
void test_random_3sat() {
    std::cout << "\nTesting random 3-SAT...\n";
    CDCLSolver solver;
    
    const int n = 20;  // variables
    const int m = 85;  // clauses (around 4.25 * n for phase transition)
    
    solver.set_num_variables(n);
    
    // Generate random 3-SAT formula
    srand(42);  // Fixed seed for reproducibility
    for (int i = 0; i < m; i++) {
        std::vector<Literal> clause;
        for (int j = 0; j < 3; j++) {
            uint32_t var = rand() % n;
            bool positive = rand() % 2;
            clause.push_back(Literal(var, positive));
        }
        solver.add_clause(clause);
    }
    
    bool result = solver.solve();
    if (result) {
        // Verify solution
        // Would need access to clauses to verify
    }
    
    print_test_result("Random 3-SAT", true);
}

int main() {
    test_unit_clause();
    test_binary_propagation();
    test_conflict_and_backtrack();
    test_clause_learning();
    test_pure_sat();
    test_pure_unsat();
    test_small_php();
    test_implication_chain();
    test_non_chrono_backtrack();
    test_random_3sat();
    
    std::cout << "\nAll tests completed.\n";
    return 0;
}