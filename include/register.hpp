#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>


namespace IN2118 {

class Register {
private:
    friend class RegisterManager;

    char value[8];

    Register() {}

public:
    // integral types or any other type with 8 bytes size and standard layout
    template <typename T>
    inline typename std::enable_if<
        std::is_integral<T>::value || (
            sizeof(T) == 8 &&
            std::is_standard_layout<T>::value
        ),
        T
    >::type get() const {
        T retval;
        memcpy(&retval, value, 8);
        return retval;
    }

    template <typename T>
    inline typename std::enable_if<
        std::is_integral<T>::value || (
            sizeof(T) == 8 &&
            std::is_standard_layout<T>::value
        )
    >::type set(const T& new_value) {
        memcpy(value, &new_value, 8);
    }

    static inline Register from_bytes(const char* value) {
        Register r;
        memcpy(r.value, value, 8);
        return r;
    }
};

};
