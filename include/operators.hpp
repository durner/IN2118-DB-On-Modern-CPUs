#pragma once

#include "register.hpp"
#include <ostream>
#include <vector>
#include <unordered_map>

namespace IN2118 {

class fixed_str {
private:
    char chars[8];

public:
    std::string str() const { return std::string{ chars, 8 }; }
};

class Operator {
public:
    virtual void open() = 0;
    virtual bool next() = 0;
    virtual std::vector<Register> getOutput() = 0;
    virtual void close() = 0;
};

class Print : public Operator {
public:
    enum class print_mode { UINT, STR };

private:
    Operator* input;
    std::ostream* stream;
    print_mode mode;

public:
    Print(Operator* input, std::ostream* stream, const print_mode mode = print_mode::UINT)
        : input(input)
        , stream(stream)
        , mode(mode)
    {
    }

    virtual void open();
    virtual bool next();
    virtual std::vector<Register> getOutput();
    virtual void close();
};

class TableScan : public Operator {
private:
    const char* relation;
    const size_t num_tuples;
    const size_t num_attributes;
    size_t pos;

public:
    TableScan(const char* relation, const size_t num_tuples, const size_t num_attributes)
        : relation(relation)
        , num_tuples(num_tuples)
        , num_attributes(num_attributes)
        , pos(0)
    {
    }

    virtual void open();
    virtual bool next();
    std::vector<Register> getOutput();
    virtual void close();
};

class Projection : public Operator {
private:
    Operator* input;
    std::vector<size_t> register_ids;

public:
    Projection(Operator* input, const std::vector<size_t> register_ids)
        : input(input)
        , register_ids(register_ids)
    {
    }

    virtual void open();
    virtual bool next();
    std::vector<Register> getOutput();
    virtual void close();
};

template<typename T>
class Selection : public Operator {
private:
    Operator* _input;
    unsigned _register;
    Register _constant;
    std::vector<Register> _output;

public:
    Selection(Operator* input, unsigned reg, T constant)
        : _input(input)
        , _register(reg)
        , _constant(Register::from_bytes(reinterpret_cast<const char*>(&constant)))
    {
    }

    virtual void open() {
        _input->open();
    }
    
    virtual bool next() {
        while(_input->next()) {
            _output = _input->getOutput();
            if(_constant == _output[_register]) {
                return true;
            }
        }
        return false;
    }

    std::vector<Register> getOutput() {
        return _output;
    }

    virtual void close() {
        _input->close();
    }
};

class HashJoin : public Operator {
private:
    Operator* _input_left;
    Operator* _input_right;
    unsigned _register_left;
    unsigned _register_right;
    std::vector<Register> _output;
    std::unordered_multimap<Register, std::vector<Register>> map;

public:
    HashJoin(Operator* input_left, Operator* input_right, unsigned register_left, unsigned register_right)
        : _input_left(input_left)
        , _input_right(input_right)
        , _register_left(register_left)
        , _register_right(register_right)
    {
    }

    virtual void open();
    virtual bool next();
    std::vector<Register> getOutput();
    virtual void close();
};
};
