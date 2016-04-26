#include "buffer_manager.hpp"

namespace IN2118 {

CBufferManager::CBufferManager(unsigned page_count)
{
    this->_max_count = page_count;
    this->_buffer_count = 0;
    pthread_mutex_init(&this->_lru_mutex, NULL);
    pthread_rwlock_init(&this->_hashmap_lock, NULL);
}

CBufferFrame& CBufferManager::fixPage(uint64_t page_id, bool exclusive)
{
    CBufferFrame* buffer_frame;

    pthread_rwlock_rdlock(&this->_hashmap_lock);

    auto find_result = this->_frame_map.find(page_id);
    if (find_result != this->_frame_map.end()) {
        buffer_frame = find_result->second;

        // delete element from lru list if it exists there
        pthread_mutex_lock(&this->_lru_mutex);
        this->_lru_list.remove(buffer_frame);
        pthread_mutex_unlock(&this->_lru_mutex);
    }
    else {
        // switch to write lock
        pthread_rwlock_unlock(&this->_hashmap_lock);
        pthread_rwlock_wrlock(&this->_hashmap_lock);

        buffer_frame = NULL;
        int segment_number = page_id >> 48;

        auto find_fd = this->_fd_map.find(segment_number);

        int fd;
        if (find_fd != this->_fd_map.end()) {
            std::string file = "tmp/" + std::to_string(segment_number);
            fd = open(file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            if (fd < 0)
                std::cerr << "Looks like your segment couldn't be created" << std::endl;
        }
        else
            fd = find_fd->second;

        if (this->_buffer_count < this->_max_count) {
            this->_buffer_count++;
            CBufferFrame* bf = new CBufferFrame(fd, page_id);
            this->_frame_map.emplace(page_id, bf);
            buffer_frame = bf;
        }
        else {
            pthread_mutex_lock(&this->_lru_mutex);

            if (this->_lru_list.empty()) {
                std::cerr << "The buffer is full and we cannot evict a page... I'm sorry!" << std::endl;
                buffer_frame = NULL;
            }
            else {
                CBufferFrame* evict_frame = this->_lru_list.back();
                evict_frame->storeOnDisk();
                this->_lru_list.pop_back();
                this->_frame_map.erase(evict_frame->getPageId());
                delete evict_frame;

                CBufferFrame* bf = new CBufferFrame(fd, page_id);
                this->_frame_map.emplace(page_id, bf);
                buffer_frame = bf;
            }

            pthread_mutex_unlock(&this->_lru_mutex);
        }
    }

    pthread_rwlock_unlock(&this->_hashmap_lock);

    if (buffer_frame != NULL)
        buffer_frame->lock(exclusive);

    return *buffer_frame;
}

void CBufferManager::unfixPage(CBufferFrame& frame, bool is_dirty)
{
    frame.unlock(is_dirty);

    pthread_mutex_lock(&this->_lru_mutex);
    this->_lru_list.push_front(&frame);
    pthread_mutex_unlock(&this->_lru_mutex);
}

CBufferManager::~CBufferManager()
{
    pthread_rwlock_wrlock(&this->_hashmap_lock);
    pthread_rwlock_destroy(&this->_hashmap_lock);

    for (auto frame : this->_frame_map)
        delete frame.second;

    for (auto segment : this->_fd_map)
        close(segment.second);
}

} // ns
