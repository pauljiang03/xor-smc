#pragma once
#include <cstdint>

namespace xor_smc {

class Literal {
public:
    Literal(uint32_t var, bool positive) 
        : data_((var << 1) | static_cast<uint32_t>(positive)) {}
    
    uint32_t var_id() const { return data_ >> 1; }
    bool is_positive() const { return data_ & 1; }
    
private:
    uint32_t data_;  // Variable ID in upper bits, sign in lowest bit
};

}