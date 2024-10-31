#include <iostream>
#include <vector>
#include <cassert>
#include <chrono>
#include "xor_smc/Solver.hpp"
#include "xor_smc/Literal.hpp"

using namespace xor_smc;
using namespace std::chrono;

void print_result(const std::string& test_name, bool success) {
    std::cout << test_name << ": " << (success ? "PASSED ✓" : "FAILED ✗") << "\n";
}

// Test 1: XOR Chain with a Twist
// Creates XOR relationships but with additional constraints
void test_xor_chain_with_twist() {
    std::cout << "\nRunning XOR Chain with Twist test...\n";
    Solver solver;
    const int n = 5;
    solver.set_num_variables(n);
    
    // Create XOR chain: x0 ⊕ x1 ⊕ x2 ⊕ x3 ⊕ x4 = 1
    // Using CNF encoding of XOR
    for (int i = 0; i < n-1; i++) {
        solver.add_clause({Literal(i, false), Literal(i+1, false)});
        solver.add_clause({Literal(i, true), Literal(i+1, true)});
    }
    
    // Add twist: x0 ∨ x2, ¬x1 ∨ x3, x2 ∨ ¬x4
    solver.add_clause({Literal(0, true), Literal(2, true)});
    solver.add_clause({Literal(1, false), Literal(3, true)});
    solver.add_clause({Literal(2, true), Literal(4, false)});
    
    bool result = solver.solve();
    assert(result); // Should be SAT
    print_result("XOR Chain with Twist", result);
}

// Test 2: Circular Dependencies
// Creates a cycle of implications with escape clauses
void test_circular_dependencies() {
    std::cout << "\nRunning Circular Dependencies test...\n";
    Solver solver;
    solver.set_num_variables(4);
    
    // Create circle: x0 => x1 => x2 => x3 => x0
    solver.add_clause({Literal(0, false), Literal(1, true)});
    solver.add_clause({Literal(1, false), Literal(2, true)});
    solver.add_clause({Literal(2, false), Literal(3, true)});
    solver.add_clause({Literal(3, false), Literal(0, true)});
    
    // Add escape clauses
    solver.add_clause({Literal(0, true), Literal(2, true)});
    solver.add_clause({Literal(1, true), Literal(3, true)});
    
    bool result = solver.solve();
    assert(result); // Should be SAT
    print_result("Circular Dependencies", result);
}

// Test 3: Almost-Pigeonhole
// Like pigeonhole but with one escape clause
void test_almost_pigeonhole() {
    std::cout << "\nRunning Almost-Pigeonhole test...\n";
    Solver solver;
    const int holes = 3;
    const int pigeons = 4;
    solver.set_num_variables(pigeons * holes);
    
    auto var = [holes](int p, int h) -> uint32_t {
        return p * holes + h;
    };
    
    // Each pigeon in a hole
    for (int p = 0; p < pigeons; p++) {
        std::vector<Literal> clause;
        for (int h = 0; h < holes; h++) {
            clause.push_back(Literal(var(p, h), true));
        }
        solver.add_clause(clause);
    }
    
    // At most one pigeon per hole (except for hole 0)
    for (int h = 1; h < holes; h++) {
        for (int p1 = 0; p1 < pigeons; p1++) {
            for (int p2 = p1 + 1; p2 < pigeons; p2++) {
                solver.add_clause({
                    Literal(var(p1, h), false),
                    Literal(var(p2, h), false)
                });
            }
        }
    }
    
    bool result = solver.solve();
    assert(result); // Should be SAT due to the missing constraint
    print_result("Almost-Pigeonhole", result);
}

// Test 4: Cascading Unit Propagation
void test_cascading_propagation() {
    std::cout << "\nRunning Cascading Propagation test...\n";
    Solver solver;
    const int n = 8;
    solver.set_num_variables(n);
    
    // Create cascade: x0 => x1 => x2 => ... with side constraints
    for (int i = 0; i < n-1; i++) {
        solver.add_clause({Literal(i, false), Literal(i+1, true)});
        if (i % 2 == 0) {
            solver.add_clause({Literal(i, true), Literal(i+1, true)});
        }
    }
    
    // Force start condition
    solver.add_clause({Literal(0, true)});
    
    bool result = solver.solve();
    assert(result);
    assert(solver.get_value(n-1) == true); // Last variable must be true
    print_result("Cascading Propagation", result);
}

