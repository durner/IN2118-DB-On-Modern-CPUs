#pragma once

#include "buffer_frame.hpp"
#include "segment.hpp"
#include <cstdint>
#include <cstdio>
#include <stdlib.h>
#include <string.h>

namespace IN2118 {

struct CSlottedPageHeader {
    uint16_t _count_slots;
    uint16_t _next_slot;
    uint16_t _data;
    uint16_t _fragmented_space;
};

const uint16_t HEADER_SIZE = sizeof(CSlottedPageHeader);
const uint16_t DATA_SIZE = PAGE_SIZE - HEADER_SIZE;

struct CSlot {
    uint16_t _offset = 0;
    uint16_t _length = PAGE_SIZE;

public:
    bool IsFree() { return _offset == 0 && _length == PAGE_SIZE; }
    void Delete()
    {
        _offset = 0;
        _length = PAGE_SIZE;
    }
};

const uint16_t SLOT_SIZE = sizeof(CSlot);

class CSlottedPage {
private:
    CSlottedPageHeader _header;
    union {
        CSlot _slots[DATA_SIZE / SLOT_SIZE];
        char _data[DATA_SIZE];
    };

    void FindNextSlot();

public:
    CSlottedPage();
    ~CSlottedPage();

    bool IsFree(uint16_t size);
    uint16_t Allocate(uint16_t size);
    uint16_t Allocate(uint16_t size, uint16_t slot_id);
    void Store(uint16_t slot_id, const char* data);
    void Delete(uint16_t slot_id);
    void Redirect(uint16_t slot_id, TID tid);

    char* GetData(uint16_t slot_id);
    uint16_t GetLength(uint16_t slot_id);
    int64_t GetFreeSpace();
};

}; // ns
