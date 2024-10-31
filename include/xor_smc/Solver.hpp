#pragma once
#include "CDCLSolver.hpp"
#include <vector>
#include <random>

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
                       
    // Helper methods
    uint32_t compute_T(uint32_t n, uint32_t k) const;
    
    // Member variables - order matters for initialization
    std::mt19937 rng_;      // Random number generator
    double eta_;            // Error probability bound
    bool debug_;            // Debug output control
};

}