#include "xor_smc/CDCLSolver.hpp"
#include <cassert>
#include <iostream>

namespace xor_smc {

CDCLSolver::CDCLSolver() : decision_level_(0) {
    std::cout << "Creating CDCLSolver\n";
}

void CDCLSolver::set_num_variables(uint32_t num_vars) {
    std::cout << "Setting number of variables to " << num_vars << "\n";
    assignments_.resize(num_vars, {-1, false, nullptr});
}

void CDCLSolver::add_clause(const std::vector<Literal>& literals) {
    if (literals.empty()) {
        std::cout << "Adding empty clause - formula is UNSAT\n";
        clauses_.push_back(std::make_shared<Clause>(literals));
        return;
    }

    std::cout << "Adding clause: (";
    for (size_t i = 0; i < literals.size(); i++) {
        if (i > 0) std::cout << " ∨ ";
        std::cout << (literals[i].is_positive() ? "" : "¬") << "x" << literals[i].var_id();
    }
    std::cout << ")\n";

    // For unit clauses, check for direct contradictions
    if (literals.size() == 1) {
        uint32_t var = literals[0].var_id();
        bool value = literals[0].is_positive();
        
        // Check if we already have the opposite value at level 0
        if (assignments_[var].level == 0 && assignments_[var].value != value) {
            std::cout << "Direct contradiction found on variable x" << var << "\n";
            // Add empty clause to indicate UNSAT
            clauses_.push_back(std::make_shared<Clause>(std::vector<Literal>()));
            return;
        }
    }

    clauses_.push_back(std::make_shared<Clause>(literals));
}

bool CDCLSolver::check_clause(const std::shared_ptr<Clause>& clause) const {
    if (clause->literals.empty()) {
        std::cout << "Empty clause is always unsatisfied\n";
        return false;
    }

    std::cout << "Checking clause (";
    for (size_t i = 0; i < clause->literals.size(); i++) {
        if (i > 0) std::cout << " ∨ ";
        std::cout << (clause->literals[i].is_positive() ? "" : "¬") 
                  << "x" << clause->literals[i].var_id();
    }
    std::cout << ") with assignment: ";
    
    for (size_t i = 0; i < assignments_.size(); i++) {
        std::cout << "x" << i << "=" << assignments_[i].value << " ";
    }
    std::cout << "\n";

    // Check each literal
    for (const auto& lit : clause->literals) {
        bool assigned_value = assignments_[lit.var_id()].value;
        bool needed_value = lit.is_positive();
        
        std::cout << "  Checking literal " << (lit.is_positive() ? "" : "¬") 
                  << "x" << lit.var_id() << ": assigned=" << assigned_value 
                  << ", needs=" << needed_value << "\n";
        
        if (assigned_value == needed_value) {
            std::cout << "  -> Literal satisfies clause\n";
            return true;
        }
    }

    std::cout << "  -> No literal satisfies clause\n";
    return false;
}

bool CDCLSolver::solve() {
    std::cout << "\nStarting solve with " << clauses_.size() 
              << " clauses and " << assignments_.size() << " variables\n";

    // First check for empty clauses
    for (const auto& clause : clauses_) {
        if (clause->literals.empty()) {
            std::cout << "Formula contains empty clause - UNSAT\n";
            return false;
        }
    }

    // Try all possible assignments
    uint64_t max_assignments = 1ULL << assignments_.size();
    
    for (uint64_t i = 0; i < max_assignments; i++) {
        // Set current assignment
        std::cout << "\nTrying assignment #" << i << ": ";
        for (size_t j = 0; j < assignments_.size(); j++) {
            assignments_[j].value = (i & (1ULL << j)) != 0;
            std::cout << "x" << j << "=" << assignments_[j].value << " ";
        }
        std::cout << "\n";

        // Check if all clauses are satisfied
        bool all_satisfied = true;
        for (const auto& clause : clauses_) {
            if (!check_clause(clause)) {
                std::cout << "Clause not satisfied - trying next assignment\n";
                all_satisfied = false;
                break;
            }
        }

        if (all_satisfied) {
            std::cout << "Found satisfying assignment!\n";
            return true;
        }
    }

    std::cout << "Tried all possible assignments - formula is UNSAT\n";
    return false;
}

bool CDCLSolver::get_value(uint32_t var_id) const {
    assert(var_id < assignments_.size());
    return assignments_[var_id].value;
}

}