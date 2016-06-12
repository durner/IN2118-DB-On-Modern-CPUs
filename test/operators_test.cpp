#include <cassert>
#include <iostream>
#include <sstream>
#include "operators.hpp"


char* make_relation(size_t num_tuples, size_t num_attributes) {
    uint64_t* relation = new uint64_t[num_tuples * num_attributes];
    for (size_t i = 0; i < num_tuples; i++) {
        for (size_t j = 0; j < num_attributes; j++) {
            relation[i * num_attributes + j] = 100 * (i + 1) + j + 1;
        }
    }
    return reinterpret_cast<char*>(relation);
}



int main(int argc, char** argv) {
    using namespace IN2118;

    char* small_relation = make_relation(100, 4);

    // print after tablescan
    TableScan scan_small_relation{small_relation, 100, 4};
    std::stringstream output;
    Print print_small_relation(&scan_small_relation, &output);
    print_small_relation.open();
    while (print_small_relation.next()) {
    }
    print_small_relation.close();
    for (size_t i = 0; i < 100; i++) {
        for (size_t j = 0; j < 4; j++) {
            uint64_t value;
            output >> value;
            assert(value == 100 * (i + 1) + j + 1);
        }
    }

    // HashJoin
    TableScan scan_small_relation_2{small_relation, 100, 4};
    HashJoin hashjoin(&scan_small_relation, &scan_small_relation_2, 2, 2);
    Print hashjoin_printer(&hashjoin, &output);
    hashjoin_printer.open();
    while (hashjoin_printer.next()) {
    }
    hashjoin_printer.close();

    delete[] small_relation;

    std::cout << "All tests successful!" << std::endl;
    return 0;
}
