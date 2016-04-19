all:		clean gen sort buffer

clean:
			rm -rf ./bin

buffer:
			@mkdir -p bin
			g++ -Wall -Werror -march=native -std=c++11 -g -O3 -Iinclude -o bin/buffer test/buffer.cpp src/buffer_manager.cpp

gen:
			@mkdir -p bin
			g++ -g -O3 -Werror utils/gen.cpp -o bin/gen

sort:
			@mkdir -p bin
			g++ -Wall -Werror -march=native -std=c++11 -g -O3 -Iinclude -o bin/sort test/sort.cpp src/external_merge_sort.cpp
