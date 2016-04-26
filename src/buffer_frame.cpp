#include "buffer_frame.hpp"

namespace IN2118 {

CBufferFrame::CBufferFrame(int fd, uint64_t page_id)
{
    pthread_rwlock_init(&this->latch, NULL);
    this->fd = fd;
    this->page_id = page_id;
    this->data = NULL;
    this->offset = (page_id & 0x00000000FFFFFFFF) * page_size;
}

void* CBufferFrame::getData()
{
    if (this->in_buffer) {
        if (this->data == NULL)
            this->data = static_cast<void*>(new uint8_t[page_size]);
        ssize_t result = pread(this->fd, this->data, page_size, this->offset);
        if (result < 0)
            std::cerr << "Looks like your data couldn't be read" << std::endl;
    }
    return this->data;
}

void CBufferFrame::storeOnDisk()
{
    if (this->is_dirty) {
        ssize_t result = pwrite(this->fd, this->data, page_size, this->offset);
        if (result < 0)
            std::cerr << "Looks like your data couldn't be written" << std::endl;
        this->is_dirty = 0;
    }
}

void CBufferFrame::lock(bool exclusive)
{
    if (exclusive)
        pthread_rwlock_wrlock(&this->latch);
    else
        pthread_rwlock_rdlock(&this->latch);
}

void CBufferFrame::unlock() { pthread_rwlock_unlock(&this->latch); }

CBufferFrame::~CBufferFrame()
{
    pthread_rwlock_destroy(&this->latch);
    this->storeOnDisk();

    if (this->data != NULL)
        delete[] static_cast<uint8_t*>(this->data);
}

}; // ns
