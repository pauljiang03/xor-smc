#include "xor_smc/CDCLSolver.hpp"
#include <iostream>
#include <vector>

using namespace xor_smc;

void print_result(const std::string& test_name, bool result) {
    std::cout << "\n=== " << test_name << " ===\n";
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n\n";
}

void test_pigeon_hole() {
    // Pigeonhole principle: Try to fit n+1 pigeons into n holes
    // Should be UNSAT as it's impossible
    std::cout << "Testing Pigeonhole Principle (3 pigeons, 2 holes)\n";
    
    CDCLSolver solver;
    solver.set_num_variables(6);  // p1h1, p1h2, p2h1, p2h2, p3h1, p3h2
    
    // Each pigeon must be in at least one hole
    solver.add_clause({Literal(0, true), Literal(1, true)});   // p1 in h1 or h2
    solver.add_clause({Literal(2, true), Literal(3, true)});   // p2 in h1 or h2
    solver.add_clause({Literal(4, true), Literal(5, true)});   // p3 in h1 or h2
    
    // No two pigeons in same hole
    // Hole 1
    solver.add_clause({Literal(0, false), Literal(2, false)}); // not(p1h1 and p2h1)
    solver.add_clause({Literal(0, false), Literal(4, false)}); // not(p1h1 and p3h1)
    solver.add_clause({Literal(2, false), Literal(4, false)}); // not(p2h1 and p3h1)
    
    // Hole 2
    solver.add_clause({Literal(1, false), Literal(3, false)}); // not(p1h2 and p2h2)
    solver.add_clause({Literal(1, false), Literal(5, false)}); // not(p1h2 and p3h2)
    solver.add_clause({Literal(3, false), Literal(5, false)}); // not(p2h2 and p3h2)
    
    print_result("Pigeonhole Problem", solver.solve());
}

void test_graph_coloring() {
    // Graph coloring: Try to color a graph with 3 colors
    // Example: Complete graph with 4 vertices (should be UNSAT with 2 colors)
    std::cout << "Testing Graph Coloring (K4 with 2 colors)\n";
    
    CDCLSolver solver;
    // Variables: v1c1, v1c2, v2c1, v2c2, v3c1, v3c2, v4c1, v4c2
    solver.set_num_variables(8);
    
    // Each vertex must have at least one color
    solver.add_clause({Literal(0, true), Literal(1, true)});  // v1
    solver.add_clause({Literal(2, true), Literal(3, true)});  // v2
    solver.add_clause({Literal(4, true), Literal(5, true)});  // v3
    solver.add_clause({Literal(6, true), Literal(7, true)});  // v4
    
    // Each vertex can't have both colors
    solver.add_clause({Literal(0, false), Literal(1, false)}); // v1
    solver.add_clause({Literal(2, false), Literal(3, false)}); // v2
    solver.add_clause({Literal(4, false), Literal(5, false)}); // v3
    solver.add_clause({Literal(6, false), Literal(7, false)}); // v4
    
    // Adjacent vertices must have different colors
    // v1-v2
    solver.add_clause({Literal(0, false), Literal(2, false)}); // not same color 1
    solver.add_clause({Literal(1, false), Literal(3, false)}); // not same color 2
    
    // v1-v3
    solver.add_clause({Literal(0, false), Literal(4, false)});
    solver.add_clause({Literal(1, false), Literal(5, false)});
    
    // v1-v4
    solver.add_clause({Literal(0, false), Literal(6, false)});
    solver.add_clause({Literal(1, false), Literal(7, false)});
    
    // v2-v3
    solver.add_clause({Literal(2, false), Literal(4, false)});
    solver.add_clause({Literal(3, false), Literal(5, false)});
    
    // v2-v4
    solver.add_clause({Literal(2, false), Literal(6, false)});
    solver.add_clause({Literal(3, false), Literal(7, false)});
    
    // v3-v4
    solver.add_clause({Literal(4, false), Literal(6, false)});
    solver.add_clause({Literal(5, false), Literal(7, false)});
    
    print_result("Graph Coloring Problem", solver.solve());
}

void test_sudoku_constraints() {
    // Just testing a small portion of Sudoku constraints (2x2 grid)
    std::cout << "Testing Small Sudoku Constraints\n";
    
    CDCLSolver solver;
    // Variables: cell_ij_v (i=row, j=col, v=value)
    // For 2x2, we need 16 variables (4 cells, 4 possible values each)
    solver.set_num_variables(16);
    
    // Each cell must have at least one value
    solver.add_clause({Literal(0, true), Literal(1, true), Literal(2, true), Literal(3, true)});   // cell 00
    solver.add_clause({Literal(4, true), Literal(5, true), Literal(6, true), Literal(7, true)});   // cell 01
    solver.add_clause({Literal(8, true), Literal(9, true), Literal(10, true), Literal(11, true)}); // cell 10
    solver.add_clause({Literal(12, true), Literal(13, true), Literal(14, true), Literal(15, true)}); // cell 11
    
    // Row constraints (no same value in same row)
    // First row
    for (int v = 0; v < 4; v++) {
        solver.add_clause({Literal(v, false), Literal(v+4, false)}); // cells 00 and 01
    }
    
    // Second row
    for (int v = 0; v < 4; v++) {
        solver.add_clause({Literal(v+8, false), Literal(v+12, false)}); // cells 10 and 11
    }
    
    // Add a known value to make it interesting
    solver.add_clause({Literal(0, true)}); // cell 00 must be value 0
    
    print_result("Sudoku Constraints", solver.solve());
}

void test_queens(int n = 4) {
    std::cout << "Testing " << n << "-Queens Problem\n";
    
    CDCLSolver solver;
    // Variables: qrc (queen at row r, column c)
    solver.set_num_variables(n * n);
    
    // At least one queen per row
    for (int r = 0; r < n; r++) {
        std::vector<Literal> row_clause;
        for (int c = 0; c < n; c++) {
            row_clause.push_back(Literal(r * n + c, true));
        }
        solver.add_clause(row_clause);
    }
    
    // No two queens in same column
    for (int c = 0; c < n; c++) {
        for (int r1 = 0; r1 < n; r1++) {
            for (int r2 = r1 + 1; r2 < n; r2++) {
                solver.add_clause({
                    Literal(r1 * n + c, false),
                    Literal(r2 * n + c, false)
                });
            }
        }
    }
    
    // No two queens on same diagonal
    for (int r1 = 0; r1 < n; r1++) {
        for (int c1 = 0; c1 < n; c1++) {
            // Primary diagonal
            for (int i = 1; r1 + i < n && c1 + i < n; i++) {
                solver.add_clause({
                    Literal(r1 * n + c1, false),
                    Literal((r1 + i) * n + (c1 + i), false)
                });
            }
            // Secondary diagonal
            for (int i = 1; r1 + i < n && c1 - i >= 0; i++) {
                solver.add_clause({
                    Literal(r1 * n + c1, false),
                    Literal((r1 + i) * n + (c1 - i), false)
                });
            }
        }
    }
    
    bool result = solver.solve();
    print_result("N-Queens Problem", result);
    
    if (result) {
        // Print solution
        std::cout << "Solution found:\n";
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) {
                std::cout << (solver.get_value(r * n + c) ? "Q " : ". ");
            }
            std::cout << "\n";
        }
    }
}

int main() {
    test_queens();
    test_pigeon_hole();
    test_graph_coloring();
    test_sudoku_constraints();
    return 0;
}
