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

    for (uint16_t i = 0; i < this->_size; i++) {
        CBufferFrame& frame = this->_buffer_manager.fixPage(this->GetPageID(i), true);
        CSlottedPage* page = static_cast<CSlottedPage*>(frame.getData());
#if DEBUG_DETAIL
        std::cout << "USE\t" << this->GetID() << "\t" << i << std::endl;
#endif
        if (page->IsFree(buffer_size)) {
            uint16_t slot_id = page->Allocate(buffer_size);
            page->Store(slot_id, buffer);
            this->_buffer_manager.unfixPage(frame, true);
            return TID{ i, slot_id };
        }
        else {
#if DEBUG_DETAIL
            std::cout << "FULL"
                      << "\t bytes free: " << page->GetFreeSpace() << std::endl;
#endif

            this->_buffer_manager.unfixPage(frame, false);
        }
    }

    // slotted page gets created
    uint64_t page_id = this->_size;
    this->_size++;
#if DEBUG
    std::cout << "CREATE\t" << this->GetID() << "\t" << page_id << std::endl;
#endif
    CBufferFrame& frame = this->_buffer_manager.fixPage(this->GetPageID(page_id), true);
    auto data = frame.getData();
    memcpy(data, new CSlottedPage(), PAGE_SIZE);
    CSlottedPage* page = static_cast<CSlottedPage*>(data);

    // allocate a slot and store the record
    uint16_t slot_id = page->Allocate(buffer_size);
    page->Store(slot_id, buffer);
    this->_buffer_manager.unfixPage(frame, true);
    return TID{ page_id, slot_id };
}

bool CSPSegment::remove(TID tid)
{
    uint64_t page_id = tid._page_id;
    uint16_t slot_id = tid._slot_id;

    CBufferFrame& frame = this->_buffer_manager.fixPage(this->GetPageID(page_id), true);
    CSlottedPage* page = static_cast<CSlottedPage*>(frame.getData());
    page->Delete(slot_id);
    this->_buffer_manager.unfixPage(frame, true);
    return true;
}

Record CSPSegment::lookup(TID tid)
{
    uint64_t page_id = tid._page_id;
    uint16_t slot_id = tid._slot_id;

    CBufferFrame& frame = this->_buffer_manager.fixPage(this->GetPageID(page_id), false);
    CSlottedPage* page = static_cast<CSlottedPage*>(frame.getData());

    if (page->GetLength(slot_id) == 0 && page->GetData(slot_id) != 0) {
        TID* tid_redirected = reinterpret_cast<TID*>(page->GetData(slot_id));
        CBufferFrame& redirect_frame = this->_buffer_manager.fixPage(this->GetPageID(tid_redirected->_page_id), false);
        CSlottedPage* page_redirected = static_cast<CSlottedPage*>(redirect_frame.getData());
        Record rec{ page_redirected->GetLength(tid_redirected->_slot_id),
            page_redirected->GetData(tid_redirected->_slot_id) };
        this->_buffer_manager.unfixPage(frame, false);
        this->_buffer_manager.unfixPage(redirect_frame, false);
        return rec;
    }
    else {
        Record rec{ page->GetLength(slot_id), page->GetData(slot_id) };
        this->_buffer_manager.unfixPage(frame, false);
        return rec;
    }
}

bool CSPSegment::update(TID tid, const Record r)
{
    CBufferFrame& frame = this->_buffer_manager.fixPage(this->GetPageID(tid._page_id), true);
    CSlottedPage* page = static_cast<CSlottedPage*>(frame.getData());
    uint64_t data_size = r.getLen();
    uint16_t slot_id = tid._slot_id;

    if (page->IsFree(data_size)) {
        // save on same page
        page->Allocate(data_size, slot_id);
        page->Store(slot_id, r.getData());
    }
    else {
        // redirect needed
        this->_buffer_manager.unfixPage(frame, false);
        TID redirect_tid = this->insert(r);
        frame = this->_buffer_manager.fixPage(this->GetPageID(tid._page_id), true);
        page = static_cast<CSlottedPage*>(frame.getData());
        page->Redirect(slot_id, redirect_tid);
    }

    this->_buffer_manager.unfixPage(frame, true);
    return true;
}

}; // ns
