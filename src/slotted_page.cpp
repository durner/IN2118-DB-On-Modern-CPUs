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
    int64_t required = size + SLOT_SIZE;
    return this->GetFreeSpace() >= required;
}

uint16_t CSlottedPage::Allocate(uint16_t size)
{
    assert(this->IsFree(size));
    uint16_t slot_id = this->_header._next_slot;
    this->_header._data -= size;
    this->_slots[slot_id] = CSlot{ this->_header._data, size };
    this->FindNextSlot();
    return slot_id;
}

void CSlottedPage::Delete(uint16_t slot_id)
{
    if (this->_slots[slot_id]._offset == this->_header._data)
        this->_header._data += this->_slots[slot_id]._length;

    this->_slots[slot_id].Delete();

    if (slot_id < this->_header._next_slot)
        this->_header._next_slot = slot_id;
}

void CSlottedPage::Store(uint16_t slot_id, const char* data)
{
    memcpy(this->_data + this->_slots[slot_id]._offset, data, this->_slots[slot_id]._length);
}

char* CSlottedPage::GetData(uint16_t slot_id) { return this->_data + this->_slots[slot_id]._offset; }

uint16_t CSlottedPage::GetLength(uint16_t slot_id) { return this->_slots[slot_id]._length; }

void CSlottedPage::FindNextSlot()
{
    uint16_t next_slot = this->_header._next_slot;
    next_slot++;

    // TODO: reuse slots

    this->_header._count_slots++;
    this->_header._next_slot = next_slot;
}

}; // ns
