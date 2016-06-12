#include "operators.hpp"


namespace IN2118 {

void Print::open() {
    input->open();
}

bool Print::next() {
    if (input->next()) {
        for (const auto& reg : input->getOutput()) {
            switch (mode) {
                case print_mode::UINT:
                    (*stream) << reg.get<uint64_t>() << '\t';
                    break;
                case print_mode::STR:
                    (*stream) << reg.get<fixed_str>().str() << '\t';
                    break;
            }
        }
        (*stream) << std::endl;
        return true;
    } else {
        return false;
    }
}

std::vector<Register> Print::getOutput() {
    return std::vector<Register>{};
}

void Print::close() {
    input->close();
}


void TableScan::open() {
    pos = 0;
}

bool TableScan::next() {
    return pos < num_tuples;
}

std::vector<Register> TableScan::getOutput() {
    std::vector<Register> regs;
    regs.reserve(num_attributes);
    for (size_t i = 0; i < num_attributes; i++) {
        regs.push_back(Register::from_bytes(relation + (pos * num_attributes + i) * 8));
    }
    pos++;
    return regs;
}

void TableScan::close() {
}

void HashJoin::open() {
    _input_left->open();
    _input_right->open();

    // create the hashmap
    while (_input_left->next()) {
        std::vector<Register> regs = _input_left->getOutput();
        map.insert({regs[_register_left], regs});
    }
}

void HashJoin::close() {}
bool HashJoin::next() {
    return false;
}

std::vector<Register> HashJoin::getOutput() {
    std::vector<Register> regs;
    return regs;
}

};
