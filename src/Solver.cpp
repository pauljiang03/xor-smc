#include "xor_smc/Solver.hpp"
#include <cmath>
#include <algorithm>

namespace xor_smc {

Solver::Solver(double eta) 
    : eta_(eta), rng_(std::random_device{}()) {}

bool Solver::solve(const Formula& phi,
                  const std::vector<Formula>& f,
                  const std::vector<uint32_t>& q,
                  uint32_t n_vars) {
    
    uint32_t k = f.size();
    uint32_t T = compute_T(n_vars, k);
    
    // For each repetition
    for (uint32_t t = 0; t < T; ++t) {
        CDCLSolver solver;
        solver.set_num_variables(n_vars);
        
        // Add main formula φ
        for (const auto& clause : phi.clauses()) {
            solver.add_clause(clause.literals());
        }
        
        // For each counting constraint
        for (size_t i = 0; i < f.size(); ++i) {
            if (!solve_with_xor(solver, f[i], n_vars, q[i])) {
                return false;
            }
        }
        
        if (!solver.solve()) {
            return false;
        }
    }
    
    return true;
}

bool Solver::solve_with_xor(CDCLSolver& solver,
                          const Formula& formula,
                          uint32_t num_vars,
                          uint32_t num_constraints) {
    // Add original formula clauses
    for (const auto& clause : formula.clauses()) {
        solver.add_clause(clause.literals());
    }
    
    // Add XOR constraints
    for (uint32_t i = 0; i < num_constraints; ++i) {
        // Randomly select variables for XOR constraint
        std::vector<uint32_t> xor_vars;
        std::uniform_int_distribution<> dis(0, 1);
        
        for (uint32_t var = 0; var < num_vars; ++var) {
            if (dis(rng_)) {
                xor_vars.push_back(var);
            }
        }
        
        if (!xor_vars.empty()) {
            add_xor_constraint(solver, xor_vars);
        }
    }
    
    return solver.solve();
}

void Solver::add_xor_constraint(CDCLSolver& solver,
                              const std::vector<uint32_t>& variables) {
    if (variables.empty()) return;
    
    // Convert XOR to CNF using Tseitin transformation
    size_t n = variables.size();
    std::uniform_int_distribution<> dis(0, 1);
    bool rhs = dis(rng_) == 1;  // Random right-hand side
    
    // Generate clauses for all possible assignments
    for (size_t i = 0; i < (1u << n); ++i) {
        // Count ones in this assignment
        size_t ones = 0;
        for (size_t j = 0; j < n; ++j) {
            if (i & (1u << j)) ones++;
        }
        
        // Add clause if parity doesn't match desired rhs
        if ((ones % 2) != rhs) {
            std::vector<Literal> clause;
            for (size_t j = 0; j < n; ++j) {
                bool positive = (i & (1u << j)) == 0;
                clause.emplace_back(variables[j], positive);
            }
            solver.add_clause(clause);
        }
    }
}

uint32_t Solver::compute_T(uint32_t n, uint32_t k) const {
    // Compute parameters as per the paper
    uint32_t c = static_cast<uint32_t>(std::ceil(std::log2(k + 1) + 1));
    
    // Compute α(c,k)
    double alpha = 0.5 * std::log(std::pow(2.0, c - 1) / (k * std::pow(2.0, c + 1)));
    
    // Compute required number of repetitions T
    return static_cast<uint32_t>(std::ceil(((n + k) * std::log(2) - std::log(eta_)) / alpha));
}

} // namespace xor_smc
