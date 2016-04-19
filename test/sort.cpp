#include "external_merge_sort.hpp"

uint64_t GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
        return -1;

    uint64_t memSize = atoi(argv[3]);
    memSize *= 1024 * 1024;

    int fd_input = open(argv[1], O_RDONLY);
    int fd_output = open(argv[2], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    uint64_t size = GetFileSize(argv[1]) / sizeof(uint64_t);

    IN2118::ExternalSort(fd_input, size, fd_output, memSize);

    bool correct = true;

    uint64_t compare_value = 0;
    uint64_t current_value = 0;
    lseek(fd_output, 0L, SEEK_SET);

    int io_result = read(fd_output, &current_value, sizeof(uint64_t));

    while (io_result > 0) {
        compare_value = current_value;
        io_result = read(fd_output, &current_value, sizeof(uint64_t));

        // first check needed bc of last current value is empty!
        if (io_result > 0 && compare_value > current_value)
            correct = false;
    }

    uint64_t size_output = GetFileSize(argv[2]) / sizeof(uint64_t);
    if (size != size_output)
        correct = false;

    if (correct) {
        std::cout << "\nSUCCESS: The List has been successfully sorted with the k-way merge strategy!\n" << std::endl;
        close(fd_input);
        close(fd_output);
        return 0;
    }
    else
        std::cout << "\nFAILURE: Unfortunatley, some things got messed up -> List is not correctly sorted!\n"
                  << std::endl;
    close(fd_input);
    close(fd_output);

    return -1;
}
