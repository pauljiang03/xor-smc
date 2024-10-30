#pragma once
#include <cstdint>
#include <string>

namespace xor_smc {

class Literal {
public:
    Literal(uint32_t var_id, bool positive = true) 
        : var_id_(var_id), positive_(positive) {}
    
    uint32_t var_id() const { return var_id_; }
    bool is_positive() const { return positive_; }
    Literal negate() const { return Literal(var_id_, !positive_); }
    
    bool operator==(const Literal& other) const {
        return var_id_ == other.var_id_ && positive_ == other.positive_;
    }
    
    std::string to_string() const {
        return (positive_ ? "" : "-") + std::to_string(var_id_);
    }

private:
    uint32_t var_id_;
    bool positive_;
};

} 
