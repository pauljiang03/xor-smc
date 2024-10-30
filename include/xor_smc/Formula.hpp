#pragma once
#include "Literal.hpp"
#include <vector>

namespace xor_smc {

class Clause {
public:
    explicit Clause(const std::vector<Literal>& literals) 
        : literals_(literals) {}
    
    const std::vector<Literal>& literals() const { return literals_; }

private:
    std::vector<Literal> literals_;
};

class Formula {
public:
    void add_clause(const std::vector<Literal>& literals) {
        clauses_.emplace_back(literals);
    }
    
    const std::vector<Clause>& clauses() const { return clauses_; }
    
    size_t num_variables() const {
        size_t max_var = 0;
        for (const auto& clause : clauses_) {
            for (const auto& lit : clause.literals()) {
                max_var = std::max(max_var, static_cast<size_t>(lit.var_id()));
            }
        }
        return max_var + 1;
    }

private:
    std::vector<Clause> clauses_;
};

}
