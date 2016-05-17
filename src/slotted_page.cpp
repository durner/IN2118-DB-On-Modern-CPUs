#include "slotted_page.hpp"
#include <assert.h>
#include <math.h>
#include <queue>

namespace IN2118 {
CSlottedPage::CSlottedPage()
{
    this->_header._count_slots = 0;
    this->_header._next_slot = 0;
    this->_header._data = DATA_SIZE;
    this->_header._fragmented_space = 0;
}
CSlottedPage::~CSlottedPage() {}

int64_t CSlottedPage::GetFreeSpace()
{
    int64_t lower = this->_header._count_slots * SLOT_SIZE;
    int64_t upper = this->_header._data;
    return upper - lower;
}

bool CSlottedPage::IsFree(uint16_t size)
{
    if (size < sizeof(TID))
        size = sizeof(TID);
    int64_t required = size + SLOT_SIZE;
    return this->GetFreeSpace() >= required;
}

uint16_t CSlottedPage::Allocate(uint16_t size)
{
    assert(this->IsFree(size));
    uint16_t slot_id = this->_header._next_slot;

    this->_slots[slot_id]._length = size;
    // ensure the storage of a TID
    if (size < sizeof(TID))
        size = sizeof(TID);
    this->_header._data -= size;
    this->_slots[slot_id]._offset = this->_header._data;


    this->FindNextSlot();
    return slot_id;
}

uint16_t CSlottedPage::Allocate(uint16_t size, uint16_t slot_id)
{
    assert(this->IsFree(size));

    this->_slots[slot_id]._length = size;

    // ensure storage of a TID
    if (size < sizeof(TID))
        size = sizeof(TID);
    this->_header._data -= size;
    this->_slots[slot_id]._offset = this->_header._data;

    return slot_id;
}

void CSlottedPage::Delete(uint16_t slot_id)
{
    if (this->_slots[slot_id]._offset == this->_header._data)
        this->_header._data += this->_slots[slot_id]._length;

    this->_slots[slot_id].Delete();

    // reuse the slot later on
    //if (slot_id < this->_header._next_slot) {
    //    this->_header._next_slot = slot_id;
    //}
}

void CSlottedPage::Store(uint16_t slot_id, const char* data)
{
    memcpy(this->_data + this->_slots[slot_id]._offset, data, this->_slots[slot_id]._length);
}

void CSlottedPage::Redirect(uint16_t slot_id, TID tid)
{
    this->_slots[slot_id]._length = 0;
    memcpy(this->_data + this->_slots[slot_id]._offset, &tid, sizeof(TID));
}


char* CSlottedPage::GetData(uint16_t slot_id) { return this->_data + this->_slots[slot_id]._offset; }

uint16_t CSlottedPage::GetLength(uint16_t slot_id) { return this->_slots[slot_id]._length; }

void CSlottedPage::FindNextSlot()
{
    uint16_t next_slot = this->_header._next_slot + 1;

    // due to reusing of old slots the next slot might not be free!
    //while (!this->_slots[next_slot].IsFree() && next_slot <= this->_header._count_slots) {
    //    next_slot++;
    //}
    //if (next_slot > this->_header._count_slots)
        this->_header._count_slots++;
    this->_header._next_slot = next_slot;
}

}; // ns
