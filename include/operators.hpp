#pragma once

#include "register.hpp"
#include <ostream>
#include <vector>


namespace IN2118 {

class fixed_str {
private:
    char chars[8];

public:
    std::string str() const {
        return std::string{chars, 8};
    }
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
    enum class print_mode {UINT, STR};

private:
    Operator* input;
    std::ostream* stream;
    print_mode mode;

public:
    Print(Operator* input, std::ostream* stream,
          const print_mode mode = print_mode::UINT)
    : input(input), stream(stream), mode(mode) {}

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
    TableScan(const char* relation, const size_t num_tuples,
              const size_t num_attributes)
    : relation(relation), num_tuples(num_tuples),
      num_attributes(num_attributes), pos(0) {}

    virtual void open();
    virtual bool next();
    std::vector<Register> getOutput();
    virtual void close();
};

};
