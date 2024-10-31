#include "xor_smc/Solver.hpp"
#include <cassert>
#include <queue>
#include <unordered_set>
#include <algorithm>

namespace xor_smc {

Solver::Solver() : decision_level_(0) {
    std::cout << "Creating Solver...\n";
}

void Solver::set_num_variables(uint32_t num_vars) {
    std::cout << "Setting number of variables to " << num_vars << "\n";
    assignments_.resize(num_vars, {-1, false, nullptr});
    watches_.resize(num_vars * 2);  // Two watch lists per variable (pos/neg)
    trail_.reserve(num_vars);
    seen_.resize(num_vars, false);
}

void Solver::print_clause(const std::shared_ptr<Clause>& clause) const {
    std::cout << "(";
    for (size_t i = 0; i < clause->literals.size(); i++) {
        if (i > 0) std::cout << " ∨ ";
        const auto& lit = clause->literals[i];
        std::cout << (lit.is_positive() ? "" : "¬") << "x" << lit.var_id();
    }
    std::cout << ")";
}

void Solver::print_assignment() const {
    for (size_t i = 0; i < assignments_.size(); i++) {
        if (assignments_[i].level != -1) {
            std::cout << "x" << i << "=" << assignments_[i].value 
                     << "@" << assignments_[i].level << " ";
        }
    }
    std::cout << "\n";
}

void Solver::add_clause(const std::vector<Literal>& literals) {
    if (literals.empty()) {
        std::cout << "Adding empty clause - formula is UNSAT\n";
        clauses_.push_back(std::make_shared<Clause>(literals));
        return;
    }

    //std::cout << "Adding clause: ";
    //print_clause(std::make_shared<Clause>(literals));
    //std::cout << "\n";

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

void Solver::attach_watch(const std::shared_ptr<Clause>& clause, size_t watch_idx) {
    const auto& lit = clause->literals[watch_idx];
    uint32_t watch_list = lit.var_id() * 2 + !lit.is_positive();
    watches_[watch_list].push_back(clause);
}

void Solver::detach_watch(const std::shared_ptr<Clause>& clause, size_t watch_idx) {
    const auto& lit = clause->literals[watch_idx];
    uint32_t watch_list = lit.var_id() * 2 + !lit.is_positive();
    auto& watch_vector = watches_[watch_list];
    
    auto it = std::find(watch_vector.begin(), watch_vector.end(), clause);
    if (it != watch_vector.end()) {
        watch_vector.erase(it);
    }
}

bool Solver::update_watches(const std::shared_ptr<Clause>& clause, const Literal& false_lit) {
    //std::cout << "Updating watches for clause: ";
    //print_clause(clause);
    //std::cout << "\n";
    
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
            
            //std::cout << "Found new watch: x" << var << "\n";
            return true;
        }
    }
    
    //std::cout << "No new watch found\n";
    return false;  // No new watch found
}

bool Solver::assign(uint32_t var, bool value, int level, const std::shared_ptr<Clause>& reason) {
    //std::cout << "Assigning x" << var << " = " << value << " @ level " << level << "\n";
    
    assignments_[var] = Assignment{level, value, reason};
    trail_.push_back(var);
    propagation_queue_.push_back(var);
    return true;
}

void Solver::unassign(uint32_t var) {
    assignments_[var] = Assignment{-1, false, nullptr};
}

bool Solver::propagate() {
    while (!propagation_queue_.empty()) {
        uint32_t var = propagation_queue_.back();
        propagation_queue_.pop_back();
        
        bool value = assignments_[var].value;
        uint32_t watch_idx = var * 2 + value;  // Watch list for false literal
        
        //std::cout << "Propagating x" << var << " = " << value << "\n";
        
        // Check all clauses watching this literal's negation
        auto& watch_list = watches_[watch_idx];
        for (size_t i = 0; i < watch_list.size();) {
            auto clause = watch_list[i];
            
            if (update_watches(clause, Literal(var, !value))) {
                i++;
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
                    conflict_clause_ = clause;
                    return false;
                }
                i++;
                continue;
            }
            
            // Conflict
            //std::cout << "Conflict found in clause: ";
            //print_clause(clause);
            //std::cout << "\n";
            conflict_clause_ = clause;
            return false;
        }
    }
    
    return true;
}

