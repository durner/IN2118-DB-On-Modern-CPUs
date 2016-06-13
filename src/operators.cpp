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


void Projection::open() {
    input->open();
}

bool Projection::next() {
    return input->next();
}

std::vector<Register> Projection::getOutput() {
    auto input_regs = input->getOutput();
    std::vector<Register> output;
    output.reserve(register_ids.size());
    for (const auto reg_id : register_ids) {
        output.push_back(input_regs.at(reg_id));
    }
    return output;
}

void Projection::close() {
    input->close();
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

void HashJoin::close() {
    _input_left->close();
    _input_right->close();
}

bool HashJoin::next() {
    // run over right side until we find a match
    while (_input_right->next()) {
        // select element that is used for probing
        std::vector<Register> regs = _input_right->getOutput();
        Register right_register = regs[_register_right];
        std::unordered_map<Register, std::vector<Register>>::iterator it = map.find(right_register);
        // probing successful
        if (it != map.end()) {
            std::vector<Register> left_vector = it->second;
            size_t size = left_vector.size() + regs.size();
            // store current output
            _output = std::vector<Register>();
            _output.reserve(size);
            _output.insert(_output.end(), left_vector.begin(), left_vector.end());
            _output.insert(_output.end(), regs.begin(), regs.end());
            return true;
        }
    }
    // end of right table
    return false;
}

std::vector<Register> HashJoin::getOutput() {
    return _output;
}

};
