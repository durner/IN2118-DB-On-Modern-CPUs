#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>


namespace IN2118 {

class Register {
private:
    friend class RegisterManager;

    alignas(8) char value[8];

    Register() {}

public:
    // any type with at most 8 bytes size and standard layout
    template <typename T>
    inline typename std::enable_if<
        sizeof(T) <= 8 &&
        std::is_standard_layout<T>::value,
        T
    >::type get() const {
        T retval;
        memcpy(&retval, value, sizeof(T));
        return retval;
    }

    template <typename T>
    inline typename std::enable_if<
        sizeof(T) <= 8 &&
        std::is_standard_layout<T>::value
    >::type set(const T& new_value) {
        memcpy(value, &new_value, sizeof(T));
        memset(value + sizeof(T), 0, 8 - sizeof(T));
    }

    static inline Register from_bytes(const char* value) {
        Register r;
        memcpy(r.value, value, 8);
        return r;
    }

    bool operator==(const Register& reg) const {
        return memcmp(value, reg.value, 8) == 0;
    }

    size_t getHash() const {
        constexpr uint64_t prime = (1ull << 63) - 471;
        uint64_t typvalue = get<uint64_t>();
        return typvalue * prime;
    }

};
};

namespace std {
template <>
struct hash<IN2118::Register> {
    size_t operator()(const IN2118::Register& r) const { return r.getHash(); };
};
};
