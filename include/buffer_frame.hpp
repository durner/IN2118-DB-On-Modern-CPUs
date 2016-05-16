#pragma once

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>

namespace IN2118 {

const int PAGE_SIZE = 4*4096;
#define page_size 4*4096

class CBufferFrame {
public:
    uint64_t getPageId();
    void* getData();
    void lock(bool exclusive);
    void unlock();
    void setDirty();
    CBufferFrame(int fd, uint64_t page_id);
    ~CBufferFrame();
    int _number_of_locks;

private:
    void* _data;

    // latch mutex
    pthread_rwlock_t _latch;

    uint64_t _page_id; // page id
    int _fd; // file descriptor of the segment
    off_t _offset; // offset in the file

    bool _is_dirty = 0;

    void storeOnDisk();
};

}; // ns
