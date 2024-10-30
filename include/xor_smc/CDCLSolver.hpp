#pragma once
#include "Literal.hpp"
#include <vector>
#include <memory>

namespace xor_smc {

class CDCLSolver {
public:
    class Clause {
    public:
        explicit Clause(const std::vector<Literal>& lits) 
            : literals(lits) {}
        
        std::vector<Literal> literals;
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
    bool check_clause(const std::shared_ptr<Clause>& clause) const;
    void print_clause(const std::vector<Literal>& literals) const;
    void print_state() const;
    
    std::vector<Assignment> assignments_;
    std::vector<std::shared_ptr<Clause>> clauses_;
    int decision_level_;
};

}