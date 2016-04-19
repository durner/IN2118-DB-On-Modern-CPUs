#pragma once

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <unordered_map>

namespace IN2118 {

struct CBufferFrame {
    void* data;

    // latch mutex
    pthread_rwlock_t latch;

    uint64_t lsn;
    uint64_t page_id;

    // after unfixing the page may be fixed again but already dirty. Hence, we need to write the page first.
    bool dirty;
    bool fixed;

    void* getData() { return data; }
};

class CBufferManager {
public:
    CBufferManager(unsigned page_count);
    CBufferFrame& fixPage(uint64_t page_id, bool exclusive);
    void unfixPage(CBufferFrame& frame, bool is_dirty);
    ~CBufferManager();

private:
    CBufferFrame* _buffer_frames;
    pthread_mutex_t _hashtable_lock;

};

}; // ns
