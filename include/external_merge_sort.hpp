#pragma once

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <queue>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>

namespace IN2118 {

template <typename ElementType>
struct CQueueElement {
    ElementType value;
    int buffer_index;

    bool operator<(const CQueueElement<ElementType>& qe) const { return qe.value < value; }

    bool operator==(const CQueueElement<ElementType>& qe) const { return qe.value == value; }
};

template <typename ElementType>
struct CValueBuffer {
    ElementType* buffer;
    int file_index;
    int buffer_offset;
    int buffer_length;
};

void ExternalSort(int fd_input, uint64_t size, int fd_output, uint64_t mem_size);

}; // ns
