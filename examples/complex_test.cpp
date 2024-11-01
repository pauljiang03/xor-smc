#include "xor_smc/Solver.hpp"
#include <iostream>
#include <vector>

using namespace xor_smc;

void print_result(const std::string& test_name, bool result) {
    std::cout << "\n=== " << test_name << " ===\n";
    std::cout << "Result: " << (result ? "SAT" : "UNSAT") << "\n\n";
}

void test_pigeon_hole() {
    //should be unsat

    std::cout << "Testing Pigeonhole Principle (3 pigeons, 2 holes)\n";
    
    Solver solver;
    solver.set_num_variables(6);  
    
    solver.add_clause({Literal(0, true), Literal(1, true)});   
    solver.add_clause({Literal(2, true), Literal(3, true)});   
    solver.add_clause({Literal(4, true), Literal(5, true)});   
    
    solver.add_clause({Literal(0, false), Literal(2, false)}); 
    solver.add_clause({Literal(0, false), Literal(4, false)});
    solver.add_clause({Literal(2, false), Literal(4, false)}); 
    
    solver.add_clause({Literal(1, false), Literal(3, false)});
    solver.add_clause({Literal(1, false), Literal(5, false)}); 
    solver.add_clause({Literal(3, false), Literal(5, false)}); 
    
    print_result("Pigeonhole Problem", solver.solve());
}

void test_graph_coloring() {
    std::cout << "Testing Graph Coloring (K4 with 2 colors)\n";
    //unsat
    Solver solver;
    solver.set_num_variables(8);
    
    solver.add_clause({Literal(0, true), Literal(1, true)});  
    solver.add_clause({Literal(2, true), Literal(3, true)});  
    solver.add_clause({Literal(4, true), Literal(5, true)});  
    solver.add_clause({Literal(6, true), Literal(7, true)});  
    
    solver.add_clause({Literal(0, false), Literal(1, false)}); 
    solver.add_clause({Literal(2, false), Literal(3, false)}); 
    solver.add_clause({Literal(4, false), Literal(5, false)}); 
    solver.add_clause({Literal(6, false), Literal(7, false)}); 
    
    solver.add_clause({Literal(0, false), Literal(2, false)}); 
    solver.add_clause({Literal(1, false), Literal(3, false)}); 
    
    solver.add_clause({Literal(0, false), Literal(4, false)});
    solver.add_clause({Literal(1, false), Literal(5, false)});
    
    solver.add_clause({Literal(0, false), Literal(6, false)});
    solver.add_clause({Literal(1, false), Literal(7, false)});
    
    solver.add_clause({Literal(2, false), Literal(4, false)});
    solver.add_clause({Literal(3, false), Literal(5, false)});
    
    solver.add_clause({Literal(2, false), Literal(6, false)});
    solver.add_clause({Literal(3, false), Literal(7, false)});
    
    solver.add_clause({Literal(4, false), Literal(6, false)});
    solver.add_clause({Literal(5, false), Literal(7, false)});
    
    print_result("Graph Coloring Problem", solver.solve());
}

void test_sudoku_constraints() {
    std::cout << "Testing Small Sudoku Constraints\n";
    
    Solver solver;

    solver.set_num_variables(16);
    
    solver.add_clause({Literal(0, true), Literal(1, true), Literal(2, true), Literal(3, true)});   // cell 00
    solver.add_clause({Literal(4, true), Literal(5, true), Literal(6, true), Literal(7, true)});   // cell 01
    solver.add_clause({Literal(8, true), Literal(9, true), Literal(10, true), Literal(11, true)}); // cell 10
    solver.add_clause({Literal(12, true), Literal(13, true), Literal(14, true), Literal(15, true)}); // cell 11
    
 
    for (int v = 0; v < 4; v++) {
        solver.add_clause({Literal(v, false), Literal(v+4, false)}); // cells 00 and 01
    }
    
    for (int v = 0; v < 4; v++) {
        solver.add_clause({Literal(v+8, false), Literal(v+12, false)}); 
    }
    
    solver.add_clause({Literal(0, true)}); 
    
    print_result("Sudoku Constraints", solver.solve());
}

void test_queens(int n = 4) {
    std::cout << "Testing " << n << "-Queens Problem\n";
    
    Solver solver;
    solver.set_num_variables(n * n);
    
    for (int r = 0; r < n; r++) {
        std::vector<Literal> row_clause;
        for (int c = 0; c < n; c++) {
            row_clause.push_back(Literal(r * n + c, true));
        }
        solver.add_clause(row_clause);
    }
    
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
    
    for (int r1 = 0; r1 < n; r1++) {
        for (int c1 = 0; c1 < n; c1++) {
            for (int i = 1; r1 + i < n && c1 + i < n; i++) {
                solver.add_clause({
                    Literal(r1 * n + c1, false),
                    Literal((r1 + i) * n + (c1 + i), false)
                });
            }
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

void test_hard_unsat() {
    std::cout << "\nRunning test_hard_unsat (Pure Pigeonhole)\n";
    
    const int N = 8;  
    Solver solver;
    
    solver.set_num_variables((N + 1) * N);
    
    auto pigeon_var = [&](int pigeon, int hole) -> uint32_t {
        return pigeon * N + hole;
    };
    
    for (int p = 0; p < N + 1; p++) {
        std::vector<Literal> clause;
        for (int h = 0; h < N; h++) {
            clause.push_back(Literal(pigeon_var(p, h), true));
        }
        solver.add_clause(clause);
    }
    
    for (int h = 0; h < N; h++) {
        for (int p1 = 0; p1 < N + 1; p1++) {
            for (int p2 = p1 + 1; p2 < N + 1; p2++) {
                solver.add_clause({
                    Literal(pigeon_var(p1, h), false),
                    Literal(pigeon_var(p2, h), false)
                });
            }
        }
    }
    
    bool result = solver.solve();
    std::cout << "Hard UNSAT Result: " << (result ? "SAT" : "UNSAT") << "\n";
    assert(!result); // should definitely be UNSAT
}

int main() {
    test_queens();
    test_pigeon_hole();
    test_graph_coloring();
    test_sudoku_constraints();
    test_hard_unsat();
    return 0;
}
