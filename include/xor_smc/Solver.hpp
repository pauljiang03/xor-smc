#pragma once
#include "Literal.hpp"
#include <vector>
#include <memory>
#include <array>
#include <random>

namespace xor_smc {

class Solver {
public:
    // Constructor
    Solver();

    // Core interface
    void set_num_variables(uint32_t num_vars);
    bool solve();
    void add_clause(const std::vector<Literal>& literals);
    void add_unit_clause(const Literal& lit);

    // SMC and XOR handling
    void convert_xor_to_cnf(
        const std::vector<Literal>& xor_lits,
        std::vector<std::vector<Literal>>& cnf_clauses
    );

    bool solve_smc(
        const std::vector<uint32_t>& thresholds,
        const std::vector<std::vector<uint32_t>>& counting_variables,
        const std::vector<std::vector<uint32_t>>& fixed_variables,
        int num_xor_tries = 10,
        double confidence = 0.99
    );

    // Model access
    std::vector<bool> get_model() const;
    void add_blocking_clause(const std::vector<bool>& model);
    bool get_value(uint32_t var_id) const;
    uint32_t num_variables() const;
    uint32_t num_clauses() const;

private:
    // Internal classes
    class Clause {
    public:
        explicit Clause(const std::vector<Literal>& lits) 
            : literals(lits), watched{0, 1} {}
        
        std::vector<Literal> literals;
        std::array<size_t, 2> watched;
    };

    struct Assignment {
        int level;
        bool value;
        std::shared_ptr<Clause> reason;
    };

    // Watch list management
    void attach_watch(const std::shared_ptr<Clause>& clause, size_t watch_idx);
    void detach_watch(const std::shared_ptr<Clause>& clause, size_t watch_idx);
    bool update_watches(const std::shared_ptr<Clause>& clause, const Literal& false_lit);

    // Assignment and propagation
    bool assign(uint32_t var, bool value, int level, const std::shared_ptr<Clause>& reason);
    void unassign(uint32_t var);
    bool propagate();

    // Conflict analysis
    std::shared_ptr<Clause> analyze_conflict(const std::shared_ptr<Clause>& conflict);
    int compute_backtrack_level(const std::shared_ptr<Clause>& learnt_clause);
    void backtrack(int level);

    // Debug helpers
    void print_clause(const std::shared_ptr<Clause>& clause) const;
    void print_assignment() const;

    // Member variables
    std::vector<Assignment> assignments_;
    std::vector<std::shared_ptr<Clause>> clauses_;
    std::vector<std::vector<std::shared_ptr<Clause>>> watches_;
    std::vector<uint32_t> trail_;
    std::vector<uint32_t> propagation_queue_;
    std::vector<bool> seen_;
    std::shared_ptr<Clause> conflict_clause_;
    int decision_level_;
    std::mt19937 rng_;
};

} 