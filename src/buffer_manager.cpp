#include "buffer_manager.hpp"

namespace IN2118 {

CBufferManager::CBufferManager(unsigned page_count)
{
    this->_max_count = page_count;
    this->_buffer_count = 0;
    pthread_mutex_init(&this->_lru_mutex, NULL);
    pthread_mutex_init(&this->_hashmap_lock, NULL);
}

CBufferFrame& CBufferManager::fixPage(uint64_t page_id, bool exclusive)
{
    CBufferFrame* buffer_frame;
#if DEBUG
    printf("FIX:\t%lX\n", page_id);
#endif
    pthread_mutex_lock(&this->_hashmap_lock);

    auto find_result = this->_frame_map.find(page_id);
    if (find_result != this->_frame_map.end()) {
        buffer_frame = find_result->second;
        pthread_mutex_lock(&this->_lru_mutex);
        this->_lru_list.remove(buffer_frame->getPageId());
        buffer_frame->_number_of_locks++;
        pthread_mutex_unlock(&this->_lru_mutex);
    }
    else {
        int segment_number = page_id >> 48;
        auto find_fd = this->_fd_map.find(segment_number);

        int fd;
        if (find_fd == this->_fd_map.end()) {
            std::string file = "tempfiles_" + std::to_string(segment_number);
            fd = open(file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            if (fd < 0)
                std::cerr << "Looks like your segment couldn't be created" << std::endl;
            else
                this->_fd_map.emplace(segment_number, fd);
        }
        else
            fd = find_fd->second;

        pthread_mutex_lock(&this->_lru_mutex);
        if (this->_buffer_count >= this->_max_count) {
            if (this->_lru_list.empty()) {
                std::cerr << "The buffer is full and we cannot evict a page... I'm sorry!" << std::endl;
                buffer_frame = NULL;
            }

            uint64_t evict_id = this->_lru_list.back();
            this->_lru_list.pop_back();
            auto find_result = this->_frame_map.find(evict_id);
            CBufferFrame* evict = find_result->second;
            evict->storeOnDisk();
            this->_frame_map.erase(evict_id);
            delete evict;
        }
        else
            this->_buffer_count++;

        CBufferFrame* bf = new CBufferFrame(fd, page_id);
        this->_frame_map.emplace(page_id, bf);
        buffer_frame = bf;

        buffer_frame->_number_of_locks = 1;
        pthread_mutex_unlock(&this->_lru_mutex);
    }

    pthread_mutex_unlock(&this->_hashmap_lock);

    if (buffer_frame != NULL)
        buffer_frame->lock(exclusive);

    return *buffer_frame;
}

void CBufferManager::unfixPage(CBufferFrame& frame, bool is_dirty)
{
    uint64_t frame_id = frame.getPageId();
#if DEBUG
    printf("UNFIX:\t%lX\n", frame_id);
#endif
    if (is_dirty)
        frame.setDirty();

    frame.unlock();
    pthread_mutex_lock(&this->_lru_mutex);

    frame._number_of_locks--;
    if (frame._number_of_locks == 0)
        this->_lru_list.push_front(frame_id);

    pthread_mutex_unlock(&this->_lru_mutex);
}

CBufferManager::~CBufferManager()
{
    pthread_mutex_lock(&this->_hashmap_lock);

    for (auto frame : this->_frame_map) {
        delete frame.second;
    }

    for (auto segment : this->_fd_map)
        close(segment.second);

    this->_lru_list.clear();
    this->_fd_map.clear();
    this->_frame_map.clear();

    pthread_mutex_destroy(&this->_hashmap_lock);
    pthread_mutex_destroy(&this->_lru_mutex);
}

} // ns
