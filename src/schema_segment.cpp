#include "schema_segment.hpp"

namespace IN2118 {

CSchemaSegment::CSchemaSegment(CBufferManager& bm, uint16_t segment_id, std::unique_ptr<Schema> schema)
        : CSegment(bm, segment_id), schema(std::move(schema))
{
    load();
}

CSchemaSegment::CSchemaSegment(CBufferManager& bm, uint16_t segment_id)
        : CSegment(bm, segment_id), schema(std::unique_ptr<Schema>(new Schema()))
{
    write();
}

void CSchemaSegment::load()
{

    CBufferFrame& bf = this->_buffer_manager.fixPage(this->GetPageID(0), true);
    char* frame_data = static_cast<char*>(bf.getData());


    memcpy(frame_data, schema.get(), IN2118::PAGE_SIZE);

    this->_buffer_manager.unfixPage(bf, true);
}

void CSchemaSegment::write()
{
    CBufferFrame& bf = this->_buffer_manager.fixPage(this->GetPageID(0), true);

    const char* data = static_cast<char*>(bf.getData());
    char buffer[IN2118::PAGE_SIZE];

    memcpy(buffer, data, IN2118::PAGE_SIZE);

    schema.reset(reinterpret_cast<Schema*>(buffer));

    this->_buffer_manager.unfixPage(bf, false);
}
};
