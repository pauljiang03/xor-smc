#pragma once
#include "xor_smc/CDCLSolver.hpp"
#include <vector>
#include <random>
#include <memory>
#include <iostream>

namespace xor_smc {

class Solver {
public:
    explicit Solver(double eta = 0.01);
    
    // Main SMC interface
    bool solve(const std::vector<std::vector<Literal>>& phi,
              const std::vector<std::vector<std::vector<Literal>>>& f,
              const std::vector<uint32_t>& q,
              uint32_t n_vars);
              
private:
    // XOR constraint handling
    std::vector<std::vector<Literal>> generate_xor_constraints(
        const std::vector<std::vector<Literal>>& formula,
        uint32_t num_vars,
        uint32_t num_xors);
        
    bool solve_with_xor(CDCLSolver& solver,
                       const std::vector<std::vector<Literal>>& formula,
                       uint32_t num_vars,
                       uint32_t num_xors);

    // Helper functions
    uint32_t count_actual_solutions(
        const std::vector<std::vector<Literal>>& formula, 
        uint32_t num_vars) const;
    
    // Member variables
    std::mt19937 rng_;
    double eta_;
    bool debug_;
};

}