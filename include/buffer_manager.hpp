#pragma once

#include "buffer_frame.hpp"

#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <unordered_map>
#include <list>

namespace IN2118 {

class CBufferManager {
public:
    CBufferManager(unsigned page_count);
    CBufferFrame& fixPage(uint64_t page_id, bool exclusive);
    void unfixPage(CBufferFrame& frame, bool is_dirty);
    ~CBufferManager();

private:
    int _buffer_count;
    int _max_count;

    std::unordered_map<uint64_t, CBufferFrame*> _frame_map; // O(1) to get from page_id to Buffer!
    std::unordered_map<uint64_t, int> _fd_map; // O(1) to get fd from page_id
    std::list<CBufferFrame*> _lru_list;

    pthread_mutex_t _lru_mutex;
    pthread_rwlock_t _hashmap_lock;
};

}; // ns
