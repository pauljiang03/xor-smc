#pragma once
#include "CDCLSolver.hpp"
#include "Formula.hpp"  // Added this include
#include <vector>
#include <random>
#include <functional>

namespace xor_smc {

class Solver {
public:
    // Constructor with error probability bound
    explicit Solver(double eta = 0.01);
    
    // Main solving interface
    bool solve(const Formula& phi,
              const std::vector<Formula>& f,
              const std::vector<uint32_t>& q,
              uint32_t n_vars);

private:
    // Helper methods
    bool solve_with_xor(CDCLSolver& solver,
                       const Formula& formula,
                       uint32_t num_vars,
                       uint32_t num_constraints);
                       
    void add_xor_constraint(CDCLSolver& solver,
                          const std::vector<uint32_t>& variables);
                          
    uint32_t compute_T(uint32_t n, uint32_t k) const;

    // Count number of 1 bits in a number
    static size_t count_ones(size_t x) {
        size_t count = 0;
        while (x) {
            count += x & 1;
            x >>= 1;
        }
        return count;
    }

    // Member variables
    double eta_;           // Error probability bound
    std::mt19937 rng_;    // Random number generator
};

}
