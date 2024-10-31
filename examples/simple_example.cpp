#include <iostream>
#include <vector>
#include <cassert>
#include "xor_smc/CDCLSolver.hpp"
#include "xor_smc/Literal.hpp"

using namespace xor_smc;

// Helper function to print test results
void print_result(const std::string& test_name, bool success) {
    std::cout << "Test " << test_name << ": " << (success ? "PASSED" : "FAILED") << "\n";
}

// Test 1: Simple UNSAT case requiring conflict learning
void test_conflicting_implications() {
    std::cout << "\nRunning test_conflicting_implications\n";
    CDCLSolver solver;
    solver.set_num_variables(3);
    
    // x0 => x1, x0 => ¬x1 (contradiction when x0 is true)
    solver.add_clause({Literal(0, false), Literal(1, true)});   // ¬x0 ∨ x1
    solver.add_clause({Literal(0, false), Literal(1, false)});  // ¬x0 ∨ ¬x1
    solver.add_clause({Literal(0, true)});                      // x0
    
    bool result = solver.solve();
    print_result("conflicting_implications", !result);  // Should be UNSAT
}

// Test 2: Complex SAT case requiring multiple unit propagations
void test_chain_propagation() {
    std::cout << "\nRunning test_chain_propagation\n";
    CDCLSolver solver;
    solver.set_num_variables(5);
    
    // Create a chain of implications: x0 => x1 => x2 => x3 => x4
    solver.add_clause({Literal(0, false), Literal(1, true)});   // ¬x0 ∨ x1
    solver.add_clause({Literal(1, false), Literal(2, true)});   // ¬x1 ∨ x2
    solver.add_clause({Literal(2, false), Literal(3, true)});   // ¬x2 ∨ x3
    solver.add_clause({Literal(3, false), Literal(4, true)});   // ¬x3 ∨ x4
    solver.add_clause({Literal(0, true)});                      // x0
    
    bool result = solver.solve();
    if (result) {
        // Check propagation worked correctly
        assert(solver.get_value(0) == true);
        assert(solver.get_value(1) == true);
        assert(solver.get_value(2) == true);
        assert(solver.get_value(3) == true);
        assert(solver.get_value(4) == true);
    }
    print_result("chain_propagation", result);  // Should be SAT
}

// Test 3: UNSAT case requiring multiple conflict clauses
void test_complex_unsat() {
    std::cout << "\nRunning test_complex_unsat\n";
    CDCLSolver solver;
    solver.set_num_variables(4);
    
    // Create a complex UNSAT formula
    solver.add_clause({Literal(0, true), Literal(1, true)});     // x0 ∨ x1
    solver.add_clause({Literal(0, false), Literal(2, true)});    // ¬x0 ∨ x2
    solver.add_clause({Literal(1, false), Literal(2, true)});    // ¬x1 ∨ x2
    solver.add_clause({Literal(2, false), Literal(3, true)});    // ¬x2 ∨ x3
    solver.add_clause({Literal(2, false), Literal(3, false)});   // ¬x2 ∨ ¬x3
    
    bool result = solver.solve();
    print_result("complex_unsat", !result);  // Should be UNSAT
}

// Test 4: SAT case requiring non-chronological backtracking
void test_backjumping() {
    std::cout << "\nRunning test_backjumping\n";
    CDCLSolver solver;
    solver.set_num_variables(6);
    
    // Create a formula where naive DPLL would take exponential time
    solver.add_clause({Literal(0, true), Literal(1, true)});     // x0 ∨ x1
    solver.add_clause({Literal(2, true), Literal(3, true)});     // x2 ∨ x3
    solver.add_clause({Literal(4, true), Literal(5, true)});     // x4 ∨ x5
    solver.add_clause({Literal(1, false), Literal(3, false), Literal(5, false)});  // ¬x1 ∨ ¬x3 ∨ ¬x5
    
    bool result = solver.solve();
    print_result("backjumping", result);  // Should be SAT
}

// Test 5: Complex SAT case with many interdependencies
void test_interdependent_variables() {
    std::cout << "\nRunning test_interdependent_variables\n";
    CDCLSolver solver;
    solver.set_num_variables(5);
    
    // Create interdependencies between variables
    solver.add_clause({Literal(0, true), Literal(1, true)});     // x0 ∨ x1
    solver.add_clause({Literal(1, true), Literal(2, true)});     // x1 ∨ x2
    solver.add_clause({Literal(2, true), Literal(3, true)});     // x2 ∨ x3
    solver.add_clause({Literal(3, true), Literal(4, true)});     // x3 ∨ x4
    solver.add_clause({Literal(0, false), Literal(4, false)});   // ¬x0 ∨ ¬x4
    solver.add_clause({Literal(1, false), Literal(3, false)});   // ¬x1 ∨ ¬x3
    
    bool result = solver.solve();
    print_result("interdependent_variables", result);  // Should be SAT
}

// Test 6: Pigeonhole principle (UNSAT)
void test_pigeonhole() {
    std::cout << "\nRunning test_pigeonhole\n";
    CDCLSolver solver;
    
    // Encode putting 4 pigeons in 3 holes (impossible)
    const int pigeons = 4;
    const int holes = 3;
    solver.set_num_variables(pigeons * holes);
    
    // Each pigeon must be in at least one hole
    for (int p = 0; p < pigeons; p++) {
        std::vector<Literal> clause;
        for (int h = 0; h < holes; h++) {
            clause.push_back(Literal(p * holes + h, true));
        }
        solver.add_clause(clause);
    }
    
    // No two pigeons in the same hole
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
    print_result("pigeonhole", !result);  // Should be UNSAT
}

int main() {
    test_conflicting_implications();
    test_chain_propagation();
    test_complex_unsat();
    test_backjumping();
    test_interdependent_variables();
    test_pigeonhole();
    
    return 0;
}