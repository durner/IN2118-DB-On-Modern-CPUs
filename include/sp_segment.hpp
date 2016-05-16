#pragma once

#include "schema/record.hpp"
#include "segment.hpp"
#include <fcntl.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

namespace IN2118 {

class CSPSegment : public CSegment {
public:
    TID insert(const Record& r);
    // Searches through the segment’s pages looking for a page with enough space to store r.
    // Returns the TID identifying the location where r was stored. Note: This can be implemented much more
    // efficiently with a free space bitmap as described in chapter 3, slide 3, but you are not required to do this.

    bool remove(TID tid);
    // Deletes the record pointed to by tid and updates the page header accordingly.

    Record lookup(TID tid);
    // Returns the read-only record (Record.hpp) associated with TID tid.

    bool update(TID tid, const Record& r);
    // Updates the record pointed to by tid with the content of record r.

    CSPSegment(uint16_t segment_id, CBufferManager& buffer_manager);

private:
    uint64_t _page_number = 0;
};

}; // ns
