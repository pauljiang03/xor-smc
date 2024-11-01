#include "xor_smc/Solver.hpp"
#include <iostream>
#include <cassert>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <random>

namespace xor_smc {

Solver::Solver() : decision_level_(0), rng_(std::random_device{}()) {
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

void Solver::add_unit_clause(const Literal& lit) {
    std::vector<Literal> clause = {lit};
    add_clause(clause);
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
            
            return true;
        }
    }
    
    return false;  // No new watch found
}

bool Solver::assign(uint32_t var, bool value, int level, const std::shared_ptr<Clause>& reason) {
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
            conflict_clause_ = clause;
            return false;
        }
    }
    
    return true;
}

std::shared_ptr<Solver::Clause> Solver::analyze_conflict(
    const std::shared_ptr<Clause>& conflict) {
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
            Literal(var, !assignments_[var].value)
        );
    }
    
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

// SMC-specific functions

std::vector<Literal> Solver::generate_xor_constraint(double density) {
    std::vector<Literal> xor_lits;
    std::bernoulli_distribution d(density);
    
    // Add each variable with probability density
    for (uint32_t var = 0; var < num_variables(); var++) {
        if (d(rng_)) {
            xor_lits.push_back(Literal(var, true));
        }
    }
    
    // Add random parity bit
    if (d(rng_)) {
        xor_lits.push_back(Literal(num_variables(), true));
    }
    
    return xor_lits;
}


void Solver::convert_xor_to_cnf(
    const std::vector<Literal>& xor_lits, 
    std::vector<std::vector<Literal>>& cnf_clauses) {
    
    size_t n = xor_lits.size();
    if (n == 0) return;
    
    // For each possible assignment to variables
    for (size_t i = 0; i < (1u << n); i++) {
        int ones = __builtin_popcount(i);
        
        // If this assignment violates XOR (even number of 1s)
        // add clause to forbid it
        if (ones % 2 == 0) {
            std::vector<Literal> clause;
            for (size_t j = 0; j < n; j++) {
                bool is_one = (i & (1 << j));
                // Add literal that would make this assignment false
                clause.push_back(Literal(
                    xor_lits[j].var_id(),
                    is_one != xor_lits[j].is_positive()
                ));
            }
            cnf_clauses.push_back(clause);
        }
    }
}

void Solver::add_xor_clause(const std::vector<Literal>& literals) {
    std::vector<std::vector<Literal>> cnf_clauses;
    convert_xor_to_cnf(literals, cnf_clauses);
    for (const auto& clause : cnf_clauses) {
        add_clause(clause);
    }
}


std::vector<bool> Solver::get_model() const {
    std::vector<bool> model(num_variables());
    for (uint32_t i = 0; i < num_variables(); i++) {
        model[i] = get_value(i);
    }
    return model;
}

void Solver::add_blocking_clause(const std::vector<bool>& model) {
    std::vector<Literal> blocking;
    for (uint32_t i = 0; i < model.size(); i++) {
        blocking.push_back(Literal(i, !model[i]));
    }
    add_clause(blocking);
}

bool Solver::get_value(uint32_t var_id) const {
    assert(var_id < assignments_.size());
    assert(assignments_[var_id].level != -1);  // Variable must be assigned
    return assignments_[var_id].value;
}

uint32_t Solver::num_variables() const {
    return assignments_.size();
}

uint32_t Solver::num_clauses() const {
    return clauses_.size();
}

std::vector<Literal> Solver::generate_xor_constraint_over_vars(
    const std::vector<uint32_t>& vars,
    double density) {
    
    std::vector<Literal> xor_constraint;
    bool parity = std::bernoulli_distribution(0.5)(rng_); // Random parity bit
    
    // Select each variable with probability 1/2
    for (uint32_t var : vars) {
        if (std::bernoulli_distribution(0.5)(rng_)) {
            // Include variable with random polarity
            xor_constraint.push_back(Literal(var, true));
        }
    }
    
    // If parity bit is 1, negate the entire constraint
    if (parity) {
        // Add a true constant to force odd parity
        if (!vars.empty()) {
            xor_constraint.push_back(Literal(vars[0], true));  
        }
    }
    
    return xor_constraint;
}

void Solver::add_xor_clause_over_vars(const std::vector<uint32_t>& vars) {
    auto xor_constraint = generate_xor_constraint_over_vars(vars);
    add_xor_clause(xor_constraint);
}

bool Solver::solve_smc(
    const std::vector<uint32_t>& thresholds,
    const std::vector<std::vector<uint32_t>>& counting_variables,
    const std::vector<std::vector<uint32_t>>& fixed_variables,
    int num_xor_tries,
    double confidence) {
    
    const int T = 10;  // Number of trials
    const int c = 1;  // Constant from paper
    
    for (size_t i = 0; i < thresholds.size(); i++) {
        int successes = 0;
        
        int q = (thresholds[i] <= 1) ? 0 : std::ceil(std::log2(thresholds[i]));
        std::cout << "\nTesting threshold " << thresholds[i] << " using " 
                  << q << " XORs\n";
        
        for (int t = 0; t < T; t++) {
            auto saved_clauses = clauses_;
            auto saved_assignment = assignments_;
            
            // Fix required variables
            for (uint32_t var : fixed_variables[i]) {
                if (saved_assignment[var].level != -1) {
                    add_unit_clause(Literal(var, saved_assignment[var].value));
                }
            }
            
            // Build XOR constraints that enforce parity
            for (int j = 0; j < q; j++) {
                // Select variables with prob 1/2
                std::vector<Literal> vars_in_xor;
                for (uint32_t var : counting_variables[i]) {
                    if (std::bernoulli_distribution(0.5)(rng_)) {
                        vars_in_xor.push_back(Literal(var, true));
                    }
                }
                
                // Random parity bit
                bool parity = std::bernoulli_distribution(0.5)(rng_);
                
                // Skip if no variables selected
                if (vars_in_xor.empty()) {
                    continue;
                }
                
                // Generate all assignments with wrong parity
                for (size_t mask = 0; mask < (1u << vars_in_xor.size()); mask++) {
                    int ones = __builtin_popcount(mask);
                    if ((ones % 2) != parity) {
                        // This assignment violates parity - add clause to forbid it
                        std::vector<Literal> clause;
                        for (size_t k = 0; k < vars_in_xor.size(); k++) {
                            bool val = (mask >> k) & 1;
                            clause.push_back(Literal(vars_in_xor[k].var_id(), !val));
                        }
                        add_clause(clause);
                    }
                }
            }
            
            bool is_sat = solve();
            
            if (is_sat) {
                successes++;
                std::cout << "Trial " << t << ": SAT\n";
            } else {
                std::cout << "Trial " << t << ": UNSAT\n";
            }
            
            clauses_ = saved_clauses;
            assignments_ = saved_assignment;
        }
        
        std::cout << "Had " << successes << " successes out of " << T << " trials\n";
        
        if (successes <= T/2) {
            return false;
        }
    }
    
    return true;
}

}