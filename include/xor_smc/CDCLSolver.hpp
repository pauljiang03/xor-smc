#pragma once
#include "Literal.hpp"
#include <vector>
#include <memory>
#include <unordered_map>

namespace xor_smc {

class CDCLSolver {
public:
    class Clause {
    public:
        explicit Clause(const std::vector<Literal>& lits) 
            : literals(lits), learned(false) {}
        
        std::vector<Literal> literals;
        bool learned;
    };
    
    struct Assignment {
        int level;                     // Decision level
        bool value;                    // Assigned value
        std::shared_ptr<Clause> reason;  // Reason for propagation
    };

    CDCLSolver();
    
    // Core CDCL functionality
    void add_clause(const std::vector<Literal>& literals);
    bool solve();
    
    // Utility functions
    void set_num_variables(uint32_t num_vars);
    bool get_value(uint32_t var_id) const;
    
private:
    // CDCL components
    bool unit_propagate();
    std::shared_ptr<Clause> analyze_conflict(const std::shared_ptr<Clause>& conflict);
    void backtrack(int level);
    Literal pick_branching_variable();
    
    // Added the missing declaration
    int compute_backtrack_level(const std::shared_ptr<Clause>& learned_clause);
    
    // Data structures
    std::vector<Assignment> assignments_;
    std::vector<std::shared_ptr<Clause>> clauses_;
    std::vector<std::vector<std::shared_ptr<Clause>>> watches_;
    std::vector<uint32_t> vsids_score_;    // VSIDS heuristic scores
    std::vector<bool> phase_saving_;       // Phase saving
    int decision_level_;
};

}
