#pragma once
#include "schema/Schema.hpp"
#include "schema/record.hpp"
#include "segment.hpp"
#include <iostream>
#include <memory>

namespace IN2118 {
class CSchemaSegment : public CSegment {
public:
    CSchemaSegment(CBufferManager& buffer_manager, uint16_t segment_id, std::unique_ptr<Schema> schema);

    CSchemaSegment(CBufferManager& buffer_manager, uint16_t segment_id);

    std::unique_ptr<Schema> getSchema() { return std::move(schema); }

private:
    std::unique_ptr<Schema> schema;

    void write();
    void load();
};

}; // ns
