#pragma once

#include "buffer_manager.hpp"
#include <atomic>
#include <typeinfo>

namespace IN2118 {

struct TID {
    uint64_t _page_id;
    uint32_t _slot_id;

    bool operator==(const TID& tid) const { return _page_id == tid._page_id && _slot_id == tid._slot_id; }

    bool operator<(const TID& tid) const
    {
        return _page_id < tid._page_id || (_page_id == tid._page_id && _slot_id < tid._slot_id);
    }
};

class CSegment {
protected:
    uint16_t _id;
    std::atomic<int64_t> _size; // size in pages
    CBufferManager& _buffer_manager;

public:
    CSegment(CBufferManager& buffer_manager, uint16_t id)
        : _id(id)
        , _size(0)
        , _buffer_manager(buffer_manager)
    {
    }

    uint16_t GetID() { return _id; }

    int64_t GetSize() { return _size.load(); }

    uint64_t GetPageID(uint64_t page_number)
    {
        uint64_t result = _id;
        result = result << 48;
        result += page_number;
        return result;
    }
};

} // ns

namespace std {
template <>
struct hash<IN2118::TID> {
    size_t operator()(const IN2118::TID& tid) const
    {
        return (hash<uint64_t>()(tid._page_id) ^ (hash<uint16_t>()(tid._slot_id) << 1)) >> 1;
    }
};
}
