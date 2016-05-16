#include "slotted_page.hpp"
#include "sp_segment.hpp"

namespace IN2118 {

CSPSegment::CSPSegment(uint16_t segment_id, CBufferManager& buffer_manger)
    : CSegment(buffer_manger, segment_id)
{
}

TID CSPSegment::insert(const Record& r)
{
    auto buffer = r.getData();
    auto buffer_size = r.getLen();

    for (uint16_t i = 0; i < this->_page_number; i++) {
        CBufferFrame& frame = _buffer_manager.fixPage(this->GetPageID(i), true);
        CSlottedPage* page = static_cast<CSlottedPage*>(frame.getData());
#if DEBUG_DETAIL
        std::cout << "USE\t" << this->GetID() << "\t" << i << std::endl;
#endif
        if (page->IsFree(buffer_size)) {
            uint32_t slot_id = page->Allocate(buffer_size);
            page->Store(slot_id, buffer);
            _buffer_manager.unfixPage(frame, true);
            return TID{ i, slot_id };
        }
        else {
#if DEBUG_DETAIL
            std::cout << "FULL"
                      << "\t bytes free: " << page->GetFreeSpace() << std::endl;
#endif

            _buffer_manager.unfixPage(frame, false);
        }
    }

    // slotted page gets created
    uint64_t page_id = this->_page_number++;
#if DEBUG
    std::cout << "CREATE\t" << this->GetID() << "\t" << page_id << std::endl;
#endif
    CBufferFrame& frame = _buffer_manager.fixPage(this->GetPageID(page_id), true);
    auto data = frame.getData();
    memcpy(data, new CSlottedPage(), PAGE_SIZE);
    CSlottedPage* page = static_cast<CSlottedPage*>(data);

    // allocate a slot and store the record
    uint32_t slot_id = page->Allocate(buffer_size);
    page->Store(slot_id, buffer);
    _buffer_manager.unfixPage(frame, true);
    return TID{ page_id, slot_id };
}

bool CSPSegment::remove(TID tid)
{
    uint64_t page_id = tid._page_id;
    uint32_t slot_id = tid._slot_id;

    CBufferFrame& frame = _buffer_manager.fixPage(this->GetPageID(page_id), true);
    CSlottedPage* page = static_cast<CSlottedPage*>(frame.getData());
    page->Delete(slot_id);
    _buffer_manager.unfixPage(frame, true);
    return true;
}

Record CSPSegment::lookup(TID tid)
{
    uint64_t page_id = tid._page_id;
    uint32_t slot_id = tid._slot_id;

    CBufferFrame& frame = _buffer_manager.fixPage(this->GetPageID(page_id), false);
    CSlottedPage* page = (CSlottedPage*)frame.getData();

    Record rec{ page->GetLength(slot_id), page->GetData(slot_id) };
    _buffer_manager.unfixPage(frame, false);
    return rec;
}

/* bool CSPSegment::update(TID tid, const Record& r) {
    auto buffer = r.getData();
        auto buffer_size = r.getLen();

        // first get old and check if the size is the same
        uint64_t page_id = tid._page_id;
        uint32_t slot_id = tid._slot_id;


    auto frame = bufferManager.fixPage(PID(segmentId, pageId), true);
        SlottedPage* page = (SlottedPage*) frame->getData();
        auto serialized = page->getData(slotId);
        auto oldRecord = Record::deserialize(serialized);

        if(oldRecord->getLen() == r->getLen()){
                // just overwrite
                page->storeData(slotId, buffer);

        } else {
                if(!page->canReallocateSlot(slotId, bufferSize)) {
                        std::cout << "SPSegment.update could not allocate size " << bufferSize << " (from " <<
oldRecord->getLen() << ")" << std::endl;

                        bufferManager.unfixPage(frame, false);
                        return false;
                }

                // reallocate
                page->reallocateSlot(slotId, bufferSize);
                page->storeData(slotId, buffer);
        }

        bufferManager.unfixPage(frame, true);
        return true;
} */

}; // ns