std::shared_ptr<Solver::Clause> Solver::analyze_conflict(const std::shared_ptr<Clause>& conflict) {
    //std::cout << "Analyzing conflict\n";
    
    std::vector<Literal> learnt_literals;
    std::unordered_set<uint32_t> seen_vars;
    int counter = 0;
    int conflict_level = static_cast<int>(decision_level_);
    
    // Add all literals from conflict clause
    for (const auto& lit : conflict->literals) {
        uint32_t var = lit.var_id();
        if (assignments_[var].level > 0) {
            seen_[var] = true;
            seen_vars.insert(var);
            if (assignments_[var].level == conflict_level) {
                counter++;
            }
        }
    }
    
    // Process implications until we reach the First UIP
    for (int i = static_cast<int>(trail_.size()) - 1; i >= 0 && counter > 0; i--) {
        uint32_t var = trail_[i];
        
        if (!seen_[var]) continue;
        
        seen_[var] = false;
        seen_vars.erase(var);
        
        const auto& reason = assignments_[var].reason;
        if (!reason) continue;  // Skip decision variables
        
        // Add literals from reason clause
        for (const auto& lit : reason->literals) {
            uint32_t reason_var = lit.var_id();
            if (reason_var == var) continue;
            
            if (!seen_[reason_var] && assignments_[reason_var].level > 0) {
                seen_[reason_var] = true;
                seen_vars.insert(reason_var);
                if (assignments_[reason_var].level == conflict_level) {
                    counter++;
                }
            }
        }
        
        counter--;
    }
    
    // Construct learnt clause
    for (uint32_t var : seen_vars) {
        learnt_literals.push_back(
            Literal(var, !assignments_[var].value)  // Note: Negated value
        );
    }
    
    // Create and return the learnt clause
    return std::make_shared<Clause>(learnt_literals);
}

int Solver::compute_backtrack_level(const std::shared_ptr<Clause>& learnt_clause) {
    int max_level = 0;
    int second_max_level = 0;
    
    for (const auto& lit : learnt_clause->literals) {
        int level = assignments_[lit.var_id()].level;
        if (level > max_level) {
            second_max_level = max_level;
            max_level = level;
        } else if (level > second_max_level && level < max_level) {
            second_max_level = level;
        }
    }
    
    return second_max_level;
}

void Solver::backtrack(int level) {
    //std::cout << "Backtracking to level " << level << "\n";
    
    while (!trail_.empty() && assignments_[trail_.back()].level > level) {
        uint32_t var = trail_.back();
        unassign(var);
        seen_[var] = false;
        trail_.pop_back();
    }
    
    propagation_queue_.clear();
    decision_level_ = level;
}

bool Solver::solve() {
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
    
    // Main CDCL loop
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
        
        // Make decision
        decision_level_++;
        //std::cout << "\nDecision level " << decision_level_ 
                  //<< ": trying x" << next_var << " = true\n";
                  
        if (!assign(next_var, true, decision_level_, nullptr) || 
            !propagate()) {
                
            // Analyze conflict and learn new clause
            auto learnt_clause = analyze_conflict(conflict_clause_);
            
            if (learnt_clause->literals.empty() || decision_level_ == 0) {
                std::cout << "Learned empty clause - UNSAT\n";
                return false;
            }
            
            // Add learned clause and backtrack
            int backtrack_level = compute_backtrack_level(learnt_clause);
            backtrack(backtrack_level);
            
            // Add learned clause to clause database
            //std::cout << "Learned clause: ";
            //print_clause(learnt_clause);
            //std::cout << "\n";
            
            clauses_.push_back(learnt_clause);
            attach_watch(learnt_clause, 0);
            if (learnt_clause->literals.size() > 1) {
                attach_watch(learnt_clause, 1);
            }
            
            // Unit propagate learned clause
            uint32_t unit_var = learnt_clause->literals[0].var_id();
            bool unit_value = learnt_clause->literals[0].is_positive();
            if (!assign(unit_var, unit_value, backtrack_level, learnt_clause) ||
                !propagate()) {
                std::cout << "Conflict after learning - UNSAT\n";
                return false;
            }
        }
    }
}

bool Solver::get_value(uint32_t var_id) const {
    assert(var_id < assignments_.size());
    return assignments_[var_id].value;
}

}