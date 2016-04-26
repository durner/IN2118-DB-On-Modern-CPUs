#pragma once

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>

namespace IN2118 {

#define page_size 4096

class CBufferFrame {
public:
    void* getData();
    void lock(bool exclusie);
    void unlock();
    void storeOnDisk();
    CBufferFrame(int fd, uint64_t page_id);
    ~CBufferFrame();

private:
    void* data;

    // latch mutex
    pthread_rwlock_t latch;

    uint64_t lsn;

    uint64_t page_id; // page id
    int fd; // file descriptor of the segment
    off_t offset; // offset in the file

    // after unfixing the page may be fixed again but already dirty. Hence, we need to write the page first.
    bool is_dirty = 0;
    bool is_fixed = 0;
    bool in_buffer = 0;
};

}; // ns
