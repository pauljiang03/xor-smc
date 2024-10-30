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
        int level;
        bool value;
        std::shared_ptr<Clause> reason;
    };

    CDCLSolver();
    
    void add_clause(const std::vector<Literal>& literals);
    bool solve();
    void set_num_variables(uint32_t num_vars);
    bool get_value(uint32_t var_id) const;
    
private:
    // Watched literal handling
    void attach_watch(const std::shared_ptr<Clause>& clause, size_t watch_idx);
    void detach_watch(const std::shared_ptr<Clause>& clause, size_t watch_idx);
    bool update_watches(const std::shared_ptr<Clause>& clause, const Literal& false_lit);
    
    // Unit propagation
    bool propagate();
    bool assign(uint32_t var, bool value, int level, const std::shared_ptr<Clause>& reason);
    void unassign(uint32_t var);
    
    // Debug helpers
    void print_clause(const std::shared_ptr<Clause>& clause) const;
    void print_assignment() const;
    
    // Data members
    std::vector<Assignment> assignments_;
    std::vector<std::shared_ptr<Clause>> clauses_;
    std::vector<std::vector<std::shared_ptr<Clause>>> watches_;  // Two watch lists per variable
    std::vector<uint32_t> propagation_queue_;
    int decision_level_;
};

}