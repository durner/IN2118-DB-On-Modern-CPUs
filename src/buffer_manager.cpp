#include "buffer_manager.hpp"

namespace IN2118 {

CBufferManager::CBufferManager(unsigned page_count)
{
    _buffer_frames = new CBufferFrame[page_count];
}

CBufferFrame& CBufferManager::fixPage(uint64_t page_id, bool exclusive)
{
    return *_buffer_frames;
}

void CBufferManager::unfixPage(CBufferFrame& frame, bool is_dirty) {}

CBufferManager::~CBufferManager() {}

} // ns
