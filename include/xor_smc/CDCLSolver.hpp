#pragma once
#include "Literal.hpp"
#include <vector>
#include <memory>
#include <iostream>

namespace xor_smc {

class CDCLSolver {
public:
    class Clause {
    public:
        explicit Clause(const std::vector<Literal>& lits) 
            : literals(lits), watched{0, 1} {}
        
        std::vector<Literal> literals;
        std::array<size_t, 2> watched;  // Indices of watched literals
    };
    
    struct Assignment {
        int level;              // Decision level
        bool value;             // Assigned value
        std::shared_ptr<Clause> reason;  // Reason clause for propagated assignments
    };

    CDCLSolver();
    
    // Main interface
    void add_clause(const std::vector<Literal>& literals);
    bool solve();
    void set_num_variables(uint32_t num_vars);
    bool get_value(uint32_t var_id) const;
    
private:
    // Watched literal handling
    void attach_watch(const std::shared_ptr<Clause>& clause, size_t watch_idx);
    void detach_watch(const std::shared_ptr<Clause>& clause, size_t watch_idx);
    bool update_watches(const std::shared_ptr<Clause>& clause, const Literal& false_lit);
    
    // Assignment and propagation
    bool propagate();
    bool assign(uint32_t var, bool value, int level, const std::shared_ptr<Clause>& reason);
    void unassign(uint32_t var);
    
    // CDCL specific methods
    std::shared_ptr<Clause> analyze_conflict(const std::shared_ptr<Clause>& conflict);
    int compute_backtrack_level(const std::shared_ptr<Clause>& learnt_clause);
    void backtrack(int level);
    
    // Debug helpers
    void print_clause(const std::shared_ptr<Clause>& clause) const;
    void print_assignment() const;
    
    // Core data members
    std::vector<Assignment> assignments_;          // Variable assignments
    std::vector<std::shared_ptr<Clause>> clauses_; // All clauses
    std::vector<std::vector<std::shared_ptr<Clause>>> watches_;  // Two watch lists per variable
    
    // CDCL specific data members
    std::vector<uint32_t> trail_;                 // Assignment trail for conflict analysis
    std::vector<uint32_t> propagation_queue_;     // Queue for unit propagation
    std::vector<bool> seen_;                      // Temporary array for conflict analysis
    std::shared_ptr<Clause> conflict_clause_;     // Current conflict clause
    int decision_level_;                          // Current decision level
};

}