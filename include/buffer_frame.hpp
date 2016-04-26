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
    uint64_t getPageId();
    void* getData();
    void lock(bool exclusie);
    void unlock(bool is_dirty);
    void storeOnDisk();
    CBufferFrame(int fd, uint64_t page_id);
    ~CBufferFrame();

private:
    void* _data;

    // latch mutex
    pthread_rwlock_t _latch;

    uint64_t _page_id; // page id
    int _fd; // file descriptor of the segment
    off_t _offset; // offset in the file

    // after unfixing the page may be fixed again but already dirty. Hence, we need to write the page first.
    bool _is_dirty = 0;
    bool _in_buffer = 0;
};

}; // ns
