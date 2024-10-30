#include "xor_smc/CDCLSolver.hpp"
#include <algorithm>
#include <queue>
#include <cassert>
#include <iostream>

namespace xor_smc {

CDCLSolver::CDCLSolver() : decision_level_(0) {}

void CDCLSolver::set_num_variables(uint32_t num_vars) {
    assignments_.resize(num_vars, {-1, false, nullptr});
    watches_.resize(num_vars * 2);  // Two watch lists per variable
    vsids_score_.resize(num_vars, 1);  // Initialize all scores to 1
    phase_saving_.resize(num_vars, false);
}

void CDCLSolver::add_clause(const std::vector<Literal>& literals) {
    auto clause = std::make_shared<Clause>(literals);
    clauses_.push_back(clause);

    if (literals.empty()) {
        // Empty clause means UNSAT - no need to watch anything
        return;
    }

    if (literals.size() == 1) {
        // Unit clause - directly assign at level 0
        uint32_t var_id = literals[0].var_id();
        bool value = literals[0].is_positive();
        
        if (assignments_[var_id].level == -1) {
            assignments_[var_id] = Assignment{0, value, clause};
        } else if (assignments_[var_id].value != value) {
            // Contradiction at level 0 - UNSAT
            clauses_.push_back(std::make_shared<Clause>(std::vector<Literal>())); // Add empty clause
        }
    } else {
        // For clauses of size >= 2, set up watched literals
        uint32_t watch1 = literals[0].var_id() * 2 + !literals[0].is_positive();
        uint32_t watch2 = literals[1].var_id() * 2 + !literals[1].is_positive();
        
        watches_[watch1].push_back(clause);
        if (watch1 != watch2) {
            watches_[watch2].push_back(clause);
        }
    }
}

bool CDCLSolver::solve() {
    // Reset assignments
    for (auto& assignment : assignments_) {
        assignment = Assignment{-1, false, nullptr};
    }
    decision_level_ = 0;
    
    // Initial unit propagation
    if (!unit_propagate()) {
        return false;
    }
    
    while (true) {
        if (!unit_propagate()) {
            if (decision_level_ == 0) {
                return false;  // UNSAT
            }
            
            auto learned_clause = analyze_conflict(clauses_.back());
            if (!learned_clause || learned_clause->literals.empty()) {
                return false;
            }
            
            int back_level = compute_backtrack_level(learned_clause);
            backtrack(back_level);
            add_clause(learned_clause->literals);
        } else {
            // Check if all variables are assigned
            bool all_assigned = true;
            uint32_t unassigned_var = 0;
            
            for (uint32_t var = 0; var < assignments_.size(); var++) {
                if (assignments_[var].level == -1) {
                    all_assigned = false;
                    unassigned_var = var;
                    break;
                }
            }
            
            if (all_assigned) {
                return true;  // SAT
            }
            
            // Make new decision
            decision_level_++;
            assignments_[unassigned_var] = Assignment{
                decision_level_,
                phase_saving_[unassigned_var],
                nullptr
            };
        }
    }
}

bool CDCLSolver::unit_propagate() {
    std::queue<uint32_t> propagation_queue;
    
    // Initialize with unprocessed assignments
    for (uint32_t var = 0; var < assignments_.size(); var++) {
        if (assignments_[var].level != -1) {
            propagation_queue.push(var);
        }
    }
    
    while (!propagation_queue.empty()) {
        uint32_t var = propagation_queue.front();
        propagation_queue.pop();
        
        // Check both polarities
        for (bool positive : {false, true}) {
            uint32_t watch_idx = var * 2 + !positive;
            auto& watch_list = watches_[watch_idx];
            
            for (size_t i = 0; i < watch_list.size();) {
                auto clause = watch_list[i];
                bool clause_satisfied = false;
                bool found_new_watch = false;
                
                // Look for a new watch literal
                for (const auto& lit : clause->literals) {
                    uint32_t lit_var = lit.var_id();
                    
                    if (lit_var == var) continue;
                    
                    if (assignments_[lit_var].level == -1) {
                        // Found unassigned literal - make it watched
                        uint32_t new_watch = lit_var * 2 + !lit.is_positive();
                        watches_[new_watch].push_back(clause);
                        watch_list[i] = watch_list.back();
                        watch_list.pop_back();
                        found_new_watch = true;
                        break;
                    }
                    else if (assignments_[lit_var].value == lit.is_positive()) {
                        clause_satisfied = true;
                        break;
                    }
                }
                
                if (!found_new_watch && !clause_satisfied) {
                    // Check remaining literal for unit propagation or conflict
                    bool all_false = true;
                    const Literal* unit_lit = nullptr;
                    
                    for (const auto& lit : clause->literals) {
                        uint32_t lit_var = lit.var_id();
                        if (assignments_[lit_var].level == -1) {
                            unit_lit = &lit;
                            all_false = false;
                            break;
                        }
                        else if (assignments_[lit_var].value == lit.is_positive()) {
                            all_false = false;
                            break;
                        }
                    }
                    
                    if (all_false) {
                        // Conflict found
                        clauses_.push_back(clause);  // Save conflicting clause
                        return false;
                    }
                    else if (unit_lit) {
                        // Unit propagation
                        assignments_[unit_lit->var_id()] = Assignment{
                            decision_level_,
                            unit_lit->is_positive(),
                            clause
                        };
                        propagation_queue.push(unit_lit->var_id());
                    }
                }
                
                if (!found_new_watch) {
                    i++;
                }
            }
        }
    }
    
    return true;
}

std::shared_ptr<CDCLSolver::Clause> CDCLSolver::analyze_conflict(
    const std::shared_ptr<Clause>& conflict) {
    
    std::vector<bool> seen(assignments_.size(), false);
    std::vector<Literal> learned_lits;
    int conflict_count = 0;
    
    // Add literals from conflict clause
    for (const auto& lit : conflict->literals) {
        uint32_t var = lit.var_id();
        if (assignments_[var].level > 0) {
            seen[var] = true;
            if (assignments_[var].level == decision_level_) {
                conflict_count++;
            }
            learned_lits.push_back(lit);
        }
    }
    
    int trail_pos = static_cast<int>(assignments_.size()) - 1;
    
    // Resolution steps
    while (conflict_count > 1) {
        while (trail_pos >= 0 && !seen[trail_pos]) {
            trail_pos--;
        }
        
        if (trail_pos < 0) break;
        
        uint32_t var = static_cast<uint32_t>(trail_pos);
        auto reason = assignments_[var].reason;
        seen[var] = false;
        
        if (reason) {
            for (const auto& lit : reason->literals) {
                uint32_t var2 = lit.var_id();
                if (!seen[var2] && assignments_[var2].level > 0) {
                    seen[var2] = true;
                    if (assignments_[var2].level == decision_level_) {
                        conflict_count++;
                    }
                    learned_lits.push_back(lit);
                }
            }
        }
        
        conflict_count--;
        trail_pos--;
    }
    
    return std::make_shared<Clause>(learned_lits);
}

int CDCLSolver::compute_backtrack_level(const std::shared_ptr<Clause>& learned_clause) {
    int max_level = 0;
    int second_max = 0;
    
    for (const auto& lit : learned_clause->literals) {
        int level = assignments_[lit.var_id()].level;
        if (level > max_level) {
            second_max = max_level;
            max_level = level;
        } else if (level > second_max && level < max_level) {
            second_max = level;
        }
    }
    
    return second_max;
}

void CDCLSolver::backtrack(int level) {
    for (size_t var = 0; var < assignments_.size(); var++) {
        if (assignments_[var].level > level) {
            assignments_[var].level = -1;
            assignments_[var].reason = nullptr;
        }
    }
    decision_level_ = level;
}

Literal CDCLSolver::pick_branching_variable() {
    // Use VSIDS (Variable State Independent Decaying Sum) heuristic
    uint32_t max_score = 0;
    uint32_t chosen_var = 0;
    
    for (size_t var = 0; var < assignments_.size(); var++) {
        if (assignments_[var].level == -1 && vsids_score_[var] > max_score) {
            max_score = vsids_score_[var];
            chosen_var = var;
        }
    }
    
    // Decay scores
    for (auto& score : vsids_score_) {
        score = (score * 95) / 100;
    }
    
    // Use phase saving for polarity
    return Literal(chosen_var, phase_saving_[chosen_var]);
}

bool CDCLSolver::get_value(uint32_t var_id) const {
    assert(var_id < assignments_.size());
    return assignments_[var_id].value;
}

}