// Test 5: Diamond Structure
// Creates a diamond-shaped dependency graph
void test_diamond_structure() {
    std::cout << "\nRunning Diamond Structure test...\n";
    Solver solver;
    solver.set_num_variables(4);
    
    // x0 => x1 and x2
    solver.add_clause({Literal(0, false), Literal(1, true)});
    solver.add_clause({Literal(0, false), Literal(2, true)});
    
    // x1 and x2 => x3
    solver.add_clause({Literal(1, false), Literal(3, true)});
    solver.add_clause({Literal(2, false), Literal(3, true)});
    
    // Force x0 true and x3 false - should be UNSAT
    solver.add_clause({Literal(0, true)});
    solver.add_clause({Literal(3, false)});
    
    bool result = solver.solve();
    assert(!result); // Should be UNSAT
    print_result("Diamond Structure", !result);
}

// Test 6: Nested Implications
void test_nested_implications() {
    std::cout << "\nRunning Nested Implications test...\n";
    Solver solver;
    solver.set_num_variables(6);
    
    // (x0 => x1) => (x2 => x3) => (x4 => x5)
    solver.add_clause({Literal(0, false), Literal(1, true)});
    solver.add_clause({Literal(1, false), Literal(2, false), Literal(3, true)});
    solver.add_clause({Literal(3, false), Literal(4, false), Literal(5, true)});
    
    // Add constraints to force interesting path
    solver.add_clause({Literal(0, true)});
    solver.add_clause({Literal(2, true)});
    solver.add_clause({Literal(4, true)});
    
    bool result = solver.solve();
    assert(result);
    assert(solver.get_value(5) == true); // x5 must be true
    print_result("Nested Implications", result);
}

// Test 7: Multiple Unit Clauses with Conflict
void test_multiple_units() {
    std::cout << "\nRunning Multiple Units test...\n";
    Solver solver;
    solver.set_num_variables(5);
    
    // Add several unit clauses
    solver.add_clause({Literal(0, true)});
    solver.add_clause({Literal(1, false)});
    solver.add_clause({Literal(2, true)});
    
    // Add implications that will conflict
    solver.add_clause({Literal(0, false), Literal(1, true), Literal(3, true)});
    solver.add_clause({Literal(2, false), Literal(3, false)});
    solver.add_clause({Literal(3, true), Literal(4, true)});
    solver.add_clause({Literal(4, false)});
    
    bool result = solver.solve();
    assert(!result); // Should be UNSAT
    print_result("Multiple Units", !result);
}

// Test 8: Sliding Window Constraints
void test_sliding_window() {
    std::cout << "\nRunning Sliding Window test...\n";
    Solver solver;
    const int n = 6;
    const int window = 3;
    solver.set_num_variables(n);
    
    // At most one true in each window of size 3
    for (int i = 0; i <= n - window; i++) {
        for (int j = i; j < i + window; j++) {
            for (int k = j + 1; k < i + window; k++) {
                solver.add_clause({Literal(j, false), Literal(k, false)});
            }
        }
    }
    
    // Force two variables true that should conflict
    solver.add_clause({Literal(1, true)});
    solver.add_clause({Literal(2, true)});
    
    bool result = solver.solve();
    assert(!result); // Should be UNSAT
    print_result("Sliding Window", !result);
}

int main() {
    test_xor_chain_with_twist();
    test_circular_dependencies();
    test_almost_pigeonhole();
    test_cascading_propagation();
    test_diamond_structure();
    test_nested_implications();
    test_multiple_units();
    test_sliding_window();
    
    std::cout << "\nAll verification tests completed.\n";
    return 0;
}