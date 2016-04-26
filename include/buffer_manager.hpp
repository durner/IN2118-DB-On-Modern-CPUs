#pragma once

#include "buffer_frame.hpp"

#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <unordered_map>

namespace IN2118 {

class CBufferManager {
public:
    CBufferManager(unsigned page_count);
    CBufferFrame& fixPage(uint64_t page_id, bool exclusive);
    void unfixPage(CBufferFrame& frame, bool is_dirty);
    ~CBufferManager();

private:
    std::unordered_map<uint64_t, CBufferFrame> frame_map; // O(1) to get from page_id to Buffer!
    CBufferFrame* _buffer_frames;
};

}; // ns
