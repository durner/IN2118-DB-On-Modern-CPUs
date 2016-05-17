all:		clean gen sort buffer pages

clean:
			rm -rf ./bin

pages:
			@mkdir -p bin
			g++ -Wall -Werror -march=native -std=c++11 -g -O3 -Iinclude -o bin/slotted_pages test/slotted_test.cpp src/slotted_page.cpp src/sp_segmnet.cpp include/schema/record.cpp src/schema_segment.cpp src/buffer_frame.cpp src/buffer_manager.cpp -lpthread

buffer:
			@mkdir -p bin
			g++ -Wall -Werror -march=native -std=c++11 -g -O3 -Iinclude -o bin/buffer test/buffer.cpp src/buffer_frame.cpp src/buffer_manager.cpp -lpthread

gen:
			@mkdir -p bin
			g++ -g -O3 -Werror utils/gen.cpp -o bin/gen

sort:
			@mkdir -p bin
			g++ -Wall -Werror -march=native -std=c++11 -g -O3 -Iinclude -o bin/sort test/sort.cpp src/external_merge_sort.cpp
