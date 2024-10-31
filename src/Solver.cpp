#include "xor_smc/Solver.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cassert>

namespace xor_smc {

Solver::Solver(double eta) 
    : rng_(std::random_device{}())
    , eta_(eta)
    , debug_(true) {
    if (debug_) {
        std::cout << "Creating SMC solver with η = " << eta << "\n";
    }
}

std::vector<std::vector<Literal>> Solver::generate_xor_constraints(
    const std::vector<std::vector<Literal>>& formula,
    uint32_t num_vars,
    uint32_t num_xors) {
    
    std::vector<std::vector<Literal>> xor_cnf;
    
    // Check if base formula is satisfiable
    CDCLSolver test_solver;
    test_solver.set_num_variables(num_vars);
    for (const auto& clause : formula) {
        test_solver.add_clause(clause);
    }
    if (!test_solver.solve()) {
        if (debug_) {
            std::cout << "Base formula is UNSAT\n";
        }
        return xor_cnf;
    }

    // For each XOR constraint
    for (uint32_t i = 0; i < num_xors && num_vars > 0; i++) {
        if (debug_) {
            std::cout << "Generating XOR constraint " << i + 1 << "/" << num_xors << "\n";
        }

        // Randomly select variables for this XOR
        std::vector<uint32_t> vars;
        std::uniform_int_distribution<> inclusion(0, 1);
        
        for (uint32_t v = 0; v < num_vars; v++) {
            if (inclusion(rng_)) {
                vars.push_back(v);
            }
        }
        
        // Ensure at least one variable in XOR
        if (vars.empty() && num_vars > 0) {
            std::uniform_int_distribution<> var_dis(0, num_vars - 1);
            vars.push_back(var_dis(rng_));
        }

        // Random right-hand side
        std::uniform_int_distribution<> rhs_dis(0, 1);
        bool rhs = rhs_dis(rng_);

        if (debug_) {
            std::cout << "XOR vars:";
            for (auto v : vars) std::cout << " x" << v;
            std::cout << " = " << rhs << "\n";
        }

        // Convert XOR to CNF
        size_t n = vars.size();
        for (size_t j = 0; j < (1u << n); j++) {
            // Count ones in this assignment
            size_t ones = 0;
            for (size_t k = 0; k < n; k++) {
                if (j & (1u << k)) ones++;
            }
            
            // Add clause if parity doesn't match RHS
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

    return xor_cnf;
}

uint32_t Solver::compute_T(uint32_t n, uint32_t k) const {
    // Base case - no counting constraints
    if (k == 0) return 1;

    // Compute c = ⌈log₂(k+1) + 1⌉
    uint32_t c = static_cast<uint32_t>(std::ceil(std::log2(k + 1) + 1));
    
    // Compute α(c,k)
    double two_c = std::pow(2.0, c);
    double alpha = 0.5 * std::log(two_c / (k * 2.0 * two_c));
    
    if (alpha <= 0) alpha = 0.1;  // Ensure positive value
    
    // Compute T = ⌈((n+k)ln 2 - ln η)/α⌉
    uint32_t T = static_cast<uint32_t>(
        std::ceil(((n + k) * std::log(2) - std::log(eta_)) / alpha)
    );
    
    return std::max(T, 1u);  // Ensure at least one repetition
}

bool Solver::solve_with_xor(CDCLSolver& solver,
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

    // Check if base formula is satisfiable
    if (!solver.solve()) {
        if (debug_) {
            std::cout << "Base formula is UNSAT\n";
        }
        return false;
    }

    // Generate and add XOR constraints
    auto xor_cnf = generate_xor_constraints(formula, num_vars, num_xors);
    
    // Add XOR clauses
    for (const auto& clause : xor_cnf) {
        solver.add_clause(clause);
        if (debug_) {
            std::cout << "Adding XOR clause: ";
            for (size_t i = 0; i < clause.size(); i++) {
                if (i > 0) std::cout << " ∨ ";
                std::cout << (clause[i].is_positive() ? "" : "¬")
                         << "x" << clause[i].var_id();
            }
            std::cout << "\n";
        }
    }

    // Final solve with XOR constraints
    bool result = solver.solve();
    if (debug_) {
        std::cout << (result ? "SAT" : "UNSAT") 
                  << " with XORs - " 
                  << (result ? "at least" : "less than")
                  << " 2^" << num_xors << " solutions\n";
    }
    
    return result;
}

bool Solver::solve(const std::vector<std::vector<Literal>>& phi,
                  const std::vector<std::vector<std::vector<Literal>>>& f,
                  const std::vector<uint32_t>& q,
                  uint32_t n_vars) {
    if (debug_) {
        std::cout << "\nSolving SMC problem with " << f.size() 
                  << " counting constraints\n";
    }
    
    // Validate inputs
    if (f.size() != q.size()) {
        std::cout << "Error: Number of constraints doesn't match number of thresholds\n";
        return false;
    }
    
    // Compute number of repetitions T
    uint32_t T = compute_T(n_vars, f.size());
    if (debug_) {
        std::cout << "Parameters: T = " << T << " repetitions\n";
    }
    
    // For each repetition
    for (uint32_t t = 0; t < T; t++) {
        if (debug_) {
            std::cout << "\nRepetition " << t + 1 << "/" << T << "\n";
        }
        
        CDCLSolver solver;
        solver.set_num_variables(n_vars);
        
        // Add main formula φ
        for (const auto& clause : phi) {
            solver.add_clause(clause);
        }
        
        // Add counting constraints
        bool all_constraints_satisfied = true;
        for (size_t i = 0; i < f.size(); i++) {
            if (!solve_with_xor(solver, f[i], n_vars, q[i])) {
                if (debug_) {
                    std::cout << "Counting constraint " << i 
                             << " failed - trying next repetition\n";
                }
                all_constraints_satisfied = false;
                break;
            }
        }
        
        if (!all_constraints_satisfied) continue;
        
        // Final satisfiability check
        if (!solver.solve()) {
            if (debug_) {
                std::cout << "SAT solving failed - formula is UNSAT\n";
            }
            return false;
        }
        
        if (debug_) {
            std::cout << "Found satisfying assignment!\n";
        }
        return true;
    }
    
    if (debug_) {
        std::cout << "No satisfying assignment found after " << T 
                  << " repetitions - formula is likely UNSAT\n";
    }
    return false;
}

}