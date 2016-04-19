#include "external_merge_sort.hpp"

namespace IN2118 {

void ExternalSort(int fd_input, uint64_t size, int fd_output, uint64_t mem_size)
{
    void* buffer = new uint8_t[mem_size];
    const int max_elements = mem_size / sizeof(uint64_t);
    int chunk_size = (size / max_elements);
    int elements_expected = 0;

    std::vector<FILE*> temp_file_list;

    for (int i = 0; i <= chunk_size; i++) {
        // read values into buffer
        int io_result = 0;

        // size handling for last chunk
        if (i == chunk_size) {
            elements_expected = size % max_elements;
            delete[] static_cast<uint64_t*>(buffer);
            buffer = new uint64_t[elements_expected];
        }
        else
            elements_expected = max_elements;

        elements_expected *= sizeof(uint64_t);

        io_result = read(fd_input, buffer, elements_expected);

        if (io_result != elements_expected) {
            std::cout << "Couldn't read the file!" << std::endl;
            exit(-1);
        }
        // access the buffer elements
        uint64_t* int_buffer = static_cast<uint64_t*>(buffer);

        // in place sort of std lib
        std::sort(int_buffer, int_buffer + (elements_expected / sizeof(uint64_t)));

        // save sorted data chunk in file for k-merge
        FILE* temp_file = tmpfile();

        // due to the multiplication of sizeof() also the devision is save to determine the amount of values to write
        io_result = fwrite(int_buffer, sizeof(uint64_t), elements_expected / sizeof(uint64_t), temp_file);

        if (ferror(temp_file)) {
            std::cout << "Disk is probably full!" << std::endl;
            exit(-1);
        }
        temp_file_list.push_back(temp_file);
    }

    delete[] static_cast<uint64_t*>(buffer);

    // for k merge we need one more chunk to keep track of the output
    chunk_size++;

    // since we got only memsize memory we need to distribute the memory across all chunks
    const int sub_buffer_size = max_elements / chunk_size;

    std::priority_queue<CQueueElement<uint64_t> > heap;

    // load initial first elements for each buffer
    std::vector<CValueBuffer<uint64_t> > value_buffers;
    for (int i = 0; i < chunk_size; i++) {
        CValueBuffer<uint64_t> value_buffer = { new uint64_t[sub_buffer_size], i, 0, 0 };

        rewind(temp_file_list[i]); // get to the first element of the list again!
        int io_result = fread(value_buffer.buffer, sizeof(uint64_t), sub_buffer_size, temp_file_list[i]);
        value_buffer.buffer_length = io_result;

        if (io_result > 0) {
            CQueueElement<uint64_t> qe = { *(value_buffer.buffer + value_buffer.buffer_offset), i };
            value_buffer.buffer_offset++;
            value_buffer.buffer_length--;
            heap.push(qe);
        }

        value_buffers.push_back(value_buffer);
    }

    uint64_t* output_buffer = new uint64_t[sub_buffer_size];
    int elements_output_buffer = 0;

    while (!heap.empty()) {
        // get the current lowest available value
        CQueueElement<uint64_t> lowestValue = heap.top();
        heap.pop();

        *(output_buffer + elements_output_buffer) = lowestValue.value;
        elements_output_buffer++;

        CValueBuffer<uint64_t>& value_buffer = value_buffers[lowestValue.buffer_index];

        // need to refill buffer
        if (value_buffer.buffer_length <= 0) {
            int io_result = fread(
                value_buffer.buffer, sizeof(uint64_t), sub_buffer_size, temp_file_list[lowestValue.buffer_index]);
            value_buffer.buffer_length = io_result;
            value_buffer.buffer_offset = 0;
        }

        // get next element as long the buffer file still had elements
        if (value_buffer.buffer_length > 0) {
            CQueueElement<uint64_t> qe
                = { *(value_buffer.buffer + value_buffer.buffer_offset), lowestValue.buffer_index };
            value_buffer.buffer_offset++;
            value_buffer.buffer_length--;
            heap.push(qe);
        }

        // output buffer is full and needs to be saved
        if (elements_output_buffer == sub_buffer_size) {
            // writes at offset accourding to  http://man7.org/linux/man-pages/man2/write.2.html
            unsigned int io_result = write(fd_output, output_buffer, elements_output_buffer * sizeof(uint64_t));

            if (io_result < elements_output_buffer * sizeof(uint64_t)) {
                std::cout << "Couldnt write file; disk might be full!" << std::endl;
                exit(-1);
            }

            elements_output_buffer = 0;
        }
    }

    if (elements_output_buffer > 0) {
        // writes at offset accourding to  http://man7.org/linux/man-pages/man2/write.2.html
        unsigned int io_result = write(fd_output, output_buffer, elements_output_buffer * sizeof(uint64_t));

        if (io_result < elements_output_buffer * sizeof(uint64_t)) {
            std::cout << "Couldnt write file; disk might be full!" << std::endl;
            exit(-1);
        }

        elements_output_buffer = 0;
    }

    for (int i = 0; i < chunk_size; i++) {
        delete[] value_buffers[i].buffer;
        fclose(temp_file_list[i]);
    }
    delete[] output_buffer;
}

}; // ns
