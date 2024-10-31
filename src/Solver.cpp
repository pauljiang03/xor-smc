#include "xor_smc/Solver.hpp"
#include <cmath>
#include <algorithm>
#include <cassert>
#include <set>  

namespace xor_smc {

Solver::Solver(double eta) 
    : rng_(std::random_device{}())
    , eta_(eta)
    , debug_(true) {
    if (debug_) {
        std::cout << "Creating SMC solver with η = " << eta << "\n";
    }
}

uint32_t Solver::count_actual_solutions(
    const std::vector<std::vector<Literal>>& formula, 
    uint32_t num_vars) const {
    
    if (debug_) {
        std::cout << "Counting solutions for formula with " << formula.size() << " clauses:\n";
        for (const auto& clause : formula) {
            std::cout << "  (";
            for (const auto& lit : clause) {
                std::cout << (lit.is_positive() ? "" : "¬") << "x" << lit.var_id() << " ";
            }
            std::cout << ")\n";
        }
    }

    uint32_t count = 0;
    const uint32_t max_assignments = 1u << num_vars;

    // Try all possible assignments
    for (uint32_t i = 0; i < max_assignments; i++) {
        std::vector<bool> assignment(num_vars);
        // Convert integer to binary assignment
        for (uint32_t j = 0; j < num_vars; j++) {
            assignment[j] = (i & (1u << j)) != 0;
        }

        // Check if this assignment satisfies all clauses
        bool satisfied = true;
        for (const auto& clause : formula) {
            bool clause_satisfied = false;
            for (const auto& lit : clause) {
                if (assignment[lit.var_id()] == lit.is_positive()) {
                    clause_satisfied = true;
                    break;
                }
            }
            if (!clause_satisfied) {
                satisfied = false;
                break;
            }
        }

        if (satisfied) {
            count++;
            if (debug_) {
                std::cout << "Found solution " << count << ": ";
                for (uint32_t j = 0; j < num_vars; j++) {
                    std::cout << "x" << j << "=" << assignment[j] << " ";
                }
                std::cout << "\n";
            }
        }
    }

    if (debug_) {
        std::cout << "Total solutions found: " << count << "\n";
    }

    return count;
}

void print_formula(const std::vector<std::vector<Literal>>& formula) {
    std::cout << "Formula:\n";
    for (const auto& clause : formula) {
        std::cout << "(";
        for (size_t i = 0; i < clause.size(); i++) {
            if (i > 0) std::cout << " ∨ ";
            std::cout << (clause[i].is_positive() ? "" : "¬") 
                     << "x" << clause[i].var_id();
        }
        std::cout << ")\n";
    }
}

std::vector<std::vector<Literal>> Solver::generate_xor_constraints(
    const std::vector<std::vector<Literal>>& formula,
    uint32_t num_vars,
    uint32_t num_xors) {
    
    if (debug_) {
        std::cout << "Generating " << num_xors << " XOR constraints for formula:\n";
        print_formula(formula);
    }

    std::vector<std::vector<Literal>> xor_cnf;
    
    // First collect variables that appear in the formula
    std::vector<uint32_t> relevant_vars;
    {
        std::vector<bool> var_used(num_vars, false);
        for (const auto& clause : formula) {
            for (const auto& lit : clause) {
                var_used[lit.var_id()] = true;
            }
        }
        for (uint32_t i = 0; i < num_vars; i++) {
            if (var_used[i]) {
                relevant_vars.push_back(i);
            }
        }
    }
    
    if (debug_) {
        std::cout << "Formula variables: ";
        for (auto v : relevant_vars) {
            std::cout << "x" << v << " ";
        }
        std::cout << "\n";
    }

    // If no variables in formula, can't generate XOR constraints
    if (relevant_vars.empty()) {
        if (debug_) {
            std::cout << "No variables in formula - can't generate XOR constraints\n";
        }
        return xor_cnf;
    }
    
    // For each XOR constraint
    for (uint32_t i = 0; i < num_xors; i++) {
        std::vector<uint32_t> vars;
        std::uniform_int_distribution<> inclusion(0, 1);
        
        // Include each variable with 50% probability
        for (uint32_t v : relevant_vars) {
            if (inclusion(rng_)) {
                vars.push_back(v);
            }
        }
        
        // Ensure at least one variable
        if (vars.empty()) {
            std::uniform_int_distribution<> var_dis(0, relevant_vars.size() - 1);
            vars.push_back(relevant_vars[var_dis(rng_)]);
        }

        // Random right-hand side
        std::uniform_int_distribution<> rhs_dis(0, 1);
        bool rhs = rhs_dis(rng_);
        
        if (debug_) {
            std::cout << "XOR constraint " << i + 1 << ": ";
            for (auto v : vars) std::cout << "x" << v << " ⊕ ";
            std::cout << " = " << rhs << "\n";
        }

        // Convert XOR to CNF
        size_t n = vars.size();
        for (size_t j = 0; j < (1u << n); j++) {
            size_t ones = 0;
            for (size_t k = 0; k < n; k++) {
                if (j & (1u << k)) ones++;
            }
            
            if ((ones % 2) != rhs) {
                std::vector<Literal> clause;
                for (size_t k = 0; k < n; k++) {
                    bool positive = (j & (1u << k)) == 0;
                    clause.push_back(Literal(vars[k], positive));
                }
                xor_cnf.push_back(clause);
            }
        }
    }
    
    if (debug_) {
        std::cout << "Generated " << xor_cnf.size() << " XOR CNF clauses\n";
    }
    
    return xor_cnf;
}

bool Solver::solve_with_xor(
    CDCLSolver& solver,
    const std::vector<std::vector<Literal>>& formula,
    uint32_t num_vars,
    uint32_t num_xors) {
    
    if (debug_) {
        std::cout << "Testing if formula has >= 2^" << num_xors << " solutions\n";
    }

    // Add original formula
    for (const auto& clause : formula) {
        solver.add_clause(clause);
    }
    
    if (!solver.solve()) {
        if (debug_) {
            std::cout << "Base formula is UNSAT\n";
        }
        return false;
    }

    uint32_t successes = 0;
    const uint32_t NUM_TRIES = 10;

    // Try multiple times with different XOR constraints
    for (uint32_t try_idx = 0; try_idx < NUM_TRIES; try_idx++) {
        if (debug_) {
            std::cout << "Try " << try_idx + 1 << "/" << NUM_TRIES << "\n";
        }

        CDCLSolver test_solver;
        test_solver.set_num_variables(num_vars);
        
        // Add original formula
        for (const auto& clause : formula) {
            test_solver.add_clause(clause);
        }
        
        // Add all XOR constraints at once
        auto xor_cnf = generate_xor_constraints(formula, num_vars, num_xors);
        if (debug_) {
            std::cout << "Generated " << xor_cnf.size() << " XOR CNF clauses\n";
        }

        // Check satisfiability with XOR constraints
        bool satisfied = true;
        for (const auto& clause : xor_cnf) {
            test_solver.add_clause(clause);
            if (!test_solver.solve()) {
                satisfied = false;
                break;
            }
        }
        
        if (satisfied) {
            successes++;
            if (debug_) {
                std::cout << "Trial succeeded\n";
            }
        } else {
            if (debug_) {
                std::cout << "Trial failed\n";
            }
        }
    }
    
    // More stringent requirement - require 90% success rate
    bool result = (successes >= 9 * NUM_TRIES / 10);
    if (debug_) {
        std::cout << (result ? "SAT" : "UNSAT") 
                  << " with XORs - found solutions in "
                  << successes << "/" << NUM_TRIES << " tries\n";
    }
    
    return result;
}

bool Solver::solve(
    const std::vector<std::vector<Literal>>& phi,
    const std::vector<std::vector<std::vector<Literal>>>& f,
    const std::vector<uint32_t>& q,
    uint32_t n_vars) {
    
    if (debug_) {
        std::cout << "\nSolving SMC problem with " << f.size() 
                  << " counting constraints\n";
    }
    
    if (f.size() != q.size()) {
        std::cout << "Error: Number of constraints doesn't match thresholds\n";
        return false;
    }
    
    // Check each counting constraint
    for (size_t i = 0; i < f.size(); i++) {
        if (debug_) {
            std::cout << "\nChecking constraint " << i + 1 << " of " << f.size() << "\n";
        }
        
        CDCLSolver test_solver;
        test_solver.set_num_variables(n_vars);
        
        // Add main formula
        for (const auto& clause : phi) {
            test_solver.add_clause(clause);
        }
        
        if (!solve_with_xor(test_solver, f[i], n_vars, q[i])) {
            if (debug_) {
                std::cout << "Constraint " << i + 1 << " failed\n";
            }
            return false;
        }
    }
    
    // Final satisfiability check of main formula
    CDCLSolver solver;
    solver.set_num_variables(n_vars);
    for (const auto& clause : phi) {
        solver.add_clause(clause);
    }
    return solver.solve();
}

} 