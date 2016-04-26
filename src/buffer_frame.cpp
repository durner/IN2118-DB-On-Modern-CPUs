#include "buffer_frame.hpp"

namespace IN2118 {

CBufferFrame::CBufferFrame(int fd, uint64_t page_id)
{
    pthread_rwlock_init(&this->_latch, NULL);
    this->_fd = fd;
    this->_page_id = page_id;
    this->_offset = (page_id & 0x0000FFFFFFFFFFFF) * page_size;
    this->_data = static_cast<void*>(new uint8_t[page_size]);
    ssize_t result = pread(this->_fd, this->_data, page_size, this->_offset);
    if (result < 0)
        std::cerr << "Looks like your data couldn't be read" << std::endl;
}

void* CBufferFrame::getData() { return this->_data; }

uint64_t CBufferFrame::getPageId() { return this->_page_id; }

void CBufferFrame::storeOnDisk()
{
    if (this->_is_dirty) {
        ssize_t result = pwrite(this->_fd, this->_data, page_size, this->_offset);
        if (result < 0)
            std::cerr << "Looks like your data couldn't be written" << std::endl;
        this->_is_dirty = 0;
    }
}

void CBufferFrame::lock(bool exclusive)
{
    if (exclusive)
        pthread_rwlock_wrlock(&this->_latch);
    else
        pthread_rwlock_rdlock(&this->_latch);
}

void CBufferFrame::setDirty() { this->_is_dirty = 1; }

void CBufferFrame::unlock() { pthread_rwlock_unlock(&this->_latch); }

CBufferFrame::~CBufferFrame()
{
    this->storeOnDisk();

    delete[] static_cast<uint8_t*>(this->_data);

    pthread_rwlock_destroy(&this->_latch);
}

}; // ns
