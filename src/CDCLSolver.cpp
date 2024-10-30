#include "xor_smc/CDCLSolver.hpp"
#include <cassert>
#include <queue>

namespace xor_smc {

CDCLSolver::CDCLSolver() : decision_level_(0) {
    std::cout << "Creating CDCLSolver\n";
}

void CDCLSolver::set_num_variables(uint32_t num_vars) {
    std::cout << "Setting number of variables to " << num_vars << "\n";
    assignments_.resize(num_vars, {-1, false, nullptr});
    watches_.resize(num_vars * 2);  // Two watch lists per variable (pos/neg)
}

void CDCLSolver::print_clause(const std::shared_ptr<Clause>& clause) const {
    std::cout << "(";
    for (size_t i = 0; i < clause->literals.size(); i++) {
        if (i > 0) std::cout << " ∨ ";
        const auto& lit = clause->literals[i];
        std::cout << (lit.is_positive() ? "" : "¬") << "x" << lit.var_id();
    }
    std::cout << ")";
}

void CDCLSolver::print_assignment() const {
    for (size_t i = 0; i < assignments_.size(); i++) {
        if (assignments_[i].level != -1) {
            std::cout << "x" << i << "=" << assignments_[i].value 
                     << "@" << assignments_[i].level << " ";
        }
    }
    std::cout << "\n";
}

void CDCLSolver::attach_watch(const std::shared_ptr<Clause>& clause, size_t watch_idx) {
    const auto& lit = clause->literals[watch_idx];
    uint32_t watch_list = lit.var_id() * 2 + !lit.is_positive();
    watches_[watch_list].push_back(clause);
}

void CDCLSolver::detach_watch(const std::shared_ptr<Clause>& clause, size_t watch_idx) {
    const auto& lit = clause->literals[watch_idx];
    uint32_t watch_list = lit.var_id() * 2 + !lit.is_positive();
    auto& watch_vector = watches_[watch_list];
    
    for (auto it = watch_vector.begin(); it != watch_vector.end(); ++it) {
        if (*it == clause) {
            watch_vector.erase(it);
            break;
        }
    }
}

bool CDCLSolver::update_watches(const std::shared_ptr<Clause>& clause, const Literal& false_lit) {
    std::cout << "Updating watches for clause: ";
    print_clause(clause);
    std::cout << "\n";
    
    // Find the false watch index
    size_t false_idx = clause->watched[0];
    if (clause->literals[clause->watched[1]].var_id() == false_lit.var_id()) {
        false_idx = clause->watched[1];
    }
    
    // Look for a new non-false literal to watch
    for (size_t i = 0; i < clause->literals.size(); i++) {
        if (i == clause->watched[0] || i == clause->watched[1]) continue;
        
        const auto& lit = clause->literals[i];
        uint32_t var = lit.var_id();
        
        if (assignments_[var].level == -1 ||  // Unassigned
            assignments_[var].value == lit.is_positive()) {  // Satisfying
            
            // Update watches
            detach_watch(clause, false_idx);
            clause->watched[false_idx == clause->watched[0] ? 0 : 1] = i;
            attach_watch(clause, i);
            
            std::cout << "Found new watch: x" << var << "\n";
            return true;
        }
    }
    
    std::cout << "No new watch found\n";
    return false;  // No new watch found
}

bool CDCLSolver::assign(uint32_t var, bool value, int level, const std::shared_ptr<Clause>& reason) {
    std::cout << "Assigning x" << var << " = " << value << " @ level " << level << "\n";
    
    assignments_[var] = Assignment{level, value, reason};
    propagation_queue_.push_back(var);
    return true;
}

void CDCLSolver::unassign(uint32_t var) {
    assignments_[var] = Assignment{-1, false, nullptr};
}

bool CDCLSolver::propagate() {
    while (!propagation_queue_.empty()) {
        uint32_t var = propagation_queue_.back();
        propagation_queue_.pop_back();
        
        bool value = assignments_[var].value;
        uint32_t watch_idx = var * 2 + value;  // Watch list for false literal
        
        std::cout << "Propagating x" << var << " = " << value << "\n";
        
        // Check all clauses watching this literal's negation
        auto& watch_list = watches_[watch_idx];
        for (size_t i = 0; i < watch_list.size();) {
            auto clause = watch_list[i];
            
            // Try to find new watch
            if (update_watches(clause, Literal(var, !value))) {
                i++;  // Move to next clause
                continue;
            }
            
            // No new watch found - check other watched literal
            size_t other_idx = clause->watched[0];
            if (clause->literals[clause->watched[0]].var_id() == var) {
                other_idx = clause->watched[1];
            }
            
            const auto& other_lit = clause->literals[other_idx];
            uint32_t other_var = other_lit.var_id();
            
            // If other watch is true, clause is satisfied
            if (assignments_[other_var].level != -1 && 
                assignments_[other_var].value == other_lit.is_positive()) {
                i++;
                continue;
            }
            
            // If other watch is unassigned, propagate it
            if (assignments_[other_var].level == -1) {
                if (!assign(other_var, other_lit.is_positive(), 
                          decision_level_, clause)) {
                    return false;
                }
                i++;
                continue;
            }
            
            // Conflict
            std::cout << "Conflict found in clause: ";
            print_clause(clause);
            std::cout << "\n";
            return false;
        }
    }
    
    return true;
}

void CDCLSolver::add_clause(const std::vector<Literal>& literals) {
    if (literals.empty()) {
        std::cout << "Adding empty clause - formula is UNSAT\n";
        clauses_.push_back(std::make_shared<Clause>(literals));
        return;
    }

    std::cout << "Adding clause: ";
    for (size_t i = 0; i < literals.size(); i++) {
        if (i > 0) std::cout << " ∨ ";
        std::cout << (literals[i].is_positive() ? "" : "¬") 
                  << "x" << literals[i].var_id();
    }
    std::cout << "\n";

    auto clause = std::make_shared<Clause>(literals);
    
    // For unit clauses, try to assign immediately
    if (literals.size() == 1) {
        uint32_t var = literals[0].var_id();
        if (assignments_[var].level == -1) {
            assign(var, literals[0].is_positive(), 0, clause);
        } else if (assignments_[var].value != literals[0].is_positive()) {
            // Contradiction
            clauses_.push_back(std::make_shared<Clause>(std::vector<Literal>()));
            return;
        }
    } else {
        // Set up watched literals
        attach_watch(clause, 0);
        if (literals.size() > 1) {
            attach_watch(clause, 1);
        }
    }
    
    clauses_.push_back(clause);
}

bool CDCLSolver::solve() {
    std::cout << "\nStarting solve with " << clauses_.size() 
              << " clauses and " << assignments_.size() << " variables\n";
    
    // Check for empty clauses
    for (const auto& clause : clauses_) {
        if (clause->literals.empty()) {
            std::cout << "Formula contains empty clause - UNSAT\n";
            return false;
        }
    }
    
    // Initial propagation
    if (!propagate()) {
        std::cout << "Conflict during initial propagation - UNSAT\n";
        return false;
    }
    
    // Main solving loop
    while (true) {
        // Find unassigned variable
        int next_var = -1;
        for (size_t i = 0; i < assignments_.size(); i++) {
            if (assignments_[i].level == -1) {
                next_var = i;
                break;
            }
        }
        
        // No unassigned variables - SAT
        if (next_var == -1) {
            std::cout << "All variables assigned - SAT\n";
            return true;
        }
        
        // Try assignment
        decision_level_++;
        std::cout << "\nDecision level " << decision_level_ 
                  << ": trying x" << next_var << " = true\n";
                  
        if (!assign(next_var, true, decision_level_, nullptr) || 
            !propagate()) {
            if (decision_level_ == 1) {
                std::cout << "Conflict at level 1 - UNSAT\n";
                return false;
            }
            
            // Backtrack and try false
            while (!propagation_queue_.empty()) {
                unassign(propagation_queue_.back());
                propagation_queue_.pop_back();
            }
            
            decision_level_--;
            if (!assign(next_var, false, decision_level_, nullptr) || 
                !propagate()) {
                std::cout << "Both values lead to conflict - UNSAT\n";
                return false;
            }
        }
    }
}

bool CDCLSolver::get_value(uint32_t var_id) const {
    assert(var_id < assignments_.size());
    return assignments_[var_id].value;
}

}