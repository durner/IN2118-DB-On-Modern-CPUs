#include "buffer_manager.hpp"

namespace IN2118 {

CBufferManager::CBufferManager(unsigned page_count) {}

CBufferFrame& CBufferManager::fixPage(uint64_t page_id, bool exclusive)
{
    CBufferFrame* bf = new CBufferFrame(0, page_id);
    return *bf;
}

void CBufferManager::unfixPage(CBufferFrame& frame, bool is_dirty) {}

CBufferManager::~CBufferManager() {}

} // ns
